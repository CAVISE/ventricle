#include "src/extensions/its-g5/helpers/simple_geographic_coordinate_system.hpp"

#include <ns3/enum.h>
#include <ns3/geographic-positions.h>
#include <ns3/test.h>

namespace vcle::itsg5 {
	namespace {
		class GeographicOriginTest final : public ns3::TestCase {
		public:
			GeographicOriginTest()
				: TestCase("maps the geographic origin to Cartesian zero") {
			}

		private:
			/* ns3::TestCase implementation*/
			void DoRun() override {
				const SimpleGeographicCoordinateSystem coordinateSystem({48.0, 11.0, 500.0});

				const auto cartesian = coordinateSystem.toCartesian({48.0, 11.0, 500.0});
				const auto geographic = coordinateSystem.toGeographic({0.0, 0.0, 0.0});

				NS_TEST_EXPECT_MSG_EQ_TOL(cartesian.x, 0.0, 1e-6, "origin has an east offset");
				NS_TEST_EXPECT_MSG_EQ_TOL(cartesian.y, 0.0, 1e-6, "origin has a north offset");
				NS_TEST_EXPECT_MSG_EQ_TOL(cartesian.z, 0.0, 1e-6, "origin has an altitude offset");
				NS_TEST_EXPECT_MSG_EQ_TOL(geographic.x, 48.0, 1e-9, "zero has the wrong latitude");
				NS_TEST_EXPECT_MSG_EQ_TOL(geographic.y, 11.0, 1e-9, "zero has the wrong longitude");
				NS_TEST_EXPECT_MSG_EQ_TOL(geographic.z, 500.0, 1.0, "zero has the wrong altitude");
			}
		};

		class SpheroidAttributeTest final : public ns3::TestCase {
		public:
			SpheroidAttributeTest()
				: TestCase("uses the spheroid selected by attribute") {
			}

		private:
			/* ns3::TestCase implementation*/
			void DoRun() override {
				auto coordinateSystem = ns3::CreateObject<SimpleGeographicCoordinateSystem>(GeographicPosition{48.0, 11.0, 500.0});
				coordinateSystem->SetAttribute("EarthSpheroid", ns3::EnumValue(ns3::GeographicPositions::SPHERE));
				const auto sphere = coordinateSystem->toGeographic({100000.0, 0.0, 0.0});

				coordinateSystem->SetAttribute("EarthSpheroid", ns3::EnumValue(ns3::GeographicPositions::WGS84));
				const auto wgs84 = coordinateSystem->toGeographic({100000.0, 0.0, 0.0});

				NS_TEST_EXPECT_MSG_NE(sphere.y, wgs84.y, "changing EarthSpheroid did not change the projection");
			}
		};

		class GeographicCoordinateSystemTestSuite final : public ns3::TestSuite {
		public:
			GeographicCoordinateSystemTestSuite()
				: TestSuite("ventricle-geographic-coordinate-system", Type::UNIT) {
				AddTestCase(new GeographicOriginTest(), TestCase::Duration::QUICK);
				AddTestCase(new SpheroidAttributeTest(), TestCase::Duration::QUICK);
			}
		};

		// ns-3 discovers test suites through static registration.
		// NOLINTNEXTLINE(bugprone-throwing-static-initialization)
		GeographicCoordinateSystemTestSuite GeographicCoordinateSystemTests;
	} // namespace
} // namespace vcle::itsg5
