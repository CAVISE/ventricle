#pragma once

#include "src/extensions/sumo/model/traci_port.hpp"

#include <absl/status/status.h>

#include <ns3/nstime.h>

namespace vcle::sumo {

	/**
	 * @brief Receives lifecycle notifications from a TraCI controller.
	 *
	 * Listeners are externally owned and must remain alive while registered. A non-OK
	 * callback status removes the listener from all subsequent notifications, including
	 * onTraciEnd(). Callback implementations must not modify the controller's listener list.
	 */
	class ITraciListener {
	public:
		virtual ~ITraciListener() = default;

		/**
		 * @brief Handle successful startup of the TraCI connection.
		 * @param[in] port Non-owning pointer to the controller's TraCI connection.
		 * @return OK on success; a non-OK status unregisters this listener.
		 */
		virtual absl::Status onTraciStart(TraciPort* port) = 0;

		/**
		 * @brief Handle a completed SUMO step.
		 * @param[in] port Non-owning pointer to the controller's TraCI connection.
		 * @param[in] time Current ns-3 simulation time to which SUMO has been advanced.
		 * @return OK on success; a non-OK status unregisters this listener.
		 */
		virtual absl::Status onTraciStep(TraciPort* port, ns3::Time time) = 0;

		/**
		 * @brief Handle controller shutdown before the TraCI connection is closed.
		 * @param[in] port Non-owning pointer to the controller's TraCI connection.
		 */
		virtual void onTraciEnd(TraciPort* port) = 0;
	};

} // namespace vcle::sumo
