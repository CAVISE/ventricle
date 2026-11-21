#pragma once

#include <cstdint>

#include <vanetza/geonet/address.hpp>
#include <vanetza/geonet/mib.hpp>
#include <vanetza/net/mac_address.hpp>

namespace vcle::itsg5 {

	struct StackConfig {
		vanetza::MacAddress MacAddress;
		vanetza::geonet::Address GeoNetworkingAddress;
		vanetza::geonet::MIB GeoNetworking;
		std::uint_fast32_t RandomSeed = 0;
	};

} // namespace vcle::itsg5
