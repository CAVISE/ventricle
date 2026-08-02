#include "src/extensions/its-g5/helpers/type_glue.hpp"
#include "src/extensions/its-g5/model/runtime.hpp"

#include <chrono>
#include <ns3/simulator.h>
#include <ns3/test.h>

namespace vcle::itsg5 {
	namespace {
		using namespace std::chrono_literals;

		class TimeConversionTest final : public ns3::TestCase {
		public:
			TimeConversionTest()
				: TestCase("converts time at Vanetza microsecond precision") {
			}

		private:
			void DoRun() override {
				const auto duration = vanetza::Clock::duration(123456us);
				NS_TEST_EXPECT_MSG_EQ(
					TypeGlue::convert<ns3::Time>(duration).GetMicroSeconds(),
					123456,
					"Vanetza duration changed during conversion"
				);
				NS_TEST_EXPECT_MSG_EQ(
					TypeGlue::convert<vanetza::Clock::duration>(ns3::MicroSeconds(987654)).count(),
					987654,
					"ns-3 duration changed during conversion"
				);
			}
		};

		class RuntimeSchedulingTest final : public ns3::TestCase {
		public:
			RuntimeSchedulingTest()
				: TestCase("runs scheduled callbacks on ns-3 time") {
			}

		private:
			void DoRun() override {
				Runtime runtime;
				bool called = false;

				runtime.schedule(25ms, [&](vanetza::Clock::time_point now) {
					called = true;
					NS_TEST_EXPECT_MSG_EQ(
						now.time_since_epoch().count(),
						std::chrono::duration_cast<vanetza::Clock::duration>(25ms).count(),
						"callback received wrong simulation time"
					);
				});

				ns3::Simulator::Run();
				NS_TEST_EXPECT_MSG_EQ(called, true, "scheduled callback did not run");
				ns3::Simulator::Destroy();
			}
		};

		class RuntimeCancellationTest final : public ns3::TestCase {
		public:
			RuntimeCancellationTest()
				: TestCase("cancels callbacks by Vanetza scope") {
			}

		private:
			void DoRun() override {
				Runtime runtime;
				bool called = false;
				const int scope = 0;

				runtime.schedule(10ms, [&](vanetza::Clock::time_point) { called = true; }, &scope);
				runtime.cancel(&scope);

				ns3::Simulator::Run();
				NS_TEST_EXPECT_MSG_EQ(called, false, "cancelled callback ran");
				ns3::Simulator::Destroy();
			}
		};

		class ItsG5TestSuite final : public ns3::TestSuite {
		public:
			ItsG5TestSuite()
				: TestSuite("ventricle-its-g5-runtime", Type::UNIT) {
				AddTestCase(new TimeConversionTest(), TestCase::Duration::QUICK);
				AddTestCase(new RuntimeSchedulingTest(), TestCase::Duration::QUICK);
				AddTestCase(new RuntimeCancellationTest(), TestCase::Duration::QUICK);
			}
		};

		// ns-3 discovers test suites through static registration.
		// NOLINTNEXTLINE(bugprone-throwing-static-initialization)
		ItsG5TestSuite ItsG5Tests;
	} // namespace
} // namespace vcle::itsg5

int main(
	int argc, char** argv
) {
	return ns3::TestRunner::Run(argc, argv);
}
