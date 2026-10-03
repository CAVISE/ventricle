#pragma once

#include <cstdint>

#include <vanetza/geonet/address.hpp>
#include <vanetza/geonet/mib.hpp>
#include <vanetza/net/mac_address.hpp>

namespace vcle::itsg5 {

	/** Configuration copied into one installed ITS-G5 stack. */
	struct StackConfig {
		vanetza::MacAddress macAddress_;
		vanetza::geonet::Address geoNetworkingAddress_;
		vanetza::geonet::MIB geoNetworking_;
		std::uint_fast32_t randomSeed_ = 0;
	};

} // namespace vcle::itsg5
