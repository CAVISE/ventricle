#include "src/extensions/its-g5/helpers/position_conversion.hpp"

#include <cmath>
#include <numbers>
#include <vanetza/common/confident_quantity.hpp>
#include <vanetza/units/angle.hpp>
#include <vanetza/units/length.hpp>
#include <vanetza/units/velocity.hpp>

namespace vcle::itsg5 {
	vanetza::PositionFix ToPositionFix(const ns3::MobilityModel& mobility,
									   const IGeographicCoordinateSystem& coordinateSystem,
									   vanetza::Clock::time_point timestamp) {
		const auto position = coordinateSystem.ToGeographic(mobility.GetPosition());
		const auto velocity = mobility.GetVelocity();
		auto heading = std::atan2(velocity.x, velocity.y) * 180.0 / std::numbers::pi;
		if (heading < 0.0) {
			heading += 360.0;
		}

		vanetza::PositionFix fix;
		fix.timestamp = timestamp;
		fix.latitude = position.x * vanetza::units::degree;
		fix.longitude = position.y * vanetza::units::degree;
		fix.course = vanetza::ConfidentQuantity<vanetza::units::TrueNorth>(
			vanetza::units::TrueNorth::from_value(heading));
		fix.speed = vanetza::ConfidentQuantity<vanetza::units::Velocity>(
			vanetza::units::Velocity::from_value(std::hypot(velocity.x, velocity.y)));
		fix.altitude = vanetza::ConfidentQuantity<vanetza::units::Length>(
			vanetza::units::Length::from_value(position.z));
		return fix;
	}
} // namespace vcle::itsg5
