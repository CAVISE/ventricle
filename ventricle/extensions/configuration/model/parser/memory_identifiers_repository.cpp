#include <ventricle/extensions/configuration/model/parser/memory_identifiers_repository.hpp>
#include <ventricle/extensions/configuration/model/parser/nearest_scope_selection_policy.hpp>

#include <format>
#include <unordered_map>
#include <utility>

#include <ns3/abort.h>

using namespace vcle;
using namespace vcle::config;

struct MemoryIdentifiersRepository::Bindings {
	std::shared_ptr<Bindings> parent_;
	std::unordered_map<std::string, ns3::Ptr<ns3::Object>> objects_;
};

MemoryIdentifiersRepository::MemoryIdentifiersRepository(std::shared_ptr<IIdentifierSelectionPolicy> policy)
	: scope_(std::make_shared<Bindings>())
	, policy_(policy ? std::move(policy) : std::make_shared<NearestScopeSelectionPolicy>()) {
}

MemoryIdentifiersRepository::MemoryIdentifiersRepository(std::shared_ptr<Bindings> scope, std::shared_ptr<IIdentifierSelectionPolicy> policy)
	: scope_(std::move(scope))
	, policy_(std::move(policy)) {
}

MemoryIdentifiersRepository MemoryIdentifiersRepository::createScope() const {
	auto child = std::make_shared<Bindings>();
	child->parent_ = scope_;
	return MemoryIdentifiersRepository(std::move(child), policy_);
}

void MemoryIdentifiersRepository::enter() {
	auto child = std::make_shared<Bindings>();
	child->parent_ = scope_;
	scope_ = std::move(child);
}

void MemoryIdentifiersRepository::exit() noexcept {
	NS_ABORT_MSG_UNLESS(scope_->parent_, "cannot exit the root identifier scope");

	auto parent = scope_->parent_;
	scope_ = std::move(parent);
}

Expected<> MemoryIdentifiersRepository::add(const std::string& id, ns3::Ptr<ns3::Object> object) {
	if (id.empty()) {
		return makeError<EnumeratedErrorInfo<ErrorCode>>(ErrorCode::EMPTY_IDENTIFIER, "identifier is empty");
	}
	if (!object) {
		return makeError<EnumeratedErrorInfo<ErrorCode>>(ErrorCode::NULL_OBJECT, std::format("object for identifier '{}' is null", id));
	}

	if (!scope_->objects_.try_emplace(id, std::move(object)).second) {
		return makeError<EnumeratedErrorInfo<ErrorCode>>(ErrorCode::DUPLICATE_IDENTIFIER, std::format("identifier '{}' already exists", id));
	}

	return {};
}

Expected<ns3::Ptr<ns3::Object>> MemoryIdentifiersRepository::get(const std::string& id) const {
	if (id.empty()) {
		return makeError<EnumeratedErrorInfo<ErrorCode>>(ErrorCode::EMPTY_IDENTIFIER, "identifier is empty");
	}

	IdentifiersRepositoryContext context;
	policy_->setContext(context);
	ns3::Ptr<ns3::Object> selected;
	for (std::shared_ptr<const Bindings> scope = scope_; scope; scope = scope->parent_, ++context.distance_) {
		const auto found = scope->objects_.find(id);
		if (found == scope->objects_.end()) {
			continue;
		}

		auto candidate = found->second;
		switch (policy_->select(candidate)) {
		case IdentifierSelectionDecision::PRESERVE:
			break;
		case IdentifierSelectionDecision::SELECT_THIS:
			selected = std::move(candidate);
			break;
		case IdentifierSelectionDecision::DROP:
			selected = nullptr;
			break;
		}
	}

	if (!selected) {
		return makeError<EnumeratedErrorInfo<ErrorCode>>(
			ErrorCode::NO_SELECTED_BINDING,
			std::format("no visible binding was selected for identifier '{}'", id)
		);
	}

	return selected;
}
