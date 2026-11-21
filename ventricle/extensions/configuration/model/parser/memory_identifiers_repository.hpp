#pragma once

#include <ventricle/core/defs.hpp>
#include <ventricle/extensions/configuration/model/parser/identifier_selection_policy.hpp>
#include <ventricle/extensions/configuration/model/parser/identifiers_repository.hpp>

#include <memory>

namespace vcle::config {

	/// @brief A view of one scope's bindings; copies share that scope and its selection policy.
	class MemoryIdentifiersRepository final : public IIdentifiersRepository {
	public:
		/// @brief Create a root scope. A null policy selects the default nearest-scope policy.
		explicit MemoryIdentifiersRepository(
			std::shared_ptr<IIdentifierSelectionPolicy> policy = nullptr
		);

		/**
		 * @brief Create an empty child view that retains its ancestors and shares the selection
		 * policy. Child bindings remain hidden from ancestors and siblings. The parent view is
		 * unchanged.
		 */
		MemoryIdentifiersRepository createScope() const;

		/* IIdentifiersRepository implementation*/
		VCLE_NODISCARD Expected<> add(const std::string& id, ns3::Ptr<ns3::Object> object) override;
		/** Evaluate matching bindings from this scope towards the root; scopes without a match
		 * still count towards distance_. */
		VCLE_NODISCARD Expected<ns3::Ptr<ns3::Object>> get(const std::string& id) const override;

	private:
		struct Scope;

		MemoryIdentifiersRepository(
			std::shared_ptr<Scope> scope, std::shared_ptr<IIdentifierSelectionPolicy> policy
		);

	private:
		std::shared_ptr<Scope> scope_;
		std::shared_ptr<IIdentifierSelectionPolicy> policy_;
	};

} // namespace vcle::config
