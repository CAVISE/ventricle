#include "src/extensions/sumo/model/traci_node_manager.hpp"
#include "src/extensions/sumo/model/traci_node_listener.hpp"

#include <absl/status/status.h>

using namespace vcle::sumo;

absl::StatusOr<ns3::Ptr<ns3::Node>> TraciNodeManager::addVehicle(const std::string& vehicleId) {
	if (auto [iterator, inserted] = vehicles_.emplace(vehicleId, nullptr); !inserted) {
		auto error = absl::AlreadyExistsError("vehicle id is already on the list");
		error.SetPayload("vehicleId", absl::Cord(vehicleId));
		return error;
	} else if (auto node = DynamicNodeManager::addNode(); node) {
		iterator->second = node;
		invokeListeners(&ITraciNodeListener::onVehicleAdded, std::string_view(vehicleId), node);
		return node;
	} else {
		vehicles_.erase(iterator);
	}

	return absl::ResourceExhaustedError("SUMO node pool is exhausted");
}

absl::Status TraciNodeManager::removeVehicle(const std::string& vehicleId) {
	const auto iterator = vehicles_.find(vehicleId);
	if (iterator == vehicles_.end()) {
		auto error = absl::NotFoundError("SUMO vehicle is not active");
		error.SetPayload("vehicleId", absl::Cord(vehicleId));
		return error;
	}

	const auto node = iterator->second;
	invokeListeners(&ITraciNodeListener::onVehicleRemoved, std::string_view(vehicleId), node);

	DynamicNodeManager::removeNode(node);
	vehicles_.erase(iterator);

	return absl::OkStatus();
}

ns3::Ptr<ns3::Node> TraciNodeManager::findVehicle(const std::string& vehicleId) const {
	if (const auto iterator = vehicles_.find(vehicleId); iterator != vehicles_.end()) {
		return iterator->second;
	}

	return nullptr;
}

const TraciNodeManager::VehicleMap& TraciNodeManager::vehicles() const {
	return vehicles_;
}
