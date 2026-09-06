#include "src/core/simulation/listeners.hpp"

#include <ns3/test.h>

namespace vcle {
	namespace {
		struct TestListener {
			bool fail = false;
			int statusCalls = 0;
			int total = 0;

			absl::Status update() {
				++statusCalls;
				return fail ? absl::InternalError("test failure") : absl::OkStatus();
			}

			void notify(int value) {
				total += value;
			}
		};

		class TestSource : public PublisherBase<TestListener> {
		public:
			using PublisherBase<TestListener>::invokeListeners;
		};

		class ListenersTestCase final : public ns3::TestCase {
		public:
			ListenersTestCase()
				: TestCase("dispatches void callbacks and removes failed listeners without skipping survivors") {
			}

		private:
			/* ns3::TestCase implementation*/
			void DoRun() override {
				TestListener failedFirst {true};
				TestListener failedSecond {true};
				TestListener survivor;
				TestSource source;
				source.addListener(&failedFirst);
				source.addListener(&failedSecond);
				source.addListener(&survivor);

				source.invokeListeners(&TestListener::notify, 2);
				NS_TEST_ASSERT_MSG_EQ(failedFirst.total, 2, "first void callback was not invoked once");
				NS_TEST_ASSERT_MSG_EQ(failedSecond.total, 2, "second void callback was not invoked once");
				NS_TEST_ASSERT_MSG_EQ(survivor.total, 2, "last void callback was not invoked once");

				source.invokeListeners(&TestListener::update);
				source.invokeListeners(&TestListener::update);
				NS_TEST_ASSERT_MSG_EQ(failedFirst.statusCalls, 1, "failed listener was retained");
				NS_TEST_ASSERT_MSG_EQ(failedSecond.statusCalls, 1, "adjacent failed listener was skipped or retained");
				NS_TEST_ASSERT_MSG_EQ(survivor.statusCalls, 2, "successful listener was skipped or removed");

				source.removeListener(&failedFirst);
				source.removeListener(&survivor);
				source.invokeListeners(&TestListener::notify, 3);
				NS_TEST_ASSERT_MSG_EQ(survivor.total, 2, "removed listener received a callback");

				source.addListener(&survivor);
				source.invokeListeners(&TestListener::notify, 3);
				NS_TEST_ASSERT_MSG_EQ(survivor.total, 5, "listener could not be registered again");
			}
		};

		class ListenersTestSuite final : public ns3::TestSuite {
		public:
			ListenersTestSuite()
				: TestSuite("ventricle-listeners", Type::UNIT) {
				AddTestCase(new ListenersTestCase, Duration::QUICK);
			}
		};

		ListenersTestSuite listenersTests;
	} // namespace
} // namespace vcle
