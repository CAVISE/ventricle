#pragma once

#include <ventricle/core/error.hpp>

#include <ns3/attribute.h>
#include <ns3/type-id.h>

namespace vcle::config::detail {
	class Context;

	/**
	 * @brief Populate the active property schema from an attribute's initial value and checker.
	 * @param[in,out] ctx Context containing the active property schema and its backing storage.
	 * @param[in] attribute Metadata whose checker describes and serializes @p value.
	 * @param[in] value Borrowed, non-null initial value; no model object is constructed or changed.
	 * @pre @p ctx has an active schema frame and @p attribute has a non-null checker.
	 *
	 * Supports strings, signed and unsigned integers, floating-point numbers, booleans, times,
	 * data rates, and enum values. Times and data rates use their serialized string form; enums
	 * use the checker's named choices. Retains any borrowed strings through @ref Context::store.
	 *
	 * Returns @c std::errc::function_not_supported for other value types. Numeric checker bounds
	 * are not exported, and failure does not roll back changes to the active frame.
	 * @see Context::invoke for discarding a failed frame.
	 */
	Expected<> writeAttributeValue(Context& ctx, const ns3::TypeId::AttributeInformation& attribute, const ns3::AttributeValue* value);

} // namespace vcle::config::detail
