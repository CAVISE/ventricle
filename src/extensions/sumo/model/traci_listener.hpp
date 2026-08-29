#pragma once

#include "src/extensions/sumo/model/traci_port.hpp"

#include <absl/status/status.h>

#include <ns3/nstime.h>

namespace vcle::sumo {

	/** Receives lifecycle events while the TraCI connection is usable. */
	class ITraciListener {
	public:
		virtual ~ITraciListener() = default;

		/** Invoked after the TraCI connection has started; an error removes this listener. */
		virtual absl::Status onTraciStart(TraciPort* port) = 0;
		/** Invoked after one SUMO step; an error removes this listener from subsequent updates. */
		virtual absl::Status onTraciStep(TraciPort* port, ns3::Time time) = 0;
		/** Invoked immediately before the TraCI connection is closed. */
		virtual void onTraciEnd(TraciPort* port) = 0;
	};

} // namespace vcle::sumo
