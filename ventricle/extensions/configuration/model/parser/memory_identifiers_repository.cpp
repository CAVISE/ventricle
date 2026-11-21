#include <ventricle/extensions/configuration/model/parser/memory_identifiers_repository.hpp>
#include <ventricle/extensions/configuration/model/parser/nearest_scope_selection_policy.hpp>

#include <system_error>
#include <unordered_map>
#include <utility>

using namespace vcle;
using namespace vcle::config;

struct MemoryIdentifiersRepository::Scope {
	std::shared_ptr<const Scope> parent_;
	std::unordered_map<std::string, ns3::Ptr<ns3::Object>> objects_;
};

MemoryIdentifiersRepository::MemoryIdentifiersRepository(
	std::shared_ptr<IIdentifierSelectionPolicy> policy
)
	: scope_(std::make_shared<Scope>())
	, policy_(policy ? std::move(policy) : std::make_shared<NearestScopeSelectionPolicy>()) {
}

MemoryIdentifiersRepository::MemoryIdentifiersRepository(
	std::shared_ptr<Scope> scope, std::shared_ptr<IIdentifierSelectionPolicy> policy
)
	: scope_(std::move(scope))
	, policy_(std::move(policy)) {
}

MemoryIdentifiersRepository MemoryIdentifiersRepository::createScope() const {
	auto child = std::make_shared<Scope>();
	child->parent_ = scope_;
	return MemoryIdentifiersRepository(std::move(child), policy_);
}

Expected<> MemoryIdentifiersRepository::add(
	const std::string& id, ns3::Ptr<ns3::Object> object
) {
	if (id.empty()) {
		return makeError(std::errc::invalid_argument, "identifier is empty");
	}
	if (!object) {
		return makeError(std::errc::invalid_argument, "object for identifier '{}' is null", id);
	}

	if (!scope_->objects_.try_emplace(id, std::move(object)).second) {
		return makeError(std::errc::file_exists, "identifier '{}' already exists", id);
	}

	return {};
}

Expected<ns3::Ptr<ns3::Object>> MemoryIdentifiersRepository::get(
	const std::string& id
) const {
	if (id.empty()) {
		return makeError(std::errc::invalid_argument, "identifier is empty");
	}

	IdentifiersRepositoryContext context;
	policy_->setContext(context);
	ns3::Ptr<ns3::Object> selected;
	for (std::shared_ptr<const Scope> scope = scope_; scope;
		 scope = scope->parent_, ++context.distance_) {
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
		return makeError(
			std::errc::no_such_file_or_directory,
			"no visible binding was selected for identifier '{}'",
			id
		);
	}

	return selected;
}
