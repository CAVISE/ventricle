#pragma once

#include <algorithm>
#include <functional>
#include <type_traits>
#include <vector>

#include <absl/log/absl_check.h>
#include <absl/log/absl_log.h>
#include <absl/status/status.h>

namespace vcle {

	/**
	 * @brief Base class for publishers that register listeners and dispatch callbacks to them.
	 * @tparam Listener Listener interface accepted by the callbacks.
	 *
	 * Listeners are externally owned. Registration does not extend their lifetime, and removal
	 * does not destroy them. Each listener must remain alive until it is removed or this base
	 * is destroyed.
	 */
	template <typename Listener>
	class PublisherBase {
	public:

		/**
		 * @brief Append a listener to the notification list.
		 * @param[in] listener Externally owned listener to register.
		 * @pre @p listener is non-null and is not already registered.
		 */
		void addListener(Listener* listener) {
			ABSL_CHECK(listener) << "Null listener";
			ABSL_CHECK(std::ranges::find(listeners_, listener) == listeners_.end()) << "Listener is already registered";

			listeners_.push_back(listener);
		}

		/**
		 * @brief Stop notifying a listener without destroying it.
		 * @param[in] listener Listener to remove; an unregistered listener has no effect.
		 * @pre @p listener is non-null; passing nullptr terminates the process.
		 */
		void removeListener(Listener* listener) {
			ABSL_CHECK(listener) << "Null listener";
			std::erase(listeners_, listener);
		}

	protected:
		/** @brief Clear the registration list without destroying its listeners. */
		~PublisherBase() = default;

		/**
		 * @brief Invoke a callback on each registered listener, removing listeners that report failure.
		 * @tparam F Callable type accepting a Listener pointer followed by the supplied arguments.
		 * @tparam Args Types of the additional callback arguments.
		 * @param[in] method Member-function pointer.
		 * @param[in] args Additional arguments, stored by value and passed as lvalues to each callback.
		 *
		 * A non-OK status is logged and its listener is removed before dispatch continues.
		 * Successful and void callbacks leave their listeners registered. Other return types
		 * are rejected at compile time.
		 *
		 * @pre Callbacks must not modify this listener list or destroy the source during dispatch.
		 */
		template <typename F, typename... Args>
		void invokeListeners(F method, Args... args) {
			using ReturnType = std::invoke_result_t<F&, Listener*, Args&...>;
			static_assert(std::is_same_v<ReturnType, absl::Status> || std::is_same_v<ReturnType, void>, "Listener callbacks must return absl::Status or void");

			for (auto listener = listeners_.begin(); listener != listeners_.end(); /* noop */) {
				if constexpr (std::is_same_v<ReturnType, absl::Status>) {
					if (const auto status = std::invoke(method, *listener, args...); !status.ok()) {
						ABSL_LOG(WARNING) << "Listener callback failed and listener was removed: " << status;
						listener = listeners_.erase(listener);
					} else {
						++listener;
					}
				} else if constexpr (std::is_same_v<ReturnType, void>) {
					std::invoke(method, *listener, args...);
					++listener;
				}
			}
		}

	private:
		std::vector<Listener*> listeners_; ///< Non-owning listener pointers in registration order.
	};

} // namespace vcle
