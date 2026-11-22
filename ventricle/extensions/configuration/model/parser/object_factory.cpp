#include <ventricle/core/defs.hpp>
#include <ventricle/core/error.hpp>
#include <ventricle/extensions/configuration/identifiers.hpp>
#include <ventricle/extensions/configuration/model/parser/object_factory.hpp>

#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>
#include <variant>

#include <ns3/attribute.h>
#include <ns3/boolean.h>
#include <ns3/double.h>
#include <ns3/integer.h>
#include <ns3/object.h>
#include <ns3/ptr.h>
#include <ns3/string.h>
#include <ns3/type-id.h>
#include <ns3/uinteger.h>

#include <glaze/json/generic_fwd.hpp>

using namespace vcle;
using namespace vcle::config;

namespace {

	using ErrorCode = ObjectFactory::ErrorCode;
	using ErrorInfo = ObjectFactory::ErrorInfo;

	using VariadicAttribute = std::variant<std::monostate, ns3::IntegerValue, ns3::UintegerValue, ns3::DoubleValue, ns3::BooleanValue>;

	template <ErrorCode code>
	bool isErrorOf(Error& error) {
		bool matched = false;
		error.handle([&](const ErrorInfo& info) { matched = info.code() == code; });

		return matched;
	}

	bool isAttributeNotFound(Error& error) {
		return isErrorOf<ErrorCode::ATTRIBUTE_NOT_FOUND>(error);
	}

	template <typename... Args>
	std::unexpected<Error> makeErrorLocal(ErrorCode code, std::format_string<Args...> fmt, Args&&... args) {
		return makeError<ErrorInfo>(code, std::format(fmt, std::forward<Args>(args)...));
	}

	template <typename T>
	Expected<T> fetch(const glz::generic::object_t& source, std::string_view attribute) {
		const auto attr = source.find(attribute);
		if (attr == source.end()) {
			return makeErrorLocal(ErrorCode::ATTRIBUTE_NOT_FOUND, "attribute '{}' was not found", attribute);
		}

		if (const auto& [_, value] = *attr; !value.holds<T>()) {
			return makeErrorLocal(ErrorCode::UNEXPECTED_TYPE, "attribute '{}' has an unexpected type", attribute);
		} else {
			return value.as<T>();
		}
	}

	template <typename V, typename F>
	void for_(F&& f) {
		auto invoker = [&]<std::size_t... I>(std::index_sequence<I...>) { (f.template operator()<std::variant_alternative_t<I, V>>(), ...); };

		invoker(std::make_index_sequence<std::variant_size_v<V>>());
	}

	class AttributeTypesVisitor {
	public:
		AttributeTypesVisitor(const std::string& name, const glz::generic& value, ns3::Object* object)
			: object_(object)
			, name_(name)
			, value_(value) {
		}

		bool operator()(const ns3::IntegerValue& _) {
			if (!value_.is_number()) {
				return false;
			}

			const auto number = static_cast<std::int64_t>(value_.get<double>());
			return object_->SetAttributeFailSafe(name_, ns3::IntegerValue(number));
		}

		bool operator()(const ns3::BooleanValue& _) {
			if (!value_.is_boolean()) {
				return false;
			}

			const auto boolean = value_.get<bool>();
			return object_->SetAttributeFailSafe(name_, ns3::BooleanValue(boolean));
		}

		bool operator()(const ns3::UintegerValue& _) {
			if (!value_.is_number()) {
				return false;
			}

			const auto number = static_cast<std::uint64_t>(value_.get<double>());
			return object_->SetAttributeFailSafe(name_, ns3::UintegerValue(number));
		}

		bool operator()(const ns3::DoubleValue& _) {
			if (!value_.is_number()) {
				return false;
			}

			const auto number = value_.get<double>();
			return object_->SetAttributeFailSafe(name_, ns3::DoubleValue(number));
		}

		bool operator()(const std::monostate& _) {
			return value_.is_string() && object_->SetAttributeFailSafe(name_, ns3::StringValue(value_.get<std::string>()));
		}

	private:
		ns3::Object* object_;
		const std::string& name_;
		const glz::generic& value_;
	};

	bool setAttribute(ns3::Object& object, const ns3::TypeId::AttributeInformation& attribute, const glz::generic& value) {
		// Even the fail-safe setter aborts when it looks up an obsolete attribute.
		if (attribute.supportLevel == ns3::TypeId::SupportLevel::OBSOLETE) {
			return false;
		}

		ns3::Ptr<ns3::AttributeValue> target = attribute.checker->Create();
		ns3::AttributeValue* pp = ns3::PeekPointer(target);

		VariadicAttribute variadic;
		for_<VariadicAttribute>([&]<typename Alternative>() {
			if constexpr (std::is_same_v<Alternative, std::monostate>) {
				return;
			}

			if (auto* p = dynamic_cast<Alternative*>(pp); p) {
				variadic = *p;
			}
		});

		return std::visit(AttributeTypesVisitor(attribute.name, value, &object), variadic);
	}

} // namespace

Expected<ns3::Ptr<ns3::Object>> ObjectFactory::create(const glz::generic& definition, VCLE_MAYBE_UNUSED IParserContext& context) {
	if (!definition.is_object()) {
		return makeError(std::errc::invalid_argument, "object definition must be a JSON object");
	}

	const auto& members = definition.get<glz::generic::object_t>();
	if (auto reference = fetch<std::string>(members, identifiers::reference); reference) {
		return makeError<ReferenceErrorInfo>(std::move(*reference));
	} else if (!isAttributeNotFound(reference.error())) {
		return forwardError(reference.error());
	}

	auto typeName = fetch<std::string>(members, identifiers::typeId);
	if (!typeName) {
		return forwardError(typeName.error());
	}

	ns3::TypeId type;
	if (!ns3::TypeId::LookupByNameFailSafe(*typeName, &type)) {
		return makeErrorLocal(ErrorCode::TYPE_NOT_FOUND, "attribute '{}' names an unknown ns-3 type '{}'", identifiers::typeId, *typeName);
	}

	factory_.SetTypeId(type);
	auto object = factory_.Create();

	// ns-3 attributes are provided per-end class only, meaning
	// that to actually set all attributes, we have to walk the parents
	// and fetch attributes for all parents.

	for (auto current = type;; current = current.GetParent()) {
		for (std::size_t index = 0; index < current.GetAttributeN(); ++index) {
			const auto attribute = current.GetAttribute(index);
			if (const auto configured = members.find(attribute.name); configured == members.end()) {
				continue;
			} else if (!setAttribute(*object, attribute, configured->second)) {
				return makeErrorLocal(
					ErrorCode::ATTRIBUTE_SET_FAILED,
					"attribute '{}' on type '{}' could not be set",
					attribute.name,
					type.GetName()
				);
			}
		}

		if (!current.HasParent()) {
			break;
		}
	}

	return object;
}
