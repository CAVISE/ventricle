#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <ns3/type-id.h>

namespace vcle {

	/** Metadata for one effective attribute, retaining its declaring type and checker. */
	struct AttributeDescription {
		std::string declaringType;
		ns3::TypeId::AttributeInformation attribute;
	};

	/** An explicitly selected type and its inherited attributes, sorted by name. */
	struct TypeDescription {
		std::string name;
		std::string parent;
		std::vector<AttributeDescription> attributes;
	};

	/** Inspect metadata without constructing objects or changing registered defaults. */
	TypeDescription inspectType(ns3::TypeId type);

} // namespace vcle
