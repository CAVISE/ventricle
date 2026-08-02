#include "src/extensions/its-g5/helpers/stack_helper.hpp"

#include "src/extensions/its-g5/helpers/type_glue.hpp"
#include "src/extensions/its-g5/model/runtime.hpp"
#include "src/extensions/its-g5/model/stack.hpp"

#include <ns3/abort.h>
#include <ns3/mobility-model.h>
#include <ns3/node.h>
#include <utility>

namespace vcle::itsg5 {
	StackHelper::StackHelper(
		Runtime& runtime, const IGeographicCoordinateSystem& coordinateSystem
	)
		: Runtime_(runtime)
		, CoordinateSystem_(coordinateSystem) {
	}

	void StackHelper::SetConfig(
		const StackConfig& config
	) {
		Config_ = config;
	}

	std::vector<std::shared_ptr<Stack>> StackHelper::Install(
		const ns3::NodeContainer& nodes
	) const {
		std::vector<std::shared_ptr<Stack>> stacks;
		stacks.reserve(nodes.GetN());

		for (auto it = nodes.Begin(); it != nodes.End(); ++it) {
			const auto& node = *it;
			NS_ABORT_MSG_IF(node->GetNDevices() == 0, "ITS-G5 node has no network device");

			const auto device = node->GetDevice(0);
			const auto mobility = node->GetObject<ns3::MobilityModel>();
			NS_ABORT_MSG_UNLESS(
				ns3::Mac48Address::IsMatchingType(device->GetAddress()),
				"ITS-G5 network device has no 48-bit MAC address"
			);

			auto config = Config_;
			config.MacAddress = TypeGlue::convert<vanetza::MacAddress>(
				ns3::Mac48Address::ConvertFrom(device->GetAddress())
			);
			config.GeoNetworkingAddress.mid(config.MacAddress);
			config.RandomSeed += node->GetId();
			stacks.push_back(
				std::make_shared<
					Stack>(std::move(config), Runtime_, device, mobility, CoordinateSystem_)
			);
		}

		return stacks;
	}
} // namespace vcle::itsg5
