#include "src/extensions/its-g5/model/stack.hpp"

#include <concepts>
#include <utility>

#include <ns3/simulator.h>

namespace vcle::itsg5 {
	static_assert(std::same_as<decltype(std::declval<Stack&>().router()), vanetza::geonet::Router&>);
	static_assert(std::same_as<decltype(std::declval<const Stack&>().router()), const vanetza::geonet::Router&>);
	static_assert(std::same_as<decltype(std::declval<Stack&>().btp()), vanetza::btp::PortDispatcher&>);
	static_assert(std::same_as<decltype(std::declval<const Stack&>().btp()), const vanetza::btp::PortDispatcher&>);
	static_assert(std::same_as<decltype(std::declval<Stack&>().mobility()), ns3::Ptr<ns3::MobilityModel>>);
	static_assert(std::same_as<decltype(std::declval<const Stack&>().mobility()), ns3::Ptr<const ns3::MobilityModel>>);
} // namespace vcle::itsg5

int main() {
	// Clear tracked Time objects before ns-3's static mutex is destroyed.
	ns3::Simulator::Run();
	ns3::Simulator::Destroy();
	return 0;
}
