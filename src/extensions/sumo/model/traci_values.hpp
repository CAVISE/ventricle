#pragma once

#include "src/core/defs.hpp"

#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include <ns3/abort.h>

#include <libsumo/libtraci.h>

namespace vcle::sumo {

	namespace detail {

		/**
		 * @brief Extract a native value from a libsumo result.
		 * @tparam ResultType Wrapped libsumo result type.
		 * @tparam ReturnType Native return type, including reference qualifiers where needed.
		 */
		template <typename ResultType, typename ReturnType>
		struct TraciResultTraits {
			/** @brief Unwrapped get() return type, such as double or const std::string&. */
			using Value = ReturnType;
			/** @brief Stored libsumo wrapper type, such as TraCIDouble or TraCIString. */
			using Result = ResultType;

			/** @brief Unwrap the value or return the result itself for position and color types. */
			static ReturnType get(const Result& result) {
				if constexpr (std::is_same_v<Result, std::remove_cvref_t<ReturnType>>) {
					return result;
				} else {
					return result.value;
				}
			}
		};

		using DoubleResult = TraciResultTraits<libsumo::TraCIDouble, double>;
		using IntResult = TraciResultTraits<libsumo::TraCIInt, int>;
		using StringResult = TraciResultTraits<libsumo::TraCIString, const std::string&>;
		using StringListResult = TraciResultTraits<libsumo::TraCIStringList, const std::vector<std::string>&>;
		using PositionResult = TraciResultTraits<libsumo::TraCIPosition, const libsumo::TraCIPosition&>;
		using ColorResult = TraciResultTraits<libsumo::TraCIColor, const libsumo::TraCIColor&>;

		/** @brief Test whether a variable belongs to a group of identifiers. */
		template <int Variable, int... Candidates>
		inline constexpr bool match = ((Variable == Candidates) || ...);

		/** @brief Select result traits at compile time, rejecting unsupported variable identifiers. */
		template <int Variable>
		consteval auto matchResultTypeByVariable() {
			/* clang-format off */
			if constexpr (match<
				Variable,
				libsumo::VAR_SPEED,
				libsumo::VAR_ANGLE,
				libsumo::VAR_MAXSPEED,
				libsumo::VAR_LENGTH,
				libsumo::VAR_WIDTH,
				libsumo::VAR_DELTA_T,
				libsumo::VAR_TIME
			>) {
				return std::type_identity<DoubleResult>();
			} else if constexpr (match<
				Variable,
				libsumo::VAR_SIGNALS,
				libsumo::VAR_TIME_STEP
			>) {
				return std::type_identity<IntResult>();
			} else if constexpr (match<
				Variable,
				libsumo::VAR_TYPE,
				libsumo::VAR_VEHICLECLASS
			>) {
				return std::type_identity<StringResult>();
			} else if constexpr (match<
				Variable,
				libsumo::VAR_POSITION
			>) {
				return std::type_identity<PositionResult>();
			} else if constexpr (match<
				Variable,
				libsumo::VAR_ARRIVED_VEHICLES_IDS,
				libsumo::VAR_DEPARTED_VEHICLES_IDS,
				libsumo::VAR_TELEPORT_STARTING_VEHICLES_IDS,
				libsumo::VAR_ARRIVED_PERSONS_IDS,
				libsumo::VAR_DEPARTED_PERSONS_IDS
			>) {
				return std::type_identity<StringListResult>();
			} else {
				static_assert(match<Variable>, "Unsupported TraCI variable");
			}
			/* clang-format on */
		}

		/** @brief Map a TraCI variable identifier to its result traits. */
		template <int Variable>
		using TraciVariableTraits = typename decltype(matchResultTypeByVariable<Variable>())::type;

		/** @brief Unwrapped value returned by get(), preserving reference qualifiers. */
		template <int Variable>
		using TraciVariableTraitsReturnType = typename TraciVariableTraits<Variable>::Value;

		/** @brief Non-owning pointer to the stored libsumo result wrapper returned by find(). */
		template <int Variable>
		using TraciVariableResultPointer = const typename TraciVariableTraits<Variable>::Result*;

	} // namespace detail

	/** Non-owning typed view of values returned for one TraCI step. */
	class TraciValues {
	public:
		explicit TraciValues(const libsumo::TraCIResults& values)
			: values_(values) {
		}

		/** @brief Return a typed result wrapper, or nullptr when absent; a type mismatch is a contract error. */
		template <int Variable>
		VCLE_NODISCARD detail::TraciVariableResultPointer<Variable> find() const {
			if (const auto value = values_.find(Variable); value == values_.end()) {
				return nullptr;
			} else if (auto variable = dynamic_cast<detail::TraciVariableResultPointer<Variable>>(value->second.get()); !variable) {
				NS_ABORT_MSG("variable returned by TraCI has different type than expected. This means this header is invalid; please update this");
			} else {
				return variable;
			}
		}

		/** Return an unwrapped typed value; absence or a type mismatch is a contract error. */
		template <int Variable>
		VCLE_NODISCARD detail::TraciVariableTraitsReturnType<Variable> get() const {
			if (const auto* value = find<Variable>(); value) {
				return detail::TraciVariableTraits<Variable>::get(*value);
			}

			NS_ABORT_MSG("TraCI variable is absent");
		}

		/** Return several typed values as a tuple suitable for structured binding. */
		template <int... Variables>
		VCLE_NODISCARD auto getMultiple() const {
			return std::tuple<decltype(get<Variables>())...>(get<Variables>()...);
		}

	private:
		const libsumo::TraCIResults& values_;
	};

} // namespace vcle::sumo
