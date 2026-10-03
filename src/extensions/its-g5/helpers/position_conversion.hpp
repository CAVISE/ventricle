#pragma once

#include "src/extensions/its-g5/helpers/geographic_coordinate_system.hpp"

#include <ns3/mobility-model.h>

#include <vanetza/common/position_fix.hpp>

namespace vcle::itsg5 {
	/** Convert ns-3 mobility data into a timestamped Vanetza position fix. */
	vanetza::PositionFix toPositionFix(
		const ns3::MobilityModel& mobility, const IGeographicCoordinateSystem& coordinateSystem, vanetza::Clock::time_point timestamp
	);
} // namespace vcle::itsg5
