#pragma once

#include <cstddef>
#include <ns3/object.h>

namespace vcle::config {

	/// @brief Metadata for the candidate currently being evaluated.
	struct IdentifiersRepositoryContext {
		/// @brief Zero for the lookup scope, one for its parent, and so on.
		std::size_t distance_ = 0;
	};

	/// @brief Describes how a candidate changes the selected identifier binding.
	enum class IdentifierSelectionDecision {
		PRESERVE,	 ///< Keep the current selection, including an empty selection.
		SELECT_THIS, ///< Replace the selection with the current candidate.
		DROP,		 ///< Clear the selection and continue evaluating candidates.
	};

	/// @brief Evaluates visible bindings for one identifier, handling one lookup at a time.
	class IIdentifierSelectionPolicy {
	public:
		virtual ~IIdentifierSelectionPolicy() = default;

		/**
		 * @brief Begin a lookup and reset accumulated selection state.
		 * The context is borrowed for this lookup and updated before each select().
		 * Access it only during this call or select(); it is no longer valid when the lookup
		 * returns.
		 */
		virtual void setContext(const IdentifiersRepositoryContext& context) = 0;

		/** Decide how a non-null candidate affects selection; setContext() must precede evaluation.
		 */
		virtual IdentifierSelectionDecision select(ns3::Ptr<const ns3::Object> candidate) = 0;
	};

} // namespace vcle::config
