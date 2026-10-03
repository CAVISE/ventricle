#include "src/extensions/dynamic-loader/model/loader.hpp"

#include <iostream>
#include <vector>

using namespace vcle::dynamic_loader;

int main(int argc, char** argv) {
	std::vector<std::shared_ptr<const Extension>> extensions;
	{
		Loader loader;
		for (int index = 1; index < argc; ++index) {
			const auto loaded = loader.load(argv[index]);
			if (!loaded.ok()) {
				std::cerr << loaded.status() << '\n';
				return 1;
			}

			const auto extension = *loaded;
			extensions.push_back(extension);
			for (const auto& type : extension->types) {
				ns3::TypeId registered;
				const bool shared = ns3::TypeId::LookupByNameFailSafe(type.GetName(), &registered) && registered == type;
				std::cout << "loaded " << extension.get() << ' ' << extension->name << ' ' << type.GetName() << ' ' << shared << '\n';
			}
		}
	}

	// Exercise descriptors and TypeIds after the loader has been destroyed.
	for (const auto& extension : extensions) {
		std::cout << "alive " << extension->name << ' ' << extension->types.front().GetName() << '\n';
	}
}
