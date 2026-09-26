#pragma once

#include "src/core/simulation/listeners.hpp"
#include "src/extensions/sumo/model/traci_listener.hpp"
#include "src/extensions/sumo/model/traci_node_manager.hpp"
#include "src/extensions/sumo/model/traci_port.hpp"

#include <string>

#include <absl/status/status.h>

#include <ns3/event-id.h>
#include <ns3/nstime.h>
#include <ns3/object.h>

namespace vcle::sumo {

	/**
	 * @brief Synchronizes an external SUMO simulation with the ns-3 event loop.
	 *
	 * Launch settings and the synchronization interval are configured through ns-3 attributes.
	 * Each step advances SUMO, updates vehicle-node associations, and notifies registered listeners.
	 * Listener failures are logged and remove the listener; synchronization failures stop the controller.
	 *
	 * @note The TraCI port, node manager, and listeners are externally owned.
	 */
	class Controller
		: public ns3::Object
		, public PublisherBase<ITraciListener> {
	public:
		static ns3::TypeId GetTypeId();

		/**
		 * @brief Construct a controller without starting the SUMO connection.
		 * @param[in] port Externally owned TraCI connection used to control SUMO.
		 * @param[in] nodeManager Externally owned manager of vehicle-node associations.
		 */
		Controller(TraciPort& port, TraciNodeManager& nodeManager);
		~Controller() override = default;

		Controller(const Controller&) = delete;
		Controller& operator=(const Controller&) = delete;

		/**
		 * @brief Launch SUMO, notify start listeners, and schedule the first synchronization step.
		 * @return OK when started; an error if the command is empty or the TraCI connection fails to start.
		 * @note The first step is scheduled one StepInterval after the current simulation time.
		 * Listener errors remove the listener but do not cause this operation to fail.
		 */
		absl::Status start();

		/**
		 * @brief Cancel the pending step, notify end listeners, and close the TraCI connection.
		 * @return OK when closed, or the connection's close error with controller context.
		 */
		absl::Status stop();

	private:
		/* ns3::Object implementation*/
		void DoDispose() override;

		void step();
		absl::Status processStep();

		void setCommand(std::string command);
		VCLE_NODISCARD std::string getCommand() const;
		void setLaunchPort(int port);
		VCLE_NODISCARD int getLaunchPort() const;
		void setRetries(int retries);
		VCLE_NODISCARD int getRetries() const;
		void setVerbose(bool verbose);
		VCLE_NODISCARD bool getVerbose() const;
		void setTraceFile(std::string traceFile);
		VCLE_NODISCARD std::string getTraceFile() const;
		void setTraceGetters(bool traceGetters);
		VCLE_NODISCARD bool getTraceGetters() const;

	private:
		TraciPort* port_;
		TraciNodeManager* nodeManager_;

		ns3::Time stepInterval_;
		ns3::EventId stepEvent_;
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
