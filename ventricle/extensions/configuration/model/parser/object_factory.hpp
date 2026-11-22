#pragma once

#include <ventricle/core/defs.hpp>
#include <ventricle/core/error.hpp>
#include <ventricle/extensions/configuration/model/parser/parser_context.hpp>

#include <cstdint>
#include <string>
#include <utility>

#include <ns3/object-factory.h>
#include <ns3/object.h>
#include <ns3/ptr.h>

#include <glaze/json/generic_fwd.hpp>

namespace vcle::config {

	/**
	 * @brief Creates ns-3 objects from parsed configuration fragments.
	 *
	 * @ref create reports references through @ref ReferenceErrorInfo and resolves
	 * @ref identifiers::typeId names through the ns-3 type registry. It constructs the selected
	 * object and applies supplied attributes, including inherited ones, through
	 * @ref ns3::ObjectBase::SetAttributeFailSafe. Each call supplies its own @ref IParserContext.
	 */
	class ObjectFactory {
	public:
		/// @brief Identifies factory-specific failures.
		enum class ErrorCode : std::uint8_t {
			ERROR_REFERENCE,	  ///< The definition refers to an object by identifier.
			UNEXPECTED_TYPE,	  ///< An attribute has an unexpected JSON type.
			ATTRIBUTE_NOT_FOUND,  ///< The requested attribute is absent from the definition.
			TYPE_NOT_FOUND,		  ///< The requested ns-3 type name is not registered.
			ATTRIBUTE_SET_FAILED, ///< A supplied attribute value could not be applied.
		};

		/// Factory failure carrying an @ref ErrorCode and diagnostic text.
		using ErrorInfo = EnumeratedErrorInfo<ErrorCode>;

		/// @brief Carries the owned identifier of an object reference for later resolution.
		class ReferenceErrorInfo final : public StringErrorInfo {
		public:
			explicit ReferenceErrorInfo(std::string id)
				: StringErrorInfo("object definition is a reference")
				, id_(std::move(id)) {
			}

			VCLE_NODISCARD ErrorCode code() const noexcept {
				return ErrorCode::ERROR_REFERENCE;
			}

			VCLE_NODISCARD const std::string& id() const noexcept {
				return id_;
			}

		private:
			std::string id_;
		};

		/**
		 * @brief Construct and configure an object, deferring references to the caller.
		 * @param[in] definition Parsed JSON fragment at a position expecting an object.
		 * @param[in,out] context Parser services borrowed for this creation call only.
		 * @pre The requested type derives from @ref ns3::Object and has a registered constructor.
		 * @return A @ref ReferenceErrorInfo failure for a string-valued
		 * @ref identifiers::reference member.
		 * Non-object fragments return @c std::errc::invalid_argument. Non-string references
		 * return @ref ErrorInfo with @ref ErrorCode::UNEXPECTED_TYPE. A missing reference is
		 * optional and falls through to reading the required @ref identifiers::typeId string.
		 * An absent type tag returns @ref ErrorCode::ATTRIBUTE_NOT_FOUND; a non-string type tag
		 * returns @ref ErrorCode::UNEXPECTED_TYPE. An unregistered name returns
		 * @ref ErrorCode::TYPE_NOT_FOUND.
		 *
		 * Attribute values are read by name directly beside @ref identifiers::typeId; omitted
		 * attributes retain their defaults. Numeric and Boolean attributes require JSON numbers
		 * and booleans, respectively. Integer values use @c static_cast from @c double without
		 * range or integrality checks. Other attribute types accept strings through ns-3's
		 * attribute deserialization. Unsupported JSON value kinds, obsolete attributes, and setter
		 * failures return @ref ErrorCode::ATTRIBUTE_SET_FAILED. Attributes are set after
		 * construction, so construction-only attributes cannot be overridden here.
		 * @note Model-specific string deserializers can terminate on malformed text even when
		 * invoked through @ref ns3::ObjectBase::SetAttributeFailSafe.
		 */
		VCLE_NODISCARD Expected<ns3::Ptr<ns3::Object>> create(const glz::generic& definition, IParserContext& context);

	private:
		ns3::ObjectFactory factory_;
	};

} // namespace vcle::config
