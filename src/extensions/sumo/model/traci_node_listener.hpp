#pragma once

#include <string_view>

#include <ns3/node.h>
#include <ns3/ptr.h>

namespace vcle::sumo {

	/**
	 * @brief Receives notifications when SUMO vehicles are associated with or removed from ns-3 nodes.
	 *
	 * Listeners are externally owned and must remain alive while registered with the node manager.
	 * Callback implementations must not modify the manager's listener list or vehicle associations.
	 *
	 * @note Vehicle identifiers are borrowed views. Copy an identifier to retain it beyond the callback.
	 */
	class ITraciNodeListener {
	public:
		virtual ~ITraciNodeListener() = default;

		/**
		 * @brief Handle a vehicle-node association after its node is activated and the association is registered.
		 * @param[in] vehicleId SUMO vehicle identifier, valid for the duration of the callback.
		 * @param[in] node Active ns-3 node associated with the vehicle.
		 */
		virtual void onVehicleAdded(std::string_view vehicleId, ns3::Ptr<ns3::Node> node) = 0;

		/**
		 * @brief Handle an association before it is erased and its node is deactivated and returned to the pool.
		 * @param[in] vehicleId SUMO vehicle identifier, valid for the duration of the callback.
		 * @param[in] node Still-active ns-3 node being released from the association.
		 */
		virtual void onVehicleRemoved(std::string_view vehicleId, ns3::Ptr<ns3::Node> node) = 0;
	};

} // namespace vcle::sumo
