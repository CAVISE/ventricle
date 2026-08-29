#include "src/extensions/sumo/model/controller.hpp"

#include "src/extensions/sumo/model/traci_listener.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <utility>

#include <absl/log/absl_check.h>
#include <absl/log/absl_log.h>
#include <absl/status/status_builder.h>

#include <ns3/boolean.h>
#include <ns3/integer.h>
#include <ns3/simulator.h>
#include <ns3/string.h>

using namespace vcle::sumo;

NS_OBJECT_ENSURE_REGISTERED(Controller);

ns3::TypeId Controller::GetTypeId() {
	/* clang-format off */
	static ns3::TypeId typeId =
		ns3::TypeId("vcle::sumo::Controller")
			.SetParent<ns3::Object>()
			.SetGroupName("Ventricle")
			.AddConstructor<Controller>()
			.AddAttribute(
				"StepInterval",
				"Interval between synchronized TraCI steps.",
				ns3::TimeValue(ns3::MilliSeconds(100)),
				ns3::MakeTimeAccessor(&Controller::stepInterval_),
				ns3::MakeTimeChecker(ns3::Time(1))
			)
			.AddAttribute(
				"Command",
				"Quoted command line used to launch SUMO.",
				ns3::StringValue(),
				ns3::MakeStringAccessor(&Controller::setCommand, &Controller::getCommand),
				ns3::MakeStringChecker()
			)
			.AddAttribute(
				"Port",
				"TCP port passed to libtraci; -1 selects an available port.",
				ns3::IntegerValue(-1),
				ns3::MakeIntegerAccessor(&Controller::setLaunchPort, &Controller::getLaunchPort),
				ns3::MakeIntegerChecker<int>(-1, 65535)
			)
			.AddAttribute(
				"Retries",
				"Number of connection attempts made by libtraci.",
				ns3::IntegerValue(libsumo::DEFAULT_NUM_RETRIES),
				ns3::MakeIntegerAccessor(&Controller::setRetries, &Controller::getRetries),
				ns3::MakeIntegerChecker<int>(0)
			)
			.AddAttribute(
				"Verbose",
				"Enable verbose libtraci connection output.",
				ns3::BooleanValue(false),
				ns3::MakeBooleanAccessor(&Controller::setVerbose, &Controller::getVerbose),
				ns3::MakeBooleanChecker()
			)
			.AddAttribute(
				"TraceFile",
				"File to which libtraci writes command traces; empty disables tracing.",
				ns3::StringValue(),
				ns3::MakeStringAccessor(&Controller::setTraceFile, &Controller::getTraceFile),
				ns3::MakeStringChecker()
			)
			.AddAttribute(
				"TraceGetters",
				"Include getter calls in the libtraci trace.",
				ns3::BooleanValue(true),
				ns3::MakeBooleanAccessor(&Controller::setTraceGetters, &Controller::getTraceGetters),
				ns3::MakeBooleanChecker()
			);
	/* clang-format on */
	return typeId;
}

Controller::~Controller() {
	if (const auto status = stop(); !status.ok()) {
		ABSL_LOG(ERROR) << "Failed to stop TraCI controller: " << status;
	}
}

absl::Status Controller::configure(TraciPort& port, TraciNodeManager& nodeManager) {
	if (port_) {
		return absl::FailedPreconditionError("TraCI controller already has a port configured");
	} else if (nodeManager_) {
		return absl::FailedPreconditionError("TraCI controller already has a node manager configured");
	}

	port_ = &port;
	nodeManager_ = &nodeManager;
	return absl::OkStatus();
}

absl::Status Controller::start() {
	if (!(port_ && nodeManager_)) {
		return absl::FailedPreconditionError("TraCI controller is not configured");
	}

	// This reads CLI args from a string, preserving quoted stuff.
	std::istringstream stream(config_.command_);
	std::vector<std::string> command;
	for (std::string argument; stream >> std::quoted(argument); /* noop */) {
		command.push_back(std::move(argument));
	}

	if (command.empty()) {
		return absl::InvalidArgumentError("SUMO command is empty");
	}

	/* clang-format off */
	if (const auto status = port_->start(
		command,
		config_.port_,
		config_.retries_,
		config_.verbose_,
		config_.traceFile_,
		config_.traceGetters_
	); !status.ok()) {
		return absl::StatusBuilder(status).SetPrepend() << "starting TraCI controller: ";
	}
	/* clang-format on */

	for (auto listener = listeners_.begin(); listener != listeners_.end(); /* noop */) {
		if (const auto status = (*listener)->onTraciStart(port_); !status.ok()) {
			ABSL_LOG(WARNING) << "TraCI listener failed to start and was removed: " << status;
			listener = listeners_.erase(listener);
		} else {
			++listener;
		}
	}

	stepEvent_ = ns3::Simulator::Schedule(stepInterval_, &Controller::step, this);
	return absl::OkStatus();
}

absl::Status Controller::stop() {
	if (port_ == nullptr) {
		return absl::OkStatus();
	}

	if (stepEvent_.IsPending()) {
		ns3::Simulator::Cancel(stepEvent_);
	}

	for (auto* listener : listeners_) {
		listener->onTraciEnd(port_);
	}

	return absl::StatusBuilder(port_->close()).SetPrepend() << "closing TraCI port: ";
}

void Controller::DoDispose() {
	if (const auto status = stop(); !status.ok()) {
		ABSL_LOG(WARNING) << "Stop TraCI controller: " << status;
	}

	port_ = nullptr;
	nodeManager_ = nullptr;
	ns3::Object::DoDispose();
}

void Controller::addListener(ITraciListener* listener) {
	ABSL_CHECK(listener) << "Null listener";
	ABSL_CHECK(std::ranges::find(listeners_, listener) == listeners_.end()) << "Listener is already registered";

	listeners_.push_back(listener);
}

void Controller::removeListener(ITraciListener* listener) {
	ABSL_CHECK(listener) << "Null listener";

	const auto iterator = std::ranges::find(listeners_, listener);
	if (iterator != listeners_.end()) {
		listeners_.erase(iterator);
	}
}

void Controller::step() {
	if (auto status = processStep(); !status.ok()) {
		ABSL_LOG(WARNING) << "Step error: " << status;
		if (status = stop(); !status.ok()) {
			ABSL_LOG(ERROR) << "Stop TraCI controller: " << status;
		}
		return;
	}

	stepEvent_ = ns3::Simulator::Schedule(stepInterval_, &Controller::step, this);
}

absl::Status Controller::processStep() {
	const auto now = ns3::Simulator::Now();
	if (const auto status = port_->step(now.GetSeconds()); !status.ok()) {
		return absl::StatusBuilder(status).SetPrepend() << "advancing SUMO: ";
	}

	// Remove vehicles that have arrived.
	if (const auto arrivedVehicles = port_->arrivedVehicles(); !arrivedVehicles.ok()) {
		return absl::StatusBuilder(arrivedVehicles.status()).SetPrepend() << "reading arrived vehicles: ";
	} else {
		for (const auto& vehicleId : arrivedVehicles.value()) {
			if (const auto status = nodeManager_->removeVehicle(vehicleId); !status.ok()) {
				return absl::StatusBuilder(status).SetPrepend() << "removing vehicle '" << vehicleId << "': ";
			}
		}
	}

	// Add vehicles that have departed.
	if (const auto departedVehicles = port_->departedVehicles(); !departedVehicles.ok()) {
		return absl::StatusBuilder(departedVehicles.status()).SetPrepend() << "reading departed vehicles: ";
	} else {
		for (const auto& vehicleId : departedVehicles.value()) {
			if (const auto node = nodeManager_->addVehicle(vehicleId); !node.ok()) {
				return absl::StatusBuilder(node.status()).SetPrepend() << "adding vehicle '" << vehicleId << "': ";
			}
		}
	}

	for (auto listener = listeners_.begin(); listener != listeners_.end(); /* noop */) {
		if (const auto status = (*listener)->onTraciStep(port_, now); !status.ok()) {
			ABSL_LOG(WARNING) << "TraCI listener failed to update and was removed: " << status;
			listener = listeners_.erase(listener);
		} else {
			++listener;
		}
	}

	return absl::OkStatus();
}

void Controller::setCommand(std::string command) {
	config_.command_ = std::move(command);
}

std::string Controller::getCommand() const {
	return config_.command_;
}

void Controller::setLaunchPort(int port) {
	config_.port_ = port;
}

int Controller::getLaunchPort() const {
	return config_.port_;
}

void Controller::setRetries(int retries) {
	config_.retries_ = retries;
}

int Controller::getRetries() const {
	return config_.retries_;
}

void Controller::setVerbose(bool verbose) {
	config_.verbose_ = verbose;
}

bool Controller::getVerbose() const {
	return config_.verbose_;
}

void Controller::setTraceFile(std::string traceFile) {
	config_.traceFile_ = std::move(traceFile);
}

std::string Controller::getTraceFile() const {
	return config_.traceFile_;
}

void Controller::setTraceGetters(bool traceGetters) {
	config_.traceGetters_ = traceGetters;
}

bool Controller::getTraceGetters() const {
	return config_.traceGetters_;
}
