#pragma once

#include <cstdint>
#include <cstdio>
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
		enum class Component : std::uint8_t { MAJOR, MINOR, PATCH };

		static int major() noexcept {
			return fetchVersionComponent(Component::MAJOR);
		}

		static int minor() noexcept {
			return fetchVersionComponent(Component::MINOR);
		}

		static int patch() noexcept {
			return fetchVersionComponent(Component::PATCH);
		}

		static std::string version() noexcept {
			return version_;
		}

	private:
		static int fetchVersionComponent(Component component) {
			const char* t = nullptr;

			switch (component) {
			case Component::MAJOR:
				t = "%d:%*d:%*d";
				break;
			case Component::MINOR:
				t = "%*d:%d:%*d";
				break;
			case Component::PATCH:
				t = "%*d:%*d:%d";
				break;
			}

			if (int component = 0; std::sscanf(version_, t, &component) > 0) {
				return component;
			}

			// This path should be unreachable generally.
			NS_ABORT_MSG("could not fetch version component");
		}

	private:
		static constexpr const char* version_ = VENTRICLE_VERSION;
	};

} // namespace vcle
