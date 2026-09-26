#include "src/core/simulation/nodes.hpp"

#include <vector>

#include <ns3/nstime.h>
#include <ns3/simulator.h>
#include <ns3/test.h>

namespace vcle {
	namespace {
		class TestNodePoolProvider final : public INodePoolProvider {
		public:
			explicit TestNodePoolProvider(ns3::NodeContainer nodes)
				: nodes_(std::move(nodes)) {
			}

			/* INodePoolProvider implementation*/
			ns3::NodeContainer takeNodes() override {
				++calls_;
				auto nodes = nodes_;
				nodes_ = {};
				return nodes;
			}

			std::size_t calls() const {
				return calls_;
			}

		private:
			ns3::NodeContainer nodes_;
			std::size_t calls_ = 0;
		};

		class DynamicNodeLifecycleTest final : public ns3::TestCase {
		public:
			DynamicNodeLifecycleTest()
				: TestCase("activates, returns, and reuses pooled nodes") {
			}

		private:
			/* ns3::TestCase implementation*/
			void DoRun() override {
				ns3::NodeContainer nodes;
				nodes.Create(2);
				TestNodePoolProvider provider(nodes);

				std::vector<ns3::Ptr<ns3::Node>> initialized;
				std::vector<ns3::Ptr<ns3::Node>> deinitialized;
				DynamicNodeManager manager(
					provider,
					[&](auto node) { initialized.push_back(node); },
					[&](auto node) { deinitialized.push_back(node); }
				);
				NS_TEST_EXPECT_MSG_EQ(provider.calls(), 1, "manager did not take exactly one pool");

				ns3::Simulator::Schedule(ns3::Seconds(1), [&] {
					const auto first = manager.addNode();
					const auto second = manager.addNode();

					NS_TEST_EXPECT_MSG_EQ(first, nodes.Get(0), "manager returned the wrong first node");
					NS_TEST_EXPECT_MSG_EQ(second, nodes.Get(1), "manager returned the wrong second node");
					NS_TEST_EXPECT_MSG_EQ(manager.addNode(), nullptr, "manager returned a node after exhausting its pool");

					manager.removeNode(first);
					NS_TEST_EXPECT_MSG_EQ(deinitialized.back(), first, "manager did not deinitialize the returned node");
					NS_TEST_EXPECT_MSG_EQ(manager.addNode(), first, "manager did not reuse the returned node");
				});

				ns3::Simulator::Run();
				NS_TEST_EXPECT_MSG_EQ(initialized.size(), 3, "initializer call count is incorrect");
				NS_TEST_EXPECT_MSG_EQ(deinitialized.size(), 1, "deinitializer call count is incorrect");
				ns3::Simulator::Destroy();
			}
		};

		class DynamicNodeManagerTestSuite final : public ns3::TestSuite {
		public:
			DynamicNodeManagerTestSuite()
				: TestSuite("ventricle-dynamic-node-manager", Type::UNIT) {
				AddTestCase(new DynamicNodeLifecycleTest(), TestCase::Duration::QUICK);
			}
		};

		// ns-3 discovers test suites through static registration.
		// NOLINTNEXTLINE(bugprone-throwing-static-initialization)
		DynamicNodeManagerTestSuite DynamicNodeManagerTests;
	} // namespace
} // namespace vcle
