#include "src/extensions/sumo/model/mobility/traci_mobility_manager.hpp"
#include "src/extensions/sumo/model/mobility/traci_mobility_model.hpp"
#include "src/extensions/sumo/model/traci_node_manager.hpp"
#include "src/extensions/sumo/model/traci_subscription_manager.hpp"

#include <memory>

#include <ns3/node-container.h>
#include <ns3/test.h>

namespace vcle::sumo::test {
	class TraciNodeManagerTestCase final : public ns3::TestCase {
	public:
		TraciNodeManagerTestCase()
			: TestCase("associates SUMO vehicle IDs with pooled nodes") {
		}

	private:
		/* ns3::TestCase implementation*/
		void DoRun() override {
			ns3::NodeContainer nodes;
			nodes.Create(1);
			TraciNodeManager manager(nodes, [](auto) {}, [](auto) {});

			const auto added = manager.addVehicle("vehicle-1");
			NS_TEST_ASSERT_MSG_EQ(added.ok(), true, added.status());
			const auto node = *added;
			NS_TEST_ASSERT_MSG_NE(node, nullptr, "pool should provide a node");
			NS_TEST_ASSERT_MSG_EQ(manager.findVehicle("vehicle-1"), node, "vehicle should resolve to its node");
			NS_TEST_ASSERT_MSG_EQ(
				manager.addVehicle("vehicle-1").status().code(),
				absl::StatusCode::kAlreadyExists,
				"duplicate vehicle status differs"
			);
			NS_TEST_ASSERT_MSG_EQ(
				manager.addVehicle("vehicle-2").status().code(),
				absl::StatusCode::kResourceExhausted,
				"pool exhaustion status differs"
			);

			NS_TEST_ASSERT_MSG_EQ(manager.removeVehicle("vehicle-1").ok(), true, "vehicle removal failed");
			NS_TEST_ASSERT_MSG_EQ(manager.removeVehicle("vehicle-1").code(), absl::StatusCode::kNotFound, "missing vehicle status differs");
			NS_TEST_ASSERT_MSG_EQ(manager.findVehicle("vehicle-1"), nullptr, "removed vehicle should not resolve");
			const auto reused = manager.addVehicle("vehicle-2");
			NS_TEST_ASSERT_MSG_EQ(reused.ok(), true, reused.status());
			NS_TEST_ASSERT_MSG_EQ(*reused, node, "returned node should be reusable");
		}
	};

	class TraciMobilityModelTestCase final : public ns3::TestCase {
	public:
		TraciMobilityModelTestCase()
			: TestCase("models pooled-node mobility from TraCI vehicle values") {
		}

	private:
		/* ns3::TestCase implementation*/
		void DoRun() override {
			TraciSubscriptions subscriptions;
			ns3::NodeContainer nodes;
			nodes.Create(1);
			const auto mobility = ns3::CreateObject<TraciMobilityModel>();
			nodes.Get(0)->AggregateObject(mobility);
			TraciNodeManager manager(nodes, [](auto) {}, [](auto) {});
			TraciMobilityManager mobilityManager(subscriptions.get<libtraci::Vehicle>());
			manager.addListener(mobilityManager);

			const auto added = manager.addVehicle("vehicle-1");
			NS_TEST_ASSERT_MSG_EQ(added.ok(), true, added.status());
			const auto node = *added;
			auto position = std::make_shared<libsumo::TraCIPosition>();
			position->x = 10.0;
			position->y = 20.0;
			position->z = 3.0;
			libsumo::TraCIResults results;
			results.emplace(libsumo::VAR_POSITION, std::move(position));
			results.emplace(libsumo::VAR_SPEED, std::make_shared<libsumo::TraCIDouble>(12.0));
			results.emplace(libsumo::VAR_ANGLE, std::make_shared<libsumo::TraCIDouble>(90.0));
			mobility->onTraciValues("vehicle-1", TraciValues(results), ns3::Seconds(1));

			NS_TEST_ASSERT_MSG_EQ(node->GetObject<ns3::MobilityModel>(), mobility, "model differs");
			NS_TEST_ASSERT_MSG_EQ_TOL(mobility->GetPosition().x, 10.0, 1e-9, "x position differs");
			NS_TEST_ASSERT_MSG_EQ_TOL(mobility->GetPosition().y, 20.0, 1e-9, "y position differs");
			NS_TEST_ASSERT_MSG_EQ_TOL(mobility->GetPosition().z, 3.0, 1e-9, "z position differs");
			NS_TEST_ASSERT_MSG_EQ_TOL(mobility->GetVelocity().x, 12.0, 1e-9, "x velocity differs");
			NS_TEST_ASSERT_MSG_EQ_TOL(mobility->GetVelocity().y, 0.0, 1e-9, "y velocity differs");
			NS_TEST_ASSERT_MSG_EQ(manager.removeVehicle("vehicle-1").ok(), true, "vehicle removal failed");
			manager.removeListener(mobilityManager);
		}
	};

	class TraciNodeManagerTestSuite final : public ns3::TestSuite {
	public:
		TraciNodeManagerTestSuite()
			: TestSuite("ventricle-traci-node-manager", Type::UNIT) {
			AddTestCase(new TraciNodeManagerTestCase, Duration::QUICK);
			AddTestCase(new TraciMobilityModelTestCase, Duration::QUICK);
		}
	};

	TraciNodeManagerTestSuite TraciNodeManagerTests;

} // namespace vcle::sumo::test
