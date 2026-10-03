#include "src/extensions/sumo/model/mobility/traci_mobility_manager.hpp"

#include "src/core/common.hpp"
#include "src/extensions/sumo/model/mobility/traci_mobility_model.hpp"

#include <string>

#include <absl/log/absl_check.h>
#include <absl/log/absl_log.h>

#include <ns3/log.h>

using namespace vcle::sumo;

NS_LOG_COMPONENT_DEFINE("TraciMobilityManager");

TraciMobilityManager::TraciMobilityManager(TraciVehicleSubscriptionManager& subscriptions)
	: subscriptions_(subscriptions) {
	NS_LOG_FUNCTION(this << &subscriptions);
}

void TraciMobilityManager::onVehicleAdded(std::string_view vehicleId, ns3::Ptr<ns3::Node> node) {
	NS_LOG_FUNCTION(this << vehicleId << node);

	ABSL_CHECK(node) << "TraCI mobility manager requires a node";
	const auto mobility = node->GetObject<TraciMobilityModel>();
	if (!mobility) {
		ABSL_LOG(ERROR) << "TraCI node has no mobility model";
		return;
	}

	/* clang-format off */
	if (const auto status =
		subscriptions_.subscribeMultiple<
			libsumo::VAR_POSITION,
			libsumo::VAR_SPEED,
			libsumo::VAR_ANGLE
		>(
			ns3::PeekPointer(mobility),
			std::string(vehicleId)
		);
		!status.ok()
	) {
		ABSL_LOG(ERROR) << "Subscribing vehicle mobility: " << status.message();
	}
	/* clang-format on */
}

void TraciMobilityManager::onVehicleRemoved(std::string_view vehicleId VCLE_UNUSED, ns3::Ptr<ns3::Node> node) {
	NS_LOG_FUNCTION(this << vehicleId << node);

	ABSL_CHECK(node) << "TraCI mobility manager requires a node";
	const auto mobility = node->GetObject<TraciMobilityModel>();
	if (!mobility) {
		ABSL_LOG(ERROR) << "TraCI node has no mobility model";
		return;
	}

	if (const auto status = subscriptions_.unsubscribe(ns3::PeekPointer(mobility)); !status.ok()) {
		ABSL_LOG(ERROR) << "Unsubscribing vehicle mobility: " << status.message();
	}

	mobility->SetVelocity(vcle::zeroVector);
	mobility->SetPosition(vcle::zeroVector);
}
