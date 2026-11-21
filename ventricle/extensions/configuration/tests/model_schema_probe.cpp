#include <ventricle/extensions/configuration/model/schema.hpp>

#include <array>
#include <cstdint>
#include <iostream>
#include <ns3/boolean.h>
#include <ns3/object.h>
#include <ns3/string.h>
#include <ns3/uinteger.h>
#include <string>

using namespace vcle;

namespace {
	class Sample : public ns3::Object {
	public:
		static ns3::TypeId GetTypeId() {
			static auto type = [] {
				auto id = ns3::TypeId("SchemaTest::Sample")
							  .SetParent<ns3::Object>()
							  .AddConstructor<Sample>()
							  .AddAttribute(
								  "Label",
								  "Label",
								  ns3::StringValue("default"),
								  ns3::MakeStringAccessor(&Sample::label_),
								  ns3::MakeStringChecker()
							  )
							  .AddAttribute(
								  "Count",
								  "Count",
								  ns3::UintegerValue(1),
								  ns3::MakeUintegerAccessor(&Sample::count_),
								  ns3::MakeUintegerChecker<uint32_t>(0, 10)
							  )
							  .AddAttribute(
								  "Enabled",
								  "Enabled",
								  ns3::BooleanValue(true),
								  ns3::MakeBooleanAccessor(&Sample::enabled_),
								  ns3::MakeBooleanChecker()
							  );
				return id;
			}();
			return type;
		}

	private:
		std::string label_;
		uint32_t count_ = 1;
		bool enabled_ = true;
	};
} // namespace

int main() {
	const std::array types{Sample::GetTypeId()};

	auto schema = config::generateSchema(types);
	if (!schema) {
		std::cerr << vcle::toString(schema.error());
		return 1;
	}

	std::cout << *schema;
	return 0;
}
