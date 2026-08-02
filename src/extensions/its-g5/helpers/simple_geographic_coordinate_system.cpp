#include "src/extensions/its-g5/helpers/simple_geographic_coordinate_system.hpp"

#include <ns3/enum.h>

using namespace vcle::itsg5;
using Conv = ns3::GeographicPositions;

NS_OBJECT_ENSURE_REGISTERED(SimpleGeographicCoordinateSystem);

ns3::TypeId SimpleGeographicCoordinateSystem::GetTypeId() {
	static ns3::TypeId typeId =
		ns3::TypeId("vcle::itsg5::SimpleGeographicCoordinateSystem")
			.SetParent<ns3::Object>()
			.SetGroupName("Mobility")
			.AddAttribute(
				"EarthSpheroid",
				"Earth spheroid used for geographic conversions",
				ns3::EnumValue(ns3::GeographicPositions::WGS84),
				ns3::MakeEnumAccessor<ns3::GeographicPositions::EarthSpheroidType>(
					&SimpleGeographicCoordinateSystem::Spheroid_
				),
				ns3::MakeEnumChecker(
					ns3::GeographicPositions::SPHERE,
					"Sphere",
					ns3::GeographicPositions::GRS80,
					"GRS80",
					ns3::GeographicPositions::WGS84,
					"WGS84"
				)
			);
	return typeId;
}

SimpleGeographicCoordinateSystem::SimpleGeographicCoordinateSystem(
	GeographicPosition origin
)
	: Origin_(origin) {
}

GeographicPosition SimpleGeographicCoordinateSystem::ToGeographic(
	const ns3::Vector& position
) const {
	const ns3::Vector geographic =
		Conv::TopocentricToGeographicCoordinates(position, Origin_, Spheroid_);
	return geographic;
}

ns3::Vector SimpleGeographicCoordinateSystem::ToCartesian(
	const GeographicPosition& position
) const {
	return Conv::GeographicToTopocentricCoordinates(position, Origin_, Spheroid_);
}
