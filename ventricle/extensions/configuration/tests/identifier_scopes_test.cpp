#include <ventricle/extensions/configuration/model/parser/memory_identifiers_repository.hpp>
#include <ventricle/extensions/configuration/model/parser/nearest_scope_selection_policy.hpp>

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

using namespace vcle::config;

namespace {
	using Decision = IdentifierSelectionDecision;

	class ScopeObject : public ns3::Object {
	public:
		static ns3::TypeId GetTypeId() {
			static auto type = ns3::TypeId("IdentifierScopesTest::Object")
								   .SetParent<ns3::Object>()
								   .AddConstructor<ScopeObject>();
			return type;
		}
	};

	/** Records the live lookup context and plays a sequence of selection decisions. */
	class ScriptedPolicy final : public IIdentifierSelectionPolicy {
	public:
		struct Observation {
			ns3::Ptr<const ns3::Object> object_;
			std::size_t distance_;
		};

		explicit ScriptedPolicy(
			std::vector<Decision> decisions
		)
			: decisions_(std::move(decisions)) {
		}

		/* IIdentifierSelectionPolicy implementation*/
		void setContext(
			const IdentifiersRepositoryContext& context
		) override {
			context_ = &context;
			++lookups_;
			observations_.clear();
		}

		Decision select(
			ns3::Ptr<const ns3::Object> candidate
		) override {
			if (!context_) {
				return Decision::DROP;
			}

			const auto index = observations_.size();
			observations_.push_back({candidate, context_->distance_});
			return index < decisions_.size() ? decisions_[index] : Decision::PRESERVE;
		}

	public:
		std::size_t lookups_ = 0;
		std::vector<Observation> observations_;

	private:
		std::vector<Decision> decisions_;
		const IdentifiersRepositoryContext* context_ = nullptr;
	};

	class IdentifierScopesTest : public testing::Test {
	protected:
		void add(
			IIdentifiersRepository& repository, const std::string& id, ns3::Ptr<ns3::Object> object
		) {
			const auto error = resultCode(repository.add(id, object));
			EXPECT_EQ(error, std::error_code{}) << "could not register test object";
		}

		template <typename Result>
		std::error_code resultCode(
			Result&& result
		) {
			if (result) {
				return {};
			}

			std::error_code code;
			bool matched = false;
			result.error().handle([&](const vcle::EnumeratedErrorInfo<std::errc>& info) {
				code = std::make_error_code(info.code());
				matched = true;
			});
			EXPECT_TRUE(matched) << "repository failure must retain its portable error code";
			return code;
		}

		ns3::Ptr<ns3::Object> lookup(
			const IIdentifiersRepository& repository, const std::string& id
		) {
			auto result = repository.get(id);
			if (!result) {
				const auto message = vcle::toString(result.error());
				ADD_FAILURE() << message;
				return nullptr;
			}

			return *result;
		}

		void expectMissing(
			const IIdentifiersRepository& repository, const std::string& id
		) {
			auto result = repository.get(id);
			const auto error = resultCode(result);
			EXPECT_EQ(error, std::make_error_code(std::errc::no_such_file_or_directory))
				<< "lookup must have no selection";
		}
	};

} // namespace

TEST(
	IdentifierSelectionPolicy, NearestScopePolicy
) {
	NearestScopeSelectionPolicy policy;
	IdentifiersRepositoryContext context;
	auto object = ns3::CreateObject<ScopeObject>();
	policy.setContext(context);

	context.distance_ = 4;
	EXPECT_EQ(static_cast<int>(policy.select(object)), static_cast<int>(Decision::SELECT_THIS))
		<< "first candidate was not selected";
	context.distance_ = 2;
	EXPECT_EQ(static_cast<int>(policy.select(object)), static_cast<int>(Decision::SELECT_THIS))
		<< "closer candidate was not selected";
	context.distance_ = 3;
	EXPECT_EQ(static_cast<int>(policy.select(object)), static_cast<int>(Decision::PRESERVE))
		<< "more distant candidate replaced selection";
	context.distance_ = 2;
	EXPECT_EQ(static_cast<int>(policy.select(object)), static_cast<int>(Decision::PRESERVE))
		<< "equal distance replaced selection";
	context.distance_ = 0;
	EXPECT_EQ(static_cast<int>(policy.select(object)), static_cast<int>(Decision::SELECT_THIS))
		<< "local candidate was not selected";

	IdentifiersRepositoryContext next{5};
	policy.setContext(next);
	EXPECT_EQ(static_cast<int>(policy.select(object)), static_cast<int>(Decision::SELECT_THIS))
		<< "previous lookup leaked selection state";
}

TEST_F(
	IdentifierScopesTest, ScopeVisibility
) {
	MemoryIdentifiersRepository root;
	auto global = ns3::CreateObject<ScopeObject>();
	add(root, "global", global);
	auto group = root.createScope();
	auto sibling = root.createScope();
	auto local = ns3::CreateObject<ScopeObject>();
	add(group, "local", local);
	auto nested = group.createScope();
	auto deepest = ns3::CreateObject<ScopeObject>();
	add(nested, "deepest", deepest);

	EXPECT_EQ(lookup(group, "global"), global) << "group cannot see global binding";
	EXPECT_EQ(lookup(nested, "global"), global) << "nested group cannot see global binding";
	EXPECT_EQ(lookup(nested, "local"), local) << "nested group cannot see parent binding";
	EXPECT_EQ(lookup(nested, "deepest"), deepest) << "nested group cannot see its binding";
	expectMissing(root, "local");
	expectMissing(sibling, "local");
	expectMissing(group, "deepest");
}

TEST_F(
	IdentifierScopesTest, Shadowing
) {
	MemoryIdentifiersRepository root;
	auto global = ns3::CreateObject<ScopeObject>();
	auto local = ns3::CreateObject<ScopeObject>();
	auto siblingObject = ns3::CreateObject<ScopeObject>();
	add(root, "node", global);
	auto group = root.createScope();
	add(group, "node", local);
	auto nested = group.createScope();
	auto sibling = root.createScope();
	add(sibling, "node", siblingObject);

	EXPECT_EQ(lookup(group, "node"), local) << "local binding did not shadow global binding";
	EXPECT_EQ(lookup(nested, "node"), local) << "lookup skipped the nearest ancestor binding";
	EXPECT_EQ(lookup(root, "node"), global) << "shadowing changed global binding";
	EXPECT_EQ(lookup(sibling, "node"), siblingObject)
		<< "sibling binding collided with another group";

	const auto duplicate = resultCode(group.add("node", global));
	EXPECT_EQ(duplicate, std::make_error_code(std::errc::file_exists))
		<< "duplicate in the same scope was accepted";
	EXPECT_EQ(lookup(group, "node"), local) << "duplicate replaced local binding";
}

TEST_F(
	IdentifierScopesTest, ScopeLifetime
) {
	auto object = ns3::CreateObject<ScopeObject>();
	auto group = [&] {
		MemoryIdentifiersRepository root;
		add(root, "global", object);
		return root.createScope();
	}();

	EXPECT_EQ(lookup(group, "global"), object) << "child lost the parent scope's bindings";
}

TEST_F(
	IdentifierScopesTest, LateRegistration
) {
	MemoryIdentifiersRepository root;
	auto group = root.createScope();
	expectMissing(group, "later");

	auto object = ns3::CreateObject<ScopeObject>();
	add(root, "later", object);
	EXPECT_EQ(lookup(group, "later"), object) << "child retained a stale snapshot of its parent";
}

TEST_F(
	IdentifierScopesTest, CandidateMetadata
) {
	auto policy =
		std::make_shared<ScriptedPolicy>(std::vector{Decision::SELECT_THIS, Decision::PRESERVE});
	MemoryIdentifiersRepository root(policy);
	auto global = ns3::CreateObject<ScopeObject>();
	auto local = ns3::CreateObject<ScopeObject>();
	add(root, "node", global);
	auto parent = root.createScope();
	add(parent, "other", ns3::CreateObject<ScopeObject>());
	auto group = parent.createScope();
	add(group, "node", local);
	auto sibling = root.createScope();
	add(sibling, "node", ns3::CreateObject<ScopeObject>());

	EXPECT_EQ(lookup(group, "node"), local) << "PRESERVE did not keep the local selection";
	EXPECT_EQ(policy->lookups_, 1) << "context must be set once per lookup";
	ASSERT_EQ(policy->observations_.size(), 2)
		<< "policy saw unrelated candidates or missed a visible binding";

	EXPECT_EQ(policy->observations_[0].object_, local)
		<< "lookup did not start at the current scope";
	EXPECT_EQ(policy->observations_[0].distance_, 0) << "local distance is incorrect";
	EXPECT_EQ(policy->observations_[1].object_, global) << "lookup did not reach the root binding";
	EXPECT_EQ(policy->observations_[1].distance_, 2)
		<< "scope without a matching ID was not counted";
}

namespace {
	struct SelectionCase {
		const char* name_;
		std::vector<Decision> decisions_;
		int selectedDistance_;
	};

	class SelectionDecisionsTest : public IdentifierScopesTest,
								   public testing::WithParamInterface<SelectionCase> {};

} // namespace

TEST_P(
	SelectionDecisionsTest, ResolvesAllVisibleCandidates
) {
	const auto& parameter = GetParam();
	auto policy = std::make_shared<ScriptedPolicy>(parameter.decisions_);
	MemoryIdentifiersRepository root(policy);
	auto parent = root.createScope();
	auto group = parent.createScope();
	std::vector objects{
		ns3::CreateObject<ScopeObject>(),
		ns3::CreateObject<ScopeObject>(),
		ns3::CreateObject<ScopeObject>()
	};
	add(group, "node", objects[0]);
	add(parent, "node", objects[1]);
	add(root, "node", objects[2]);

	if (parameter.selectedDistance_ < 0) {
		expectMissing(group, "node");
	} else {
		EXPECT_EQ(lookup(group, "node"), objects[parameter.selectedDistance_])
			<< "policy decisions produced the wrong selection";
	}
	EXPECT_EQ(policy->observations_.size(), 3)
		<< "selection stopped before all visible candidates were evaluated";
}

INSTANTIATE_TEST_SUITE_P(
	Decisions,
	SelectionDecisionsTest,
	testing::Values(
		SelectionCase{
			"PreserveEmpty", {Decision::PRESERVE, Decision::PRESERVE, Decision::PRESERVE}, -1
		},
		SelectionCase{
			"PreserveSelected", {Decision::SELECT_THIS, Decision::PRESERVE, Decision::PRESERVE}, 0
		},
		SelectionCase{
			"ReplaceSelected",
			{Decision::SELECT_THIS, Decision::SELECT_THIS, Decision::SELECT_THIS},
			2
		},
		SelectionCase{
			"DropSelected", {Decision::SELECT_THIS, Decision::DROP, Decision::PRESERVE}, -1
		},
		SelectionCase{
			"SelectAfterDrop", {Decision::SELECT_THIS, Decision::DROP, Decision::SELECT_THIS}, 2
		},
		SelectionCase{"DropEmpty", {Decision::DROP, Decision::PRESERVE, Decision::SELECT_THIS}, 2}
	),
	[](const testing::TestParamInfo<SelectionCase>& parameter) { return parameter.param.name_; }
);

TEST_F(
	IdentifierScopesTest, PolicyReset
) {
	auto policy = std::make_shared<ScriptedPolicy>(std::vector{Decision::SELECT_THIS});
	MemoryIdentifiersRepository root(policy);
	auto first = ns3::CreateObject<ScopeObject>();
	auto second = ns3::CreateObject<ScopeObject>();
	add(root, "first", first);
	add(root, "second", second);

	EXPECT_EQ(lookup(root, "first"), first) << "first lookup failed";
	EXPECT_EQ(lookup(root, "second"), second) << "lookup reused previous policy state";
	expectMissing(root, "missing");
	EXPECT_EQ(policy->lookups_, 3) << "missing lookup did not reset policy context";
	EXPECT_TRUE(policy->observations_.empty()) << "missing lookup evaluated a stale candidate";

	auto group = root.createScope();
	EXPECT_EQ(lookup(group, "first"), first) << "child scope did not inherit the policy";
	EXPECT_EQ(policy->lookups_, 4) << "child lookup did not set a new context";
}
