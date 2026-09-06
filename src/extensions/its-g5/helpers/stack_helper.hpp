#pragma once

#include "src/core/defs.hpp"
#include "src/extensions/its-g5/helpers/geographic_coordinate_system.hpp"
#include "src/extensions/its-g5/model/stack_config.hpp"

#include <memory>
#include <vector>

#include <absl/status/statusor.h>

#include <ns3/node-container.h>
#include <ns3/ptr.h>

namespace vcle::itsg5 {
	class Runtime;
	class Stack;

	/** Configures and installs ITS-G5 stacks on ns-3 nodes. */
	class StackHelper {
	public:
		/** Bind stack installations to @p runtime and @p coordinateSystem. */
		StackHelper(Runtime& runtime, const IGeographicCoordinateSystem& coordinateSystem);

		/** Set the configuration copied into subsequently installed stacks. */
		void setConfig(const StackConfig& config);
		/** Install one ITS-G5 stack on every node in @p nodes. */
		absl::StatusOr<std::vector<std::shared_ptr<Stack>>> install(const ns3::NodeContainer& nodes) const;

	private:
		Runtime& runtime_;
		const IGeographicCoordinateSystem& coordinateSystem_;
		StackConfig config_;
	};
} // namespace vcle::itsg5
