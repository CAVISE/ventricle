#include <ventricle/core/error.hpp>
#include <ventricle/extensions/configuration/model/schema_context.hpp>
#include <ventricle/extensions/configuration/model/schema_values.hpp>

#include <memory>
#include <ranges>
#include <utility>

#include <ns3/boolean.h>
#include <ns3/data-rate.h>
#include <ns3/double.h>
#include <ns3/integer.h>
#include <ns3/nstime.h>
#include <ns3/string.h>
#include <ns3/uinteger.h>

using namespace vcle;

using namespace vcle::config::detail;

namespace {

	template <typename T>
	Expected<> writeValue(Context& ctx, const ns3::TypeId::AttributeInformation& attribute, const T* value);

	template <>
	Expected<> writeValue(Context& ctx, const ns3::TypeId::AttributeInformation& /* attribute */, const ns3::IntegerValue* value) {
		auto& schema = ctx.schema();
		schema.type = "integer";
		schema.defaultValue = value->Get();

		return {};
	}

	template <>
	Expected<> writeValue(Context& ctx, const ns3::TypeId::AttributeInformation& /* attribute */, const ns3::UintegerValue* value) {
		ctx.schema().type = "integer";
		ctx.schema().defaultValue = value->Get();
		return {};
	}

	template <>
	Expected<> writeValue(Context& ctx, const ns3::TypeId::AttributeInformation& /* attribute */, const ns3::StringValue* value) {
		auto& schema = ctx.schema();
		schema.type = "string";
		schema.defaultValue = ctx.store(value->Get());

		return {};
	}

	template <>
	Expected<> writeValue(Context& ctx, const ns3::TypeId::AttributeInformation& /* attribute */, const ns3::DoubleValue* value) {
		auto& schema = ctx.schema();
		schema.type = "number";
		schema.defaultValue = value->Get();

		return {};
	}

	template <>
	Expected<> writeValue(Context& ctx, const ns3::TypeId::AttributeInformation& /* attribute */, const ns3::BooleanValue* value) {
		auto& schema = ctx.schema();
		schema.type = "boolean";
		schema.defaultValue = value->Get();

		return {};
	}

	template <>
	Expected<> writeValue(Context& ctx, const ns3::TypeId::AttributeInformation& attribute, const ns3::TimeValue* value) {
		auto& schema = ctx.schema();
		schema.type = "string";
		schema.defaultValue = ctx.store(value->SerializeToString(attribute.checker));

		return {};
	}

	template <>
	Expected<> writeValue(Context& ctx, const ns3::TypeId::AttributeInformation& attribute, const ns3::DataRateValue* value) {
		auto& schema = ctx.schema();
		schema.type = "string";
		schema.defaultValue = ctx.store(value->SerializeToString(attribute.checker));
		return {};
	}

	template <typename... Types>
	Expected<> castValue(Context& ctx, const ns3::TypeId::AttributeInformation& attribute, const ns3::AttributeValue* value) {
		auto tryType = [&]<typename T>() -> Expected<bool> {
			const auto* typed = dynamic_cast<const T*>(value);
			if (!typed) {
				return false;
			}

			if (auto status = writeValue<T>(ctx, attribute, typed); !status) {
				return std::unexpected(std::move(status.error()));
			}

			return true;
		};

		auto failure = Error::success();
		auto dispatch = [&]<typename T>() {
			auto result = tryType.template operator()<T>();
			if (!result) {
				failure = std::move(result.error());
				return true;
			}

			// Stop on either a match or an error
			return *result;
		};

		if (!(dispatch.template operator()<Types>() || ...)) {
			return makeError(std::errc::function_not_supported, "cannot parse attribute type; probably it is unsupported");
		}

		if (failure) {
			return std::unexpected(std::move(failure));
		}
		return {};
	}

} // namespace

Expected<> vcle::config::detail::writeAttributeValue(
	Context& ctx, const ns3::TypeId::AttributeInformation& attribute, const ns3::AttributeValue* value
) {
	// TODO: think of smthing better please
	if (attribute.checker->GetValueTypeName().starts_with("ns3::EnumValue<")) {
		auto& schema = ctx.schema();
		schema.type = "string";
		schema.defaultValue = ctx.store(value->SerializeToString(attribute.checker));

		const std::string_view choices = ctx.store(attribute.checker->GetUnderlyingTypeInformation());
		schema.enumeration.emplace();

		// The checker exposes names separated by '|'.
		for (const auto choice : choices | std::views::split('|')) {
			schema.enumeration->push_back(std::string_view(choice.begin(), choice.end()));
		}

		return {};
	}

	return castValue<
		ns3::StringValue,
		ns3::IntegerValue,
		ns3::UintegerValue,
		ns3::DoubleValue,
		ns3::BooleanValue,
		ns3::TimeValue,
		ns3::DataRateValue>(ctx, attribute, value);
}
