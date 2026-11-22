#pragma once

#include <ventricle/core/defs.hpp>
#include <ventricle/core/error.hpp>

#include <any>
#include <span>
#include <string>

#include <glaze/json/generic_fwd.hpp>

namespace vcle::config {

	/**
	 * @brief Provides scoped control settings to configuration block processors.
	 *
	 * Pair each successful @ref entry with @ref exit to restore the enclosing block's settings.
	 * Use @ref get for a typed current value or @ref stack to inspect an entry's history.
	 */
	class IParserContext {
	public:
		virtual ~IParserContext() = default;

		/**
		 * @brief Push settings from an object block, inheriting omitted registered entries.
		 * @param[in] block Glaze object whose direct members contain control settings.
		 * @return Success after pushing one frame for every entry, or an error without changing
		 * the values or scope depth. Members not registered with the context are ignored.
		 */
		VCLE_NODISCARD virtual Expected<> entry(const glz::generic& block) = 0;

		/**
		 * @brief Pop one block's settings, restoring the enclosing values.
		 * @return An error if no block is active; initial defaults are never popped.
		 */
		VCLE_NODISCARD virtual Expected<> exit() = 0;

		/**
		 * @brief Borrow an entry's values, ordered from its initial default to its current value.
		 * @return An empty span for an unregistered name. Views and references must be reacquired
		 * after @ref entry or @ref exit, including failed operations.
		 */
		VCLE_NODISCARD virtual std::span<const std::any> stack(const std::string& name) const noexcept = 0;

		/**
		 * @brief Borrow an entry's current value as its registered type.
		 * @return Null for an unregistered name or a different type. The lifetime follows @ref stack.
		 */
		template <typename T>
		VCLE_NODISCARD const T* get(const std::string& name) const noexcept {
			const auto values = stack(name);
			return values.empty() ? nullptr : std::any_cast<T>(&values.back());
		}
	};

} // namespace vcle::config
