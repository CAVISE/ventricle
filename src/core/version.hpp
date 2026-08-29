#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include <absl/strings/numbers.h>
#include <absl/strings/str_split.h>

#include <ns3/abort.h>

namespace vcle {

	/**
	 * @brief Provides access to the Ventricle semantic version.
	 *
	 * The version is embedded by the build system and exposed both as a complete
	 * string and as individual numeric components.
	 */
	class Version {
	public:
		/** Selects one numeric semantic-version component. */
		enum class Component : std::uint8_t { MAJOR, MINOR, PATCH };

		/** Return the major version component. */
		static int major() noexcept {
			return fetchVersionComponent(Component::MAJOR);
		}

		/** Return the minor version component. */
		static int minor() noexcept {
			return fetchVersionComponent(Component::MINOR);
		}

		/** Return the patch version component. */
		static int patch() noexcept {
			return fetchVersionComponent(Component::PATCH);
		}

		/** Return the complete semantic version string. */
		static std::string version() noexcept {
			return version_;
		}

	private:
		static int fetchVersionComponent(Component component) {
			const std::vector<std::string_view> components = absl::StrSplit(version_, '.');
			NS_ABORT_MSG_UNLESS(components.size() == 3, "Ventricle version is not semantic");
			int value;
			if (absl::SimpleAtoi(components.at(static_cast<std::size_t>(component)), &value)) {
				return value;
			}
			NS_ABORT_MSG("Ventricle version component is not numeric");
		}

	private:
		static constexpr const char* version_ = VENTRICLE_VERSION;
	};

} // namespace vcle
