#pragma once

#include <ventricle/core/error.hpp>

#include <span>
#include <string>

#include <ns3/type-id.h>

namespace vcle::config {

	/**
	 * @brief Generate a JSON Schema describing explicitly exposed ns-3 types.
	 * @param[in] types Unique, initialized type identifiers to expose as configurable objects.
	 * @return Serialized schema JSON, or an error if a type, attribute, or serialization step
	 * cannot be represented.
	 *
	 * Each type accepts an attribute object keyed by its registered name. Arbitrary entry names
	 * instead accept objects containing a required @ref identifiers::typeId alongside the
	 * attribute values. A registered type name may also be used as a named entry.
	 * The collections listed in
	 * @ref detail::holderNames expose the same entry forms without recursively nesting holders.
	 *
	 * Includes inherited attributes that support construction and have setters, using their
	 * original initial values as defaults without constructing model objects. Obsolete attributes
	 * are omitted and deprecated attributes are marked. Attribute objects reject unknown fields.
	 * Only the supplied types are exposed; other registered types are not discovered automatically.
	 *
	 * Duplicate or uninitialized type identifiers, hidden types, duplicate attribute names, and
	 * unsupported attribute values produce errors. Generation does not parse configuration,
	 * resolve identifiers, or validate documents. Scalar checker bounds are not exported.
	 * @see detail::writeAttributeValue for supported attribute values.
	 */
	Expected<std::string> generateSchema(std::span<const ns3::TypeId> types);

} // namespace vcle::config
