#pragma once

#include <string_view>

#include <ns3/node.h>
#include <ns3/ptr.h>

namespace vcle::sumo {

	/** Receives changes to SUMO vehicle and ns-3 node associations. */
	class ITraciNodeListener {
	public:
		virtual ~ITraciNodeListener() = default;

		/** Handle a newly active vehicle-node association. */
		virtual void onVehicleAdded(std::string_view vehicleId, ns3::Ptr<ns3::Node> node) = 0;
		/** Handle an association immediately before its node is deactivated. */
		virtual void onVehicleRemoved(std::string_view vehicleId, ns3::Ptr<ns3::Node> node) = 0;
	};

} // namespace vcle::sumo
