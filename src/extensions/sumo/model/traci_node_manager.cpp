#include "src/extensions/sumo/model/traci_node_manager.hpp"

#include "src/extensions/sumo/model/traci_node_listener.hpp"

#include <algorithm>

#include <ns3/abort.h>

using namespace vcle::sumo;

void TraciNodeManager::removeNode(ns3::Ptr<ns3::Node> node) {
	for (auto iterator = vehicles_.begin(); iterator != vehicles_.end(); ++iterator) {
		if (iterator->second == node) {
			for (auto* listener : listeners_) {
				listener->onVehicleRemoved(iterator->first, node);
			}

			vehicles_.erase(iterator);
			break;
		}
	}

	DynamicNodeManager::removeNode(node);
}

absl::StatusOr<ns3::Ptr<ns3::Node>> TraciNodeManager::addVehicle(const std::string& vehicleId) {
	if (vehicles_.contains(vehicleId)) {
		return absl::AlreadyExistsError("SUMO vehicle is already active");
	}

	if (auto node = DynamicNodeManager::addNode(); node) {
		vehicles_.emplace(vehicleId, node);
		for (auto* listener : listeners_) {
			listener->onVehicleAdded(vehicleId, node);
		}
		return node;
	} else {
		return absl::ResourceExhaustedError("SUMO node pool is exhausted");
	}
}

absl::Status TraciNodeManager::removeVehicle(const std::string& vehicleId) {
	const auto iterator = vehicles_.find(vehicleId);
	if (iterator == vehicles_.end()) {
		return absl::NotFoundError("SUMO vehicle is not active");
	}

	const auto node = iterator->second;
	for (auto* listener : listeners_) {
		listener->onVehicleRemoved(vehicleId, node);
	}

	vehicles_.erase(iterator);
	DynamicNodeManager::removeNode(node);
	return absl::OkStatus();
}

ns3::Ptr<ns3::Node> TraciNodeManager::findVehicle(const std::string& vehicleId) const {
	const auto iterator = vehicles_.find(vehicleId);
	return iterator == vehicles_.end() ? nullptr : iterator->second;
}

const TraciNodeManager::VehicleMap& TraciNodeManager::vehicles() const {
	return vehicles_;
}

void TraciNodeManager::addListener(ITraciNodeListener& listener) {
	if (std::ranges::find(listeners_, &listener) == listeners_.end()) {
		listeners_.push_back(&listener);
	}
}

void TraciNodeManager::removeListener(ITraciNodeListener& listener) {
	const auto iterator = std::ranges::find(listeners_, &listener);
	if (iterator != listeners_.end()) {
		listeners_.erase(iterator);
	}
}
