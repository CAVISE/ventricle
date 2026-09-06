#pragma once

#include "src/extensions/sumo/model/traci_values.hpp"

#include <string_view>

#include <ns3/nstime.h>

namespace vcle::sumo {

	/**
	 * @brief Consumes results delivered by a TraCI subscription manager.
	 *
	 * Subscribers are externally owned and must remain alive while their subscriptions are active.
	 * Notifications provide a borrowed view of the results for a subscribed domain object.
	 */
	class ITraciSubscriber {
	public:
		/** @brief Destroy the subscriber without cancelling its subscriptions. */
		virtual ~ITraciSubscriber() = default;

		/**
		 * @brief Handle subscription results delivered during the current TraCI step.
		 * @param[in] objectId Subscribed domain object identifier, borrowed for this callback.
		 * @param[in] values Non-owning typed view of the object's latest subscription results.
		 * @param[in] time Current ns-3 simulation time associated with the results.
		 * @note Copy the identifier and any needed values to retain them beyond this callback.
		 * Copying TraciValues itself does not extend the lifetime of its underlying results;
		 * pointers and references obtained from the view must not be retained without copying their contents.
		 */
		virtual void onTraciValues(std::string_view objectId, TraciValues values, ns3::Time time) = 0;
	};

} // namespace vcle::sumo
