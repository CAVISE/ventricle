#pragma once

#include <ns3/event-id.h>
#include <unordered_map>
#include <vanetza/common/runtime.hpp>

namespace vcle::itsg5 {
	class Runtime final : public vanetza::Runtime {
	public:
		void schedule(vanetza::Clock::time_point time, const Callback& callback,
					  const void* scope = nullptr) override;
		void schedule(vanetza::Clock::duration delay, const Callback& callback,
					  const void* scope = nullptr) override;
		void cancel(const void* scope) override;

		vanetza::Clock::time_point now() const override;

	private:
		void RemoveExpiredEvents();

		std::unordered_multimap<const void*, ns3::EventId> Events_;
	};
} // namespace vcle::itsg5
