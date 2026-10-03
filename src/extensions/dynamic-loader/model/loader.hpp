#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <absl/status/statusor.h>

#include <ns3/type-id.h>

namespace vcle::dynamic_loader {

	/** Host-owned metadata and public TypeIds collected through the C registration callback. */
	struct Extension {
		std::string name;
		std::string version;
		std::vector<ns3::TypeId> types;
	};

	/**
	 * Load explicitly requested shared extensions using the versioned component interface.
	 * Libraries stay loaded even after this object is destroyed or entrypoint validation fails:
	 * their static initializers may already have registered code pointers with ns-3.
	 * Use during single-threaded setup, before constructing simulation components.
	 */
	class Loader {
	public:
		/**
		 * Load a library by filesystem path, returning shared ownership of collected metadata.
		 * Repeated loads of the same canonical path return the cached metadata.
		 * Missing libraries, invalid entrypoints, and conflicting extension names return errors.
		 */
		absl::StatusOr<std::shared_ptr<const Extension>> load(const std::filesystem::path& path);

	private:
		std::map<std::filesystem::path, std::shared_ptr<const Extension>> extensions_;
	};

} // namespace vcle::dynamic_loader
