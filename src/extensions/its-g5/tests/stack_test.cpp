#include "src/extensions/its-g5/model/stack.hpp"

#include <concepts>
#include <utility>

namespace vcle::itsg5 {
	static_assert(
		std::same_as<decltype(std::declval<Stack&>().Router()), vanetza::geonet::Router&>
	);
	static_assert(std::same_as<
				  decltype(std::declval<const Stack&>().Router()),
				  const vanetza::geonet::Router&>);
	static_assert(
		std::same_as<decltype(std::declval<Stack&>().Btp()), vanetza::btp::PortDispatcher&>
	);
	static_assert(std::same_as<
				  decltype(std::declval<const Stack&>().Btp()),
				  const vanetza::btp::PortDispatcher&>);
	static_assert(
		std::same_as<decltype(std::declval<Stack&>().Mobility()), ns3::Ptr<ns3::MobilityModel>>
	);
	static_assert(std::same_as<
				  decltype(std::declval<const Stack&>().Mobility()),
				  ns3::Ptr<const ns3::MobilityModel>>);
} // namespace vcle::itsg5

int main() {
	return 0;
}
