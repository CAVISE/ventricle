#include "src/extensions/sumo/model/traci_subscription_manager.hpp"

#include <algorithm>

#include <absl/log/absl_check.h>

using namespace vcle::sumo;

template <typename Domain>
absl::Status TraciDomainSubscriptionManager<Domain>::subscribe(int variable, ITraciSubscriber* subscriber, std::string objectId) {
	return subscribeVariables(std::span(&variable, 1), subscriber, std::move(objectId));
}

template <typename Domain>
absl::Status TraciDomainSubscriptionManager<Domain>::subscribeVariables(
	std::span<const int> variables, ITraciSubscriber* subscriber, std::string objectId
) {
	ABSL_CHECK(subscriber) << "Subscriber is nullptr";

	auto& subscriptions = subscriptions_[objectId];
	auto state = std::ranges::find(subscriptions, subscriber, &SubscriberState::subscriber_);
	if (state == subscriptions.end()) {
		state = subscriptions.emplace(subscriptions.end(), SubscriberState{.subscriber_ = subscriber});
	}

	bool changed = state->state_ != SubscriptionState::Active;
	state->state_ = SubscriptionState::Active;

	for (const int variable : variables) {
		const auto position = std::ranges::lower_bound(state->variables_, variable);
		if (position == state->variables_.end() || *position != variable) {
			state->variables_.insert(position, variable);
			changed = true;
		}
	}

	return changed ? refresh(objectId) : absl::OkStatus();
}

template <typename Domain>
absl::Status TraciDomainSubscriptionManager<Domain>::unsubscribe(ITraciSubscriber* subscriber) {
	ABSL_CHECK(subscriber) << "Subscriber is nullptr";

	for (auto& [objectId, subscriptions] : subscriptions_) {
		bool objectChanged = false;
		for (auto& state : subscriptions) {
			if (state.subscriber_ == subscriber && state.state_ == SubscriptionState::Active) {
				// Keep the record stable until the current callback cycle has finished.
				state.state_ = SubscriptionState::Stale;
				objectChanged = true;
			}
		}

		if (objectChanged) {
			if (const auto status = refresh(objectId); !status.ok()) {
				return status;
			}
		}
	}

	return absl::OkStatus();
}

template <typename Domain>
absl::Status TraciDomainSubscriptionManager<Domain>::onTraciStart(TraciPort* port) {
	ABSL_CHECK(port) << "Port is nullptr";
	port_ = port;

	for (const auto& subscription : subscriptions_) {
		if (const auto status = refresh(subscription.first); !status.ok()) {
			return status;
		}
	}

	return absl::OkStatus();
}

template <typename Domain>
absl::Status TraciDomainSubscriptionManager<Domain>::onTraciStep(TraciPort* port, ns3::Time time) {
	ABSL_CHECK(port) << "Port is nullptr";

	for (const auto& [objectId, subscriptions] : subscriptions_) {
		const auto results = port->subscriptionResults<Domain>(objectId);
		if (!results.ok()) {
			return results.status();
		}

		for (const SubscriberState& state : subscriptions) {
			if (state.state_ == SubscriptionState::Stale) {
				continue;
			}
			state.subscriber_->onTraciValues(objectId, TraciValues(*results), time);
		}
	}

	for (auto object = subscriptions_.begin(); object != subscriptions_.end();) {
		std::erase_if(object->second, [](const auto& state) { return state.state_ == SubscriptionState::Stale; });
		if (object->second.empty()) {
			subscriptions_.erase(object++);
		} else {
			++object;
		}
	}

	return absl::OkStatus();
}

template <typename Domain>
void TraciDomainSubscriptionManager<Domain>::onTraciEnd(TraciPort* port) {
	ABSL_CHECK(port) << "TraCI subscription manager requires a port";
	port_ = nullptr;
}

template <typename Domain>
absl::Status TraciDomainSubscriptionManager<Domain>::refresh(const std::string& objectId) {
	if (!port_) {
		return absl::OkStatus();
	}

	std::vector<int> variables;
	for (const auto& state : subscriptions_.at(objectId)) {
		if (state.state_ == SubscriptionState::Stale) {
			continue;
		}
		for (const auto variable : state.variables_) {
			const auto position = std::ranges::lower_bound(variables, variable);
			if (position == variables.end() || *position != variable) {
				variables.insert(position, variable);
			}
		}
	}

	if (variables.empty()) {
		return port_->unsubscribe<Domain>(objectId);
	}
	return port_->subscribe<Domain>(objectId, variables);
}

absl::Status TraciSubscriptions::onTraciStart(TraciPort* port) {
	ABSL_CHECK(port) << "TraCI subscriptions require a port";
	if (port_) {
		return absl::FailedPreconditionError("TraCI subscriptions are already active");
	}
	port_ = port;
	for (const auto& managerEntry : managers_) {
		const auto& manager = managerEntry.second;
		if (const auto status = manager->onTraciStart(port); !status.ok()) {
			for (const auto& rollbackEntry : managers_) {
				rollbackEntry.second->onTraciEnd(port);
			}
			port_ = nullptr;
			return status;
		}
	}
	return absl::OkStatus();
}

absl::Status TraciSubscriptions::onTraciStep(TraciPort* port, ns3::Time time) {
	ABSL_CHECK(port) << "TraCI subscriptions require a port";
	for (const auto& managerEntry : managers_) {
		const auto& manager = managerEntry.second;
		if (const auto status = manager->onTraciStep(port, time); !status.ok()) {
			return status;
		}
	}
	return absl::OkStatus();
}

void TraciSubscriptions::onTraciEnd(TraciPort* port) {
	ABSL_CHECK(port) << "TraCI subscriptions require a port";
	for (const auto& managerEntry : managers_) {
		managerEntry.second->onTraciEnd(port);
	}
	port_ = nullptr;
}

template class vcle::sumo::TraciDomainSubscriptionManager<libtraci::Vehicle>;
template class vcle::sumo::TraciDomainSubscriptionManager<libtraci::Person>;
template class vcle::sumo::TraciDomainSubscriptionManager<libtraci::Junction>;
template class vcle::sumo::TraciDomainSubscriptionManager<libtraci::Simulation>;
