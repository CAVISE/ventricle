#pragma once

#include <vector>

#include <absl/container/flat_hash_map.h>

#include <ns3/event-id.h>

#include <vanetza/common/runtime.hpp>

namespace vcle::itsg5 {
	/** Implements Vanetza scheduling through the ns-3 event loop. */
	class Runtime final : public vanetza::Runtime {
	public:
		/* vanetza::Runtime implementation*/
		void schedule(vanetza::Clock::time_point time, const Callback& callback, const void* scope = nullptr) override;
		void schedule(vanetza::Clock::duration delay, const Callback& callback, const void* scope = nullptr) override;
		void cancel(const void* scope) override;

		vanetza::Clock::time_point now() const override;

	private:
		void removeExpiredEvents();

		absl::flat_hash_map<const void*, std::vector<ns3::EventId>> events_;
	};
} // namespace vcle::itsg5
