#include "src/extensions/its-g5/model/access_adapter.hpp"

#include "src/extensions/its-g5/helpers/packet_conversion.hpp"
#include "src/extensions/its-g5/helpers/type_glue.hpp"

#include <ns3/abort.h>

#include <vanetza/dcc/data_request.hpp>

namespace vcle::itsg5 {
	AccessAdapter::AccessAdapter(ns3::Ptr<ns3::NetDevice> device)
		: device_(std::move(device)) {
		NS_ABORT_MSG_UNLESS(device_, "ITS-G5 access adapter requires a network device");
	}

	void AccessAdapter::request(const vanetza::dcc::DataRequest& request, std::unique_ptr<vanetza::ChunkPacket> packet) {
		if (packet == nullptr) {
			return;
		}

		device_->Send(toNs3Packet(*packet), TypeGlue::convert<ns3::Mac48Address>(request.destination), request.ether_type.host());
	}
} // namespace vcle::itsg5
