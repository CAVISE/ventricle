#pragma once

#include "src/core/defs.hpp"

#include <string>
#include <tuple>
#include <vector>

#include <ns3/abort.h>

#include <libsumo/libtraci.h>

namespace vcle::sumo {

	/** Maps a native C++ value to its libsumo result wrapper. */
	template <typename Value>
	struct TraciResultTraits;

	template <>
	struct TraciResultTraits<double> {
		using Result = libsumo::TraCIDouble;
		static double get(const Result& result) {
			return result.value;
		}
	};

	template <>
	struct TraciResultTraits<int> {
		using Result = libsumo::TraCIInt;
		static int get(const Result& result) {
			return result.value;
		}
	};

	template <>
	struct TraciResultTraits<std::string> {
		using Result = libsumo::TraCIString;
		static const std::string& get(const Result& result) {
			return result.value;
		}
	};

	template <>
	struct TraciResultTraits<std::vector<std::string>> {
		using Result = libsumo::TraCIStringList;
		static const std::vector<std::string>& get(const Result& result) {
			return result.value;
		}
	};

	template <>
	struct TraciResultTraits<libsumo::TraCIPosition> {
		using Result = libsumo::TraCIPosition;
		static const libsumo::TraCIPosition& get(const Result& result) {
			return result;
		}
	};

	template <>
	struct TraciResultTraits<libsumo::TraCIColor> {
		using Result = libsumo::TraCIColor;
		static const libsumo::TraCIColor& get(const Result& result) {
			return result;
		}
	};

	/** Maps a TraCI variable identifier to its native and wrapped result types. */
	template <int Variable>
	struct TraciVariableTraits;

	template <>
	struct TraciVariableTraits<libsumo::VAR_SPEED> : TraciResultTraits<double> {};
	template <>
	struct TraciVariableTraits<libsumo::VAR_POSITION> : TraciResultTraits<libsumo::TraCIPosition> {};
	template <>
	struct TraciVariableTraits<libsumo::VAR_ANGLE> : TraciResultTraits<double> {};
	template <>
	struct TraciVariableTraits<libsumo::VAR_MAXSPEED> : TraciResultTraits<double> {};
	template <>
	struct TraciVariableTraits<libsumo::VAR_TYPE> : TraciResultTraits<std::string> {};
	template <>
	struct TraciVariableTraits<libsumo::VAR_VEHICLECLASS> : TraciResultTraits<std::string> {};
	template <>
	struct TraciVariableTraits<libsumo::VAR_LENGTH> : TraciResultTraits<double> {};
	template <>
	struct TraciVariableTraits<libsumo::VAR_WIDTH> : TraciResultTraits<double> {};
	template <>
	struct TraciVariableTraits<libsumo::VAR_SIGNALS> : TraciResultTraits<int> {};
	template <>
	struct TraciVariableTraits<libsumo::VAR_ARRIVED_VEHICLES_IDS> : TraciResultTraits<std::vector<std::string>> {};
	template <>
	struct TraciVariableTraits<libsumo::VAR_DEPARTED_VEHICLES_IDS> : TraciResultTraits<std::vector<std::string>> {};
	template <>
	struct TraciVariableTraits<libsumo::VAR_TELEPORT_STARTING_VEHICLES_IDS> : TraciResultTraits<std::vector<std::string>> {};
	template <>
	struct TraciVariableTraits<libsumo::VAR_ARRIVED_PERSONS_IDS> : TraciResultTraits<std::vector<std::string>> {};
	template <>
	struct TraciVariableTraits<libsumo::VAR_DEPARTED_PERSONS_IDS> : TraciResultTraits<std::vector<std::string>> {};
	template <>
	struct TraciVariableTraits<libsumo::VAR_DELTA_T> : TraciResultTraits<double> {};
	template <>
	struct TraciVariableTraits<libsumo::VAR_TIME> : TraciResultTraits<double> {};
	template <>
	struct TraciVariableTraits<libsumo::VAR_TIME_STEP> : TraciResultTraits<int> {};

	/** Non-owning typed view of values returned for one TraCI step. */
	class TraciValues {
	public:
		explicit TraciValues(const libsumo::TraCIResults& values)
			: values_(values) {
		}

		/** Return a typed result wrapper, or `nullptr` when absent or of another type. */
		template <int Variable>
		VCLE_NODISCARD const typename TraciVariableTraits<Variable>::Result* find() const {
			const auto value = values_.find(Variable);
			if (value == values_.end()) {
				return nullptr;
			}
			return dynamic_cast<const typename TraciVariableTraits<Variable>::Result*>(value->second.get());
		}

		/** Return an unwrapped typed value; absence or a type mismatch is a contract error. */
		template <int Variable>
		VCLE_NODISCARD decltype(auto) get() const {
			const auto* value = find<Variable>();
			NS_ABORT_MSG_UNLESS(value, "TraCI variable is absent or has an unexpected type");
			return TraciVariableTraits<Variable>::get(*value);
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
