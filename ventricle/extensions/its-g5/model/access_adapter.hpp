#pragma once

#include <ns3/net-device.h>
#include <vanetza/dcc/interface.hpp>

namespace vcle::itsg5 {
	class AccessAdapter final : public vanetza::dcc::RequestInterface {
	public:
		explicit AccessAdapter(ns3::Ptr<ns3::NetDevice> device);

		void request(const vanetza::dcc::DataRequest& request,
					 std::unique_ptr<vanetza::ChunkPacket> packet) override;

	private:
		ns3::Ptr<ns3::NetDevice> Device_;
	};
} // namespace vcle::itsg5
