#pragma once

#include "../Config.hpp"

#if (!COMPAT_HAS_STD_EXPECTED && !COMPAT_HAS_STD_VARIANT) || defined(COMPAT_ENABLE_UNION_EXPECTED)

#include <cstddef>
#include <cstdint>
#include <utility>
#include <type_traits>
#include <new>
#include <stdexcept>
#include <exception>

#ifndef COMPAT_THROW_OR_ABORT
#  if defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND)
#    define COMPAT_THROW_OR_ABORT(ex) throw (ex)
#  else
#    include <cstdlib>
#    define COMPAT_THROW_OR_ABORT(ex) std::abort()
#  endif
#endif

namespace compat {
namespace detail {

#ifndef COMPAT_BAD_EXPECTED_ACCESS_DEFINED
#define COMPAT_BAD_EXPECTED_ACCESS_DEFINED

/// <summary>
/// Exception thrown when accessing an expected object's value or error illegally.
/// </summary>
/// <typeparam name="E">Error value type.</typeparam>
template <typename E>
class bad_expected_access : public std::exception {
public:
    /// <summary>
    /// Default constructor for bad_expected_access without error payload.
    /// </summary>
    bad_expected_access() noexcept : m_has_error(false) {}

    /// <summary>
    /// Constructs bad_expected_access storing the error payload.
    /// </summary>
    /// <param name="err">The error value to store.</param>
    explicit bad_expected_access(E err) : m_has_error(true) {
        ::new (static_cast<void*>(&m_storage.m_val)) E(std::move(err));
    }

    /// <summary>
    /// Copy constructor for bad_expected_access.
    /// </summary>
    /// <param name="other">Instance to copy.</param>
    bad_expected_access(const bad_expected_access& other) : m_has_error(other.m_has_error) {
        if (m_has_error) {
            ::new (static_cast<void*>(&m_storage.m_val)) E(other.get_err());
        }
    }

    /// <summary>
    /// Move constructor for bad_expected_access.
    /// </summary>
    /// <param name="other">Instance to move.</param>
    bad_expected_access(bad_expected_access&& other) noexcept : m_has_error(other.m_has_error) {
        if (m_has_error) {
            ::new (static_cast<void*>(&m_storage.m_val)) E(std::move(other.get_err()));
        }
    }

    /// <summary>
    /// Copy assignment operator.
    /// </summary>
    /// <param name="other">Instance to copy.</param>
    /// <returns>Reference to this instance.</returns>
    bad_expected_access& operator=(const bad_expected_access& other) {
        if (this != &other) {
            if (m_has_error) {
                get_err().~E();
                m_has_error = false;
            }
            if (other.m_has_error) {
                ::new (static_cast<void*>(&m_storage.m_val)) E(other.get_err());
                m_has_error = true;
            }
        }
        return *this;
    }

    /// <summary>
    /// Move assignment operator.
    /// </summary>
    /// <param name="other">Instance to move.</param>
    /// <returns>Reference to this instance.</returns>
    bad_expected_access& operator=(bad_expected_access&& other) noexcept {
        if (this != &other) {
            if (m_has_error) {
                get_err().~E();
                m_has_error = false;
            }
            if (other.m_has_error) {
                ::new (static_cast<void*>(&m_storage.m_val)) E(std::move(other.get_err()));
                m_has_error = true;
            }
        }
        return *this;
    }

    /// <summary>
    /// Destructor destroying error payload if present.
    /// </summary>
    ~bad_expected_access() noexcept override {
        if (m_has_error) {
            get_err().~E();
        }
    }

    /// <summary>
    /// Explanatory description of the exception.
    /// </summary>
    /// <returns>Null-terminated character sequence describing the error.</returns>
    const char* what() const noexcept override {
        return "bad expected access";
    }

    /// <summary>
    /// Obtains mutable reference to the stored error.
    /// </summary>
    /// <returns>Reference to stored error.</returns>
    E& error() & noexcept { return get_err(); }

    /// <summary>
    /// Obtains const reference to the stored error.
    /// </summary>
    /// <returns>Const reference to stored error.</returns>
    const E& error() const & noexcept { return get_err(); }

    /// <summary>
    /// Obtains mutable rvalue reference to the stored error.
    /// </summary>
    /// <returns>Rvalue reference to stored error.</returns>
    E&& error() && noexcept { return std::move(get_err()); }

    /// <summary>
    /// Obtains const rvalue reference to the stored error.
    /// </summary>
    /// <returns>Const rvalue reference to stored error.</returns>
    const E&& error() const && noexcept { return std::move(get_err()); }

private:
    union Storage {
        char m_dummy;
        E m_val;
        Storage() noexcept : m_dummy(0) {}
        ~Storage() noexcept {}
    } m_storage;
    bool m_has_error;

    E& get_err() noexcept { return m_storage.m_val; }
    const E& get_err() const noexcept { return m_storage.m_val; }
};

/// <summary>
/// Explicit specialization of bad_expected_access for void error type.
/// </summary>
template <>
class bad_expected_access<void> : public std::exception {
public:
    /// <summary>
    /// Default constructor.
    /// </summary>
    bad_expected_access() noexcept = default;

    /// <summary>
    /// Explanatory description of the exception.
    /// </summary>
    /// <returns>Null-terminated character sequence describing the error.</returns>
    const char* what() const noexcept override {
        return "bad expected access";
    }
};

#endif // COMPAT_BAD_EXPECTED_ACCESS_DEFINED

namespace union_impl {

using detail::bad_expected_access;

/// <summary>
/// Tag type indicating an unexpected error value construction.
/// </summary>
struct unexpect_t {
    explicit unexpect_t() = default;
};

/// <summary>
/// In-place tag constant to trigger unexpected error value construction.
/// </summary>
constexpr unexpect_t unexpect{};

/// <summary>
/// Container holding an error value for expected types.
/// </summary>
/// <typeparam name="E">Error value type.</typeparam>
template <typename E>
class unexpected {
public:
    constexpr unexpected(const unexpected&) = default;
    constexpr unexpected(unexpected&&) = default;
    unexpected& operator=(const unexpected&) = default;
    unexpected& operator=(unexpected&&) = default;
    ~unexpected() = default;

    /// <summary>
    /// Constructs unexpected from a const lvalue reference error.
    /// </summary>
    /// <param name="err">The error value.</param>
    explicit constexpr unexpected(const E& err) : m_err(err) {}

    /// <summary>
    /// Constructs unexpected from an rvalue reference error.
    /// </summary>
    /// <param name="err">The error value to move.</param>
    explicit constexpr unexpected(E&& err) : m_err(std::move(err)) {}

    /// <summary>
    /// Constructs unexpected in-place with forwarded arguments.
    /// </summary>
    /// <typeparam name="Args">Constructor argument types.</typeparam>
    /// <param name="args">Forwarded constructor arguments.</param>
    template <typename... Args>
    explicit constexpr unexpected(Args&&... args) : m_err(std::forward<Args>(args)...) {}

    /// <summary>
    /// Obtains const reference to the stored error.
    /// </summary>
    /// <returns>Const reference to stored error.</returns>
    constexpr const E& error() const & noexcept { return m_err; }

    /// <summary>
    /// Obtains mutable reference to the stored error.
    /// </summary>
    /// <returns>Mutable reference to stored error.</returns>
    E& error() & noexcept { return m_err; }

    /// <summary>
    /// Obtains const rvalue reference to the stored error.
    /// </summary>
    /// <returns>Const rvalue reference to stored error.</returns>
    constexpr const E&& error() const && noexcept { return std::move(m_err); }

    /// <summary>
    /// Obtains mutable rvalue reference to the stored error.
    /// </summary>
    /// <returns>Mutable rvalue reference to stored error.</returns>
    E&& error() && noexcept { return std::move(m_err); }

    /// <summary>
    /// Obtains const reference to stored error (alias for error).
    /// </summary>
    /// <returns>Const reference to stored error.</returns>
    constexpr const E& value() const & noexcept { return m_err; }

    /// <summary>
    /// Obtains mutable reference to stored error (alias for error).
    /// </summary>
    /// <returns>Mutable reference to stored error.</returns>
    E& value() & noexcept { return m_err; }

    /// <summary>
    /// Swaps stored error with another unexpected instance.
    /// </summary>
    /// <param name="other">Instance to swap with.</param>
    void swap(unexpected& other) noexcept {
        using std::swap;
        swap(m_err, other.m_err);
    }

private:
    E m_err;
};

/// <summary>
/// Equality comparison for unexpected.
/// </summary>
/// <typeparam name="E">Error value type.</typeparam>
/// <param name="lhs">Left hand unexpected.</param>
/// <param name="rhs">Right hand unexpected.</param>
/// <returns>True if both hold equal errors.</returns>
template <typename E>
inline constexpr bool operator==(const unexpected<E>& lhs, const unexpected<E>& rhs) noexcept {
    return lhs.error() == rhs.error();
}

/// <summary>
/// Inequality comparison for unexpected.
/// </summary>
/// <typeparam name="E">Error value type.</typeparam>
/// <param name="lhs">Left hand unexpected.</param>
/// <param name="rhs">Right hand unexpected.</param>
/// <returns>True if errors are not equal.</returns>
template <typename E>
inline constexpr bool operator!=(const unexpected<E>& lhs, const unexpected<E>& rhs) noexcept {
    return !(lhs == rhs);
}

/// <summary>
/// Conditional storage base class enabling trivial destructibility propagation for expected.
/// </summary>
/// <typeparam name="T">Expected value type.</typeparam>
/// <typeparam name="E">Error value type.</typeparam>
/// <typeparam name="IsTrivial">Whether both T and E are trivially destructible.</typeparam>
template <typename T, typename E, bool IsTrivial = (std::is_trivially_destructible<T>::value && std::is_trivially_destructible<E>::value)>
struct ExpectedStorageBase;

/// <summary>
/// Specialization of ExpectedStorageBase for types requiring non-trivial destruction.
/// </summary>
/// <typeparam name="T">Expected value type.</typeparam>
/// <typeparam name="E">Error value type.</typeparam>
template <typename T, typename E>
struct ExpectedStorageBase<T, E, false> {
    union Storage {
        T m_val;
        E m_err;
        Storage() noexcept {}
        ~Storage() noexcept {}
    } m_storage;
    bool m_has_value;

    ExpectedStorageBase() noexcept : m_has_value(false) {}
    explicit ExpectedStorageBase(bool has_val) noexcept : m_has_value(has_val) {}

    ExpectedStorageBase(const ExpectedStorageBase&) noexcept : m_has_value(false) {}
    ExpectedStorageBase(ExpectedStorageBase&&) noexcept : m_has_value(false) {}
    ExpectedStorageBase& operator=(const ExpectedStorageBase&) noexcept { return *this; }
    ExpectedStorageBase& operator=(ExpectedStorageBase&&) noexcept { return *this; }

    ~ExpectedStorageBase() {
        destroy();
    }

    void destroy() noexcept {
        if (COMPAT_UNLIKELY(m_has_value)) {
            m_storage.m_val.~T();
        } else {
            m_storage.m_err.~E();
        }
    }
};

/// <summary>
/// Specialization of ExpectedStorageBase for trivially destructible types.
/// </summary>
/// <typeparam name="T">Expected value type.</typeparam>
/// <typeparam name="E">Error value type.</typeparam>
template <typename T, typename E>
struct ExpectedStorageBase<T, E, true> {
    union Storage {
        T m_val;
        E m_err;
        Storage() noexcept {}
        ~Storage() = default;
    } m_storage;
    bool m_has_value;

    ExpectedStorageBase() noexcept : m_has_value(false) {}
    explicit ExpectedStorageBase(bool has_val) noexcept : m_has_value(has_val) {}

    ExpectedStorageBase(const ExpectedStorageBase&) = default;
    ExpectedStorageBase(ExpectedStorageBase&&) = default;
    ExpectedStorageBase& operator=(const ExpectedStorageBase&) = default;
    ExpectedStorageBase& operator=(ExpectedStorageBase&&) = default;
    ~ExpectedStorageBase() = default;

    void destroy() noexcept {}
};

/// <summary>
/// Forward declaration of expected template.
/// </summary>
template <typename T, typename E>
class expected;

/// <summary>
/// Discriminated union result type representing either an expected value T or an unexpected error E.
/// Implemented using C++11 unrestricted union for backward compatibility with C++11 and C++14.
/// </summary>
/// <typeparam name="T">Expected value type.</typeparam>
/// <typeparam name="E">Error value type.</typeparam>
template <typename T, typename E>
class expected : private ExpectedStorageBase<T, E> {
    using Base = ExpectedStorageBase<T, E>;
    using Base::m_storage;
    using Base::m_has_value;
    using Base::destroy;

public:
    using value_type = T;
    using error_type = E;
    using unexpected_type = unexpected<E>;

    /// <summary>
    /// Default constructor initializing value when T is default-constructible.
    /// </summary>
    template <typename U = T, typename = typename std::enable_if<std::is_default_constructible<U>::value>::type>
    expected() : Base(true) {
        ::new (static_cast<void*>(&m_storage.m_val)) T();
    }

    /// <summary>
    /// Constructs expected holding a copy of val.
    /// </summary>
    /// <param name="val">Value to copy.</param>
    expected(const T& val) : Base(true) {
        ::new (static_cast<void*>(&m_storage.m_val)) T(val);
    }

    /// <summary>
    /// Constructs expected holding a moved val.
    /// </summary>
    /// <param name="val">Value to move.</param>
    expected(T&& val) : Base(true) {
        ::new (static_cast<void*>(&m_storage.m_val)) T(std::move(val));
    }

    /// <summary>
    /// Constructs expected holding a copy of an error.
    /// </summary>
    /// <param name="unexp">The unexpected error wrapper.</param>
    expected(const unexpected<E>& unexp) : Base(false) {
        ::new (static_cast<void*>(&m_storage.m_err)) E(unexp.error());
    }

    /// <summary>
    /// Constructs expected holding a moved error.
    /// </summary>
    /// <param name="unexp">The unexpected error wrapper.</param>
    expected(unexpected<E>&& unexp) : Base(false) {
        ::new (static_cast<void*>(&m_storage.m_err)) E(std::move(unexp.error()));
    }

    /// <summary>
    /// Constructs expected holding an in-place constructed error.
    /// </summary>
    /// <typeparam name="Args">Constructor argument types for error.</typeparam>
    /// <param name="args">Forwarded arguments.</param>
    template <typename... Args>
    explicit expected(unexpect_t, Args&&... args) : Base(false) {
        ::new (static_cast<void*>(&m_storage.m_err)) E(std::forward<Args>(args)...);
    }

    /// <summary>
    /// Copy constructor.
    /// </summary>
    /// <param name="other">Instance to copy.</param>
    expected(const expected& other) : Base(other.m_has_value) {
        if (COMPAT_UNLIKELY(m_has_value)) {
            ::new (static_cast<void*>(&m_storage.m_val)) T(other.m_storage.m_val);
        } else {
            ::new (static_cast<void*>(&m_storage.m_err)) E(other.m_storage.m_err);
        }
    }

    /// <summary>
    /// Move constructor.
    /// </summary>
    /// <param name="other">Instance to move.</param>
    expected(expected&& other) noexcept : Base(other.m_has_value) {
        if (COMPAT_UNLIKELY(m_has_value)) {
            ::new (static_cast<void*>(&m_storage.m_val)) T(std::move(other.m_storage.m_val));
        } else {
            ::new (static_cast<void*>(&m_storage.m_err)) E(std::move(other.m_storage.m_err));
        }
    }

    /// <summary>
    /// Defaulted destructor invoking ExpectedStorageBase conditionally.
    /// </summary>
    ~expected() = default;

    /// <summary>
    /// Copy assignment operator.
    /// </summary>
    /// <param name="other">Instance to copy.</param>
    /// <returns>Reference to self.</returns>
    expected& operator=(const expected& other) {
        if (this != &other) {
            destroy();
            m_has_value = other.m_has_value;
            if (COMPAT_UNLIKELY(m_has_value)) {
                ::new (static_cast<void*>(&m_storage.m_val)) T(other.m_storage.m_val);
            } else {
                ::new (static_cast<void*>(&m_storage.m_err)) E(other.m_storage.m_err);
            }
        }
        return *this;
    }

    /// <summary>
    /// Move assignment operator.
    /// </summary>
    /// <param name="other">Instance to move.</param>
    /// <returns>Reference to self.</returns>
    expected& operator=(expected&& other) noexcept {
        if (this != &other) {
            destroy();
            m_has_value = other.m_has_value;
            if (COMPAT_UNLIKELY(m_has_value)) {
                ::new (static_cast<void*>(&m_storage.m_val)) T(std::move(other.m_storage.m_val));
            } else {
                ::new (static_cast<void*>(&m_storage.m_err)) E(std::move(other.m_storage.m_err));
            }
        }
        return *this;
    }

    /// <summary>
    /// Value assignment operator.
    /// </summary>
    /// <param name="val">Value to copy-assign.</param>
    /// <returns>Reference to self.</returns>
    expected& operator=(const T& val) {
        destroy();
        m_has_value = true;
        ::new (static_cast<void*>(&m_storage.m_val)) T(val);
        return *this;
    }

    /// <summary>
    /// Value move assignment operator.
    /// </summary>
    /// <param name="val">Value to move-assign.</param>
    /// <returns>Reference to self.</returns>
    expected& operator=(T&& val) {
        destroy();
        m_has_value = true;
        ::new (static_cast<void*>(&m_storage.m_val)) T(std::move(val));
        return *this;
    }

    /// <summary>
    /// Unexpected assignment operator.
    /// </summary>
    /// <param name="unexp">Unexpected error wrapper to assign.</param>
    /// <returns>Reference to self.</returns>
    expected& operator=(const unexpected<E>& unexp) {
        destroy();
        m_has_value = false;
        ::new (static_cast<void*>(&m_storage.m_err)) E(unexp.error());
        return *this;
    }

    /// <summary>
    /// Unexpected move assignment operator.
    /// </summary>
    /// <param name="unexp">Unexpected error wrapper to move-assign.</param>
    /// <returns>Reference to self.</returns>
    expected& operator=(unexpected<E>&& unexp) {
        destroy();
        m_has_value = false;
        ::new (static_cast<void*>(&m_storage.m_err)) E(std::move(unexp.error()));
        return *this;
    }

    /// <summary>
    /// Checks whether expected contains a value.
    /// </summary>
    /// <returns>True if containing value, false if containing error.</returns>
    constexpr bool has_value() const noexcept {
        return m_has_value;
    }

    /// <summary>
    /// Boolean conversion operator testing value presence.
    /// </summary>
    /// <returns>True if containing value, false if containing error.</returns>
    constexpr explicit operator bool() const noexcept {
        return m_has_value;
    }

    /// <summary>
    /// Obtains mutable reference to the contained value or throws bad_expected_access.
    /// </summary>
    /// <returns>Reference to value.</returns>
    /// <exception cref="bad_expected_access&lt;E&gt;">Thrown when holding error.</exception>
    T& value() & {
        if (COMPAT_UNLIKELY(!m_has_value)) {
            COMPAT_THROW_OR_ABORT(bad_expected_access<E>(error()));
        }
        return m_storage.m_val;
    }

    /// <summary>
    /// Obtains const reference to the contained value or throws bad_expected_access.
    /// </summary>
    /// <returns>Const reference to value.</returns>
    /// <exception cref="bad_expected_access&lt;E&gt;">Thrown when holding error.</exception>
    const T& value() const & {
        if (COMPAT_UNLIKELY(!m_has_value)) {
            COMPAT_THROW_OR_ABORT(bad_expected_access<E>(error()));
        }
        return m_storage.m_val;
    }

    /// <summary>
    /// Obtains rvalue reference to the contained value or throws bad_expected_access.
    /// </summary>
    /// <returns>Rvalue reference to value.</returns>
    /// <exception cref="bad_expected_access&lt;E&gt;">Thrown when holding error.</exception>
    T&& value() && {
        if (COMPAT_UNLIKELY(!m_has_value)) {
            COMPAT_THROW_OR_ABORT(bad_expected_access<E>(std::move(error())));
        }
        return std::move(m_storage.m_val);
    }

    /// <summary>
    /// Obtains const rvalue reference to the contained value or throws bad_expected_access.
    /// </summary>
    /// <returns>Const rvalue reference to value.</returns>
    /// <exception cref="bad_expected_access&lt;E&gt;">Thrown when holding error.</exception>
    const T&& value() const && {
        if (COMPAT_UNLIKELY(!m_has_value)) {
            COMPAT_THROW_OR_ABORT(bad_expected_access<E>(std::move(error())));
        }
        return std::move(m_storage.m_val);
    }

    /// <summary>
    /// Obtains mutable reference to the contained error or throws bad_expected_access.
    /// </summary>
    /// <returns>Reference to error.</returns>
    /// <exception cref="bad_expected_access&lt;E&gt;">Thrown when holding value.</exception>
    E& error() & {
        if (COMPAT_UNLIKELY(m_has_value)) {
            COMPAT_THROW_OR_ABORT(bad_expected_access<E>());
        }
        return m_storage.m_err;
    }

    /// <summary>
    /// Obtains const reference to the contained error or throws bad_expected_access.
    /// </summary>
    /// <returns>Const reference to error.</returns>
    /// <exception cref="bad_expected_access&lt;E&gt;">Thrown when holding value.</exception>
    const E& error() const & {
        if (COMPAT_UNLIKELY(m_has_value)) {
            COMPAT_THROW_OR_ABORT(bad_expected_access<E>());
        }
        return m_storage.m_err;
    }

    /// <summary>
    /// Obtains rvalue reference to the contained error or throws bad_expected_access.
    /// </summary>
    /// <returns>Rvalue reference to error.</returns>
    /// <exception cref="bad_expected_access&lt;E&gt;">Thrown when holding value.</exception>
    E&& error() && {
        if (COMPAT_UNLIKELY(m_has_value)) {
            COMPAT_THROW_OR_ABORT(bad_expected_access<E>());
        }
        return std::move(m_storage.m_err);
    }

    /// <summary>
    /// Obtains const rvalue reference to the contained error or throws bad_expected_access.
    /// </summary>
    /// <returns>Const rvalue reference to error.</returns>
    /// <exception cref="bad_expected_access&lt;E&gt;">Thrown when holding value.</exception>
    const E&& error() const && {
        if (COMPAT_UNLIKELY(m_has_value)) {
            COMPAT_THROW_OR_ABORT(bad_expected_access<E>());
        }
        return std::move(m_storage.m_err);
    }

    /// <summary>
    /// Pointer member access operator.
    /// </summary>
    /// <returns>Pointer to value.</returns>
    T* operator->() noexcept {
        return &m_storage.m_val;
    }

    /// <summary>
    /// Const pointer member access operator.
    /// </summary>
    /// <returns>Const pointer to value.</returns>
    const T* operator->() const noexcept {
        return &m_storage.m_val;
    }

    /// <summary>
    /// Dereference operator.
    /// </summary>
    /// <returns>Reference to value.</returns>
    T& operator*() & noexcept {
        return m_storage.m_val;
    }

    /// <summary>
    /// Const dereference operator.
    /// </summary>
    /// <returns>Const reference to value.</returns>
    const T& operator*() const & noexcept {
        return m_storage.m_val;
    }

    /// <summary>
    /// Rvalue dereference operator.
    /// </summary>
    /// <returns>Rvalue reference to value.</returns>
    T&& operator*() && noexcept {
        return std::move(m_storage.m_val);
    }

    /// <summary>
    /// Const rvalue dereference operator.
    /// </summary>
    /// <returns>Const rvalue reference to value.</returns>
    const T&& operator*() const && noexcept {
        return std::move(m_storage.m_val);
    }

    /// <summary>
    /// Returns stored value or fallback default value.
    /// </summary>
    /// <typeparam name="U">Type convertible to T.</typeparam>
    /// <param name="default_val">Fallback value.</param>
    /// <returns>Stored value or fallback value.</returns>
    template <typename U>
    T value_or(U&& default_val) const & {
        return COMPAT_LIKELY(m_has_value) ? m_storage.m_val : static_cast<T>(std::forward<U>(default_val));
    }

    /// <summary>
    /// Returns stored rvalue value or fallback default value.
    /// </summary>
    /// <typeparam name="U">Type convertible to T.</typeparam>
    /// <param name="default_val">Fallback value.</param>
    /// <returns>Stored value or fallback value.</returns>
    template <typename U>
    T value_or(U&& default_val) && {
        return COMPAT_LIKELY(m_has_value) ? std::move(m_storage.m_val) : static_cast<T>(std::forward<U>(default_val));
    }

    /// <summary>
    /// Invokes f on value if present, returning the resulting expected.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <param name="f">Function to invoke with value.</param>
    /// <returns>Result of f or expected containing current error.</returns>
    template <typename F>
    auto and_then(F&& f) & -> decltype(std::forward<F>(f)(value())) {
        using Ret = typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)(value()))>::type>::type;
        if (has_value()) {
            return std::forward<F>(f)(value());
        }
        return Ret(unexpect, error());
    }

    /// <summary>
    /// Invokes f on const value if present, returning the resulting expected.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <param name="f">Function to invoke with const value.</param>
    /// <returns>Result of f or expected containing current error.</returns>
    template <typename F>
    auto and_then(F&& f) const & -> decltype(std::forward<F>(f)(value())) {
        using Ret = typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)(value()))>::type>::type;
        if (has_value()) {
            return std::forward<F>(f)(value());
        }
        return Ret(unexpect, error());
    }

    /// <summary>
    /// Invokes f on moved value if present, returning the resulting expected.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <param name="f">Function to invoke with moved value.</param>
    /// <returns>Result of f or expected containing current error.</returns>
    template <typename F>
    auto and_then(F&& f) && -> decltype(std::forward<F>(f)(std::move(value()))) {
        using Ret = typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)(std::move(value())))>::type>::type;
        if (has_value()) {
            return std::forward<F>(f)(std::move(value()));
        }
        return Ret(unexpect, std::move(error()));
    }

    /// <summary>
    /// Invokes f on const moved value if present, returning the resulting expected.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <param name="f">Function to invoke with const moved value.</param>
    /// <returns>Result of f or expected containing current error.</returns>
    template <typename F>
    auto and_then(F&& f) const && -> decltype(std::forward<F>(f)(std::move(value()))) {
        using Ret = typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)(std::move(value())))>::type>::type;
        if (has_value()) {
            return std::forward<F>(f)(std::move(value()));
        }
        return Ret(unexpect, std::move(error()));
    }

    /// <summary>
    /// Invokes f on error if present, returning the resulting expected or self.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <param name="f">Function to invoke with error.</param>
    /// <returns>Self if has value, or result of f.</returns>
    template <typename F>
    auto or_else(F&& f) & -> decltype(std::forward<F>(f)(error())) {
        using Ret = typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)(error()))>::type>::type;
        if (has_value()) {
            return Ret(value());
        }
        return std::forward<F>(f)(error());
    }

    /// <summary>
    /// Invokes f on const error if present, returning the resulting expected or self.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <param name="f">Function to invoke with const error.</param>
    /// <returns>Self if has value, or result of f.</returns>
    template <typename F>
    auto or_else(F&& f) const & -> decltype(std::forward<F>(f)(error())) {
        using Ret = typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)(error()))>::type>::type;
        if (has_value()) {
            return Ret(value());
        }
        return std::forward<F>(f)(error());
    }

    /// <summary>
    /// Invokes f on moved error if present, returning the resulting expected or self.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <param name="f">Function to invoke with moved error.</param>
    /// <returns>Self if has value, or result of f.</returns>
    template <typename F>
    auto or_else(F&& f) && -> decltype(std::forward<F>(f)(std::move(error()))) {
        using Ret = typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)(std::move(error())))>::type>::type;
        if (has_value()) {
            return Ret(std::move(value()));
        }
        return std::forward<F>(f)(std::move(error()));
    }

    /// <summary>
    /// Invokes f on const moved error if present, returning the resulting expected or self.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <param name="f">Function to invoke with const moved error.</param>
    /// <returns>Self if has value, or result of f.</returns>
    template <typename F>
    auto or_else(F&& f) const && -> decltype(std::forward<F>(f)(std::move(error()))) {
        using Ret = typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)(std::move(error())))>::type>::type;
        if (has_value()) {
            return Ret(std::move(value()));
        }
        return std::forward<F>(f)(std::move(error()));
    }

    /// <summary>
    /// Transforms the stored value using non-void returning f.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <typeparam name="Ret">Inferred invocation result.</typeparam>
    /// <param name="f">Transformation function.</param>
    /// <returns>Expected containing transformed value or error.</returns>
    template <typename F, typename Ret = decltype(std::declval<F>()(std::declval<T&>())),
              typename std::enable_if<!std::is_void<Ret>::value, int32_t>::type = 0>
    expected<typename std::remove_cv<typename std::remove_reference<Ret>::type>::type, E> transform(F&& f) & {
        using U = typename std::remove_cv<typename std::remove_reference<Ret>::type>::type;
        using Res = expected<U, E>;
        if (has_value()) {
            return Res(std::forward<F>(f)(value()));
        }
        return Res(unexpect, error());
    }

    /// <summary>
    /// Transforms the stored value using void-returning f.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <typeparam name="Ret">Inferred invocation result.</typeparam>
    /// <param name="f">Transformation function.</param>
    /// <returns>Expected of void containing success or error.</returns>
    template <typename F, typename Ret = decltype(std::declval<F>()(std::declval<T&>())),
              typename std::enable_if<std::is_void<Ret>::value, int32_t>::type = 0>
    expected<void, E> transform(F&& f) & {
        if (has_value()) {
            std::forward<F>(f)(value());
            return expected<void, E>();
        }
        return expected<void, E>(unexpect, error());
    }

    /// <summary>
    /// Transforms the const stored value using non-void returning f.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <typeparam name="Ret">Inferred invocation result.</typeparam>
    /// <param name="f">Transformation function.</param>
    /// <returns>Expected containing transformed value or error.</returns>
    template <typename F, typename Ret = decltype(std::declval<F>()(std::declval<const T&>())),
              typename std::enable_if<!std::is_void<Ret>::value, int32_t>::type = 0>
    expected<typename std::remove_cv<typename std::remove_reference<Ret>::type>::type, E> transform(F&& f) const & {
        using U = typename std::remove_cv<typename std::remove_reference<Ret>::type>::type;
        using Res = expected<U, E>;
        if (has_value()) {
            return Res(std::forward<F>(f)(value()));
        }
        return Res(unexpect, error());
    }

    /// <summary>
    /// Transforms the const stored value using void-returning f.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <typeparam name="Ret">Inferred invocation result.</typeparam>
    /// <param name="f">Transformation function.</param>
    /// <returns>Expected of void containing success or error.</returns>
    template <typename F, typename Ret = decltype(std::declval<F>()(std::declval<const T&>())),
              typename std::enable_if<std::is_void<Ret>::value, int32_t>::type = 0>
    expected<void, E> transform(F&& f) const & {
        if (has_value()) {
            std::forward<F>(f)(value());
            return expected<void, E>();
        }
        return expected<void, E>(unexpect, error());
    }

    /// <summary>
    /// Transforms the moved stored value using non-void returning f.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <typeparam name="Ret">Inferred invocation result.</typeparam>
    /// <param name="f">Transformation function.</param>
    /// <returns>Expected containing transformed value or error.</returns>
    template <typename F, typename Ret = decltype(std::declval<F>()(std::declval<T&&>())),
              typename std::enable_if<!std::is_void<Ret>::value, int32_t>::type = 0>
    expected<typename std::remove_cv<typename std::remove_reference<Ret>::type>::type, E> transform(F&& f) && {
        using U = typename std::remove_cv<typename std::remove_reference<Ret>::type>::type;
        using Res = expected<U, E>;
        if (has_value()) {
            return Res(std::forward<F>(f)(std::move(value())));
        }
        return Res(unexpect, std::move(error()));
    }

    /// <summary>
    /// Transforms the moved stored value using void-returning f.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <typeparam name="Ret">Inferred invocation result.</typeparam>
    /// <param name="f">Transformation function.</param>
    /// <returns>Expected of void containing success or error.</returns>
    template <typename F, typename Ret = decltype(std::declval<F>()(std::declval<T&&>())),
              typename std::enable_if<std::is_void<Ret>::value, int32_t>::type = 0>
    expected<void, E> transform(F&& f) && {
        if (has_value()) {
            std::forward<F>(f)(std::move(value()));
            return expected<void, E>();
        }
        return expected<void, E>(unexpect, std::move(error()));
    }

    /// <summary>
    /// Transforms the const moved stored value using non-void returning f.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <typeparam name="Ret">Inferred invocation result.</typeparam>
    /// <param name="f">Transformation function.</param>
    /// <returns>Expected containing transformed value or error.</returns>
    template <typename F, typename Ret = decltype(std::declval<F>()(std::declval<const T&&>())),
              typename std::enable_if<!std::is_void<Ret>::value, int32_t>::type = 0>
    expected<typename std::remove_cv<typename std::remove_reference<Ret>::type>::type, E> transform(F&& f) const && {
        using U = typename std::remove_cv<typename std::remove_reference<Ret>::type>::type;
        using Res = expected<U, E>;
        if (has_value()) {
            return Res(std::forward<F>(f)(std::move(value())));
        }
        return Res(unexpect, std::move(error()));
    }

    /// <summary>
    /// Transforms the const moved stored value using void-returning f.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <typeparam name="Ret">Inferred invocation result.</typeparam>
    /// <param name="f">Transformation function.</param>
    /// <returns>Expected of void containing success or error.</returns>
    template <typename F, typename Ret = decltype(std::declval<F>()(std::declval<const T&&>())),
              typename std::enable_if<std::is_void<Ret>::value, int32_t>::type = 0>
    expected<void, E> transform(F&& f) const && {
        if (has_value()) {
            std::forward<F>(f)(std::move(value()));
            return expected<void, E>();
        }
        return expected<void, E>(unexpect, std::move(error()));
    }

    /// <summary>
    /// Transforms the stored error using f.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <param name="f">Error transformation function.</param>
    /// <returns>Expected containing value or transformed error.</returns>
    template <typename F>
    auto transform_error(F&& f) & -> expected<T, typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)(error()))>::type>::type> {
        using G = typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)(error()))>::type>::type;
        using Res = expected<T, G>;
        if (has_value()) {
            return Res(value());
        }
        return Res(unexpect, std::forward<F>(f)(error()));
    }

    /// <summary>
    /// Transforms the const stored error using f.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <param name="f">Error transformation function.</param>
    /// <returns>Expected containing value or transformed error.</returns>
    template <typename F>
    auto transform_error(F&& f) const & -> expected<T, typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)(error()))>::type>::type> {
        using G = typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)(error()))>::type>::type;
        using Res = expected<T, G>;
        if (has_value()) {
            return Res(value());
        }
        return Res(unexpect, std::forward<F>(f)(error()));
    }

    /// <summary>
    /// Transforms the moved stored error using f.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <param name="f">Error transformation function.</param>
    /// <returns>Expected containing value or transformed error.</returns>
    template <typename F>
    auto transform_error(F&& f) && -> expected<T, typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)(std::move(error())))>::type>::type> {
        using G = typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)(std::move(error())))>::type>::type;
        using Res = expected<T, G>;
        if (has_value()) {
            return Res(std::move(value()));
        }
        return Res(unexpect, std::forward<F>(f)(std::move(error())));
    }

    /// <summary>
    /// Transforms the const moved stored error using f.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <param name="f">Error transformation function.</param>
    /// <returns>Expected containing value or transformed error.</returns>
    template <typename F>
    auto transform_error(F&& f) const && -> expected<T, typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)(std::move(error())))>::type>::type> {
        using G = typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)(std::move(error())))>::type>::type;
        using Res = expected<T, G>;
        if (has_value()) {
            return Res(std::move(value()));
        }
        return Res(unexpect, std::forward<F>(f)(std::move(error())));
    }
};

/// <summary>
/// Conditional storage base class enabling trivial destructibility propagation for expected<void, E>.
/// </summary>
/// <typeparam name="E">Error value type.</typeparam>
/// <typeparam name="IsTrivial">Whether E is trivially destructible.</typeparam>
template <typename E, bool IsTrivial = std::is_trivially_destructible<E>::value>
struct ExpectedVoidStorageBase;

/// <summary>
/// Specialization of ExpectedVoidStorageBase for non-trivially destructible error types.
/// </summary>
/// <typeparam name="E">Error value type.</typeparam>
template <typename E>
struct ExpectedVoidStorageBase<E, false> {
    union Storage {
        char m_dummy;
        E m_err;
        Storage() noexcept : m_dummy(0) {}
        ~Storage() noexcept {}
    } m_storage;
    bool m_has_value;

    ExpectedVoidStorageBase() noexcept : m_has_value(false) {}
    explicit ExpectedVoidStorageBase(bool has_val) noexcept : m_has_value(has_val) {}

    ExpectedVoidStorageBase(const ExpectedVoidStorageBase&) noexcept : m_has_value(false) {}
    ExpectedVoidStorageBase(ExpectedVoidStorageBase&&) noexcept : m_has_value(false) {}
    ExpectedVoidStorageBase& operator=(const ExpectedVoidStorageBase&) noexcept { return *this; }
    ExpectedVoidStorageBase& operator=(ExpectedVoidStorageBase&&) noexcept { return *this; }

    ~ExpectedVoidStorageBase() {
        destroy();
    }

    void destroy() noexcept {
        if (COMPAT_UNLIKELY(!m_has_value)) {
            m_storage.m_err.~E();
        }
    }
};

/// <summary>
/// Specialization of ExpectedVoidStorageBase for trivially destructible error types.
/// </summary>
/// <typeparam name="E">Error value type.</typeparam>
template <typename E>
struct ExpectedVoidStorageBase<E, true> {
    union Storage {
        char m_dummy;
        E m_err;
        Storage() noexcept : m_dummy(0) {}
        ~Storage() = default;
    } m_storage;
    bool m_has_value;

    ExpectedVoidStorageBase() noexcept : m_has_value(false) {}
    explicit ExpectedVoidStorageBase(bool has_val) noexcept : m_has_value(has_val) {}

    ExpectedVoidStorageBase(const ExpectedVoidStorageBase&) = default;
    ExpectedVoidStorageBase(ExpectedVoidStorageBase&&) = default;
    ExpectedVoidStorageBase& operator=(const ExpectedVoidStorageBase&) = default;
    ExpectedVoidStorageBase& operator=(ExpectedVoidStorageBase&&) = default;
    ~ExpectedVoidStorageBase() = default;

    void destroy() noexcept {}
};

/// <summary>
/// Partial specialization of expected for void value type.
/// </summary>
/// <typeparam name="E">Error value type.</typeparam>
template <typename E>
class expected<void, E> : private ExpectedVoidStorageBase<E> {
    using Base = ExpectedVoidStorageBase<E>;
    using Base::m_storage;
    using Base::m_has_value;
    using Base::destroy;

public:
    using value_type = void;
    using error_type = E;
    using unexpected_type = unexpected<E>;

    /// <summary>
    /// Default constructor initializing success state.
    /// </summary>
    expected() : Base(true) {}

    /// <summary>
    /// Constructs expected from copy of error.
    /// </summary>
    /// <param name="unexp">The unexpected wrapper.</param>
    expected(const unexpected<E>& unexp) : Base(false) {
        ::new (static_cast<void*>(&m_storage.m_err)) E(unexp.error());
    }

    /// <summary>
    /// Constructs expected from moved error.
    /// </summary>
    /// <param name="unexp">The unexpected wrapper.</param>
    expected(unexpected<E>&& unexp) : Base(false) {
        ::new (static_cast<void*>(&m_storage.m_err)) E(std::move(unexp.error()));
    }

    /// <summary>
    /// In-place error constructor.
    /// </summary>
    /// <typeparam name="Args">Constructor argument types.</typeparam>
    /// <param name="args">Forwarded constructor arguments.</param>
    template <typename... Args>
    explicit expected(unexpect_t, Args&&... args) : Base(false) {
        ::new (static_cast<void*>(&m_storage.m_err)) E(std::forward<Args>(args)...);
    }

    /// <summary>
    /// Copy constructor.
    /// </summary>
    /// <param name="other">Instance to copy.</param>
    expected(const expected& other) : Base(other.m_has_value) {
        if (COMPAT_UNLIKELY(!m_has_value)) {
            ::new (static_cast<void*>(&m_storage.m_err)) E(other.m_storage.m_err);
        }
    }

    /// <summary>
    /// Move constructor.
    /// </summary>
    /// <param name="other">Instance to move.</param>
    expected(expected&& other) noexcept : Base(other.m_has_value) {
        if (COMPAT_UNLIKELY(!m_has_value)) {
            ::new (static_cast<void*>(&m_storage.m_err)) E(std::move(other.m_storage.m_err));
        }
    }

    /// <summary>
    /// Defaulted destructor invoking ExpectedVoidStorageBase conditionally.
    /// </summary>
    ~expected() = default;

    /// <summary>
    /// Copy assignment operator.
    /// </summary>
    /// <param name="other">Instance to copy.</param>
    /// <returns>Reference to self.</returns>
    expected& operator=(const expected& other) {
        if (this != &other) {
            destroy();
            m_has_value = other.m_has_value;
            if (COMPAT_UNLIKELY(!m_has_value)) {
                ::new (static_cast<void*>(&m_storage.m_err)) E(other.m_storage.m_err);
            }
        }
        return *this;
    }

    /// <summary>
    /// Move assignment operator.
    /// </summary>
    /// <param name="other">Instance to move.</param>
    /// <returns>Reference to self.</returns>
    expected& operator=(expected&& other) noexcept {
        if (this != &other) {
            destroy();
            m_has_value = other.m_has_value;
            if (COMPAT_UNLIKELY(!m_has_value)) {
                ::new (static_cast<void*>(&m_storage.m_err)) E(std::move(other.m_storage.m_err));
            }
        }
        return *this;
    }

    /// <summary>
    /// Unexpected assignment operator.
    /// </summary>
    /// <param name="unexp">Unexpected error wrapper.</param>
    /// <returns>Reference to self.</returns>
    expected& operator=(const unexpected<E>& unexp) {
        destroy();
        m_has_value = false;
        ::new (static_cast<void*>(&m_storage.m_err)) E(unexp.error());
        return *this;
    }

    /// <summary>
    /// Unexpected move assignment operator.
    /// </summary>
    /// <param name="unexp">Unexpected error wrapper.</param>
    /// <returns>Reference to self.</returns>
    expected& operator=(unexpected<E>&& unexp) {
        destroy();
        m_has_value = false;
        ::new (static_cast<void*>(&m_storage.m_err)) E(std::move(unexp.error()));
        return *this;
    }

    /// <summary>
    /// Checks whether expected is in success state.
    /// </summary>
    /// <returns>True if success, false if error.</returns>
    constexpr bool has_value() const noexcept {
        return m_has_value;
    }

    /// <summary>
    /// Boolean conversion operator testing success state.
    /// </summary>
    /// <returns>True if success, false if error.</returns>
    constexpr explicit operator bool() const noexcept {
        return m_has_value;
    }

    /// <summary>
    /// Dereference operator for expected of void.
    /// </summary>
    COMPAT_CONSTEXPR_14 void operator*() const noexcept {}

    /// <summary>
    /// Verifies success state or throws bad_expected_access.
    /// </summary>
    /// <exception cref="bad_expected_access&lt;E&gt;">Thrown when holding error.</exception>
    void value() const & {
        if (COMPAT_UNLIKELY(!m_has_value)) {
            COMPAT_THROW_OR_ABORT(bad_expected_access<E>(error()));
        }
    }

    /// <summary>
    /// Verifies success state on rvalue or throws bad_expected_access.
    /// </summary>
    /// <exception cref="bad_expected_access&lt;E&gt;">Thrown when holding error.</exception>
    void value() && {
        if (COMPAT_UNLIKELY(!m_has_value)) {
            COMPAT_THROW_OR_ABORT(bad_expected_access<E>(std::move(error())));
        }
    }

    /// <summary>
    /// Obtains mutable reference to error or throws bad_expected_access.
    /// </summary>
    /// <returns>Reference to error.</returns>
    /// <exception cref="bad_expected_access&lt;E&gt;">Thrown when in success state.</exception>
    E& error() & {
        if (COMPAT_UNLIKELY(m_has_value)) {
            COMPAT_THROW_OR_ABORT(bad_expected_access<E>());
        }
        return m_storage.m_err;
    }

    /// <summary>
    /// Obtains const reference to error or throws bad_expected_access.
    /// </summary>
    /// <returns>Const reference to error.</returns>
    /// <exception cref="bad_expected_access&lt;E&gt;">Thrown when in success state.</exception>
    const E& error() const & {
        if (COMPAT_UNLIKELY(m_has_value)) {
            COMPAT_THROW_OR_ABORT(bad_expected_access<E>());
        }
        return m_storage.m_err;
    }

    /// <summary>
    /// Obtains rvalue reference to error or throws bad_expected_access.
    /// </summary>
    /// <returns>Rvalue reference to error.</returns>
    /// <exception cref="bad_expected_access&lt;E&gt;">Thrown when in success state.</exception>
    E&& error() && {
        if (COMPAT_UNLIKELY(m_has_value)) {
            COMPAT_THROW_OR_ABORT(bad_expected_access<E>());
        }
        return std::move(m_storage.m_err);
    }

    /// <summary>
    /// Obtains const rvalue reference to error or throws bad_expected_access.
    /// </summary>
    /// <returns>Const rvalue reference to error.</returns>
    /// <exception cref="bad_expected_access&lt;E&gt;">Thrown when in success state.</exception>
    const E&& error() const && {
        if (COMPAT_UNLIKELY(m_has_value)) {
            COMPAT_THROW_OR_ABORT(bad_expected_access<E>());
        }
        return std::move(m_storage.m_err);
    }

    /// <summary>
    /// Invokes f when in success state.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <param name="f">Function taking no arguments.</param>
    /// <returns>Result of f or expected holding current error.</returns>
    template <typename F>
    auto and_then(F&& f) & -> decltype(std::forward<F>(f)()) {
        using Ret = typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)())>::type>::type;
        if (has_value()) {
            return std::forward<F>(f)();
        }
        return Ret(unexpect, error());
    }

    /// <summary>
    /// Invokes f on const instance when in success state.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <param name="f">Function taking no arguments.</param>
    /// <returns>Result of f or expected holding current error.</returns>
    template <typename F>
    auto and_then(F&& f) const & -> decltype(std::forward<F>(f)()) {
        using Ret = typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)())>::type>::type;
        if (has_value()) {
            return std::forward<F>(f)();
        }
        return Ret(unexpect, error());
    }

    /// <summary>
    /// Invokes f on rvalue when in success state.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <param name="f">Function taking no arguments.</param>
    /// <returns>Result of f or expected holding moved error.</returns>
    template <typename F>
    auto and_then(F&& f) && -> decltype(std::forward<F>(f)()) {
        using Ret = typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)())>::type>::type;
        if (has_value()) {
            return std::forward<F>(f)();
        }
        return Ret(unexpect, std::move(error()));
    }

    /// <summary>
    /// Invokes f on const rvalue when in success state.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <param name="f">Function taking no arguments.</param>
    /// <returns>Result of f or expected holding moved error.</returns>
    template <typename F>
    auto and_then(F&& f) const && -> decltype(std::forward<F>(f)()) {
        using Ret = typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)())>::type>::type;
        if (has_value()) {
            return std::forward<F>(f)();
        }
        return Ret(unexpect, std::move(error()));
    }

    /// <summary>
    /// Invokes f with error when holding error, or returns success expected.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <param name="f">Function taking error.</param>
    /// <returns>Self on success or result of f on error.</returns>
    template <typename F>
    auto or_else(F&& f) & -> decltype(std::forward<F>(f)(error())) {
        using Ret = typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)(error()))>::type>::type;
        if (has_value()) {
            return Ret();
        }
        return std::forward<F>(f)(error());
    }

    /// <summary>
    /// Invokes f with const error when holding error, or returns success expected.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <param name="f">Function taking const error.</param>
    /// <returns>Self on success or result of f on error.</returns>
    template <typename F>
    auto or_else(F&& f) const & -> decltype(std::forward<F>(f)(error())) {
        using Ret = typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)(error()))>::type>::type;
        if (has_value()) {
            return Ret();
        }
        return std::forward<F>(f)(error());
    }

    /// <summary>
    /// Invokes f with moved error when holding error, or returns success expected.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <param name="f">Function taking moved error.</param>
    /// <returns>Self on success or result of f on error.</returns>
    template <typename F>
    auto or_else(F&& f) && -> decltype(std::forward<F>(f)(std::move(error()))) {
        using Ret = typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)(std::move(error())))>::type>::type;
        if (has_value()) {
            return Ret();
        }
        return std::forward<F>(f)(std::move(error()));
    }

    /// <summary>
    /// Invokes f with const moved error when holding error, or returns success expected.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <param name="f">Function taking const moved error.</param>
    /// <returns>Self on success or result of f on error.</returns>
    template <typename F>
    auto or_else(F&& f) const && -> decltype(std::forward<F>(f)(std::move(error()))) {
        using Ret = typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)(std::move(error())))>::type>::type;
        if (has_value()) {
            return Ret();
        }
        return std::forward<F>(f)(std::move(error()));
    }

    /// <summary>
    /// Transforms success state using non-void returning f.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <typeparam name="Ret">Inferred invocation result.</typeparam>
    /// <param name="f">Function taking no arguments.</param>
    /// <returns>Expected containing transformed value or error.</returns>
    template <typename F, typename Ret = decltype(std::declval<F>()()),
              typename std::enable_if<!std::is_void<Ret>::value, int32_t>::type = 0>
    expected<typename std::remove_cv<typename std::remove_reference<Ret>::type>::type, E> transform(F&& f) & {
        using U = typename std::remove_cv<typename std::remove_reference<Ret>::type>::type;
        using Res = expected<U, E>;
        if (has_value()) {
            return Res(std::forward<F>(f)());
        }
        return Res(unexpect, error());
    }

    /// <summary>
    /// Transforms success state using void-returning f.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <typeparam name="Ret">Inferred invocation result.</typeparam>
    /// <param name="f">Function taking no arguments.</param>
    /// <returns>Expected of void containing success or error.</returns>
    template <typename F, typename Ret = decltype(std::declval<F>()()),
              typename std::enable_if<std::is_void<Ret>::value, int32_t>::type = 0>
    expected<void, E> transform(F&& f) & {
        if (has_value()) {
            std::forward<F>(f)();
            return expected<void, E>();
        }
        return expected<void, E>(unexpect, error());
    }

    /// <summary>
    /// Transforms const success state using non-void returning f.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <typeparam name="Ret">Inferred invocation result.</typeparam>
    /// <param name="f">Function taking no arguments.</param>
    /// <returns>Expected containing transformed value or error.</returns>
    template <typename F, typename Ret = decltype(std::declval<F>()()),
              typename std::enable_if<!std::is_void<Ret>::value, int32_t>::type = 0>
    expected<typename std::remove_cv<typename std::remove_reference<Ret>::type>::type, E> transform(F&& f) const & {
        using U = typename std::remove_cv<typename std::remove_reference<Ret>::type>::type;
        using Res = expected<U, E>;
        if (has_value()) {
            return Res(std::forward<F>(f)());
        }
        return Res(unexpect, error());
    }

    /// <summary>
    /// Transforms const success state using void-returning f.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <typeparam name="Ret">Inferred invocation result.</typeparam>
    /// <param name="f">Function taking no arguments.</param>
    /// <returns>Expected of void containing success or error.</returns>
    template <typename F, typename Ret = decltype(std::declval<F>()()),
              typename std::enable_if<std::is_void<Ret>::value, int32_t>::type = 0>
    expected<void, E> transform(F&& f) const & {
        if (has_value()) {
            std::forward<F>(f)();
            return expected<void, E>();
        }
        return expected<void, E>(unexpect, error());
    }

    /// <summary>
    /// Transforms moved success state using non-void returning f.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <typeparam name="Ret">Inferred invocation result.</typeparam>
    /// <param name="f">Function taking no arguments.</param>
    /// <returns>Expected containing transformed value or error.</returns>
    template <typename F, typename Ret = decltype(std::declval<F>()()),
              typename std::enable_if<!std::is_void<Ret>::value, int32_t>::type = 0>
    expected<typename std::remove_cv<typename std::remove_reference<Ret>::type>::type, E> transform(F&& f) && {
        using U = typename std::remove_cv<typename std::remove_reference<Ret>::type>::type;
        using Res = expected<U, E>;
        if (has_value()) {
            return Res(std::forward<F>(f)());
        }
        return Res(unexpect, std::move(error()));
    }

    /// <summary>
    /// Transforms moved success state using void-returning f.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <typeparam name="Ret">Inferred invocation result.</typeparam>
    /// <param name="f">Function taking no arguments.</param>
    /// <returns>Expected of void containing success or error.</returns>
    template <typename F, typename Ret = decltype(std::declval<F>()()),
              typename std::enable_if<std::is_void<Ret>::value, int32_t>::type = 0>
    expected<void, E> transform(F&& f) && {
        if (has_value()) {
            std::forward<F>(f)();
            return expected<void, E>();
        }
        return expected<void, E>(unexpect, std::move(error()));
    }

    /// <summary>
    /// Transforms const moved success state using non-void returning f.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <typeparam name="Ret">Inferred invocation result.</typeparam>
    /// <param name="f">Function taking no arguments.</param>
    /// <returns>Expected containing transformed value or error.</returns>
    template <typename F, typename Ret = decltype(std::declval<F>()()),
              typename std::enable_if<!std::is_void<Ret>::value, int32_t>::type = 0>
    expected<typename std::remove_cv<typename std::remove_reference<Ret>::type>::type, E> transform(F&& f) const && {
        using U = typename std::remove_cv<typename std::remove_reference<Ret>::type>::type;
        using Res = expected<U, E>;
        if (has_value()) {
            return Res(std::forward<F>(f)());
        }
        return Res(unexpect, std::move(error()));
    }

    /// <summary>
    /// Transforms const moved success state using void-returning f.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <typeparam name="Ret">Inferred invocation result.</typeparam>
    /// <param name="f">Function taking no arguments.</param>
    /// <returns>Expected of void containing success or error.</returns>
    template <typename F, typename Ret = decltype(std::declval<F>()()),
              typename std::enable_if<std::is_void<Ret>::value, int32_t>::type = 0>
    expected<void, E> transform(F&& f) const && {
        if (has_value()) {
            std::forward<F>(f)();
            return expected<void, E>();
        }
        return expected<void, E>(unexpect, std::move(error()));
    }

    /// <summary>
    /// Transforms stored error using f.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <param name="f">Error transformation function.</param>
    /// <returns>Expected of void containing success or transformed error.</returns>
    template <typename F>
    auto transform_error(F&& f) & -> expected<void, typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)(error()))>::type>::type> {
        using G = typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)(error()))>::type>::type;
        using Res = expected<void, G>;
        if (has_value()) {
            return Res();
        }
        return Res(unexpect, std::forward<F>(f)(error()));
    }

    /// <summary>
    /// Transforms const stored error using f.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <param name="f">Error transformation function.</param>
    /// <returns>Expected of void containing success or transformed error.</returns>
    template <typename F>
    auto transform_error(F&& f) const & -> expected<void, typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)(error()))>::type>::type> {
        using G = typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)(error()))>::type>::type;
        using Res = expected<void, G>;
        if (has_value()) {
            return Res();
        }
        return Res(unexpect, std::forward<F>(f)(error()));
    }

    /// <summary>
    /// Transforms moved stored error using f.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <param name="f">Error transformation function.</param>
    /// <returns>Expected of void containing success or transformed error.</returns>
    template <typename F>
    auto transform_error(F&& f) && -> expected<void, typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)(std::move(error())))>::type>::type> {
        using G = typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)(std::move(error())))>::type>::type;
        using Res = expected<void, G>;
        if (has_value()) {
            return Res();
        }
        return Res(unexpect, std::forward<F>(f)(std::move(error())));
    }

    /// <summary>
    /// Transforms const moved stored error using f.
    /// </summary>
    /// <typeparam name="F">Callable type.</typeparam>
    /// <param name="f">Error transformation function.</param>
    /// <returns>Expected of void containing success or transformed error.</returns>
    template <typename F>
    auto transform_error(F&& f) const && -> expected<void, typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)(std::move(error())))>::type>::type> {
        using G = typename std::remove_cv<typename std::remove_reference<decltype(std::forward<F>(f)(std::move(error())))>::type>::type;
        using Res = expected<void, G>;
        if (has_value()) {
            return Res();
        }
        return Res(unexpect, std::forward<F>(f)(std::move(error())));
    }
};

} // namespace union_impl

#if !defined(COMPAT_BENCHMARK_ISOLATE_EXPECTED)
using union_impl::unexpect_t;
using union_impl::unexpect;
using union_impl::unexpected;
using union_impl::expected;
using union_impl::bad_expected_access;
#endif

} // namespace detail

using detail::bad_expected_access;

} // namespace compat

#endif // (!COMPAT_HAS_STD_EXPECTED && !COMPAT_HAS_STD_VARIANT) || defined(COMPAT_ENABLE_UNION_EXPECTED)
