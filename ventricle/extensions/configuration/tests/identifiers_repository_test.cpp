#include <ventricle/extensions/configuration/model/parser/memory_identifiers_repository.hpp>

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <system_error>
#include <utility>

using namespace vcle::config;

namespace {
	struct ObjectLifetime {
		bool destroyed_ = false;
		bool disposed_ = false;
	};

	/** A custom object whose identity, state, and lifetime are observable by the tests. */
	class TestObject : public ns3::Object {
	public:
		static ns3::TypeId GetTypeId() {
			static auto type = ns3::TypeId("IdentifiersRepositoryTest::Object")
								   .SetParent<ns3::Object>()
								   .AddConstructor<TestObject>();
			return type;
		}

		explicit TestObject(
			std::shared_ptr<ObjectLifetime> lifetime = std::make_shared<ObjectLifetime>()
		)
			: lifetime_(std::move(lifetime)) {
		}

		~TestObject() override {
			lifetime_->destroyed_ = true;
		}

		int value() const {
			return value_;
		}

		void setValue(
			int value
		) {
			value_ = value;
		}

	protected:
		/* ns3::Object implementation*/
		void DoDispose() override {
			lifetime_->disposed_ = true;
			ns3::Object::DoDispose();
		}

	private:
		std::shared_ptr<ObjectLifetime> lifetime_;
		int value_ = 0;
	};

	class IdentifiersRepositoryTest : public testing::Test {
	protected:
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
	};

} // namespace

TEST_F(
	IdentifiersRepositoryTest, Identity
) {
	MemoryIdentifiersRepository memory;
	IIdentifiersRepository& repository = memory;
	auto object = ns3::CreateObject<TestObject>();
	object->setValue(42);

	auto error = resultCode(repository.add("node", object));
	ASSERT_EQ(error, std::error_code{}) << "registration failed";
	auto found = lookup(repository, "node");
	ASSERT_EQ(found, object) << "lookup must preserve object identity";

	auto typed = ns3::DynamicCast<TestObject>(found);
	ASSERT_NE(typed, nullptr) << "lookup lost the custom object type";

	EXPECT_EQ(typed->value(), 42) << "lookup lost the object's state";
	typed->setValue(73);
	EXPECT_EQ(object->value(), 73) << "lookup returned a copy";
	EXPECT_EQ(lookup(repository, "node"), object) << "repeated lookup changed the object";
}

TEST_F(
	IdentifiersRepositoryTest, MissingIdentifier
) {
	MemoryIdentifiersRepository repository;
	auto missing = repository.get("missing");
	const auto error = resultCode(missing);
	ASSERT_EQ(error, std::make_error_code(std::errc::no_such_file_or_directory))
		<< "missing ID must report an error";

	auto object = ns3::CreateObject<TestObject>();
	const auto added = resultCode(repository.add("missing", object));
	ASSERT_EQ(added, std::error_code{}) << "failed lookup reserved the identifier";
	EXPECT_EQ(lookup(repository, "missing"), object)
		<< "registration after failed lookup did not persist";
}

TEST_F(
	IdentifiersRepositoryTest, DuplicateIdentifier
) {
	MemoryIdentifiersRepository repository;
	auto original = ns3::CreateObject<TestObject>();
	auto replacement = ns3::CreateObject<TestObject>();
	const auto added = resultCode(repository.add("node", original));
	ASSERT_EQ(added, std::error_code{}) << "initial registration failed";

	const auto duplicate = resultCode(repository.add("node", replacement));
	EXPECT_EQ(duplicate, std::make_error_code(std::errc::file_exists))
		<< "duplicate identifier was accepted";
	EXPECT_EQ(lookup(repository, "node"), original)
		<< "duplicate registration replaced the original object";

	const auto repeated = resultCode(repository.add("node", original));
	EXPECT_EQ(repeated, std::make_error_code(std::errc::file_exists))
		<< "registering the same ID and object twice must also fail";
}

TEST_F(
	IdentifiersRepositoryTest, InvalidRegistration
) {
	MemoryIdentifiersRepository repository;
	auto object = ns3::CreateObject<TestObject>();
	const auto empty = resultCode(repository.add("", object));
	EXPECT_EQ(empty, std::make_error_code(std::errc::invalid_argument))
		<< "empty identifier was accepted";

	auto emptyLookup = repository.get("");
	const auto emptyLookupError = resultCode(emptyLookup);
	EXPECT_EQ(emptyLookupError, std::make_error_code(std::errc::invalid_argument))
		<< "empty lookup identifier was accepted";

	const auto nullObject = resultCode(repository.add("node", nullptr));
	EXPECT_EQ(nullObject, std::make_error_code(std::errc::invalid_argument))
		<< "null object was accepted";
	const auto added = resultCode(repository.add("node", object));
	ASSERT_EQ(added, std::error_code{}) << "invalid registration reserved the identifier";
	EXPECT_EQ(lookup(repository, "node"), object) << "valid registration did not persist";
}

TEST_F(
	IdentifiersRepositoryTest, Alias
) {
	MemoryIdentifiersRepository repository;
	auto object = ns3::CreateObject<TestObject>();
	const auto first = resultCode(repository.add("node", object));
	ASSERT_EQ(first, std::error_code{}) << "first registration failed";
	const auto alias = resultCode(repository.add("alias", object));
	ASSERT_EQ(alias, std::error_code{}) << "alias registration failed";

	EXPECT_EQ(lookup(repository, "node"), object) << "original identifier changed";
	EXPECT_EQ(lookup(repository, "alias"), object) << "alias points to a different object";
}

TEST_F(
	IdentifiersRepositoryTest, IdentifierStorage
) {
	MemoryIdentifiersRepository repository;
	auto first = ns3::CreateObject<TestObject>();
	auto second = ns3::CreateObject<TestObject>();
	std::string id = "node";
	const auto added = resultCode(repository.add(id, first));
	ASSERT_EQ(added, std::error_code{}) << "first registration failed";

	id = "Node";
	const auto next = resultCode(repository.add(id, second));
	ASSERT_EQ(next, std::error_code{}) << "distinct identifier was rejected";
	id.clear();

	EXPECT_EQ(lookup(repository, "node"), first) << "identifier changed with the caller's string";
	EXPECT_EQ(lookup(repository, "Node"), second) << "identifier lookup ignored case";
}

TEST_F(
	IdentifiersRepositoryTest, Ownership
) {
	auto lifetime = std::make_shared<ObjectLifetime>();
	{
		MemoryIdentifiersRepository repository;
		auto object = ns3::CreateObject<TestObject>(lifetime);
		const auto added = resultCode(repository.add("node", object));
		ASSERT_EQ(added, std::error_code{}) << "registration failed";
		object = nullptr;

		EXPECT_FALSE(lifetime->destroyed_) << "repository did not retain the object";
		EXPECT_FALSE(lifetime->disposed_) << "registration disposed the object";
		EXPECT_NE(lookup(repository, "node"), nullptr) << "retained object cannot be retrieved";
	}

	EXPECT_TRUE(lifetime->destroyed_) << "repository destruction leaked its object";
}

TEST_F(
	IdentifiersRepositoryTest, LookupOwnership
) {
	auto lifetime = std::make_shared<ObjectLifetime>();
	ns3::Ptr<ns3::Object> retained;
	{
		std::unique_ptr<IIdentifiersRepository> repository =
			std::make_unique<MemoryIdentifiersRepository>();
		const auto added =
			resultCode(repository->add("node", ns3::CreateObject<TestObject>(lifetime)));
		ASSERT_EQ(added, std::error_code{}) << "registration failed";
		retained = lookup(*repository, "node");
		ASSERT_NE(retained, nullptr) << "lookup did not return an object";
	}

	EXPECT_FALSE(lifetime->destroyed_) << "repository destroyed an object still held by its caller";
	EXPECT_FALSE(lifetime->disposed_) << "repository disposed an object still held by its caller";
	auto object = ns3::DynamicCast<TestObject>(retained);
	ASSERT_NE(object, nullptr) << "returned object lost its type";

	object->setValue(73);
	EXPECT_EQ(object->value(), 73) << "returned object is no longer usable";

	object = nullptr;
	retained = nullptr;
	EXPECT_TRUE(lifetime->destroyed_) << "lookup leaked ownership of the object";
}

TEST_F(
	IdentifiersRepositoryTest, RepositoryIsolation
) {
	MemoryIdentifiersRepository first;
	MemoryIdentifiersRepository second;
	auto firstObject = ns3::CreateObject<TestObject>();
	auto secondObject = ns3::CreateObject<TestObject>();
	const auto firstAdded = resultCode(first.add("node", firstObject));
	ASSERT_EQ(firstAdded, std::error_code{}) << "first repository registration failed";
	auto missing = second.get("node");
	const auto missingError = resultCode(missing);
	ASSERT_EQ(missingError, std::make_error_code(std::errc::no_such_file_or_directory))
		<< "second repository sees the first repository's object";

	const auto secondAdded = resultCode(second.add("node", secondObject));
	ASSERT_EQ(secondAdded, std::error_code{}) << "second repository registration failed";
	EXPECT_EQ(lookup(first, "node"), firstObject)
		<< "second registration changed the first repository";
	EXPECT_EQ(lookup(second, "node"), secondObject)
		<< "second repository contains the wrong object";
}
