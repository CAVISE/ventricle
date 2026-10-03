#pragma once

/** @file
 * @brief Connects vehicle-node lifecycle events to TraCI mobility subscriptions.
 */

#include "src/extensions/sumo/model/traci_node_listener.hpp"
#include "src/extensions/sumo/model/traci_subscription_manager.hpp"

namespace vcle::sumo {

	/**
	 * @brief Manages position, speed, and heading subscriptions for vehicle-associated nodes.
	 *
	 * Register this manager with TraciNodeManager to subscribe each newly associated node's
	 * TraciMobilityModel. Removing an association unregisters the model and resets its position
	 * and velocity to zero before the node is recycled. Missing models and subscription errors
	 * are logged.
	 *
	 * @note Nodes must have a TraciMobilityModel aggregated before activation. The subscription
	 * manager is externally owned and must outlive this manager; mobility models must remain
	 * alive while subscribed.
	 */
	class TraciMobilityManager final : public ITraciNodeListener {
	public:
		/**
		 * @brief Bind vehicle mobility updates to an existing subscription manager.
		 * @param[in] subscriptions Externally owned manager used to register mobility models.
		 */
		explicit TraciMobilityManager(TraciVehicleSubscriptionManager& subscriptions);

		/* ITraciNodeListener implementation*/
		void onVehicleAdded(std::string_view vehicleId, ns3::Ptr<ns3::Node> node) override;
		void onVehicleRemoved(std::string_view vehicleId, ns3::Ptr<ns3::Node> node) override;

	private:
		TraciVehicleSubscriptionManager& subscriptions_; ///< Borrowed vehicle-domain subscription manager.
	};

} // namespace vcle::sumo
