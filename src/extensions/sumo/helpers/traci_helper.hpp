#pragma once

#include "src/core/defs.hpp"
#include "src/extensions/sumo/model/controller.hpp"
#include "src/extensions/sumo/model/traci_port.hpp"

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <absl/status/statusor.h>

#include <ns3/attribute.h>
#include <ns3/ptr.h>

namespace vcle::sumo {
	class ITraciListener;
	class TraciNodeManager;

	/** Owns the SUMO connection and its ns-3 synchronization controller. */
	struct Installation {
		/** Owned TraCI connection. */
		std::unique_ptr<TraciPort> port_;
		/** Controller installed in the ns-3 simulation. */
		ns3::Ptr<Controller> controller_;
	};

	/** Configures and installs one SUMO connection into a simulation. */
	class TraciHelper {
	public:
		/** Construct a helper using the controller's default attributes. */
		TraciHelper() = default;

		/** Set the command used to launch SUMO. */
		void setCommand(std::vector<std::string> command);
		/** Set an attribute on controllers subsequently created by this helper. */
		void setAttribute(const std::string& name, const ns3::AttributeValue& value);
		/** Register a listener with every controller installed by this helper. */
		void addListener(ITraciListener& listener);

		/** Construct and start the SUMO integration. */
		absl::StatusOr<Installation> install(TraciNodeManager& nodeManager) const;

	private:
		std::map<std::string, ns3::Ptr<ns3::AttributeValue>> controllerAttributes_;
		std::vector<ITraciListener*> listeners_;
	};

} // namespace vcle::sumo
