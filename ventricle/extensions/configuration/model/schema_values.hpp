#pragma once

#include <ventricle/core/error.hpp>

#include <ns3/attribute.h>
#include <ns3/type-id.h>

namespace vcle::config::detail {
	class Context;

	/// @brief Emit the current property's schema using the attribute value and its checker.
	Expected<> writeAttributeValue(
		Context& ctx,
		const ns3::TypeId::AttributeInformation& attribute,
		const ns3::AttributeValue* value
	);

} // namespace vcle::config::detail
