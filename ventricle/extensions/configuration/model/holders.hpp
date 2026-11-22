#pragma once

#include <ventricle/extensions/configuration/identifiers.hpp>

#include <array>
#include <string_view>

namespace vcle::config::detail {
	/**
	 * @brief Names of the top-level collections emitted by @ref vcle::config::generateSchema.
	 *
	 * Every holder accepts the same type-keyed and named entries as the root, without nesting
	 * holders recursively. These names group entries; they do not restrict which exposed types
	 * may appear in a particular collection.
	 */
	inline constexpr std::array<std::string_view, 9> holderNames{
		identifiers::devices,
		identifiers::channels,
		identifiers::objects,
		identifiers::mobilityModels,
		identifiers::networkStacks,
		identifiers::applications,
		identifiers::nodePolicies,
		identifiers::nodes,
		identifiers::defaults
	};
} // namespace vcle::config::detail
