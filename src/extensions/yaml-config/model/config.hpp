#pragma once

#include <memory>
#include <filesystem>

#include <absl/status/statusor.h>
#include "subprojects/ns-3.48/src/core/model/type-id.h"

namespace vcle::config {

	class RuntimeConfig
		: public std::enable_shared_from_this<RuntimeConfig> {
	public:

		RuntimeConfig(std::span<ns3::TypeId> ids);

		RuntimeConfig(const RuntimeConfig&) = delete;
		RuntimeConfig(RuntimeConfig&&) = delete;

		absl::Status load(const std::filesystem::path& file);
		absl::Status load(std::istream& is);

		RuntimeConfig& setValidation(bool value);
		RuntimeConfig& setSchemaPath(std::filesystem::path& path);

		absl::Status dump(const std::filesystem::path& path) const;
		absl::Status dump(std::ostream& os) const;

	};

} // namespace vcle::yaml_config
