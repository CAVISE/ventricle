#include "src/extensions/dynamic-loader/model/loader.hpp"

#include "src/extensions/dynamic-loader/model/extension.h"

#include <algorithm>
#include <exception>
#include <string>
#include <system_error>

#include <dlfcn.h>

using namespace vcle::dynamic_loader;

namespace {
	struct Registration {
		Extension& extension;
		absl::Status status;
	};

	int registerType(void* context, const char* name) {
		auto& registration = *static_cast<Registration*>(context);
		if (!registration.status.ok()) {
			return -1;
		}

		try {
			ns3::TypeId type;
			if (!name || !*name || !ns3::TypeId::LookupByNameFailSafe(name, &type)) {
				registration.status = absl::InvalidArgumentError("Unregistered TypeId: " + std::string(name ? name : "<null>"));
				return -1;
			}
			if (std::ranges::find(registration.extension.types, type) != registration.extension.types.end()) {
				registration.status = absl::AlreadyExistsError("Duplicate TypeId: " + std::string(name));
				return -1;
			}

			registration.extension.types.push_back(type);
			return 0;
		} catch (const std::exception& exception) {
			registration.status = absl::InternalError(exception.what());
			return -1;
		}
	}
} // namespace

absl::StatusOr<std::shared_ptr<const Extension>> Loader::load(const std::filesystem::path& path) {
	std::error_code error;
	const auto canonical = std::filesystem::canonical(path, error);
	if (error) {
		return absl::NotFoundError("Cannot resolve extension '" + path.string() + "': " + error.message());
	}

	if (const auto found = extensions_.find(canonical); found != extensions_.end()) {
		return found->second;
	}

	// Never dlclose: even a rejected library may have populated ns-3's global registry.
	void* handle = dlopen(canonical.c_str(), RTLD_NOW | RTLD_LOCAL);
	if (!handle) {
		return absl::UnavailableError("Cannot load extension '" + canonical.string() + "': " + dlerror());
	}

	dlerror();
	const auto entry = reinterpret_cast<VcleExtensionEntry>(dlsym(handle, VCLE_EXTENSION_ENTRY_SYMBOL));
	if (const char* symbolError = dlerror()) {
		return absl::InvalidArgumentError("Invalid extension '" + canonical.string() + "': " + symbolError);
	}
	if (!entry) {
		return absl::InvalidArgumentError("Null extension entrypoint: " + canonical.string());
	}

	const auto* info = entry();
	if (!info) {
		return absl::InvalidArgumentError("Missing extension descriptor: " + canonical.string());
	}
	if (info->apiVersion != VCLE_EXTENSION_API_VERSION) {
		return absl::FailedPreconditionError("Unsupported extension API version: " + std::to_string(info->apiVersion));
	}
	if (!info->name || !*info->name || !info->version || !info->registerTypes) {
		return absl::InvalidArgumentError("Incomplete extension descriptor: " + canonical.string());
	}

	for (const auto& [loadedPath, loaded] : extensions_) {
		if (loaded->name == info->name) {
			return absl::AlreadyExistsError("Extension '" + loaded->name + "' already loaded from " + loadedPath.string());
		}
	}

	auto extension = std::make_shared<Extension>();
	extension->name = info->name;
	extension->version = info->version;
	Registration registration{*extension, absl::OkStatus()};
	const int result = info->registerTypes(&registerType, &registration);
	if (!registration.status.ok()) {
		return registration.status;
	}
	if (result != 0) {
		return absl::InternalError("Type registration failed for extension '" + extension->name + "'");
	}

	extensions_.emplace(canonical, extension);
	return extension;
}
