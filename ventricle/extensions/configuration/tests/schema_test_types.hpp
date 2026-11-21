#pragma once

#include <ns3/boolean.h>
#include <ns3/object.h>
#include <ns3/string.h>
#include <string>

namespace vcle::config::test {
	inline int constructions = 0;

	/// @brief Fixture exposing an inherited construction attribute.
	class Base : public ns3::Object {
	public:
		static ns3::TypeId GetTypeId() {
			static auto type = ns3::TypeId("SchemaTest::Base")
								   .SetParent<ns3::Object>()
								   .AddAttribute(
									   "Label",
									   "Inherited label",
									   ns3::StringValue("default"),
									   ns3::MakeStringAccessor(&Base::label_),
									   ns3::MakeStringChecker()
								   );
			return type;
		}

	private:
		std::string label_;
	};

	/// @brief Construction is observable so tests can detect accidental model instantiation.
	class Sample : public Base {
	public:
		Sample() {
			++constructions;
		}

		static ns3::TypeId GetTypeId() {
			static auto type = ns3::TypeId("SchemaTest::Derived")
								   .SetParent<Base>()
								   .AddConstructor<Sample>()
								   .AddAttribute(
									   "Enabled",
									   "Whether enabled",
									   ns3::BooleanValue(true),
									   ns3::MakeBooleanAccessor(&Sample::enabled_),
									   ns3::MakeBooleanChecker()
								   );
			return type;
		}

	private:
		bool enabled_ = true;
	};
} // namespace vcle::config::test
