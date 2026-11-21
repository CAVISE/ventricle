#pragma once

#include <ventricle/core/defs.hpp>
#include <ventricle/core/error.hpp>

#include <ns3/object.h>
#include <string>

namespace vcle::config {

	/// @brief Retains constructed objects under exact, nonempty identifiers.
	class IIdentifiersRepository {
	public:
		virtual ~IIdentifiersRepository() = default;

		/**
		 * @brief Retain an object under an identifier owned by the repository's registration scope.
		 * Empty identifiers and null objects return invalid_argument. Duplicate identifiers in that
		 * scope return file_exists without replacing the original binding, even when the object is
		 * unchanged. Different identifiers may refer to the same object. Failed registrations leave
		 * bindings unchanged.
		 */
		VCLE_NODISCARD virtual Expected<> add(
			const std::string& id, ns3::Ptr<ns3::Object> object
		) = 0;

		/**
		 * @brief Return shared ownership of the existing object without constructing or copying it.
		 * Empty identifiers return invalid_argument; no selected visible binding returns
		 * no_such_file_or_directory. Lookup never creates a binding. The returned object may
		 * outlive the repository, whose destruction releases its references without explicitly
		 * disposing objects still held elsewhere.
		 */
		VCLE_NODISCARD virtual Expected<ns3::Ptr<ns3::Object>> get(const std::string& id) const = 0;
	};

} // namespace vcle::config
