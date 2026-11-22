#pragma once

/**
 * @file
 * @brief Shared error payloads, checked ownership, and typed visitation for simulator modules.
 *
 * @ref vcle::Error owns an @ref vcle::IErrorInfo payload; @ref vcle::Expected combines that error
 * with a result value. Use @ref vcle::makeError to construct failures and @ref vcle::toString or
 * @ref vcle::Error::handle to acknowledge them.
 *
 * @code{.cpp}
 * vcle::Expected<int> result = vcle::makeError("record not found");
 * if (!result) {
 *     std::string diagnostic = vcle::toString(result.error());
 * }
 * @endcode
 *
 * Typed dispatch requires RTTI. Checking an @ref vcle::Expected does not acknowledge its contained
 * @ref vcle::Error.
 */

#include <cassert>
#include <concepts>
#include <exception>
#include <expected>
#include <format>
#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>
#include <vector>

namespace vcle {

	/**
	 * @brief Polymorphic diagnostic payload retaining domain-specific error details.
	 *
	 * Derive from this interface to expose structured information through @ref Error::handle.
	 * Payload presence indicates failure, independently of its diagnostic text or code value.
	 */
	class IErrorInfo {
	public:
		virtual ~IErrorInfo() = default;

		/**
		 * @brief Borrow the diagnostic text without transferring ownership.
		 * @return A view backed by storage that remains valid after this call.
		 */
		virtual std::string_view string() const = 0;
	};

	/**
	 * @brief Owns a diagnostic message and exposes it through @ref IErrorInfo::string.
	 */
	class StringErrorInfo : public IErrorInfo {
	public:
		explicit StringErrorInfo(std::string message)
			: message_(std::move(message)) {
		}

		/* IErrorInfo implementation */
		std::string_view string() const override {
			return message_;
		}

	private:
		std::string message_;
	};

	/**
	 * @brief Extends @ref StringErrorInfo with a typed enum code.
	 * @tparam Enum Enum type identifying failures in a particular domain.
	 *
	 * A code-only construction uses the underlying integer's decimal representation, not its
	 * enumerator name. Supply a message for domain-specific text.
	 *
	 * @ref code preserves the original enum type. In @ref Error::handle, place enum-specific
	 * handlers before @ref StringErrorInfo handlers, which also match this payload.
	 */
	template <typename Enum>
		requires std::is_enum_v<Enum>
	class EnumeratedErrorInfo final : public StringErrorInfo {
	public:
		explicit EnumeratedErrorInfo(Enum code)
			: EnumeratedErrorInfo(code, std::to_string(std::to_underlying(code))) {
		}

		EnumeratedErrorInfo(Enum code, std::string message)
			: StringErrorInfo(std::move(message))
			, code_(code) {
		}

		Enum code() const noexcept {
			return code_;
		}

	private:
		Enum code_;
	};

	/**
	 * @brief Owns the flat collection of failure payloads created by @ref Error::join.
	 *
	 * Entries retain their original types and input order, including duplicates. The list owns
	 * every entry; access through @ref errors borrows that storage.
	 *
	 * @ref Error::handle visits individual entries and does not accept handlers for the list
	 * itself. Use @ref toString to render the entries together; @ref string currently returns an
	 * empty view.
	 */
	class ErrorList final : public IErrorInfo {
		friend class Error;

	public:
		/**
		 * @brief Internal constructor used by @ref Error::join.
		 * @note Public only to support @c std::make_unique; not intended for direct use. Call @ref
		 * Error::join instead.
		 * @pre Both payloads are non-null and neither is an @ref ErrorList.
		 */
		ErrorList(std::unique_ptr<IErrorInfo> first, std::unique_ptr<IErrorInfo> second) {
			errors_.reserve(2);
			errors_.push_back(std::move(first));
			errors_.push_back(std::move(second));
		}

		const std::vector<std::unique_ptr<IErrorInfo>>& errors() const noexcept {
			return errors_;
		}

		/* IErrorInfo implementation */
		std::string_view string() const override {
			return "";
		}

	private:
		std::vector<std::unique_ptr<IErrorInfo>> errors_;
	};

	namespace detail {
		/**
		 * @brief Extracts the argument and return types used to validate @ref Error::handle
		 * callbacks.
		 *
		 * Supports unary function types, function pointers, and callable objects with a single
		 * non-template call operator. Generic, overloaded, and ref-qualified call operators require
		 * a typed wrapper. Unsupported signatures expose no argument or return type, allowing @ref
		 * ErrorHandler to reject them during constraint checking.
		 */
		template <typename Handler>
		struct ErrorHandlerTraits {};

		/// @brief Extracts the signature of an unambiguous, non-template call operator.
		template <typename Handler>
			requires requires { &Handler::operator(); }
		struct ErrorHandlerTraits<Handler> : ErrorHandlerTraits<decltype(&Handler::operator())> {};

		/// @brief Extracts a unary function's parameter and return types.
		template <typename Return, typename Argument>
		struct ErrorHandlerTraits<Return(Argument)> {
			using ArgumentType = Argument;
			using ReturnType = Return;
		};

		/// @brief Applies @ref ErrorHandlerTraits to a non-throwing function type.
		template <typename Return, typename Argument>
		struct ErrorHandlerTraits<Return(Argument) noexcept> : ErrorHandlerTraits<Return(Argument)> {};

		/// @brief Applies @ref ErrorHandlerTraits to the function behind a pointer.
		template <typename Function>
		struct ErrorHandlerTraits<Function*> : ErrorHandlerTraits<Function> {};

		/// @brief Extracts the signature of a mutable call operator.
		template <typename Class, typename Return, typename Argument>
		struct ErrorHandlerTraits<Return (Class::*)(Argument)> : ErrorHandlerTraits<Return(Argument)> {};

		/// @brief Extracts the signature of a const call operator.
		template <typename Class, typename Return, typename Argument>
		struct ErrorHandlerTraits<Return (Class::*)(Argument) const> : ErrorHandlerTraits<Return(Argument)> {};

		/// @brief Extracts the signature of a non-throwing mutable call operator.
		template <typename Class, typename Return, typename Argument>
		struct ErrorHandlerTraits<Return (Class::*)(Argument) noexcept> : ErrorHandlerTraits<Return(Argument)> {};

		/// @brief Extracts the signature of a non-throwing const call operator.
		template <typename Class, typename Return, typename Argument>
		struct ErrorHandlerTraits<Return (Class::*)(Argument) const noexcept> : ErrorHandlerTraits<Return(Argument)> {};
	} // namespace detail

	/// Normalize callable qualifiers before inspecting @ref detail::ErrorHandlerTraits.
	template <typename H>
	using HandlerTraits = detail::ErrorHandlerTraits<std::remove_cvref_t<H>>;

	/**
	 * @brief Unary callable returning void and accepting an @ref IErrorInfo payload by lvalue
	 * reference.
	 *
	 * Accepts mutable and const payload references, excluding @ref ErrorList. The callable must be
	 * invocable with its supplied qualifiers; generic, overloaded, and ref-qualified call operators
	 * require typed wrappers.
	 * @see Error::handle
	 * @see detail::ErrorHandlerTraits
	 */
	template <typename H>
	concept ErrorHandler = requires {
		typename HandlerTraits<H>::ArgumentType;
		typename HandlerTraits<H>::ReturnType;

		/* clang-format off */
		requires std::is_lvalue_reference_v<
			typename HandlerTraits<H>::ArgumentType
		>;

		requires std::derived_from<
			std::remove_cvref_t<
				typename HandlerTraits<H>::ArgumentType
			>,
			IErrorInfo
		>;

		requires std::same_as<
			typename HandlerTraits<H>::ReturnType,
			void
		>;

		requires(
			!std::same_as<
				std::remove_cvref_t<typename HandlerTraits<H>::ArgumentType>,
				ErrorList
			>
		);

		requires std::invocable<H, typename HandlerTraits<H>::ArgumentType>;
		/* clang-format on */
	};

	/**
	 * @brief Lightweight error class that owns an @ref IErrorInfo payload and requires failures to
	 * be checked.
	 *
	 * A null payload represents already-checked success; a non-null payload starts unchecked.
	 * Boolean conversion and @ref handle acknowledge the error while retaining its payload.
	 * Destroying or replacing an unchecked failure terminates the process, including in release
	 * builds. Moving a failure leaves the source as checked success and requires a fresh check of
	 * the destination.
	 *
	 * Use @ref success for successful operations, @ref join to combine failures, and @ref handle to
	 * inspect typed details. @ref Expected integrates this ownership contract with result values.
	 */
	class [[nodiscard]] Error final {
	public:
		explicit Error(std::unique_ptr<IErrorInfo> payload) noexcept
			: payload_(std::move(payload))
			, checked_(!payload_) {
		}

		Error(Error&& other) noexcept
			: Error(std::move(other.payload_)) {
			other.checked_ = true;
		}

		/// Require the previous value to be checked before transfer; self-assignment has no effect.
		Error& operator=(Error&& other) noexcept {
			if (this == &other) {
				return *this;
			}

			requireChecked();
			payload_ = std::move(other.payload_);
			checked_ = !payload_;
			other.checked_ = true;

			return *this;
		}

		~Error() {
			requireChecked();
		}

		/// Create an already-checked success with no payload.
		static Error success() noexcept {
			return Error(nullptr);
		}

		/**
		 * @brief Transfer failures into one error, ignoring successful inputs.
		 *
		 * With no failures, returns @ref success. A single failure keeps its original payload.
		 * Multiple failures form a flat @ref ErrorList in input order. Existing lists are flattened
		 * without cloning or deduplicating entries.
		 *
		 * Pass existing errors with @c std::move. The returned failure requires a fresh check, even
		 * if both inputs were previously checked.
		 */
		static Error join(Error first, Error second) {
			std::unique_ptr<IErrorInfo> f = first.takePayload();
			std::unique_ptr<IErrorInfo> s = second.takePayload();

			if (!f) {
				return Error(std::move(s));
			} else if (!s) {
				return Error(std::move(f));
			}

			auto fl = dynamic_cast<ErrorList*>(f.get());
			auto sl = dynamic_cast<ErrorList*>(s.get());

#if !NDEBUG
			{
				auto ensureNoNested = [](ErrorList* l) {
					if (!l) {
						return;
					}

					for (auto& e : l->errors_) {
						assert(
							dynamic_cast<ErrorList*>(e.get()) == nullptr && "error list elements cannot be error list (nested lists are not "
																			"supported)"
						);
					}
				};

				ensureNoNested(fl);
				ensureNoNested(sl);
			}
#endif

			if (fl && sl) {
				std::size_t combined = fl->errors_.size() + sl->errors_.size();
				fl->errors_.reserve(combined);

				for (auto& entry : sl->errors_) {
					fl->errors_.push_back(std::move(entry));
				}
				return Error(std::move(f));
			}

			if (fl || sl) {
				if (fl) {
					fl->errors_.push_back(std::move(s));
					return Error(std::move(f));
				} else {
					sl->errors_.insert(sl->errors_.begin(), std::move(f));
					return Error(std::move(s));
				}
			}

			// if both fl & sl are nullptr, we are dealing with two simple errors.
			return Error(std::make_unique<ErrorList>(std::move(f), std::move(s)));
		}

		/// Acknowledge this error and return true for failure, @ref Expected uses the opposite
		/// convention.
		explicit operator bool() noexcept {
			checked_ = true;
			return payload_ != nullptr;
		}

		/**
		 * @brief Acknowledge this error and visit its retained payloads with typed handlers.
		 * @param[in] handlers Callbacks satisfying @ref ErrorHandler, tried in argument order.
		 *
		 * The first matching handler runs for each payload; base-class handlers also match derived
		 * payloads. Lists are always traversed in entry order; handlers for @ref ErrorList are
		 * rejected at compile time. Success invokes no handlers.
		 *
		 * Empty handler lists and unmatched payloads still acknowledge the error. Handler
		 * exceptions propagate and stop visitation, leaving the error checked and its payloads
		 * retained. Generic and overloaded call operators require typed wrappers; see @ref
		 * detail::ErrorHandlerTraits.
		 */
		template <ErrorHandler... Handlers>
		void handle(Handlers&&... handlers) {
			checked_ = true;
			if (auto* list = dynamic_cast<ErrorList*>(payload_.get())) {
				for (const auto& entry : list->errors()) {
					static_cast<void>((tryHandle(entry.get(), std::forward<Handlers>(handlers)) || ...));
				}
				return;
			}

			static_cast<void>((tryHandle(payload_.get(), std::forward<Handlers>(handlers)) || ...));
		}

	private:
		std::unique_ptr<IErrorInfo> takePayload() noexcept {
			checked_ = true;
			return std::move(payload_);
		}

		void requireChecked() const noexcept {
			if (!checked_) {
				assert(checked_ && "error must be checked before destruction or replacement");
				std::unreachable();
			}
		}

		template <ErrorHandler Handler>
		static bool tryHandle(IErrorInfo* payload, Handler&& handler) {
			using Traits = HandlerTraits<Handler>;
			using Argument = typename Traits::ArgumentType;
			using Info = std::remove_reference_t<Argument>;

			if (auto* info = dynamic_cast<Info*>(payload)) {
				std::invoke(std::forward<Handler>(handler), *info);
				return true;
			}

			return false;
		}

	private:
		std::unique_ptr<IErrorInfo> payload_;
		bool checked_;
	};

	/**
	 * @brief Represents a result value or an owned @ref Error.
	 * @tparam T Result type; the default void represents an operation without a value.
	 *
	 * Boolean conversion is true for success and does not check the contained error. On failure,
	 * visit the error, render it with @ref toString, or transfer it with @c std::move.
	 */
	template <typename T = void>
	using Expected = std::expected<T, Error>;

	/**
	 * @brief Move an existing error into a failure for propagation through @ref Expected.
	 * @param[in,out] error Error whose payload is transferred, leaving it as checked success.
	 * @return An unexpected value owning the error, which must be handled or propagated.
	 */
	[[nodiscard]] inline std::unexpected<Error> forwardError(Error& error) noexcept {
		return std::unexpected(std::move(error));
	}

	/**
	 * @brief Return an owned diagnostic while acknowledging an error through @ref Error::handle.
	 * @param[in,out] error Error to check and render; its payload remains owned by the wrapper.
	 * @return An empty string for success, or the payload diagnostics joined by newlines in
	 * visitation order, preserving empty entries.
	 */
	inline std::string toString(Error& error) {
		std::stringstream oss{std::ios::out};
		bool first = true;

		error.handle([&](const IErrorInfo& info) {
			if (bool was = first, _ = first & 0; was) {
				oss << '\n';
			}

			oss << info.string();
		});

		return oss.str();
	}

	/**
	 * @brief Construct a typed failure for propagation through @ref Expected.
	 * @tparam ErrorType Concrete @ref IErrorInfo subclass to allocate.
	 * @param[in] args Arguments forwarded to the payload constructor.
	 * @return An unexpected value owning an unchecked @ref Error.
	 */
	template <std::derived_from<IErrorInfo> ErrorType, typename... Args>
	[[nodiscard]] std::unexpected<Error> makeError(Args&&... args) {
		return std::unexpected(Error(std::make_unique<ErrorType>(std::forward<Args>(args)...)));
	}

	/// Construct @ref EnumeratedErrorInfo with a portable @c std::errc code and formatted text.
	template <typename... Args>
	[[nodiscard]] std::unexpected<Error> makeError(std::errc code, std::format_string<Args...> format, Args&&... args) {
		return makeError<EnumeratedErrorInfo<std::errc>>(code, std::format(format, std::forward<Args>(args)...));
	}

	/// Construct @ref StringErrorInfo with formatted text for propagation through @ref Expected.
	template <typename... Args>
	[[nodiscard]] std::unexpected<Error> makeError(std::format_string<Args...> format, Args&&... args) {
		return makeError<StringErrorInfo>(std::format(format, std::forward<Args>(args)...));
	}

} // namespace vcle
