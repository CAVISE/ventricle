#include "src/extensions/its-g5/helpers/packet_conversion.hpp"

#include <array>
#include <cstdint>

#include <ns3/packet.h>
#include <ns3/test.h>

#include <vanetza/common/byte_buffer.hpp>
#include <vanetza/net/chunk_packet.hpp>
#include <vanetza/net/cohesive_packet.hpp>
#include <vanetza/net/osi_layer.hpp>

namespace vcle::itsg5 {
	namespace {
		class PacketConversionTest final : public ns3::TestCase {
		public:
			PacketConversionTest()
				: TestCase("preserves Vanetza wire bytes") {
			}

		private:
			/* ns3::TestCase implementation*/
			void DoRun() override {
				vanetza::ChunkPacket source;
				source[vanetza::OsiLayer::Network] = vanetza::ByteBuffer{0x01, 0x02};
				source[vanetza::OsiLayer::Transport] = vanetza::ByteBuffer{0x03};
				source[vanetza::OsiLayer::Application] = vanetza::ByteBuffer{0x04, 0x05};

				const auto ns3Packet = toNs3Packet(source);
				std::array<std::uint8_t, 5> bytes{};
				ns3Packet->CopyData(bytes.data(), bytes.size());

				NS_TEST_EXPECT_MSG_EQ(bytes[0], 0x01, "network bytes moved");
				NS_TEST_EXPECT_MSG_EQ(bytes[2], 0x03, "transport bytes moved");
				NS_TEST_EXPECT_MSG_EQ(bytes[4], 0x05, "application bytes moved");

				const auto restored = toVanetzaPacket(*ns3Packet);
				const auto* cohesive = boost::get<vanetza::CohesivePacket>(restored.get());
				NS_TEST_ASSERT_MSG_NE(cohesive, nullptr, "received packet is not cohesive");
				NS_TEST_EXPECT_MSG_EQ(cohesive->size(), bytes.size(), "received packet size changed");
			}
		};

		class PacketConversionTestSuite final : public ns3::TestSuite {
		public:
			PacketConversionTestSuite()
				: TestSuite("ventricle-its-g5-packet", Type::UNIT) {
				AddTestCase(new PacketConversionTest(), TestCase::Duration::QUICK);
			}
		};

		// ns-3 discovers test suites through static registration.
		// NOLINTNEXTLINE(bugprone-throwing-static-initialization)
		PacketConversionTestSuite PacketConversionTests;
	} // namespace
} // namespace vcle::itsg5
