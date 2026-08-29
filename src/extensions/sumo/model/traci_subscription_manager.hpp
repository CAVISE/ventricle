#pragma once

#include "src/core/defs.hpp"
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

	/** Manages subscriptions for exactly one libtraci domain. */
	template <typename Domain>
	class TraciDomainSubscriptionManager final : public ITraciListener {
	public:
		/** Subscribe @p subscriber to one @p variable for @p objectId. */
		absl::Status subscribe(int variable, ITraciSubscriber* subscriber, std::string objectId);

		/** Subscribe @p subscriber to several variables for @p objectId in one TraCI update. */
		template <int... Variables>
		absl::Status subscribeMultiple(ITraciSubscriber* subscriber, std::string objectId) {
			static_assert(sizeof...(Variables) > 0, "at least one TraCI variable is required");
			constexpr std::array variables{Variables...};
			return subscribeVariables(variables, subscriber, std::move(objectId));
		}

		/** Mark the registration owned by @p subscriber for removal. */
		absl::Status unsubscribe(ITraciSubscriber* subscriber);

	private:
		enum class SubscriptionState : std::int_fast8_t {
			Active,
			Stale,
		};

		struct SubscriberState {
			ITraciSubscriber* subscriber_;
			std::vector<int> variables_;
			SubscriptionState state_ = SubscriptionState::Active;
		};

		/* ITraciListener implementation*/
		absl::Status onTraciStart(TraciPort* port) override;
		absl::Status onTraciStep(TraciPort* port, ns3::Time time) override;
		void onTraciEnd(TraciPort* port) override;

		absl::Status refresh(const std::string& objectId);
		absl::Status subscribeVariables(std::span<const int> variables, ITraciSubscriber* subscriber, std::string objectId);

		absl::flat_hash_map<std::string, std::vector<SubscriberState>> subscriptions_;
		TraciPort* port_ = nullptr;
	};

	using TraciVehicleSubscriptionManager = TraciDomainSubscriptionManager<libtraci::Vehicle>;
	using TraciPersonSubscriptionManager = TraciDomainSubscriptionManager<libtraci::Person>;
	using TraciJunctionSubscriptionManager = TraciDomainSubscriptionManager<libtraci::Junction>;
	using TraciSimulationSubscriptionManager = TraciDomainSubscriptionManager<libtraci::Simulation>;

	/** Owns the domain-specific subscription managers used by a simulation. */
	class TraciSubscriptions final : public ITraciListener {
	public:
		/** Return the single subscription manager for @p Domain, creating it when needed. */
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
		absl::flat_hash_map<std::type_index, std::unique_ptr<ITraciListener>> managers_;
		TraciPort* port_ = nullptr;
	};

} // namespace vcle::sumo
