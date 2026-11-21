#include <ventricle/extensions/configuration/model/schema.hpp>
#include <ventricle/extensions/configuration/tests/schema_test_types.hpp>

#include <iostream>
#include <string>
#include <vector>

using namespace vcle::config;

int main(
	int argc, char** argv
) {
	const std::string mode = argc > 1 ? argv[1] : "valid";
	std::vector types{test::Sample::GetTypeId(), ns3::Object::GetTypeId()};
	if (mode == "duplicate") {
		types.push_back(test::Sample::GetTypeId());
	}
	if (mode == "invalid") {
		types.push_back(ns3::TypeId{});
	}

	auto result = generateSchema(types);
	if (test::constructions != 0) {
		std::cerr << "Schema generation constructed model objects";
		return 2;
	}
	if (!result) {
		std::cerr << vcle::toString(result.error());
		return 1;
	}
	std::cout << *result;
}
