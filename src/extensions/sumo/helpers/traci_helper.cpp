#include "src/extensions/sumo/helpers/traci_helper.hpp"

#include "src/extensions/sumo/model/controller.hpp"
#include "src/extensions/sumo/model/traci_listener.hpp"
#include "src/extensions/sumo/model/traci_port.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>

#include <ns3/string.h>

namespace vcle::sumo {
	TraciHelper::TraciHelper() {
		controllerFactory_.SetTypeId(Controller::GetTypeId());
	}

	void TraciHelper::setCommand(std::vector<std::string> command) {
		std::ostringstream commandLine;
		for (auto argument = command.begin(); argument != command.end(); ++argument) {
			if (argument != command.begin()) {
				commandLine << ' ';
			}
			commandLine << std::quoted(*argument);
		}
		controllerFactory_.Set("Command", ns3::StringValue(commandLine.str()));
	}

	void TraciHelper::setAttribute(const std::string& name, const ns3::AttributeValue& value) {
		controllerFactory_.Set(name, value);
	}

	void TraciHelper::addListener(ITraciListener& listener) {
		if (std::ranges::find(listeners_, &listener) == listeners_.end()) {
			listeners_.push_back(&listener);
		}
	}

	absl::StatusOr<Installation> TraciHelper::install(TraciNodeManager& nodeManager) const {
		Installation installation;
		installation.port_ = std::make_unique<TraciPort>();
		installation.controller_ = controllerFactory_.Create<Controller>();
		if (const auto status = installation.controller_->configure(*installation.port_, nodeManager); !status.ok()) {
			return status;
		}
		for (auto* listener : listeners_) {
			installation.controller_->addListener(listener);
		}
		if (const auto status = installation.controller_->start(); !status.ok()) {
			return status;
		}
		return installation;
	}

} // namespace vcle::sumo
