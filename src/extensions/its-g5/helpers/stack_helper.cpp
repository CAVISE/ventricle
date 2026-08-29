#include "src/extensions/its-g5/helpers/stack_helper.hpp"

#include "src/extensions/its-g5/helpers/type_glue.hpp"
#include "src/extensions/its-g5/model/runtime.hpp"
#include "src/extensions/its-g5/model/stack.hpp"

#include <utility>

#include <ns3/mobility-model.h>
#include <ns3/node.h>

namespace vcle::itsg5 {
	StackHelper::StackHelper(Runtime& runtime, const IGeographicCoordinateSystem& coordinateSystem)
		: runtime_(runtime)
		, coordinateSystem_(coordinateSystem) {
	}

	void StackHelper::setConfig(const StackConfig& config) {
		config_ = config;
	}

	absl::StatusOr<std::vector<std::shared_ptr<Stack>>> StackHelper::install(const ns3::NodeContainer& nodes) const {
		std::vector<std::shared_ptr<Stack>> stacks;
		stacks.reserve(nodes.GetN());

		for (auto it = nodes.Begin(); it != nodes.End(); ++it) {
			const auto& node = *it;
			if (!node) {
				return absl::InvalidArgumentError("ITS-G5 installation received a null node");
			}
			if (node->GetNDevices() == 0) {
				return absl::FailedPreconditionError("ITS-G5 node has no network device");
			}

			const auto device = node->GetDevice(0);
			const auto mobility = node->GetObject<ns3::MobilityModel>();
			if (!ns3::Mac48Address::IsMatchingType(device->GetAddress())) {
				return absl::FailedPreconditionError("ITS-G5 network device has no 48-bit MAC address");
			}
			if (!mobility) {
				return absl::FailedPreconditionError("ITS-G5 node has no mobility model");
			}

			auto config = config_;
			config.macAddress_ = TypeGlue::convert<vanetza::MacAddress>(ns3::Mac48Address::ConvertFrom(device->GetAddress()));
			config.geoNetworkingAddress_.mid(config.macAddress_);
			config.randomSeed_ += node->GetId();
			stacks.push_back(std::make_shared<Stack>(std::move(config), runtime_, device, mobility, coordinateSystem_));
		}

		return stacks;
	}
} // namespace vcle::itsg5
