#include "src/extensions/configuration/model/schema.hpp"

#include <any>
#include <format>
#include <list>
#include <memory>
#include <queue>
#include <string_view>
#include <utility>

#include <absl/status/status.h>
#include <absl/strings/str_split.h>
#include <glaze/json/schema.hpp>
#include <ns3/boolean.h>
#include <ns3/double.h>
#include <ns3/integer.h>
#include <ns3/nstime.h>
#include <ns3/string.h>

using namespace vcle::config;

namespace {

	class Context {
	public:

		using SchemasQueue = std::queue<std::shared_ptr<glz::schema>>;

		// Sets a new scheme for this context
		Context& withSchema(std::shared_ptr<glz::schema> schema) {
			schemas_.push(std::move(schema));
			return *this;
		}

		std::shared_ptr<glz::schema> peekSchema() {
			return schemas_.back();
		}

		std::shared_ptr<glz::schema> retreiveSchema() {
			auto schema = std::move(schemas_.front());
			schemas_.pop();
			return schema;
		}

		template<typename T>
		T& store(T value) {
			registry_.push_back(std::move(value));
			return std::any_cast<T&>(registry_.back());
		}

	private:
		SchemasQueue schemas_;
		std::list<std::any> registry_;
	};

	// Walk type properties and add them to the schema.
	absl::Status walkProperties(Context& ctx, const ns3::TypeId& id) {
		for (std::size_t i = 0; i < id.GetAttributeN(); ++i) {
			ns3::TypeId::AttributeInformation attribute = id.GetAttribute(i);

			// ATTR_SET means we can set attribute anytime
			// ATTR_CONSTRUCT means we cab set only upon construction
			// if attribute only has ATTR_GET set, it means it is readonly and not a subject for schema
			if (!(attribute.flags & ns3::TypeId::ATTR_CONSTRUCT)) {
				continue;
			}

			glz::schema property {
				.description = ctx.store(attribute.help).c_str(),
				.type = "object",
				.additionalProperties = false,
			};

			switch (attribute.supportLevel) {
				case ns3::TypeId::SupportLevel::DEPRECATED:
					property.deprecated = true;
					break;
				case ns3::TypeId::SupportLevel::OBSOLETE:
					continue;
				default:
					/* noop */
					break;
			}

			// We prefer original initial value, since the schema generator will not modify it anyways.
			const auto* value = ns3::PeekPointer(attribute.originalInitialValue);

			// TODO: we could possibly extend the check contract to supply config-friendly
			// checkers that can pass conditions to schema, like min/max value for integers, for example.
			if (auto casted = dynamic_cast<const ns3::StringValue*>(value)) {
				property.type = "string";
				property.defaultValue = ctx.store(casted->Get());
			} else if (auto casted = dynamic_cast<const ns3::IntegerValue*>(value)) {
				property.type = "integer";
				property.defaultValue = casted->Get();
			} else if (auto casted = dynamic_cast<const ns3::DoubleValue*>(value)) {
				property.type = "number";
				property.defaultValue = casted->Get();
			} else if (auto casted = dynamic_cast<const ns3::BooleanValue*>(value)) {
				property.type = "boolean";
				property.defaultValue = casted->Get();
			} else if (auto casted = dynamic_cast<const ns3::TimeValue*>(value)) {
				property.type = "string";
				property.defaultValue = ctx.store(casted->SerializeToString(attribute.checker));
			} else if (attribute.checker->GetValueTypeName().starts_with("ns3::EnumValue<")) {
				property.type = "string";
				property.defaultValue = ctx.store(value->SerializeToString(attribute.checker));

				const std::string_view choices = ctx.store(attribute.checker->GetUnderlyingTypeInformation());
				property.enumeration.emplace();

				// The checker exposes names separated by '|'.
				for (const std::string_view choice : absl::StrSplit(choices, '|')) {
					property.enumeration->push_back(choice);
				}
			} else {
				return absl::UnimplementedError("cannot parse attribute type; probably it is unsupported");
			}

			if (const auto& [_, hasInserted] = ctx.peekSchema()->properties->try_emplace(ctx.store(attribute.name), std::move(property)); !hasInserted) {
				auto err = absl::AlreadyExistsError("attribute was already inserted into properties, duplicate found");
				err.SetPayload("attribute_name", absl::Cord(attribute.name));
				return err;
			}
		}

		return absl::OkStatus();
	}

	absl::Status schemaForID(Context& context, const ns3::TypeId& id) {
		if (id.MustHideFromDocumentation()) {
			auto err = absl::InternalError("provided type is supposed to be hidden from user");
			err.SetPayload("type", absl::Cord(id.GetName()));
			return err;
		}

		auto schema = context.peekSchema();
		schema->description = context.store(std::format("Configuration for type {}", id.GetName()));
		schema->type = "object";
		schema->properties.emplace();

		// Enumerate each declaring type once, stopping at the self-parented root.
		auto current = id;
		while (true) {
			if (auto status = walkProperties(context, current); !status.ok()) {
				return status;
			}

			const auto parent = current.GetParent();
			if (parent == current) {
				break;
			}
			current = parent;
		}

		return absl::OkStatus();
	}

}

absl::StatusOr<std::string> vcle::config::generateSchema(std::span<const ns3::TypeId> types) {
	glz::schema root {
		.title =
			"Ventricle auto-generated configuration schema. "
			"This schema supports objects specifically provided at generation time - "
			"refer to docs for how to use support your extensions",
		.description = "This root element provides access to simulator-devided extension types.",
		.type = "object",
		.additionalProperties = false,
	};

	Context ctx;
	root.properties.emplace();

	for (const ns3::TypeId& id : types) {
		ctx.withSchema(std::make_shared<glz::schema>());
		if (auto status = schemaForID(ctx, id); !status.ok()) {
			return status;
		}

		auto schema = ctx.retreiveSchema();
		if (const auto& [_, hasInserted] = root.properties->try_emplace(ctx.store(id.GetName()), std::move(*schema)); !hasInserted) {
			auto err = absl::AlreadyExistsError("type was already inserted into properties, duplicate found");
			err.SetPayload("type_name", absl::Cord(id.GetName()));
			return err;
		}
	}

	auto out = glz::write_json(root);
	if (!out.has_value()) {
		auto err = absl::InternalError("failed to write JSON");
		err.SetPayload("error", absl::Cord(out.error().custom_error_message));
		return err;
	}

	return out.value();
}
