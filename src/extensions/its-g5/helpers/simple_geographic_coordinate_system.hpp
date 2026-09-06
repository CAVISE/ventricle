#pragma once

#include "src/extensions/its-g5/helpers/geographic_coordinate_system.hpp"

#include <ns3/geographic-positions.h>
#include <ns3/object.h>

namespace vcle::itsg5 {

	/**
	 * @brief Earth-referenced coordinate system backed by ns-3 geodesy utilities.
	 *
	 * Cartesian coordinates use the ns-3 topocentric convention: X points east,
	 * Y points north, and Z points up from the configured geographic origin. The
	 * `EarthSpheroid` ns-3 attribute selects the Earth model used for conversion.
	 * Refer to ns3 documentation on differences.
	 */
	class SimpleGeographicCoordinateSystem final : public ns3::Object, public IGeographicCoordinateSystem {
	public:
		/* ns3 runtime configuration. */
		static ns3::TypeId GetTypeId();

		/** Construct a coordinate system centered at @p origin. */
		explicit SimpleGeographicCoordinateSystem(GeographicPosition origin);

		/* IGeographicCoordinateSystem implementation*/
		GeographicPosition toGeographic(const ns3::Vector& position) const override;
		ns3::Vector toCartesian(const GeographicPosition& position) const override;

	private:
		ns3::Vector origin_;
		ns3::GeographicPositions::EarthSpheroidType spheroid_ = ns3::GeographicPositions::WGS84;
	};

} // namespace vcle::itsg5
