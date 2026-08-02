#include "src/extensions/its-g5/model/stack.hpp"

#include "src/extensions/its-g5/helpers/packet_conversion.hpp"
#include "src/extensions/its-g5/helpers/position_conversion.hpp"
#include "src/extensions/its-g5/helpers/type_glue.hpp"
#include "src/extensions/its-g5/model/runtime.hpp"

#include <ns3/abort.h>
#include <ns3/mac48-address.h>
#include <utility>

namespace vcle::itsg5 {
	Stack::Stack(
		StackConfig config,
		Runtime& runtime,
		ns3::Ptr<ns3::NetDevice> device,
		ns3::Ptr<ns3::MobilityModel> mobility,
		const IGeographicCoordinateSystem& coordinateSystem
	)
		: Config_(std::move(config))
		, Stack_{
			  .Access = AccessAdapter(std::move(device)),
			  .Router = vanetza::geonet::Router(runtime, Config_.GeoNetworking),
			  .Btp = vanetza::btp::PortDispatcher(),
		  }
		, Mobility_(std::move(mobility))
		, Runtime_(runtime)
		, CoordinateSystem_(coordinateSystem) {
		NS_ABORT_MSG_UNLESS(Mobility_, "ITS-G5 stack requires an ns-3 mobility model");

		Stack_.Router.set_address(Config_.GeoNetworkingAddress);
		Stack_.Router.set_random_seed(Config_.RandomSeed);
		Stack_.Router.set_access_interface(&Stack_.Access);
		Stack_.Router.set_transport_handler(vanetza::geonet::UpperProtocol::BTP_B, &Stack_.Btp);
		UpdatePosition();
	}

	void Stack::UpdatePosition() {
		Stack_.Router.update_position(ToPositionFix(*Mobility_, CoordinateSystem_, Runtime_.now()));
	}

	void Stack::Indicate(
		ns3::Ptr<const ns3::Packet> packet,
		const ns3::Address& source,
		const ns3::Address& destination
	) {
		if (packet == nullptr || !ns3::Mac48Address::IsMatchingType(source) ||
			!ns3::Mac48Address::IsMatchingType(destination)) {
			return;
		}

		Stack_.Router.indicate(
			ToVanetzaPacket(*packet),
			TypeGlue::convert<vanetza::MacAddress>(ns3::Mac48Address::ConvertFrom(source)),
			TypeGlue::convert<vanetza::MacAddress>(ns3::Mac48Address::ConvertFrom(destination))
		);
	}

	vanetza::geonet::Router& Stack::Router() {
		return Stack_.Router;
	}

	const vanetza::geonet::Router& Stack::Router() const {
		return Stack_.Router;
	}

	vanetza::btp::PortDispatcher& Stack::Btp() {
		return Stack_.Btp;
	}

	const vanetza::btp::PortDispatcher& Stack::Btp() const {
		return Stack_.Btp;
	}

	ns3::Ptr<ns3::MobilityModel> Stack::Mobility() {
		return Mobility_;
	}

	ns3::Ptr<const ns3::MobilityModel> Stack::Mobility() const {
		return Mobility_;
	}
} // namespace vcle::itsg5
