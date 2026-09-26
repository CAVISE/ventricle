#include "src/extensions/its-g5/helpers/position_conversion.hpp"

#include <ns3/constant-velocity-mobility-model.h>
#include <ns3/test.h>

#include <vanetza/units/angle.hpp>

namespace vcle::itsg5 {
	namespace {
		class FixedCoordinateSystem final : public IGeographicCoordinateSystem {
		public:
			/* IGeographicCoordinateSystem implementation*/
			VCLE_NODISCARD GeographicPosition toGeographic(const ns3::Vector&) const override {
				return {48.0, 11.0, 505.0};
			}

			VCLE_NODISCARD ns3::Vector toCartesian(const GeographicPosition&) const override {
				return {1.0, 2.0, 3.0};
			}
		};

		class PositionConversionTest final : public ns3::TestCase {
		public:
			PositionConversionTest()
				: TestCase("maps the scenario origin to geographic coordinates") {
			}

		private:
			/* ns3::TestCase implementation*/
			void DoRun() override {
				const auto mobility = ns3::CreateObject<ns3::ConstantVelocityMobilityModel>();
				mobility->SetPosition({0.0, 0.0, 5.0});
				mobility->SetVelocity({3.0, 4.0, 0.0});
				const FixedCoordinateSystem coordinateSystem;

				const auto fix = toPositionFix(*mobility, coordinateSystem, vanetza::Clock::time_point{});

				NS_TEST_EXPECT_MSG_EQ_TOL(fix.latitude.value(), 48.0, 1e-12, "origin latitude changed");
				NS_TEST_EXPECT_MSG_EQ_TOL(fix.longitude.value(), 11.0, 1e-12, "origin longitude changed");
				NS_TEST_EXPECT_MSG_EQ_TOL(fix.speed.value().value(), 5.0, 1e-12, "velocity magnitude changed");
				NS_TEST_EXPECT_MSG_EQ_TOL(fix.altitude->value().value(), 505.0, 1e-12, "altitude offset changed");
			}
		};

		class PositionConversionTestSuite final : public ns3::TestSuite {
		public:
			PositionConversionTestSuite()
				: TestSuite("ventricle-its-g5-position", Type::UNIT) {
				AddTestCase(new PositionConversionTest(), TestCase::Duration::QUICK);
			}
		};

		// ns-3 discovers test suites through static registration.
		// NOLINTNEXTLINE(bugprone-throwing-static-initialization)
		PositionConversionTestSuite PositionConversionTests;
	} // namespace
} // namespace vcle::itsg5
