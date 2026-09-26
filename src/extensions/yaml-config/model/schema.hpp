#pragma once

#include <span>
#include <string>

#include <absl/status/statusor.h>
#include <ns3/type-id.h>

namespace vcle::config {

	/** Caller-provided schema dialect URI, title, and description. */
	struct Metadata {
		std::string schema;
		std::string title;
		std::string description;
	};

	/**
	 * Generate a YAML-encoded JSON Schema for explicitly exposed TypeIds.
	 * Includes inherited construction attributes and their declared defaults.
	 * Unsupported attribute values and duplicate types return an error.
	 */
	absl::StatusOr<std::string> generateSchema(std::span<const ns3::TypeId> types, const Metadata& metadata);

} // namespace vcle::yaml_config
