#include "src/extensions/its-g5/helpers/type_glue.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

#include <ns3/mac48-address.h>
#include <ns3/test.h>

#include <vanetza/net/mac_address.hpp>

namespace vcle::itsg5 {
	namespace {
		class AddressConversionTest final : public ns3::TestCase {
		public:
			AddressConversionTest()
				: TestCase("preserves all MAC address octets") {
			}

		private:
			/* ns3::TestCase implementation*/
			void DoRun() override {
				vanetza::MacAddress source;
				source.octets = {0x02, 0x11, 0x22, 0x33, 0x44, 0x55};

				const auto ns3Address = TypeGlue::convert<ns3::Mac48Address>(source);
				const auto restored = TypeGlue::convert<vanetza::MacAddress>(ns3Address);

				for (std::size_t i = 0; i < source.octets.size(); ++i) {
					NS_TEST_EXPECT_MSG_EQ(restored.octets[i], source.octets[i], "address octet changed during round-trip conversion");
				}
			}
		};

		class AddressConversionTestSuite final : public ns3::TestSuite {
		public:
			AddressConversionTestSuite()
				: TestSuite("ventricle-its-g5-address", Type::UNIT) {
				AddTestCase(new AddressConversionTest(), TestCase::Duration::QUICK);
			}
		};

		// ns-3 discovers test suites through static registration.
		// NOLINTNEXTLINE(bugprone-throwing-static-initialization)
		AddressConversionTestSuite AddressConversionTests;
	} // namespace
} // namespace vcle::itsg5
