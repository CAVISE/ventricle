#pragma once

#include "src/core/simulation/listeners.hpp"
#include "src/core/simulation/nodes.hpp"
#include "src/extensions/sumo/model/traci_node_listener.hpp"

#include <string>

#include <absl/container/flat_hash_map.h>
#include <absl/status/status.h>
#include <absl/status/statusor.h>

#include <ns3/ptr.h>

namespace vcle::sumo {

	/**
	 * @brief Extends the generic recyclable node pool with SUMO vehicle identity.
	 * @note Use addVehicle() and removeVehicle() for SUMO vehicles. The inherited addNode()
	 * and removeNode() operations do not update vehicle associations or notify vehicle listeners.
	 */
	class TraciNodeManager
		: public DynamicNodeManager
		, public PublisherBase<ITraciNodeListener> {
	public:
		/** Maps SUMO vehicle identifiers to their active ns-3 nodes. */
		using VehicleMap = absl::flat_hash_map<std::string, ns3::Ptr<ns3::Node>>;

		using DynamicNodeManager::DynamicNodeManager;

		/** Activate a pooled node and associate it with @p vehicleId. */
		absl::StatusOr<ns3::Ptr<ns3::Node>> addVehicle(const std::string& vehicleId);
		/** Remove the vehicle association and return its node to the pool. */
		absl::Status removeVehicle(const std::string& vehicleId);
		/** Find the active node associated with @p vehicleId. */
		VCLE_NODISCARD ns3::Ptr<ns3::Node> findVehicle(const std::string& vehicleId) const;
		/** Access all active vehicle-to-node associations. */
		VCLE_NODISCARD const VehicleMap& vehicles() const;

	private:
		VehicleMap vehicles_;
	};

} // namespace vcle::sumo
