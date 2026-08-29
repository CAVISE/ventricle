#pragma once

#include "src/extensions/sumo/model/traci_subscriber.hpp"

#include <ns3/constant-velocity-mobility-model.h>

namespace vcle::sumo {

	/** Represents one ns-3 node's mobility using TraCI vehicle data. */
	class TraciMobilityModel final : public ns3::ConstantVelocityMobilityModel, public ITraciSubscriber {
	public:
		static ns3::TypeId GetTypeId();

		/* ns3::MobilityModel implementation*/
		ns3::Ptr<ns3::MobilityModel> Copy() const override;

		/* ITraciSubscriber implementation*/
		void onTraciValues(std::string_view objectId, TraciValues values, ns3::Time time) override;
	};

} // namespace vcle::sumo
