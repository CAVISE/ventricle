#include "src/extensions/its-g5/model/runtime.hpp"

#include "src/extensions/its-g5/helpers/type_glue.hpp"

#include <algorithm>
#include <ns3/simulator.h>

namespace vcle::itsg5 {
	void Runtime::schedule(
		vanetza::Clock::time_point time, const Callback& callback, const void* scope
	) {
		const auto delay = std::max(time - now(), vanetza::Clock::duration::zero());
		schedule(delay, callback, scope);
	}

	void Runtime::schedule(
		vanetza::Clock::duration delay, const Callback& callback, const void* scope
	) {
		RemoveExpiredEvents();
		auto event =
			ns3::Simulator::Schedule(TypeGlue::convert<ns3::Time>(delay), [this, callback] {
				callback(now());
				RemoveExpiredEvents();
			});
		Events_.emplace(scope, event);
	}

	void Runtime::cancel(
		const void* scope
	) {
		if (scope == nullptr) {
			return;
		}

		const auto [begin, end] = Events_.equal_range(scope);
		for (auto it = begin; it != end; ++it) {
			ns3::Simulator::Cancel(it->second);
		}
		Events_.erase(begin, end);
	}

	vanetza::Clock::time_point Runtime::now() const {
		return TypeGlue::convert<vanetza::Clock::time_point>(ns3::Simulator::Now());
	}

	void Runtime::RemoveExpiredEvents() {
		std::erase_if(Events_, [](const auto& entry) {
			return ns3::Simulator::IsExpired(entry.second);
		});
	}
} // namespace vcle::itsg5
