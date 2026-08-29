#include "src/extensions/its-g5/helpers/packet_conversion.hpp"

#include <ns3/packet.h>

#include <vanetza/common/byte_buffer.hpp>
#include <vanetza/net/cohesive_packet.hpp>

using namespace vcle::itsg5;

ns3::Ptr<ns3::Packet> vcle::itsg5::toNs3Packet(const vanetza::ChunkPacket& packet) {
	vanetza::ByteBuffer wire;
	wire.reserve(packet.size());

	// This effectively flattens the packet into the continuous array
	auto layers = vanetza::osi_layer_range(vanetza::OsiLayer::Network, vanetza::OsiLayer::Application);
	for (const auto layer : layers) {
		vanetza::ByteBuffer bytes;
		packet.layer(layer).convert(bytes);
		wire.insert(wire.end(), bytes.begin(), bytes.end());
	}

	return ns3::Create<ns3::Packet>(wire.data(), wire.size());
}

std::unique_ptr<vanetza::UpPacket> vcle::itsg5::toVanetzaPacket(const ns3::Packet& packet) {
	vanetza::ByteBuffer wire(packet.GetSize());
	packet.CopyData(wire.data(), wire.size());

	// Here we are lucky that we could use cohesive packet which is the
	// same continuos array.
	vanetza::CohesivePacket converted(std::move(wire), vanetza::OsiLayer::Network);
	return std::make_unique<vanetza::UpPacket>(converted);
}
