#include <ventricle/core/error.hpp>
#include <ventricle/extensions/configuration/model/schema_context.hpp>
#include <ventricle/extensions/configuration/model/schema_values.hpp>

#include <format>
#include <utility>

using namespace vcle;

using namespace vcle::config::detail;

Context& Context::withSchema(glz::schema schema) {
	schemas_.push(std::move(schema));
	return *this;
}

glz::schema& Context::schema() {
	return schemas_.top();
}

Expected<> Context::walkProperty(ns3::TypeId::AttributeInformation attribute) {
	if (!(attribute.flags & ns3::TypeId::ATTR_CONSTRUCT) || !attribute.accessor->HasSetter()) {
		return {};
	}

	glz::schema property{
		.description = store(attribute.help).c_str(),
		.type = "object",
		.additionalProperties = false,
	};

	switch (attribute.supportLevel) {
	case ns3::TypeId::SupportLevel::DEPRECATED:
		property.deprecated = true;
		break;
	case ns3::TypeId::SupportLevel::OBSOLETE:
		return {};
	default:
		/* noop */
		break;
	}

	// We prefer original initial value, since the schema generator will not modify it anyways.
	const ns3::AttributeValue* value = ns3::PeekPointer(attribute.originalInitialValue);

	auto result = withSchema(std::move(property)).invoke([&]() { return writeAttributeValue(*this, attribute, value); });

	if (!result) {
		return std::unexpected(std::move(result.error()));
	}

	if (const auto& [_, hasInserted] = schema().properties->try_emplace(store(attribute.name), std::move(*result)); !hasInserted) {
		return makeError(
			std::errc::file_exists,
			"attribute was already inserted into properties, duplicate found (attribute_name: {})",
			attribute.name
		);
	}

	return {};
}

// Walk type properties and add them to the schema.
Expected<> Context::walkProperties(const ns3::TypeId& id) {
	for (std::size_t i = 0; i < id.GetAttributeN(); ++i) {
		auto status = walkProperty(id.GetAttribute(i));
		if (!status) {
			return std::unexpected(std::move(status.error()));
		}
	}

	return {};
}

Expected<> Context::schemaForID(const ns3::TypeId& id) {
	if (id.MustHideFromDocumentation()) {
		return makeError(std::errc::io_error, "provided type is supposed to be hidden from user (type: {})", id.GetName());
	}

	auto& schema = this->schema();
	schema.description = store("Configuration for type {}", id.GetName());
	schema.type = "object";
	schema.properties.emplace();
	schema.additionalProperties = false;

	// Enumerate each declaring type once, stopping at the self-parented root.
	auto current = id;
	while (true) {
		if (auto status = walkProperties(current); !status) {
			return std::unexpected(std::move(status.error()));
		}

		const auto parent = current.GetParent();
		if (parent == current) {
			break;
		}
		current = parent;
	}

	return {};
}
