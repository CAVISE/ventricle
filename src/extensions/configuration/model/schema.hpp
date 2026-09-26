#pragma once

#include <span>
#include <string>

#include <absl/status/statusor.h>
#include <ns3/type-id.h>

namespace vcle::config {

	/**
	 * Generate a JSON Schema for explicitly exposed TypeIds.
	 * Includes inherited construction attributes and their declared defaults.
	 * Unsupported attribute values and duplicate types return an error.
	 */
	absl::StatusOr<std::string> generateSchema(std::span<const ns3::TypeId> types);

} // namespace vcle::config
