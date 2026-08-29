#include "src/extensions/sumo/model/traci_port.hpp"

#include <absl/log/absl_log.h>

using namespace vcle::sumo;

TraciPort::~TraciPort() {
	if (port_ != -1) {
		if (auto status = close(); !status.ok()) {
			ABSL_LOG(WARNING) << "Close TraCI port: " << status.message();
		}
	}
}

absl::Status TraciPort::start(
	const std::vector<std::string>& command, int port, int retries, bool verbose, const std::string& traceFile, bool traceGetters
) {
	if (port_ != -1) {
		return absl::FailedPreconditionError("TraCI port is already connected");
	}

	/* clang-format off */
	if (const auto result = invoke(
			"starting simulation",
			&libtraci::Simulation::start,
			command,
			port,
			retries,
			"default",
			verbose,
			traceFile,
			traceGetters,
			nullptr
		);
		!result.ok()
	) {
		return result.status();
	} else {
		port_ = result->first;
		return absl::OkStatus();
	}
	/* clang-format on */
}

absl::Status TraciPort::step(double time) {
	return invoke("advancing simulation", &libtraci::Simulation::step, time);
}

absl::Status TraciPort::close() {
	if (port_ == -1) {
		return absl::OkStatus();
	}

	const auto status = invoke("closing simulation", &libtraci::Simulation::close, "Ventricle requested termination.");
	if (status.ok()) {
		port_ = -1;
	}

	return status;
}

int TraciPort::port() const {
	return port_;
}

absl::StatusOr<std::vector<std::string>> TraciPort::departedVehicles() const {
	return invoke("reading departed vehicles", &libtraci::Simulation::getDepartedIDList);
}

absl::StatusOr<std::vector<std::string>> TraciPort::arrivedVehicles() const {
	return invoke("reading arrived vehicles", &libtraci::Simulation::getArrivedIDList);
}
