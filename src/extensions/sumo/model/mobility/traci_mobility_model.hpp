#pragma once

/** @file
 * @brief Defines an ns-3 mobility model driven by TraCI vehicle results.
 */

#include "src/extensions/sumo/model/traci_subscriber.hpp"

#include <ns3/constant-velocity-mobility-model.h>

namespace vcle::sumo {

	/**
	 * @brief Updates a node's position and velocity from TraCI vehicle subscription results.
	 *
	 * Each update requires VAR_POSITION, VAR_SPEED, and VAR_ANGLE. Position coordinates are
	 * copied directly into ns-3; speed and heading determine horizontal velocity, with zero
	 * vertical velocity. A heading of zero points along positive Y, and 90 degrees along positive X.
	 * The inherited constant-velocity model extrapolates movement between updates.
	 *
	 * @note Aggregate this model onto a node before TraciMobilityManager registers its vehicle.
	 * Results are applied at the current ns-3 time; the callback timestamp is not used to
	 * compensate for delayed updates. Missing or incorrectly typed required values are contract errors.
	 */
	class TraciMobilityModel final : public ns3::ConstantVelocityMobilityModel, public ITraciSubscriber {
	public:
		/**
		 * @brief Describe the model's ns-3 type and parent mobility model.
		 * @return Registered TraciMobilityModel type identifier.
		 */
		static ns3::TypeId GetTypeId();

		/* ns3::MobilityModel implementation*/
		ns3::Ptr<ns3::MobilityModel> Copy() const override;

		/* ITraciSubscriber implementation*/
		void onTraciValues(std::string_view objectId, TraciValues values, ns3::Time time) override;
	};

} // namespace vcle::sumo
