#include "src/extensions/dynamic-loader/model/extension.h"

#include <ns3/object.h>

#ifndef NULL_DESCRIPTOR
namespace {
	int registerTypes(VcleRegisterType registrar, void* context) {
		try {
			static const auto type = ns3::TypeId("loader-test::Component").SetParent<ns3::Object>();
			return registrar(context, type.GetName().c_str());
		} catch (...) {
			return -1;
		}
	}
} // namespace
#endif

VCLE_DECL_BEGIN

const VcleExtensionInfo* vcleGetExtensionInfo() {
#ifdef NULL_DESCRIPTOR
	return nullptr;
#else
	static const VcleExtensionInfo info{VCLE_EXTENSION_API_VERSION, "loader-test", "1", &registerTypes};
	return &info;
#endif
}

VCLE_DECL_END
