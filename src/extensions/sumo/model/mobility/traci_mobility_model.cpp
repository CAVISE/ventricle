#include "src/extensions/sumo/model/mobility/traci_mobility_model.hpp"

#include <cmath>
#include <numbers>

#include <ns3/log.h>

using namespace vcle::sumo;

NS_LOG_COMPONENT_DEFINE("TraciMobilityModel");
NS_OBJECT_ENSURE_REGISTERED(TraciMobilityModel);

ns3::TypeId TraciMobilityModel::GetTypeId() {
	NS_LOG_FUNCTION_NOARGS();

	/* clang-format off */
	static ns3::TypeId typeId =
		ns3::TypeId("vcle::sumo::TraciMobilityModel")
			.SetParent<ns3::ConstantVelocityMobilityModel>()
			.SetGroupName("Ventricle");
	/* clang-format on */
	return typeId;
}

ns3::Ptr<ns3::MobilityModel> TraciMobilityModel::Copy() const {
	NS_LOG_FUNCTION(this);

	auto copy = ns3::CreateObject<TraciMobilityModel>();
	copy->SetPosition(GetPosition());
	copy->SetVelocity(GetVelocity());
	return copy;
}

void TraciMobilityModel::onTraciValues(std::string_view objectId VCLE_UNUSED, TraciValues values, ns3::Time time VCLE_UNUSED) {
	NS_LOG_FUNCTION(this << objectId << time);

	/* clang-format off */
	const auto [position, speed, heading] =
		values.getMultiple<
			libsumo::VAR_POSITION,
			libsumo::VAR_SPEED,
			libsumo::VAR_ANGLE
		>();
	/* clang-format on */
	const auto radians = heading * std::numbers::pi / 180.0;

	SetPosition({position.x, position.y, position.z});
	SetVelocity({speed * std::sin(radians), speed * std::cos(radians), 0.0});
}
