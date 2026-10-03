#pragma once

#include <ns3/net-device.h>

#include <vanetza/dcc/interface.hpp>

namespace vcle::itsg5 {
	/** Adapts Vanetza access requests to an ns-3 network device. */
	class AccessAdapter final : public vanetza::dcc::RequestInterface {
	public:
		/** Bind the adapter to @p device. */
		explicit AccessAdapter(ns3::Ptr<ns3::NetDevice> device);

		/* vanetza::dcc::RequestInterface implementation*/
		void request(const vanetza::dcc::DataRequest& request, std::unique_ptr<vanetza::ChunkPacket> packet) override;

	private:
		ns3::Ptr<ns3::NetDevice> device_;
	};
} // namespace vcle::itsg5
