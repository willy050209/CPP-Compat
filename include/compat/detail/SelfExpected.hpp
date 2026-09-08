#pragma once

#include "../Config.hpp"

#if (!COMPAT_HAS_STD_EXPECTED && COMPAT_HAS_STD_VARIANT) || defined(COMPAT_ENABLE_VARIANT_EXPECTED)

#include <variant>
#include <utility>
#include <type_traits>
#include <stdexcept>

namespace compat {
namespace detail {
namespace variant_impl {

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
template <typename E>
inline constexpr bool operator==(const unexpected<E>& lhs, const unexpected<E>& rhs) noexcept {
    return lhs.error() == rhs.error();
}

/// <summary>
/// Inequality comparison for unexpected.
/// </summary>
template <typename E>
inline constexpr bool operator!=(const unexpected<E>& lhs, const unexpected<E>& rhs) noexcept {
    return !(lhs == rhs);
}

/// <summary>
/// Expected implementation built on top of std::variant for C++17 and C++20 environments.
/// </summary>
/// <typeparam name="T">Expected value type.</typeparam>
/// <typeparam name="E">Error value type.</typeparam>
template <typename T, typename E>
class expected {
public:
    using value_type = T;
    using error_type = E;
    using unexpected_type = unexpected<E>;

    /// <summary>
    /// Default constructor initializing value.
    /// </summary>
    template <typename U = T, typename = typename std::enable_if<std::is_default_constructible<U>::value>::type>
    constexpr expected() : m_var(T()) {}

    /// <summary>
    /// Value copy constructor.
    /// </summary>
    /// <param name="val">Value to copy.</param>
    constexpr expected(const T& val) : m_var(val) {}

    /// <summary>
    /// Value move constructor.
    /// </summary>
    /// <param name="val">Value to move.</param>
    constexpr expected(T&& val) : m_var(std::move(val)) {}

    /// <summary>
    /// Unexpected copy constructor.
    /// </summary>
    /// <param name="unexp">Unexpected error to copy.</param>
    constexpr expected(const unexpected<E>& unexp) : m_var(unexp) {}

    /// <summary>
    /// Unexpected move constructor.
    /// </summary>
    /// <param name="unexp">Unexpected error to move.</param>
    constexpr expected(unexpected<E>&& unexp) : m_var(std::move(unexp)) {}

    /// <summary>
    /// In-place error constructor.
    /// </summary>
    /// <typeparam name="Args">Constructor argument types.</typeparam>
    /// <param name="args">Forwarded arguments.</param>
    template <typename... Args>
    explicit constexpr expected(unexpect_t, Args&&... args)
        : m_var(std::in_place_type<unexpected<E>>, std::forward<Args>(args)...) {}

    expected(const expected&) = default;
    expected(expected&&) = default;
    expected& operator=(const expected&) = default;
    expected& operator=(expected&&) = default;

    expected& operator=(const T& val) {
        m_var = val;
        return *this;
    }

    expected& operator=(T&& val) {
        m_var = std::move(val);
        return *this;
    }

    expected& operator=(const unexpected<E>& unexp) {
        m_var = unexp;
        return *this;
    }

    expected& operator=(unexpected<E>&& unexp) {
        m_var = std::move(unexp);
        return *this;
    }

    /// <summary>
    /// Checks whether expected contains a value.
    /// </summary>
    /// <returns>True if value is present, false if error.</returns>
    constexpr bool has_value() const noexcept {
        return std::holds_alternative<T>(m_var);
    }

    /// <summary>
    /// Boolean conversion operator testing value presence.
    /// </summary>
    /// <returns>True if value is present, false if error.</returns>
    constexpr explicit operator bool() const noexcept {
        return has_value();
    }

    /// <summary>
    /// Returns mutable reference to value or throws std::logic_error.
    /// </summary>
    /// <returns>Reference to value.</returns>
    /// <exception cref="std::logic_error">Thrown when holding error.</exception>
    T& value() & {
        if (!has_value()) {
            throw std::logic_error("Bad expected access: object contains error");
        }
        return std::get<T>(m_var);
    }

    /// <summary>
    /// Returns const reference to value or throws std::logic_error.
    /// </summary>
    /// <returns>Const reference to value.</returns>
    /// <exception cref="std::logic_error">Thrown when holding error.</exception>
    const T& value() const & {
        if (!has_value()) {
            throw std::logic_error("Bad expected access: object contains error");
        }
        return std::get<T>(m_var);
    }

    /// <summary>
    /// Returns rvalue reference to value or throws std::logic_error.
    /// </summary>
    /// <returns>Rvalue reference to value.</returns>
    /// <exception cref="std::logic_error">Thrown when holding error.</exception>
    T&& value() && {
        if (!has_value()) {
            throw std::logic_error("Bad expected access: object contains error");
        }
        return std::get<T>(std::move(m_var));
    }

    /// <summary>
    /// Returns const rvalue reference to value or throws std::logic_error.
    /// </summary>
    /// <returns>Const rvalue reference to value.</returns>
    /// <exception cref="std::logic_error">Thrown when holding error.</exception>
    const T&& value() const && {
        if (!has_value()) {
            throw std::logic_error("Bad expected access: object contains error");
        }
        return std::get<T>(std::move(m_var));
    }

    /// <summary>
    /// Returns mutable reference to error or throws std::logic_error.
    /// </summary>
    /// <returns>Reference to error.</returns>
    /// <exception cref="std::logic_error">Thrown when holding value.</exception>
    E& error() & {
        if (has_value()) {
            throw std::logic_error("Bad expected access: object contains value");
        }
        return std::get<unexpected<E>>(m_var).error();
    }

    /// <summary>
    /// Returns const reference to error or throws std::logic_error.
    /// </summary>
    /// <returns>Const reference to error.</returns>
    /// <exception cref="std::logic_error">Thrown when holding value.</exception>
    const E& error() const & {
        if (has_value()) {
            throw std::logic_error("Bad expected access: object contains value");
        }
        return std::get<unexpected<E>>(m_var).error();
    }

    /// <summary>
    /// Returns rvalue reference to error or throws std::logic_error.
    /// </summary>
    /// <returns>Rvalue reference to error.</returns>
    /// <exception cref="std::logic_error">Thrown when holding value.</exception>
    E&& error() && {
        if (has_value()) {
            throw std::logic_error("Bad expected access: object contains value");
        }
        return std::get<unexpected<E>>(std::move(m_var)).error();
    }

    /// <summary>
    /// Returns const rvalue reference to error or throws std::logic_error.
    /// </summary>
    /// <returns>Const rvalue reference to error.</returns>
    /// <exception cref="std::logic_error">Thrown when holding value.</exception>
    const E&& error() const && {
        if (has_value()) {
            throw std::logic_error("Bad expected access: object contains value");
        }
        return std::get<unexpected<E>>(std::move(m_var)).error();
    }

    /// <summary>
    /// Pointer member access operator.
    /// </summary>
    /// <returns>Pointer to value.</returns>
    T* operator->() noexcept {
        return &std::get<T>(m_var);
    }

    /// <summary>
    /// Const pointer member access operator.
    /// </summary>
    /// <returns>Const pointer to value.</returns>
    const T* operator->() const noexcept {
        return &std::get<T>(m_var);
    }

    /// <summary>
    /// Dereference operator.
    /// </summary>
    /// <returns>Reference to value.</returns>
    T& operator*() & noexcept {
        return std::get<T>(m_var);
    }

    /// <summary>
    /// Const dereference operator.
    /// </summary>
    /// <returns>Const reference to value.</returns>
    const T& operator*() const & noexcept {
        return std::get<T>(m_var);
    }

    /// <summary>
    /// Rvalue dereference operator.
    /// </summary>
    /// <returns>Rvalue reference to value.</returns>
    T&& operator*() && noexcept {
        return std::get<T>(std::move(m_var));
    }

    /// <summary>
    /// Const rvalue dereference operator.
    /// </summary>
    /// <returns>Const rvalue reference to value.</returns>
    const T&& operator*() const && noexcept {
        return std::get<T>(std::move(m_var));
    }

    /// <summary>
    /// Returns stored value or fallback default value.
    /// </summary>
    /// <typeparam name="U">Type convertible to T.</typeparam>
    /// <param name="default_val">Fallback value.</param>
    /// <returns>Stored value or fallback value.</returns>
    template <typename U>
    T value_or(U&& default_val) const & {
        return has_value() ? std::get<T>(m_var) : static_cast<T>(std::forward<U>(default_val));
    }

    /// <summary>
    /// Returns stored rvalue value or fallback default value.
    /// </summary>
    /// <typeparam name="U">Type convertible to T.</typeparam>
    /// <param name="default_val">Fallback value.</param>
    /// <returns>Stored value or fallback value.</returns>
    template <typename U>
    T value_or(U&& default_val) && {
        return has_value() ? std::get<T>(std::move(m_var)) : static_cast<T>(std::forward<U>(default_val));
    }

private:
    std::variant<T, unexpected<E>> m_var;
};

/// <summary>
/// Partial specialization of std::variant expected for void.
/// </summary>
/// <typeparam name="E">Error value type.</typeparam>
template <typename E>
class expected<void, E> {
public:
    using value_type = void;
    using error_type = E;
    using unexpected_type = unexpected<E>;

    /// <summary>
    /// Default constructor initializing success state.
    /// </summary>
    constexpr expected() : m_var(std::monostate{}) {}

    /// <summary>
    /// Constructs expected from copy of error.
    /// </summary>
    /// <param name="unexp">The unexpected wrapper.</param>
    constexpr expected(const unexpected<E>& unexp) : m_var(unexp) {}

    /// <summary>
    /// Constructs expected from moved error.
    /// </summary>
    /// <param name="unexp">The unexpected wrapper.</param>
    constexpr expected(unexpected<E>&& unexp) : m_var(std::move(unexp)) {}

    /// <summary>
    /// In-place error constructor.
    /// </summary>
    /// <typeparam name="Args">Constructor argument types.</typeparam>
    /// <param name="args">Forwarded constructor arguments.</param>
    template <typename... Args>
    explicit constexpr expected(unexpect_t, Args&&... args)
        : m_var(std::in_place_type<unexpected<E>>, std::forward<Args>(args)...) {}

    expected(const expected&) = default;
    expected(expected&&) = default;
    expected& operator=(const expected&) = default;
    expected& operator=(expected&&) = default;

    expected& operator=(const unexpected<E>& unexp) {
        m_var = unexp;
        return *this;
    }

    expected& operator=(unexpected<E>&& unexp) {
        m_var = std::move(unexp);
        return *this;
    }

    /// <summary>
    /// Checks whether expected is in success state.
    /// </summary>
    /// <returns>True if success, false if error.</returns>
    constexpr bool has_value() const noexcept {
        return std::holds_alternative<std::monostate>(m_var);
    }

    /// <summary>
    /// Boolean conversion operator testing success state.
    /// </summary>
    /// <returns>True if success, false if error.</returns>
    constexpr explicit operator bool() const noexcept {
        return has_value();
    }

    /// <summary>
    /// Verifies success state or throws std::logic_error.
    /// </summary>
    /// <exception cref="std::logic_error">Thrown when holding error.</exception>
    void value() const {
        if (!has_value()) {
            throw std::logic_error("Bad expected access: object contains error");
        }
    }

    /// <summary>
    /// Obtains mutable reference to error or throws std::logic_error.
    /// </summary>
    /// <returns>Reference to error.</returns>
    /// <exception cref="std::logic_error">Thrown when holding value.</exception>
    E& error() & {
        if (has_value()) {
            throw std::logic_error("Bad expected access: object contains value");
        }
        return std::get<unexpected<E>>(m_var).error();
    }

    /// <summary>
    /// Obtains const reference to error or throws std::logic_error.
    /// </summary>
    /// <returns>Const reference to error.</returns>
    /// <exception cref="std::logic_error">Thrown when holding value.</exception>
    const E& error() const & {
        if (has_value()) {
            throw std::logic_error("Bad expected access: object contains value");
        }
        return std::get<unexpected<E>>(m_var).error();
    }

    /// <summary>
    /// Obtains rvalue reference to error or throws std::logic_error.
    /// </summary>
    /// <returns>Rvalue reference to error.</returns>
    /// <exception cref="std::logic_error">Thrown when holding value.</exception>
    E&& error() && {
        if (has_value()) {
            throw std::logic_error("Bad expected access: object contains value");
        }
        return std::get<unexpected<E>>(std::move(m_var)).error();
    }

    /// <summary>
    /// Obtains const rvalue reference to error or throws std::logic_error.
    /// </summary>
    /// <returns>Const rvalue reference to error.</returns>
    /// <exception cref="std::logic_error">Thrown when holding value.</exception>
    const E&& error() const && {
        if (has_value()) {
            throw std::logic_error("Bad expected access: object contains value");
        }
        return std::get<unexpected<E>>(std::move(m_var)).error();
    }

private:
    std::variant<std::monostate, unexpected<E>> m_var;
};

} // namespace variant_impl

#if !defined(COMPAT_BENCHMARK_ISOLATE_EXPECTED)
using variant_impl::unexpect_t;
using variant_impl::unexpect;
using variant_impl::unexpected;
using variant_impl::expected;
#endif

} // namespace detail
} // namespace compat

#endif // (!COMPAT_HAS_STD_EXPECTED && COMPAT_HAS_STD_VARIANT) || defined(COMPAT_ENABLE_VARIANT_EXPECTED)
