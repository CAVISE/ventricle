#pragma once

#include "src/core/type_glue.hpp"

#include <ns3/mac48-address.h>
#include <ns3/nstime.h>
#include <vanetza/common/clock.hpp>
#include <vanetza/net/mac_address.hpp>

namespace vcle {

	template <>
	struct TypeGlueConverter<vanetza::MacAddress, ns3::Mac48Address> {
		static ns3::Mac48Address convert(
			vanetza::MacAddress address
		) {
			ns3::Mac48Address converted;
			converted.CopyFrom(address.octets.data());
			return converted;
		}
	};

	template <>
	struct TypeGlueConverter<ns3::Mac48Address, vanetza::MacAddress> {
		static vanetza::MacAddress convert(
			ns3::Mac48Address address
		) {
			vanetza::MacAddress converted;
			address.CopyTo(converted.octets.data());
			return converted;
		}
	};

	template <>
	struct TypeGlueConverter<vanetza::Clock::duration, ns3::Time> {
		static ns3::Time convert(
			vanetza::Clock::duration duration
		) {
			return ns3::MicroSeconds(duration.count());
		}
	};

	template <>
	struct TypeGlueConverter<ns3::Time, vanetza::Clock::duration> {
		static vanetza::Clock::duration convert(
			const ns3::Time& duration
		) {
			return vanetza::Clock::duration(duration.GetMicroSeconds());
		}
	};

	template <>
	struct TypeGlueConverter<ns3::Time, vanetza::Clock::time_point> {
		static vanetza::Clock::time_point convert(
			const ns3::Time& time
		) {
			return vanetza::Clock::time_point(TypeGlue::convert<vanetza::Clock::duration>(time));
		}
	};

} // namespace vcle
