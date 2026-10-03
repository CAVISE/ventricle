#include "src/extensions/dynamic-loader/model/extension.h"

#include "src/extensions/its-g5/helpers/simple_geographic_coordinate_system.hpp"

namespace {
	int registerTypes(VcleRegisterType registrar, void* context) {
		try {
			return registrar(context, vcle::itsg5::SimpleGeographicCoordinateSystem::GetTypeId().GetName().c_str());
		} catch (...) {
			return -1;
		}
	}
} // namespace

VCLE_DECL_BEGIN

const VcleExtensionInfo* vcleGetExtensionInfo() {
	static const VcleExtensionInfo info{VCLE_EXTENSION_API_VERSION, "its-g5", VENTRICLE_VERSION, &registerTypes};
	return &info;
}

VCLE_DECL_END
