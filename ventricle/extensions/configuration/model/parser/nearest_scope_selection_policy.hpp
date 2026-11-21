#pragma once

#include <ventricle/extensions/configuration/model/parser/identifier_selection_policy.hpp>

#include <optional>

namespace vcle::config {

	/// @brief Selects the closest visible binding, preserving the first candidate when distances
	/// tie.
	class NearestScopeSelectionPolicy final : public IIdentifierSelectionPolicy {
	public:
		/* IIdentifierSelectionPolicy implementation*/
		void setContext(const IdentifiersRepositoryContext& context) override;
		IdentifierSelectionDecision select(ns3::Ptr<const ns3::Object> candidate) override;

	private:
		const IdentifiersRepositoryContext* context_ = nullptr;
		std::optional<std::size_t> selectedDistance_;
	};

} // namespace vcle::config
