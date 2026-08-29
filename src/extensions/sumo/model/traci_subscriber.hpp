#pragma once

#include "src/extensions/sumo/model/traci_values.hpp"

#include <string_view>

#include <ns3/nstime.h>

namespace vcle::sumo {

	/** Consumes the values produced for one TraCI variable subscription. */
	class ITraciSubscriber {
	public:
		virtual ~ITraciSubscriber() = default;

		/** Handle values returned for @p objectId during the current TraCI step. */
		virtual void onTraciValues(std::string_view objectId, TraciValues values, ns3::Time time) = 0;
	};

} // namespace vcle::sumo
