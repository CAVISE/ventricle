#include "src/core/simulation/type_description.hpp"

#include <map>
#include <utility>

using namespace vcle;

TypeDescription vcle::inspectType(ns3::TypeId type) {
	TypeDescription description{type.GetName(), type.GetParent().GetName(), {}};
	std::map<std::string, AttributeDescription> attributes;
	for (;;) {
		for (std::size_t index = 0; index < type.GetAttributeN(); ++index) {
			auto attribute = type.GetAttribute(index);
			// Walking from child to parent preserves the most specific declaration.
			attributes.try_emplace(attribute.name, AttributeDescription{type.GetName(), attribute});
		}
		const auto parent = type.GetParent();
		if (parent == type) {
			break;
		}
		type = parent;
	}
	for (auto& [name, attribute] : attributes) {
		description.attributes.push_back(std::move(attribute));
	}
	return description;
}
