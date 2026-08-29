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

	/** Owns the active libtraci connection to an external SUMO process. */
	class TraciPort final {
	public:
		/** Construct an inactive TraCI connection. */
		TraciPort() = default;
		~TraciPort();

		TraciPort(const TraciPort&) = delete;
		TraciPort& operator=(const TraciPort&) = delete;

		/** Start SUMO using the supplied libtraci launch parameters. */
		absl::Status start(
			const std::vector<std::string>& command, int port, int retries, bool verbose, const std::string& traceFile, bool traceGetters
		);
		/** Advance SUMO to @p time in seconds. */
		absl::Status step(double time);
		/** Close the active TraCI connection. */
		absl::Status close();
		/** Return the connected TCP port, or `-1` when SUMO is not running. */
		VCLE_NODISCARD int port() const;

		/** Return vehicle IDs which departed during the latest step. */
		VCLE_NODISCARD absl::StatusOr<std::vector<std::string>> departedVehicles() const;
		/** Return vehicle IDs which arrived during the latest step. */
		VCLE_NODISCARD absl::StatusOr<std::vector<std::string>> arrivedVehicles() const;

		/** Register variables for @p objectId in a libtraci domain. */
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
				return invoke("subscribing", static_cast<Subscribe>(&Domain::subscribe), variables, begin, end, parameters);
			} else {
				using Subscribe = void (*)(const std::string&, const std::vector<int>&, double, double, const libsumo::TraCIResults&);
				return invoke("subscribing", static_cast<Subscribe>(&Domain::subscribe), objectId, variables, begin, end, parameters);
			}
		}

		/** Remove the variable subscription for @p objectId in a libtraci domain. */
		template <typename Domain>
		absl::Status unsubscribe(const std::string& objectId) {
			if constexpr (std::same_as<Domain, libtraci::Simulation>) {
				using Subscribe = void (*)(const std::vector<int>&, double, double, const libsumo::TraCIResults&);
				return invoke(
					"unsubscribing",
					static_cast<Subscribe>(&Domain::subscribe),
					std::vector<int>{},
					libsumo::INVALID_DOUBLE_VALUE,
					libsumo::INVALID_DOUBLE_VALUE,
					libsumo::TraCIResults{}
				);
			} else {
				using Unsubscribe = void (*)(const std::string&);
				return invoke("unsubscribing", static_cast<Unsubscribe>(&Domain::unsubscribe), objectId);
			}
		}

		/** Retrieve the latest subscription results for @p objectId. */
		template <typename Domain>
		absl::StatusOr<libsumo::TraCIResults> subscriptionResults(const std::string& objectId) const {
			if constexpr (std::same_as<Domain, libtraci::Simulation>) {
				using GetResults = const libsumo::TraCIResults (*)();
				return invoke("reading subscription results", static_cast<GetResults>(&Domain::getSubscriptionResults));
			} else {
				using GetResults = const libsumo::TraCIResults (*)(const std::string&);
				return invoke("reading subscription results", static_cast<GetResults>(&Domain::getSubscriptionResults), objectId);
			}
		}

	private:
		template <typename Command, typename... Arguments>
		static auto invoke(std::string_view operation, Command&& command, Arguments&&... arguments) {
			using Result = std::invoke_result_t<Command, Arguments...>;
			if constexpr (std::is_void_v<Result>) {
				try {
					std::invoke(std::forward<Command>(command), std::forward<Arguments>(arguments)...);
					return absl::OkStatus();
				} catch (const libsumo::TraCIException& error) {
					return absl::UnavailableError(absl::StrCat(operation, ": ", error.what()));
				}
			} else {
				using Status = absl::StatusOr<std::remove_cvref_t<Result>>;
				try {
					return Status(std::invoke(std::forward<Command>(command), std::forward<Arguments>(arguments)...));
				} catch (const libsumo::TraCIException& error) {
					return Status(absl::UnavailableError(absl::StrCat(operation, ": ", error.what())));
				}
			}
		}

	private:
		int port_ = -1;
	};

} // namespace vcle::sumo
