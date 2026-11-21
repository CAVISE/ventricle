#pragma once

#include <ventricle/core/error.hpp>

#include <ns3/type-id.h>
#include <span>
#include <string>

namespace vcle::config {

	/**
	 * @brief Generate a JSON Schema for explicitly exposed TypeIds.
	 * Includes inherited construction attributes, default values, named typeId/overrides entries,
	 * and component and node holders. Namespaces must be expanded before validation. Unsupported
	 * attribute values and duplicate or uninitialized types return an error.
	 */
	Expected<std::string> generateSchema(std::span<const ns3::TypeId> types);

} // namespace vcle::config
