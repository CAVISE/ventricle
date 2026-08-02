#pragma once

#include "src/extensions/its-g5/helpers/geographic_coordinate_system.hpp"
#include "src/extensions/its-g5/model/access_adapter.hpp"
#include "src/extensions/its-g5/model/runtime.hpp"
#include "src/extensions/its-g5/model/stack_config.hpp"

#include <ns3/mobility-model.h>
#include <ns3/net-device.h>
#include <ns3/packet.h>
#include <vanetza/btp/port_dispatcher.hpp>
#include <vanetza/geonet/router.hpp>

namespace vcle::itsg5 {

	class Stack {
	public:
		/**
		 * Construct an ITS-G5 stack attached to an ns-3 network device and mobility model.
		 */
		Stack(
			StackConfig config,
			Runtime& runtime,
			ns3::Ptr<ns3::NetDevice> device,
			ns3::Ptr<ns3::MobilityModel> mobility,
			const IGeographicCoordinateSystem& coordinateSystem
		);

		/**
		 * Pass an incoming link-layer packet to the GeoNetworking router.
		 */
		void Indicate(
			ns3::Ptr<const ns3::Packet> packet,
			const ns3::Address& source,
			const ns3::Address& destination
		);

		/** Update the GeoNetworking position vector from the ns-3 mobility model. */
		void UpdatePosition();

		/** Access the GeoNetworking router. */
		vanetza::geonet::Router& Router();
		/** Access the GeoNetworking router. */
		const vanetza::geonet::Router& Router() const;

		/** Access the BTP port dispatcher. */
		vanetza::btp::PortDispatcher& Btp();
		/** Access the BTP port dispatcher. */
		const vanetza::btp::PortDispatcher& Btp() const;

		/** Access the ns-3 mobility model. */
		ns3::Ptr<ns3::MobilityModel> Mobility();
		/** Access the ns-3 mobility model without permitting mutation. */
		ns3::Ptr<const ns3::MobilityModel> Mobility() const;

	private:
		StackConfig Config_;
		struct {
			AccessAdapter Access;
			vanetza::geonet::Router Router;
			vanetza::btp::PortDispatcher Btp;
		} Stack_;
		ns3::Ptr<ns3::MobilityModel> Mobility_;
		Runtime& Runtime_;
		const IGeographicCoordinateSystem& CoordinateSystem_;
	};
} // namespace vcle::itsg5
