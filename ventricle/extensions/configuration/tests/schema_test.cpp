#include <ventricle/extensions/configuration/model/schema.hpp>
#include <ventricle/extensions/configuration/tests/schema_test_types.hpp>

#include <array>
#include <gtest/gtest.h>
#include <string>

using namespace vcle::config;

TEST(
	ConfigurationSchema, GeneratesWithoutConstructingModels
) {
	const auto before = test::constructions;
	const std::array types{test::Sample::GetTypeId()};
	auto result = generateSchema(types);
	ASSERT_TRUE(result) << vcle::toString(result.error());
	EXPECT_FALSE(result->empty());
	EXPECT_EQ(test::constructions, before);
}

TEST(
	ConfigurationSchema, RejectsDuplicateTypes
) {
	const std::array types{test::Sample::GetTypeId(), test::Sample::GetTypeId()};
	auto result = generateSchema(types);
	ASSERT_FALSE(result);
	EXPECT_NE(vcle::toString(result.error()).find("Duplicate exposed TypeId"), std::string::npos);
}

TEST(
	ConfigurationSchema, RejectsUninitializedTypes
) {
	const std::array types{ns3::TypeId{}};
	auto result = generateSchema(types);
	ASSERT_FALSE(result);
	EXPECT_NE(vcle::toString(result.error()).find("uninitialized TypeId"), std::string::npos);
}
