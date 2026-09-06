#pragma once

#include "src/core/defs.hpp"
#include "src/extensions/its-g5/helpers/type_glue.hpp"

#include <ns3/vector.h>

namespace vcle::itsg5 {
	/**
	 * Geographic coordinates stored as latitude, longitude, and altitude in
	 * `x`, `y`, and `z`, respectively. Angles are in degrees and altitude is in
	 * metres.
	 */
	using GeographicPosition = ns3::Vector3D;

	/**
	 * @brief Converts between ns-3 Cartesian and geographic coordinates.
	 *
	 * Implementations are injected into consumers so scenarios can select their
	 * projection.
	 */
	class IGeographicCoordinateSystem {
	public:
		virtual ~IGeographicCoordinateSystem() = default;

		/** Convert an ns-3 Cartesian @p position to geographic coordinates. */
		VCLE_NODISCARD virtual GeographicPosition toGeographic(const ns3::Vector& position) const = 0;
		/** Convert a geographic @p position to ns-3 Cartesian coordinates. */
		VCLE_NODISCARD virtual ns3::Vector toCartesian(const GeographicPosition& position) const = 0;
	};
} // namespace vcle::itsg5
