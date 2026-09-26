#include "src/extensions/its-g5/model/runtime.hpp"

#include "src/extensions/its-g5/helpers/type_glue.hpp"

#include <algorithm>

#include <ns3/simulator.h>

namespace vcle::itsg5 {
	void Runtime::schedule(vanetza::Clock::time_point time, const Callback& callback, const void* scope) {
		const auto delay = std::max(time - now(), vanetza::Clock::duration::zero());
		schedule(delay, callback, scope);
	}

	void Runtime::schedule(vanetza::Clock::duration delay, const Callback& callback, const void* scope) {
		removeExpiredEvents();
		auto event = ns3::Simulator::Schedule(TypeGlue::convert<ns3::Time>(delay), [this, callback] {
			callback(now());
			removeExpiredEvents();
		});
		events_[scope].push_back(event);
	}

	void Runtime::cancel(const void* scope) {
		if (scope == nullptr) {
			return;
		}

		const auto events = events_.find(scope);
		if (events == events_.end()) {
			return;
		}
		for (const auto& event : events->second) {
			ns3::Simulator::Cancel(event);
		}
		events_.erase(events);
	}

	vanetza::Clock::time_point Runtime::now() const {
		return TypeGlue::convert<vanetza::Clock::time_point>(ns3::Simulator::Now());
	}

	void Runtime::removeExpiredEvents() {
		for (auto events = events_.begin(); events != events_.end();) {
			std::erase_if(events->second, [](const auto& event) { return ns3::Simulator::IsExpired(event); });
			if (events->second.empty()) {
				events_.erase(events++);
			} else {
				++events;
			}
		}
	}
} // namespace vcle::itsg5
