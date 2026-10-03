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
		void indicate(ns3::Ptr<const ns3::Packet> packet, const ns3::Address& source, const ns3::Address& destination);

		/** Update the GeoNetworking position vector from the ns-3 mobility model. */
		void updatePosition();

		/** Access the GeoNetworking router. */
		vanetza::geonet::Router& router();
		/** Access the GeoNetworking router. */
		const vanetza::geonet::Router& router() const;

		/** Access the BTP port dispatcher. */
		vanetza::btp::PortDispatcher& btp();
		/** Access the BTP port dispatcher. */
		const vanetza::btp::PortDispatcher& btp() const;

		/** Access the ns-3 mobility model. */
		ns3::Ptr<ns3::MobilityModel> mobility();
		/** Access the ns-3 mobility model without permitting mutation. */
		ns3::Ptr<const ns3::MobilityModel> mobility() const;

	private:
		StackConfig config_;
		struct {
			AccessAdapter access_;
			vanetza::geonet::Router router_;
			vanetza::btp::PortDispatcher btp_;
		} stack_;
		ns3::Ptr<ns3::MobilityModel> mobility_;
		Runtime& runtime_;
		const IGeographicCoordinateSystem& coordinateSystem_;
	};
} // namespace vcle::itsg5
