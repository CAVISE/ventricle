#pragma once

#include "src/extensions/sumo/model/traci_node_listener.hpp"
#include "src/extensions/sumo/model/traci_subscription_manager.hpp"

namespace vcle::sumo {

	/** Manages TraCI mobility subscriptions for vehicle-associated nodes. */
	class TraciMobilityManager final : public ITraciNodeListener {
	public:
		/** Bind mobility updates to @p subscriptions. */
		explicit TraciMobilityManager(TraciVehicleSubscriptionManager& subscriptions);

		/* ITraciNodeListener implementation*/
		void onVehicleAdded(std::string_view vehicleId, ns3::Ptr<ns3::Node> node) override;
		void onVehicleRemoved(std::string_view vehicleId, ns3::Ptr<ns3::Node> node) override;

	private:
		TraciVehicleSubscriptionManager& subscriptions_;
	};

} // namespace vcle::sumo
