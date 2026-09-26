#pragma once

#define VCLE_NODISCARD [[nodiscard]]
#define VCLE_UNUSED [[maybe_unused]]

#ifdef __cplusplus
#define VCLE_DECL_BEGIN extern "C" {
#define VCLE_DECL_END }
#else
#define VCLE_DECL_BEGIN
#define VCLE_DECL_END
#endif
