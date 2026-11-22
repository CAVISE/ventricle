#pragma once

#include <ventricle/core/defs.hpp>
#include <ventricle/extensions/configuration/model/parser/identifiers_repository.hpp>

#include <memory>

namespace vcle::config {

	/**
	 * @brief Stores identifier bindings in a current scope with access to its ancestors.
	 *
	 * Use @ref add and @ref get through the @ref IIdentifiersRepository contract, and pair
	 * @ref enter with @ref exit to process a child scope. Each view
	 * has its own current-scope position. Copies share the bindings and policy at the time of
	 * copying; entering or exiting on one view does not change another view's current scope.
	 * The default @ref NearestScopeSelectionPolicy makes the closest binding shadow its ancestors.
	 *
	 * @ref createScope creates a separate child view. Scopes remain alive while referenced by a
	 * view or descendant; exiting releases only this view's reference to the child scope.
	 *
	 * Scopes and their shared policy are not synchronized. Lookups sharing a policy must not
	 * overlap or reenter it.
	 */
	class MemoryIdentifiersRepository final : public IIdentifiersRepository {
	public:
		/**
		 * @brief Create an empty root scope with a shared selection policy.
		 * @param[in] policy Policy retained by this view, its copies, and its children. A null
		 * pointer creates a @ref NearestScopeSelectionPolicy.
		 */
		explicit MemoryIdentifiersRepository(std::shared_ptr<IIdentifierSelectionPolicy> policy = nullptr);

		/**
		 * @brief Create an empty child scope that retains its ancestors and shares the policy.
		 *
		 * Child bindings are hidden from ancestors and siblings. Later registrations in an
		 * ancestor remain visible to the child, even after the original ancestor view is
		 * destroyed. Creating a child does not change this view's bindings.
		 */
		MemoryIdentifiersRepository createScope() const;

		/* IIdentifiersRepository implementation*/
		/// @brief Enter an empty child scope; allocation failure leaves the current scope intact.
		void enter() override;

		/**
		 * @brief Restore the parent scope, releasing this view's reference to the child.
		 * @pre A parent scope exists; attempting to exit the root aborts.
		 * @see IIdentifiersRepository::exit for the scope restoration contract.
		 */
		void exit() noexcept override;

		VCLE_NODISCARD Expected<> add(const std::string& id, ns3::Ptr<ns3::Object> object) override;
		/**
		 * @brief Evaluate matching bindings from this scope towards the root.
		 *
		 * Resets the policy once per nonempty lookup, then evaluates every matching binding,
		 * including matches beyond an already selected object. Scopes without a match still
		 * count towards @ref IdentifiersRepositoryContext::distance_. The policy's final
		 * selection determines the result; ancestors and siblings cannot see child bindings.
		 * @see IIdentifiersRepository::get for ownership and failure semantics.
		 */
		VCLE_NODISCARD Expected<ns3::Ptr<ns3::Object>> get(const std::string& id) const override;

	private:
		/// @brief Shared bindings for one scope, retaining its parent scope when present.
		struct Bindings;

		MemoryIdentifiersRepository(std::shared_ptr<Bindings> scope, std::shared_ptr<IIdentifierSelectionPolicy> policy);

	private:
		std::shared_ptr<Bindings> scope_;
		std::shared_ptr<IIdentifierSelectionPolicy> policy_;
	};

} // namespace vcle::config
