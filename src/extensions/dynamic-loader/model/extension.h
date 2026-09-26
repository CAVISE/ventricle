#pragma once

#include "src/core/defs.hpp"

#include <stdint.h>

VCLE_DECL_BEGIN

#define VCLE_EXTENSION_API_VERSION 1
#define VCLE_EXTENSION_ENTRY_SYMBOL "vcleGetExtensionInfo"

/**
 * Report a public TypeId by its registered name. Returns zero on success.
 * The host consumes the name during the call; context and callback must not be retained.
 * The extension must register the TypeId with ns-3 before reporting its name.
 */
typedef int (*VcleRegisterType)(void* context, const char* name);

/**
 * C-compatible extension descriptor. Strings and descriptor live until process exit.
 * Registration runs synchronously, returns zero on success, and must propagate a
 * nonzero host callback result. No exception may cross this interface.
 * ns-3 components still require the same ns-3 runtime and compatible C++ ABI.
 */
typedef struct VcleExtensionInfo {
	uint32_t apiVersion;
	const char* name;
	const char* version;
	int (*registerTypes)(VcleRegisterType registrar, void* context);
} VcleExtensionInfo;

/** Entry point returns library-owned metadata; the host checks apiVersion before registration. */
typedef const VcleExtensionInfo* (*VcleExtensionEntry)(void);
const VcleExtensionInfo* vcleGetExtensionInfo(void);

VCLE_DECL_END
