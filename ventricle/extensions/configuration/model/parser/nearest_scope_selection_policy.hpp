#pragma once

#include <ventricle/extensions/configuration/model/parser/identifiers_repository.hpp>

#include <optional>

namespace vcle::config {

	/**
	 * @brief Selects the candidate with the smallest scope distance, preserving the first tie.
	 *
	 * @ref setContext clears the previous lookup's distance. Each @ref select uses
	 * @ref IdentifiersRepositoryContext::distance_ and ignores the object's contents. The first
	 * candidate is selected; a later candidate replaces it only if it is closer.
	 *
	 * Calling @ref select before @ref setContext aborts. The borrowed context must remain alive
	 * throughout evaluation, as required by @ref IIdentifierSelectionPolicy.
	 */
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
