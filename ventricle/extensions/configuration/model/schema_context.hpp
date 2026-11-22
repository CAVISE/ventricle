#pragma once

/**
 * @file
 * @brief Traversal state and backing storage for configuration schema generation.
 *
 * @ref vcle::config::detail::Context supports nested schema construction for
 * @ref vcle::config::generateSchema. Schema fields may borrow stored strings, so serialization
 * must finish before their backing context is destroyed. Schema stack operations must be paired;
 * callbacks that throw do not automatically restore the stack.
 */

#include <ventricle/core/error.hpp>

#include <any>
#include <format>
#include <list>
#include <stack>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include <ns3/type-id.h>

#include <glaze/json/schema.hpp>

namespace vcle::config::detail {
	/**
	 * @brief Builds nested schemas while retaining the values borrowed by their fields.
	 *
	 * Pair @ref withSchema with @ref invoke to populate and extract one schema frame. Within the
	 * callback, @ref schemaForID fills an object schema and @ref schema exposes the active frame.
	 * Nested operations use the same pairing to restore their caller's frame.
	 *
	 * @ref store retains values independently of the schema stack. Popping a frame does not
	 * release this storage; the context must outlive every schema that borrows from it.
	 */
	class Context {
	public:
		/**
		 * @brief Push a schema frame for a subsequent @ref invoke call.
		 *
		 * The supplied value becomes the active frame; any previous frame remains underneath it.
		 */
		Context& withSchema(glz::schema schema);

		/// @pre A frame pushed by @ref withSchema is active.
		glz::schema& schema();

		/**
		 * @brief Populate the active object schema from a type and its inherited attributes.
		 * @pre @p id is initialized and a fresh frame pushed by @ref withSchema is active.
		 *
		 * Returns an error for hidden types, duplicate attribute names, or unsupported values.
		 * Failure may leave the active frame partially populated; @ref invoke discards that
		 * frame when its callback propagates the error.
		 * @see vcle::config::generateSchema for attribute filtering and schema behavior.
		 */
		Expected<> schemaForID(const ns3::TypeId& id);

		/**
		 * @brief Populate and pop the active frame, returning its schema or the callback's error.
		 * @param[in] functor Nullary callable returning @ref vcle::Expected with no value.
		 * @pre A frame pushed by @ref withSchema is active. Any frames pushed by @p functor
		 * must be popped before it returns.
		 *
		 * Pops the frame on both success and a returned error, restoring the previous frame if
		 * present. Stored values remain alive. Exceptions propagate without popping the frame.
		 */
		template <typename F>
		Expected<glz::schema> invoke(F functor) {
			static_assert(std::is_invocable_r_v<Expected<>, F>);

			auto status = functor();
			// pop the context schema
			auto schema = std::move(schemas_.top());
			schemas_.pop();
			if (!status) {
				return std::unexpected(std::move(status.error()));
			}
			return schema;
		}

		/**
		 * @brief Format and retain a string for use by schema fields.
		 * @return A view into the retained string, valid while its backing storage is alive.
		 * @see Context for storage lifetime requirements.
		 */
		template <typename... Args>
		std::string_view store(std::format_string<Args...> fmt, Args&&... args) {
			std::string str = std::format(fmt, std::forward<Args>(args)...);
			// store returns persistent refs, so this is safe
			return store<std::string>(std::move(str));
		}

		/**
		 * @brief Retain a value in stable storage independently of the active schema frame.
		 * @tparam T Copy-constructible type accepted by @c std::any.
		 * @return A reference that remains valid across later insertions and frame removal.
		 *
		 * Storing pointers or views does not extend the lifetime of the data they reference.
		 * Keep stored strings unchanged while schema fields borrow their contents.
		 * @see Context for storage lifetime requirements.
		 */
		template <typename T>
		T& store(T value) {
			registry_.push_back(std::move(value));
			return std::any_cast<T&>(registry_.back());
		}

	private:
		Expected<> walkProperty(ns3::TypeId::AttributeInformation attribute);
		Expected<> walkProperties(const ns3::TypeId& id);

	private:
		std::stack<glz::schema> schemas_;
		std::list<std::any> registry_;
	};

} // namespace vcle::config::detail
