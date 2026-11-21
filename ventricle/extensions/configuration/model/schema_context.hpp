#pragma once

#include <ventricle/core/error.hpp>

#include <any>
#include <format>
#include <glaze/json/schema.hpp>
#include <list>
#include <ns3/type-id.h>
#include <stack>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace vcle::config::detail {
	/// @brief Holds schema traversal state and storage backing schema string views.
	class Context {
	public:
		// Sets a new scheme for this context
		Context& withSchema(glz::schema schema);

		// Access the current schema object.
		glz::schema& schema();

		/// @brief Populate the current schema from a TypeId and its inherited attributes.
		Expected<> schemaForID(const ns3::TypeId& id);

		template <typename F>
		Expected<glz::schema> invoke(
			F functor
		) {
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

		// Formats and stores strings. Helper for common use in schema descriptive fields.
		template <typename... Args>
		std::string_view store(
			std::format_string<Args...> fmt, Args&&... args
		) {
			std::string str = std::format(fmt, std::forward<Args>(args)...);
			// store returns persistent refs, so this is safe
			return store<std::string>(std::move(str));
		}

		template <typename T>
		T& store(
			T value
		) {
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
