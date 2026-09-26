#include "src/extensions/dynamic-loader/model/extension.h"

#ifndef BAD_VERSION
static int registerTypes(VcleRegisterType registrar, void* context) {
#ifdef UNKNOWN_TYPE
	return registrar(context, "loader-test::Unknown");
#else
	return registrar(context, "ns3::Object");
#endif
}
#endif

const VcleExtensionInfo* vcleGetExtensionInfo(void) {
#ifdef BAD_VERSION
	static const VcleExtensionInfo info = {VCLE_EXTENSION_API_VERSION + 1, 0, 0, 0};
#else
	static const VcleExtensionInfo info = {VCLE_EXTENSION_API_VERSION, "c-extension", "1", registerTypes};
#endif
	return &info;
}
