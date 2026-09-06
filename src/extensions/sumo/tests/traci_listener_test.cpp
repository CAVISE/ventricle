#include "src/extensions/sumo/model/controller.hpp"
#include "src/extensions/sumo/model/traci_values.hpp"

#include <ns3/boolean.h>
#include <ns3/integer.h>
#include <ns3/string.h>
#include <ns3/test.h>

namespace vcle::sumo::test {

	class TraciValuesTestCase final : public ns3::TestCase {
	public:
		TraciValuesTestCase()
			: TestCase("provides typed access to TraCI subscription values") {
		}

	private:
		/* ns3::TestCase implementation*/
		void DoRun() override {
			libsumo::TraCIResults results;
			results.emplace(libsumo::VAR_SPEED, std::make_shared<libsumo::TraCIDouble>(42.0));
			results.emplace(libsumo::VAR_ANGLE, std::make_shared<libsumo::TraCIDouble>(90.0));
			const auto type = std::make_shared<libsumo::TraCIString>();
			type->value = "passenger";
			const auto ids = std::make_shared<libsumo::TraCIStringList>();
			ids->value = {"vehicle-1"};
			const auto position = std::make_shared<libsumo::TraCIPosition>();
			results.emplace(libsumo::VAR_TYPE, type);
			results.emplace(libsumo::VAR_ARRIVED_VEHICLES_IDS, ids);
			results.emplace(libsumo::VAR_POSITION, position);
			results.emplace(libsumo::VAR_SIGNALS, std::make_shared<libsumo::TraCIInt>(3));
			const TraciValues values(results);
			const auto [speed, angle] = values.getMultiple<libsumo::VAR_SPEED, libsumo::VAR_ANGLE>();

			static_assert(std::is_same_v<decltype(values.get<libsumo::VAR_SPEED>()), double>);
			static_assert(std::is_same_v<decltype(values.get<libsumo::VAR_SIGNALS>()), int>);
			static_assert(std::is_same_v<decltype(values.get<libsumo::VAR_TYPE>()), const std::string&>);
			static_assert(std::is_same_v<decltype(values.get<libsumo::VAR_ARRIVED_VEHICLES_IDS>()), const std::vector<std::string>&>);
			static_assert(std::is_same_v<decltype(values.get<libsumo::VAR_POSITION>()), const libsumo::TraCIPosition&>);
			NS_TEST_ASSERT_MSG_EQ(values.get<libsumo::VAR_SIGNALS>(), 3, "integer value differs");
			NS_TEST_ASSERT_MSG_EQ(&values.get<libsumo::VAR_TYPE>(), &type->value, "string should reference the stored value");
			NS_TEST_ASSERT_MSG_EQ(&values.get<libsumo::VAR_ARRIVED_VEHICLES_IDS>(), &ids->value, "list should reference the stored value");
			NS_TEST_ASSERT_MSG_EQ(&values.get<libsumo::VAR_POSITION>(), position.get(), "position should reference the result itself");

			NS_TEST_ASSERT_MSG_EQ_TOL(speed, 42.0, 1e-9, "typed value differs");
			NS_TEST_ASSERT_MSG_EQ_TOL(angle, 90.0, 1e-9, "second typed value differs");
			NS_TEST_ASSERT_MSG_EQ(values.find<libsumo::VAR_MAXSPEED>(), nullptr, "missing value should not resolve");
		}
	};

	class ControllerAttributesTestCase final : public ns3::TestCase {
	public:
		ControllerAttributesTestCase()
			: TestCase("exposes libtraci launch parameters as controller attributes") {
		}

	private:
		/* ns3::TestCase implementation*/
		void DoRun() override {
			TraciPort traciPort;
			NS_TEST_ASSERT_MSG_EQ(traciPort.port(), -1, "new TraCI port should be disconnected");

			TraciNodeManager nodeManager(ns3::NodeContainer{}, [](auto) {}, [](auto) {});
			const auto controller = ns3::CreateObject<Controller>(traciPort, nodeManager);
			controller->SetAttribute("Command", ns3::StringValue("sumo -c \"scenario file.sumocfg\""));
			controller->SetAttribute("Port", ns3::IntegerValue(8813));
			controller->SetAttribute("Retries", ns3::IntegerValue(4));
			controller->SetAttribute("Verbose", ns3::BooleanValue(true));
			controller->SetAttribute("TraceFile", ns3::StringValue("traci.trace"));
			controller->SetAttribute("TraceGetters", ns3::BooleanValue(false));

			ns3::StringValue command;
			ns3::IntegerValue port;
			ns3::IntegerValue retries;
			ns3::BooleanValue verbose;
			ns3::StringValue traceFile;
			ns3::BooleanValue traceGetters;
			controller->GetAttribute("Command", command);
			controller->GetAttribute("Port", port);
			controller->GetAttribute("Retries", retries);
			controller->GetAttribute("Verbose", verbose);
			controller->GetAttribute("TraceFile", traceFile);
			controller->GetAttribute("TraceGetters", traceGetters);

			NS_TEST_ASSERT_MSG_EQ(command.Get(), "sumo -c \"scenario file.sumocfg\"", "command attribute differs");
			NS_TEST_ASSERT_MSG_EQ(port.Get(), 8813, "port attribute differs");
			NS_TEST_ASSERT_MSG_EQ(retries.Get(), 4, "retry attribute differs");
			NS_TEST_ASSERT_MSG_EQ(verbose.Get(), true, "verbose attribute differs");
			NS_TEST_ASSERT_MSG_EQ(traceFile.Get(), "traci.trace", "trace file attribute differs");
			NS_TEST_ASSERT_MSG_EQ(traceGetters.Get(), false, "trace-getter attribute differs");
		}
	};

	class TraciListenerTestSuite final : public ns3::TestSuite {
	public:
		TraciListenerTestSuite()
			: TestSuite("ventricle-traci-listener", Type::UNIT) {
			AddTestCase(new TraciValuesTestCase, Duration::QUICK);
			AddTestCase(new ControllerAttributesTestCase, Duration::QUICK);
		}
	};

	TraciListenerTestSuite TraciListenerTests;

} // namespace vcle::sumo::test
