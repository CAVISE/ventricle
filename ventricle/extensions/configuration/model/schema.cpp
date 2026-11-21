#include <ventricle/extensions/configuration/model/holders.hpp>
#include <ventricle/extensions/configuration/model/schema.hpp>
#include <ventricle/extensions/configuration/model/schema_context.hpp>

#include <format>
#include <memory>
#include <set>
#include <utility>

using namespace vcle;

using namespace vcle::config;
using namespace vcle::config::detail;

Expected<std::string> vcle::config::generateSchema(
	std::span<const ns3::TypeId> types
) {
	std::set<ns3::TypeId> seen;
	for (auto type : types) {
		if (!type.GetUid()) {
			return makeError(std::errc::invalid_argument, "uninitialized TypeId");
		}
		if (!seen.insert(type).second) {
			return makeError(std::errc::invalid_argument, "Duplicate exposed TypeId");
		}
	}

	glz::schema root{
		.title = "Ventricle auto-generated configuration schema. "
				 "This schema supports objects specifically provided at generation time - "
				 "refer to docs for how to use support your extensions",
		.description = "This root element provides access to simulator-devided extension types.",
		.type = "object",
		.additionalProperties = false,
	};

	Context ctx;
	root.properties.emplace();
	auto namedEntries = std::make_shared<glz::schema>();
	namedEntries->oneOf.emplace();

	for (const ns3::TypeId& id : types) {
		auto schema = ctx.withSchema(glz::schema{}).invoke([&]() { return ctx.schemaForID(id); });
		if (!schema) {
			return std::unexpected(std::move(schema.error()));
		}

		glz::schema named;
		named.type = "object";
		named.additionalProperties = false;
		named.required.emplace().emplace_back("typeId");
		if (schema->required && !schema->required->empty()) {
			named.required->emplace_back("overrides");
		}
		auto& properties = named.properties.emplace();
		properties["typeId"].constant = ctx.store(id.GetName());
		properties["overrides"] = *schema;
		namedEntries->oneOf->push_back(std::move(named));

		if (const auto& [_, hasInserted] =
				root.properties->try_emplace(ctx.store(id.GetName()), std::move(*schema));
			!hasInserted) {
			return makeError(
				std::errc::file_exists,
				"type was already inserted into properties, duplicate found (type_name: {})",
				id.GetName()
			);
		}
	}

	if (!types.empty()) {
		root.additionalProperties = namedEntries;
		// A named entry may itself use a registered TypeId as its ID.
		for (auto& [name, entry] : *root.properties) {
			glz::schema alternatives;
			alternatives.anyOf.emplace().push_back(std::move(entry));
			alternatives.anyOf->push_back(*namedEntries);
			entry = std::move(alternatives);
		}
	}

	// Holder entries follow the same rules as root entries, without recursively nesting holders.
	glz::schema holder;
	holder.type = "object";
	holder.properties = root.properties;
	holder.additionalProperties = root.additionalProperties;
	for (const auto name : holderNames) {
		(*root.properties)[name] = holder;
	}

	auto out = glz::write_json(root);
	if (!out.has_value()) {
		return makeError(
			std::errc::io_error,
			"failed to write JSON (error: {})",
			out.error().custom_error_message
		);
	}

	return *out;
}
