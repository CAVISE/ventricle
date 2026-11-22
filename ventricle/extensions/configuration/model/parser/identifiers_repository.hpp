#pragma once

/**
 * @file
 * @brief Identifier repository errors, ownership rules, and binding selection contracts.
 *
 * @ref vcle::config::IIdentifiersRepository defines registration and lookup, including its
 * @ref vcle::config::IIdentifiersRepository::ErrorCode failures. Scoped implementations use
 * @ref vcle::config::IIdentifierSelectionPolicy to select among visible bindings. Policies borrow
 * lookup metadata and must finish using it before the lookup returns.
 */

#include <ventricle/core/defs.hpp>
#include <ventricle/core/error.hpp>

#include <cstddef>
#include <cstdint>
#include <string>

#include <ns3/object.h>

namespace vcle::config {

	/// @brief Describes a candidate's scope during an @ref IIdentifierSelectionPolicy lookup.
	struct IdentifiersRepositoryContext {
		/// Number of parent links from the lookup scope, including scopes without a matching ID.
		std::size_t distance_ = 0;
	};

	/// @brief Changes the current binding selection without ending evaluation of later candidates.
	enum class IdentifierSelectionDecision : std::int_fast8_t {
		PRESERVE,	 ///< Keep the selected object, or leave the selection empty.
		SELECT_THIS, ///< Select the current candidate, replacing any previous selection.
		DROP,		 ///< Clear the selection; a later candidate may still be selected.
	};

	/**
	 * @brief Chooses an object by evaluating the visible bindings of one identifier.
	 *
	 * Call @ref setContext once at the start of each lookup, then @ref select for every matching
	 * candidate. Each decision updates the repository's selection; it does not stop traversal.
	 * The policy may keep per-lookup state, so lookups sharing an instance must not overlap or
	 * reenter it.
	 */
	class IIdentifierSelectionPolicy {
	public:
		virtual ~IIdentifierSelectionPolicy() = default;

		/**
		 * @brief Begin a lookup and reset accumulated selection state.
		 * @param[in] context Borrowed metadata updated by the repository before each @ref select.
		 *
		 * Called even when a nonempty identifier has no visible bindings. A policy may retain a
		 * pointer to the context, but may read it only during this call or subsequent @ref select
		 * calls in the same lookup. Its lifetime ends when the lookup returns.
		 */
		virtual void setContext(const IdentifiersRepositoryContext& context) = 0;

		/**
		 * @brief Decide how the current candidate changes the selection.
		 * @param[in] candidate Non-null object bound to the requested identifier in a visible
		 * scope.
		 * @pre @ref setContext has initialized this lookup and its context is still alive.
		 * @return The update to apply before evaluating the next candidate.
		 */
		virtual IdentifierSelectionDecision select(ns3::Ptr<const ns3::Object> candidate) = 0;
	};

	/**
	 * @brief Retains constructed objects under exact, case-sensitive identifiers.
	 *
	 * Use @ref add to register an object and @ref get to acquire a reference to a selected
	 * binding. The repository owns its identifier strings and retains objects through
	 * @c ns3::Ptr; registration and lookup neither construct nor copy the objects. Different
	 * identifiers may refer to the same object.
	 *
	 * @ref enter opens an empty child registration scope on this repository. Lookups can select
	 * bindings from that scope and its ancestors. Pair each @ref enter with @ref exit to restore
	 * the previous scope; bindings introduced in the child are no longer visible from it.
	 *
	 * Returned objects may outlive the repository. Releasing bindings does not explicitly
	 * dispose objects still held elsewhere. Failures carry @ref ErrorCode in an
	 * @ref vcle::EnumeratedErrorInfo payload. Inspect them through @ref vcle::Error::handle or
	 * propagate them according to the ownership contract of @ref vcle::Error.
	 */
	class IIdentifiersRepository {
	public:
		/// @brief Distinguishes registration and lookup failures within an identifier repository.
		enum class ErrorCode : std::int_fast8_t {
			EMPTY_IDENTIFIER,	  ///< Registration or lookup received an empty identifier.
			NULL_OBJECT,		  ///< Registration received a null object for a nonempty identifier.
			DUPLICATE_IDENTIFIER, ///< The registration scope already contains this identifier.
			NO_SELECTED_BINDING,  ///< No visible binding exists or the policy selected none.
		};

		virtual ~IIdentifiersRepository() = default;

		/// @brief Enter an empty child registration scope, preserving ancestor bindings.
		virtual void enter() = 0;

		/**
		 * @brief Restore the parent registration scope.
		 * @pre A child scope opened by @ref enter is active.
		 */
		virtual void exit() noexcept = 0;

		/**
		 * @brief Retain an object under an identifier in the current registration scope.
		 *
		 * Copies @p id, so subsequent changes to the caller's string have no effect. An empty
		 * identifier returns @ref ErrorCode::EMPTY_IDENTIFIER; a null @p object with a nonempty
		 * identifier returns @ref ErrorCode::NULL_OBJECT. An existing binding in the same scope
		 * returns @ref ErrorCode::DUPLICATE_IDENTIFIER, even if it refers to the same object.
		 * Failed registration leaves all bindings unchanged.
		 */
		VCLE_NODISCARD virtual Expected<> add(const std::string& id, ns3::Ptr<ns3::Object> object) = 0;

		/**
		 * @brief Return a retained reference to the object selected for an identifier.
		 *
		 * Lookup does not create or reserve a binding. An empty @p id returns
		 * @ref ErrorCode::EMPTY_IDENTIFIER. If no visible binding is selected, returns
		 * @ref ErrorCode::NO_SELECTED_BINDING. Implementations define scope visibility and
		 * selection order.
		 * @see MemoryIdentifiersRepository::get for scoped lookup behavior.
		 */
		VCLE_NODISCARD virtual Expected<ns3::Ptr<ns3::Object>> get(const std::string& id) const = 0;
	};

} // namespace vcle::config
