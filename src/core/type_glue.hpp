#pragma once

#include <concepts>
#include <type_traits>
#include <utility>

namespace vcle {

	template <typename From, typename To>
	struct TypeGlueConverter;

	/**
	 * @brief Describes a registered conversion between two ecosystem types.
	 *
	 * A conversion is registered by specializing TypeGlueConverter for a source
	 * and destination type.
	 */
	template <typename From, typename To>
	concept TypeGlueConvertible = requires(From&& from) {
		{ TypeGlueConverter<std::remove_cvref_t<From>, To>::convert(std::forward<From>(from)) } -> std::same_as<To>;
	};

	/**
	 * @brief Provides a uniform entry point for conversions between ecosystems.
	 *
	 * The destination type is the first template argument because callers must
	 * select it explicitly. The source type follows and is normally deduced from
	 * the function argument, keeping calls concise: `convert<Destination>(source)`.
	 */
	class TypeGlue {
	public:
		/** Convert @p from to the explicitly selected destination type. */
		template <typename To, typename From>
			requires TypeGlueConvertible<From, To>
		static To convert(From&& from) {
			return TypeGlueConverter<std::remove_cvref_t<From>, To>::convert(std::forward<From>(from));
		}
	};

} // namespace vcle
