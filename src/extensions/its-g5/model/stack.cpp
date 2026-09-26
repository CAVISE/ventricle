#include "src/extensions/its-g5/model/stack.hpp"

#include "src/extensions/its-g5/helpers/packet_conversion.hpp"
#include "src/extensions/its-g5/helpers/position_conversion.hpp"
#include "src/extensions/its-g5/helpers/type_glue.hpp"
#include "src/extensions/its-g5/model/runtime.hpp"

#include <utility>

#include <ns3/abort.h>
#include <ns3/mac48-address.h>

namespace vcle::itsg5 {
	Stack::Stack(
		StackConfig config,
		Runtime& runtime,
		ns3::Ptr<ns3::NetDevice> device,
		ns3::Ptr<ns3::MobilityModel> mobility,
		const IGeographicCoordinateSystem& coordinateSystem
	)
		: config_(std::move(config))
		, stack_{
			  .access_ = AccessAdapter(std::move(device)),
			  .router_ = vanetza::geonet::Router(runtime, config_.geoNetworking_),
			  .btp_ = vanetza::btp::PortDispatcher(),
		  }
		, mobility_(std::move(mobility))
		, runtime_(runtime)
		, coordinateSystem_(coordinateSystem) {
		NS_ABORT_MSG_UNLESS(mobility_, "ITS-G5 stack requires an ns-3 mobility model");

		stack_.router_.set_address(config_.geoNetworkingAddress_);
		stack_.router_.set_random_seed(config_.randomSeed_);
		stack_.router_.set_access_interface(&stack_.access_);
		stack_.router_.set_transport_handler(vanetza::geonet::UpperProtocol::BTP_B, &stack_.btp_);
		updatePosition();
	}

	void Stack::updatePosition() {
		stack_.router_.update_position(toPositionFix(*mobility_, coordinateSystem_, runtime_.now()));
	}

	void Stack::indicate(ns3::Ptr<const ns3::Packet> packet, const ns3::Address& source, const ns3::Address& destination) {
		if (packet == nullptr || !ns3::Mac48Address::IsMatchingType(source) || !ns3::Mac48Address::IsMatchingType(destination)) {
			return;
		}

		stack_.router_.indicate(
			toVanetzaPacket(*packet),
			TypeGlue::convert<vanetza::MacAddress>(ns3::Mac48Address::ConvertFrom(source)),
			TypeGlue::convert<vanetza::MacAddress>(ns3::Mac48Address::ConvertFrom(destination))
		);
	}

	vanetza::geonet::Router& Stack::router() {
		return stack_.router_;
	}

	const vanetza::geonet::Router& Stack::router() const {
		return stack_.router_;
	}

	vanetza::btp::PortDispatcher& Stack::btp() {
		return stack_.btp_;
	}

	const vanetza::btp::PortDispatcher& Stack::btp() const {
		return stack_.btp_;
	}

	ns3::Ptr<ns3::MobilityModel> Stack::mobility() {
		return mobility_;
	}

	ns3::Ptr<const ns3::MobilityModel> Stack::mobility() const {
		return mobility_;
	}
} // namespace vcle::itsg5
