#pragma once

#include <array>
#include <string_view>

namespace vcle::config::detail {
	/// @brief Top-level entry collections exposed by the schema generator.
	inline constexpr std::array<std::string_view, 9> holderNames{
		"devices",
		"channels",
		"objects",
		"mobilityModels",
		"networkStacks",
		"applications",
		"nodePolicies",
		"nodes",
		"defaults"
	};
} // namespace vcle::config::detail
