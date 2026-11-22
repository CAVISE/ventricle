#include <ventricle/extensions/configuration/model/parser/nearest_scope_selection_policy.hpp>

#include <ns3/abort.h>

using namespace vcle::config;

void NearestScopeSelectionPolicy::setContext(const IdentifiersRepositoryContext& context) {
	context_ = &context;
	selectedDistance_.reset();
}

IdentifierSelectionDecision NearestScopeSelectionPolicy::select(
	ns3::Ptr<const ns3::Object> /* candidate */
) {
	NS_ABORT_MSG_UNLESS(context_, "selection context must be set before evaluating candidates");

	if (!selectedDistance_ || context_->distance_ < *selectedDistance_) {
		selectedDistance_ = context_->distance_;
		return IdentifierSelectionDecision::SELECT_THIS;
	}

	return IdentifierSelectionDecision::PRESERVE;
}
