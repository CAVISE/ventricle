#pragma once

#include "src/extensions/sumo/model/traci_listener.hpp"
#include "src/extensions/sumo/model/traci_node_manager.hpp"
#include "src/extensions/sumo/model/traci_port.hpp"

#include <string>
#include <vector>

#include <absl/status/status.h>

#include <ns3/event-id.h>
#include <ns3/nstime.h>
#include <ns3/object.h>

namespace vcle::sumo {

	/** Keeps an external SUMO simulation synchronized with the ns-3 event loop. */
	class Controller : public ns3::Object {
	public:
		static ns3::TypeId GetTypeId();

		Controller() = default;
		~Controller() override;

		Controller(const Controller&) = delete;
		Controller& operator=(const Controller&) = delete;

		/** Attach the TraCI port and node manager used by this controller. */
		absl::Status configure(TraciPort& port, TraciNodeManager& nodeManager);

		/** Start SUMO and schedule synchronization at the current simulation time. */
		absl::Status start();
		/** Cancel synchronization and close the SUMO connection. */
		absl::Status stop();
		/** Return the first asynchronous synchronization failure, if any. */
		const absl::Status& status() const;

		/** Register an externally owned lifecycle listener. */
		void addListener(ITraciListener* listener);
		/** Stop notifying a previously registered lifecycle listener. */
		void removeListener(ITraciListener* listener);

	private:
		/* ns3::Object implementation*/
		void DoDispose() override;

		void step();
		absl::Status processStep();

		/** Store the quoted SUMO command line. */
		void setCommand(std::string command);
		/** Return the quoted SUMO command line. */
		VCLE_NODISCARD std::string getCommand() const;
		/** Store the TCP port passed to libtraci. */
		void setLaunchPort(int port);
		/** Return the TCP port passed to libtraci. */
		VCLE_NODISCARD int getLaunchPort() const;
		/** Store the number of libtraci connection attempts. */
		void setRetries(int retries);
		/** Return the number of libtraci connection attempts. */
		VCLE_NODISCARD int getRetries() const;
		/** Store whether verbose libtraci output is enabled. */
		void setVerbose(bool verbose);
		/** Return whether verbose libtraci output is enabled. */
		VCLE_NODISCARD bool getVerbose() const;
		/** Store the libtraci trace output file. */
		void setTraceFile(std::string traceFile);
		/** Return the libtraci trace output file. */
		VCLE_NODISCARD std::string getTraceFile() const;
		/** Store whether getter calls are included in traces. */
		void setTraceGetters(bool traceGetters);
		/** Return whether getter calls are included in traces. */
		VCLE_NODISCARD bool getTraceGetters() const;

	private:
		TraciPort* port_ = nullptr;
		TraciNodeManager* nodeManager_ = nullptr;

		ns3::Time stepInterval_;
		ns3::EventId stepEvent_;
		std::vector<ITraciListener*> listeners_;
		absl::Status status_;

		struct {
			std::string command_;
			int port_ = -1;
			int retries_ = libsumo::DEFAULT_NUM_RETRIES;
			bool verbose_ = false;
			std::string traceFile_;
			bool traceGetters_ = true;
		} config_;
	};

} // namespace vcle::sumo
