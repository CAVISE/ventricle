#pragma once

#include "src/extensions/sumo/model/traci_listener.hpp"
#include "src/extensions/sumo/model/traci_port.hpp"
#include "src/extensions/sumo/model/traci_subscriber.hpp"

#include <array>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <typeindex>
#include <utility>
#include <vector>

#include <absl/container/flat_hash_map.h>

namespace vcle::sumo {

	/**
	 * @brief Collects variable subscriptions and delivers results for one libtraci domain.
	 * @tparam Domain Supported libtraci domain: Vehicle, Person, Junction, or Simulation.
	 *
	 * Each object's remote subscription is the union of its active subscribers' variables.
	 * Every active subscriber receives the complete result map for that object.
	 * Registrations made before startup are sent when the TraCI connection starts.
	 *
	 * @note Subscribers are externally owned and must remain alive while active. Result views
	 * are borrowed for the duration of a callback. Callbacks may unsubscribe, but must not
	 * add subscriptions during dispatch because insertion can invalidate iteration.
	 */
	template <typename Domain>
	class TraciDomainSubscriptionManager final : public ITraciListener {
	public:
		/**
		 * @brief Add a variable to a subscriber's registration, ignoring duplicate variables.
		 * @param[in] variable TraCI variable identifier.
		 * @param[in] subscriber Non-null, externally owned recipient of subscription results.
		 * @param[in] objectId Domain object identifier; use an empty identifier for Simulation.
		 * @return OK if recorded and any required remote update succeeds; otherwise the port's error.
		 * @note Local registration changes are retained if the remote update fails.
		 */
		absl::Status subscribe(int variable, ITraciSubscriber* subscriber, std::string objectId);

		/**
		 * @brief Add several variables with at most one remote subscription update.
		 * @tparam Variables Non-empty pack of TraCI variable identifiers.
		 * @param[in] subscriber Non-null, externally owned recipient of subscription results.
		 * @param[in] objectId Domain object identifier; use an empty identifier for Simulation.
		 * @return OK if recorded and any required remote update succeeds; otherwise the port's error.
		 * @note Variables are merged with the existing registration. Local changes are not rolled back on failure.
		 */
		template <int... Variables>
		absl::Status subscribeMultiple(ITraciSubscriber* subscriber, std::string objectId) {
			static_assert(sizeof...(Variables) > 0, "at least one TraCI variable is required");
			constexpr std::array variables{Variables...};
			return subscribeVariables(variables, subscriber, std::move(objectId));
		}

		/**
		 * @brief Mark a subscriber's active registrations for removal across this domain's objects.
		 * @param[in] subscriber Non-null subscriber to unregister.
		 * @return OK if no registrations exist or all remote updates succeed; otherwise the first port error.
		 * @note Marked records stop receiving notifications and are erased after a successful step dispatch.
		 * On error, processing stops and some objects may still have active registrations.
		 */
		absl::Status unsubscribe(ITraciSubscriber* subscriber);

	private:
		/** @brief Whether a registration receives results or awaits deferred removal. */
		enum class SubscriptionState : std::int_fast8_t {
			Active, ///< Receives subscription results.
			Stale, ///< Skipped during dispatch and removed during cleanup.
		};

		/** @brief One subscriber's registration for a single domain object. */
		struct SubscriberState {
			ITraciSubscriber* subscriber_; ///< Externally owned result recipient.
			std::vector<int> variables_; ///< Sorted, unique requested variable identifiers.
			SubscriptionState state_ = SubscriptionState::Active; ///< Dispatch eligibility.
		};

		/* ITraciListener implementation*/
		absl::Status onTraciStart(TraciPort* port) override;
		absl::Status onTraciStep(TraciPort* port, ns3::Time time) override;
		void onTraciEnd(TraciPort* port) override;

		/**
		 * @brief Send the union of active variables, or unsubscribe when that union is empty.
		 * @param[in] objectId Object with a local registration entry.
		 * @return OK while disconnected or on success; otherwise the port's error.
		 */
		absl::Status refresh(const std::string& objectId);
		/** @brief Merge variables into a registration and refresh its remote subscription when changed. */
		absl::Status subscribeVariables(std::span<const int> variables, ITraciSubscriber* subscriber, std::string objectId);

		absl::flat_hash_map<std::string, std::vector<SubscriberState>> subscriptions_; ///< Registrations grouped by domain object.
		TraciPort* port_ = nullptr; ///< Borrowed connection, or nullptr while detached.
	};

	/** @brief Subscription manager for the libtraci Vehicle domain. */
	using TraciVehicleSubscriptionManager = TraciDomainSubscriptionManager<libtraci::Vehicle>;
	/** @brief Subscription manager for the libtraci Person domain. */
	using TraciPersonSubscriptionManager = TraciDomainSubscriptionManager<libtraci::Person>;
	/** @brief Subscription manager for the libtraci Junction domain. */
	using TraciJunctionSubscriptionManager = TraciDomainSubscriptionManager<libtraci::Junction>;
	/** @brief Subscription manager for the libtraci Simulation domain. */
	using TraciSimulationSubscriptionManager = TraciDomainSubscriptionManager<libtraci::Simulation>;

	/**
	 * @brief Owns domain subscription managers and forwards the controller's lifecycle notifications.
	 *
	 * Register this object as an ITraciListener with the controller. Domain managers are created
	 * lazily by get() and retain their local registrations across connection shutdowns.
	 * Subscribers and the TraCI port remain externally owned.
	 */
	class TraciSubscriptions final : public ITraciListener {
	public:
		/**
		 * @brief Access the single manager for a domain, creating it on first use.
		 * @tparam Domain Supported libtraci domain: Vehicle, Person, Junction, or Simulation.
		 * @return Manager reference valid until this TraciSubscriptions object is destroyed.
		 * @note A newly created manager is attached immediately when the connection is active.
		 */
		template <typename Domain>
		TraciDomainSubscriptionManager<Domain>& get() {
			const auto type = std::type_index(typeid(Domain));
			auto [manager, inserted] = managers_.try_emplace(type);
			if (inserted) {
				auto domainManager = std::make_unique<TraciDomainSubscriptionManager<Domain>>();
				manager->second = std::move(domainManager);
				if (port_) {
					(void)manager->second->onTraciStart(port_);
				}
			}
			return static_cast<TraciDomainSubscriptionManager<Domain>&>(*manager->second);
		}

		/* ITraciListener implementation*/
		absl::Status onTraciStart(TraciPort* port) override;
		absl::Status onTraciStep(TraciPort* port, ns3::Time time) override;
		void onTraciEnd(TraciPort* port) override;

	private:
		absl::flat_hash_map<std::type_index, std::unique_ptr<ITraciListener>> managers_; ///< Owned managers indexed by domain type.
		TraciPort* port_ = nullptr; ///< Borrowed connection, or nullptr while detached.
	};

} // namespace vcle::sumo
