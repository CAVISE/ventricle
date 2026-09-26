#pragma once

#include "src/core/defs.hpp"

#include <concepts>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <absl/status/status.h>
#include <absl/status/statusor.h>
#include <absl/strings/str_cat.h>

#include <libsumo/libtraci.h>

namespace vcle::sumo {

	namespace detail {
		/** @name TraCI operation descriptions used in error statuses
		 * @{
		 */
		inline constexpr std::string_view traciStartOperation = "starting simulation";
		inline constexpr std::string_view traciStepOperation = "advancing simulation";
		inline constexpr std::string_view traciCloseOperation = "closing simulation";
		inline constexpr std::string_view traciDepartedVehiclesOperation = "reading departed vehicles";
		inline constexpr std::string_view traciArrivedVehiclesOperation = "reading arrived vehicles";
		inline constexpr std::string_view traciSubscribeOperation = "subscribing";
		inline constexpr std::string_view traciUnsubscribeOperation = "unsubscribing";
		inline constexpr std::string_view traciSubscriptionResultsOperation = "reading subscription results";
		/** @} */

		/** @brief Invoke a void command, converting TraCI exceptions into Unavailable statuses. */
		template <typename Command, typename... Arguments>
		absl::Status invokeImplVoid(std::string_view operation, Command&& command, Arguments&&... arguments) {
			try {
				std::invoke(std::forward<Command>(command), std::forward<Arguments>(arguments)...);
				return absl::OkStatus();
			} catch (const libsumo::TraCIException& error) {
				return absl::UnavailableError(absl::StrCat(operation, ": ", error.what()));
			}
		}

		/** @brief Invoke a value-returning command, wrapping its result or TraCI failure in StatusOr. */
		template <typename Command, typename... Arguments>
		auto invokeImplValue(std::string_view operation, Command&& command, Arguments&&... arguments) {
			using Result = std::invoke_result_t<Command, Arguments...>;
			using Status = absl::StatusOr<std::remove_cvref_t<Result>>;

			try {
				return Status(std::invoke(std::forward<Command>(command), std::forward<Arguments>(arguments)...));
			} catch (const libsumo::TraCIException& error) {
				return Status(absl::UnavailableError(absl::StrCat(operation, ": ", error.what())));
			}
		}
	} // namespace detail

	/**
	 * @brief Owns the libtraci connection to an external SUMO process.
	 *
	 * Wraps libtraci operations and converts libsumo::TraCIException failures into
	 * absl::UnavailableError statuses containing the operation description, serving
	 * as facade over libsumo functions. Prefer to use functionality provided by this
	 * class when accessing SUMO API.
	 */
	class TraciPort {
	public:
		TraciPort() = default;
		virtual ~TraciPort();

		TraciPort(const TraciPort&) = delete;
		TraciPort& operator=(const TraciPort&) = delete;

		/**
		 * @brief Launch SUMO and establish the TraCI connection.
		 * @param[in] command Executable and command-line arguments as separate strings.
		 * @param[in] port TCP port to use, or -1 to choose an available port.
		 * @param[in] retries Number of connection retries passed to libtraci.
		 * @param[in] verbose Whether libtraci emits verbose launch output.
		 * @param[in] traceFile Trace output filename, or an empty string to disable tracing.
		 * @param[in] traceGetters Whether getter calls are included in the trace.
		 * @return OK on success, FailedPrecondition if already connected, or Unavailable on a TraCI exception.
		 */
		absl::Status start(
			const std::vector<std::string>& command, int port, int retries, bool verbose, const std::string& traceFile, bool traceGetters
		);

		/**
		 * @brief Advance the connected SUMO simulation.
		 * @param[in] time Target simulation time in seconds; zero requests one SUMO step.
		 * @pre A TraCI connection has been established.
		 * @return OK on success, or Unavailable on a TraCI exception.
		 */
		absl::Status step(double time);

		/**
		 * @brief Close the active TraCI connection and mark this port inactive on success.
		 * @return OK if already inactive or successfully closed; Unavailable on a TraCI exception.
		 * @note A failed close retains the port number so closing can be retried.
		 */
		absl::Status close();

		/**
		 * @brief Access the TCP port recorded when the connection was established.
		 * @return Connected TCP port, or -1 before startup or after a successful close.
		 */
		VCLE_NODISCARD int port() const;

		/**
		 * @brief Retrieve vehicle IDs which departed during the latest SUMO step.
		 * @pre A TraCI connection has been established.
		 * @return Vehicle identifiers, or Unavailable on a TraCI exception.
		 */
		VCLE_NODISCARD absl::StatusOr<std::vector<std::string>> departedVehicles() const;

		/**
		 * @brief Retrieve vehicle IDs which arrived during the latest SUMO step.
		 * @pre A TraCI connection has been established.
		 * @return Vehicle identifiers, or Unavailable on a TraCI exception.
		 */
		VCLE_NODISCARD absl::StatusOr<std::vector<std::string>> arrivedVehicles() const;

		/**
		 * @brief Subscribe to variables in a libtraci domain.
		 * @tparam Domain Libtraci domain exposing a subscribe operation.
		 * @param[in] objectId Domain object identifier; ignored for libtraci::Simulation.
		 * @param[in] variables TraCI variable identifiers to subscribe to.
		 * @param[in] begin Subscription start time in seconds, or the libtraci default sentinel.
		 * @param[in] end Subscription end time in seconds, or the libtraci default sentinel.
		 * @param[in] parameters Additional parameters associated with the subscribed variables.
		 * @pre A TraCI connection has been established.
		 * @return OK on success, or Unavailable on a TraCI exception.
		 */
		template <typename Domain>
		absl::Status subscribe(
			const std::string& objectId,
			const std::vector<int>& variables,
			double begin = libsumo::INVALID_DOUBLE_VALUE,
			double end = libsumo::INVALID_DOUBLE_VALUE,
			const libsumo::TraCIResults& parameters = {}
		) {
			if constexpr (std::same_as<Domain, libtraci::Simulation>) {
				using Subscribe = void (*)(const std::vector<int>&, double, double, const libsumo::TraCIResults&);
				return invoke(detail::traciSubscribeOperation, static_cast<Subscribe>(&Domain::subscribe), variables, begin, end, parameters);
			} else {
				using Subscribe = void (*)(const std::string&, const std::vector<int>&, double, double, const libsumo::TraCIResults&);
				return invoke(detail::traciSubscribeOperation, static_cast<Subscribe>(&Domain::subscribe), objectId, variables, begin, end, parameters);
			}
		}

		/**
		 * @brief Remove a domain object's variable subscription.
		 * @tparam Domain Libtraci domain whose subscription is removed.
		 * @param[in] objectId Domain object identifier; ignored for libtraci::Simulation.
		 * @pre A TraCI connection has been established.
		 * @return OK on success, or Unavailable on a TraCI exception.
		 * @note Simulation subscriptions are cleared by subscribing to an empty variable list.
		 */
		template <typename Domain>
		absl::Status unsubscribe(const std::string& objectId) {
			if constexpr (std::same_as<Domain, libtraci::Simulation>) {
				using Subscribe = void (*)(const std::vector<int>&, double, double, const libsumo::TraCIResults&);
				return invoke(
					detail::traciUnsubscribeOperation,
					static_cast<Subscribe>(&Domain::subscribe),
					std::vector<int>{},
					libsumo::INVALID_DOUBLE_VALUE,
					libsumo::INVALID_DOUBLE_VALUE,
					libsumo::TraCIResults{}
				);
			} else {
				using Unsubscribe = void (*)(const std::string&);
				return invoke(detail::traciUnsubscribeOperation, static_cast<Unsubscribe>(&Domain::unsubscribe), objectId);
			}
		}

		/**
		 * @brief Retrieve the latest results of a domain subscription.
		 * @tparam Domain Libtraci domain exposing subscription results.
		 * @param[in] objectId Domain object identifier; ignored for libtraci::Simulation.
		 * @pre A TraCI connection has been established.
		 * @return Result map by value, or Unavailable on a TraCI exception.
		 * @note The copied map shares ownership of its result objects.
		 */
		template <typename Domain>
		absl::StatusOr<libsumo::TraCIResults> subscriptionResults(const std::string& objectId) const {
			if constexpr (std::same_as<Domain, libtraci::Simulation>) {
				using GetResults = const libsumo::TraCIResults (*)();
				return invoke(detail::traciSubscriptionResultsOperation, static_cast<GetResults>(&Domain::getSubscriptionResults));
			} else {
				using GetResults = const libsumo::TraCIResults (*)(const std::string&);
				return invoke(detail::traciSubscriptionResultsOperation, static_cast<GetResults>(&Domain::getSubscriptionResults), objectId);
			}
		}

		/**
		 * @brief Invoke libtraci operation.
		 * @tparam Command Callable type.
		 * @tparam Arguments Forwarded argument types.
		 * @param[in] operation Description prepended to an error message.
		 * @param[in] command Libtraci callable to invoke.
		 * @param[in] arguments Arguments forwarded to the callable.
		 * @return Status for void commands, or StatusOr holding the returned value otherwise.
		 * @note TraCI exceptions become Unavailable statuses; other exceptions propagate.
		 * This helper is stateless.
		 */
		template <typename Command, typename... Arguments>
		static auto invoke(std::string_view operation, Command&& command, Arguments&&... arguments) {
			using Result = std::invoke_result_t<Command, Arguments...>;
			if constexpr (std::is_void_v<Result>) {
				return detail::invokeImplVoid(operation, std::forward<Command>(command), std::forward<Arguments>(arguments)...);
			} else {
				return detail::invokeImplValue(operation, std::forward<Command>(command), std::forward<Arguments>(arguments)...);
			}
		}

	private:
		int port_ = -1; ///< Connected TCP port, or -1 while inactive.
	};

} // namespace vcle::sumo
