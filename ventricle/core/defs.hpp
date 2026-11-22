#pragma once

/**
 * @file
 * @brief Shared compiler attribute macros for consistent annotations across project code.
 */

/// @brief Requests diagnostics for discarded results using the project's common annotation.
#define VCLE_NODISCARD [[nodiscard]]

/// @brief Suppresses unused-declaration warnings using the project's common annotation.
#define VCLE_MAYBE_UNUSED [[maybe_unused]]
