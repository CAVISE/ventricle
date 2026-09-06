#include "src/extensions/sumo/helpers/traci_helper.hpp"

#include "src/extensions/sumo/model/controller.hpp"
#include "src/extensions/sumo/model/traci_listener.hpp"
#include "src/extensions/sumo/model/traci_port.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>

#include <absl/log/absl_check.h>

#include <ns3/string.h>

namespace vcle::sumo {
	void TraciHelper::setCommand(std::vector<std::string> command) {
		std::ostringstream commandLine;
		for (auto argument = command.begin(); argument != command.end(); ++argument) {
			if (argument != command.begin()) {
				commandLine << ' ';
			}
			commandLine << std::quoted(*argument);
		}
		setAttribute("Command", ns3::StringValue(commandLine.str()));
	}

	void TraciHelper::setAttribute(const std::string& name, const ns3::AttributeValue& value) {
		if (name.empty()) {
			return;
		}
		ns3::TypeId::AttributeInformation info;
		ABSL_CHECK(Controller::GetTypeId().LookupAttributeByName(name, &info)) << "Unknown controller attribute: " << name;
		auto checkedValue = info.checker->CreateValidValue(value);
		ABSL_CHECK(checkedValue) << "Invalid controller attribute value: " << name;
		controllerAttributes_[name] = checkedValue;
	}

	void TraciHelper::addListener(ITraciListener& listener) {
		if (std::ranges::find(listeners_, &listener) == listeners_.end()) {
			listeners_.push_back(&listener);
		}
	}

	absl::StatusOr<Installation> TraciHelper::install(TraciNodeManager& nodeManager) const {
		Installation installation;
		installation.port_ = std::make_unique<TraciPort>();
		installation.controller_ = ns3::CreateObject<Controller>(*installation.port_, nodeManager);
		for (const auto& [name, value] : controllerAttributes_) {
			installation.controller_->SetAttribute(name, *value);
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
