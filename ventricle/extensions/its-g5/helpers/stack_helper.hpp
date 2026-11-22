#pragma once

#include <ventricle/core/defs.hpp>
#include <ventricle/extensions/its-g5/helpers/geographic_coordinate_system.hpp>
#include <ventricle/extensions/its-g5/model/stack_config.hpp>

#include <memory>
#include <vector>

#include <ns3/node-container.h>
#include <ns3/ptr.h>

namespace vcle::itsg5 {
	class Runtime;
	class Stack;

	class StackHelper {
	public:
		StackHelper(Runtime& runtime, const IGeographicCoordinateSystem& coordinateSystem);

		void SetConfig(const StackConfig& config);
		VCLE_NODISCARD std::vector<std::shared_ptr<Stack>> Install(const ns3::NodeContainer& nodes) const;

	private:
		Runtime& Runtime_;
		const IGeographicCoordinateSystem& CoordinateSystem_;
		StackConfig Config_;
	};
} // namespace vcle::itsg5
