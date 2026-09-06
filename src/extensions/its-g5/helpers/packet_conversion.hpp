#pragma once

#include <memory>

#include <ns3/packet.h>
#include <ns3/ptr.h>

#include <vanetza/net/packet.hpp>

namespace vcle::itsg5 {

	/**
	 * @brief Flattens a layered Vanetza packet into an ns-3 packet.
	 *
	 * Bytes from the network through application layers are appended in wire
	 * order so ns-3 can transmit them as one contiguous payload.
	 */
	ns3::Ptr<ns3::Packet> toNs3Packet(const vanetza::ChunkPacket& packet);

	/**
	 * @brief Wraps an ns-3 packet payload as a Vanetza incoming packet.
	 *
	 * The complete ns-3 payload becomes a cohesive packet beginning at the
	 * network layer, where Vanetza can parse the GeoNetworking stack.
	 */
	std::unique_ptr<vanetza::UpPacket> toVanetzaPacket(const ns3::Packet& packet);

} // namespace vcle::itsg5
