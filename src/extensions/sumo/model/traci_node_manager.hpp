#pragma once

#include "src/core/simulation/nodes.hpp"

#include <string>
#include <vector>

#include <absl/container/flat_hash_map.h>
#include <absl/status/status.h>
#include <absl/status/statusor.h>

#include <ns3/ptr.h>

namespace vcle::sumo {
	class ITraciNodeListener;

	/** Extends the generic recyclable node pool with SUMO vehicle identity. */
	class TraciNodeManager final : public DynamicNodeManager {
	public:
		/** Maps SUMO vehicle identifiers to their active ns-3 nodes. */
		using VehicleMap = absl::flat_hash_map<std::string, ns3::Ptr<ns3::Node>>;

		using DynamicNodeManager::DynamicNodeManager;

		/* DynamicNodeManager implementation*/
		void removeNode(ns3::Ptr<ns3::Node> node) override;

		/** Activate a pooled node and associate it with @p vehicleId. */
		absl::StatusOr<ns3::Ptr<ns3::Node>> addVehicle(const std::string& vehicleId);
		/** Remove the vehicle association and return its node to the pool. */
		absl::Status removeVehicle(const std::string& vehicleId);
		/** Find the active node associated with @p vehicleId. */
		VCLE_NODISCARD ns3::Ptr<ns3::Node> findVehicle(const std::string& vehicleId) const;
		/** Access all active vehicle-to-node associations. */
		VCLE_NODISCARD const VehicleMap& vehicles() const;
		/** Register an externally owned vehicle-node lifecycle listener. */
		void addListener(ITraciNodeListener& listener);
		/** Stop notifying a previously registered vehicle-node lifecycle listener. */
		void removeListener(ITraciNodeListener& listener);

	private:
		VehicleMap vehicles_;
		std::vector<ITraciNodeListener*> listeners_;
	};

} // namespace vcle::sumo
