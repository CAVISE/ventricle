#include "src/extensions/dynamic-loader/model/extension.h"

#include "src/extensions/sumo/model/controller.hpp"

namespace {
	int registerTypes(VcleRegisterType registrar, void* context) {
		try {
			return registrar(context, vcle::sumo::Controller::GetTypeId().GetName().c_str());
		} catch (...) {
			return -1;
		}
	}
} // namespace

VCLE_DECL_BEGIN

const VcleExtensionInfo* vcleGetExtensionInfo() {
	static const VcleExtensionInfo info{VCLE_EXTENSION_API_VERSION, "sumo", VENTRICLE_VERSION, &registerTypes};
	return &info;
}

VCLE_DECL_END
