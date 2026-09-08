#pragma once

#include "../Config.hpp"

#if !COMPAT_HAS_STD_EXPECTED && !COMPAT_HAS_STD_VARIANT

#include <cstddef>
#include <cstdint>
#include <utility>
#include <type_traits>
#include <new>
#include <stdexcept>

namespace compat {
namespace detail {

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
/// Discriminated union result type representing either an expected value T or an unexpected error E.
/// Implemented using C++11 unrestricted union for backward compatibility with C++11 and C++14.
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
    /// Default constructor initializing value when T is default-constructible.
    /// </summary>
    template <typename U = T, typename = typename std::enable_if<std::is_default_constructible<U>::value>::type>
    expected() : m_has_value(true) {
        ::new (static_cast<void*>(&m_storage.m_val)) T();
    }

    /// <summary>
    /// Constructs expected holding a copy of val.
    /// </summary>
    /// <param name="val">Value to copy.</param>
    expected(const T& val) : m_has_value(true) {
        ::new (static_cast<void*>(&m_storage.m_val)) T(val);
    }

    /// <summary>
    /// Constructs expected holding a moved val.
    /// </summary>
    /// <param name="val">Value to move.</param>
    expected(T&& val) : m_has_value(true) {
        ::new (static_cast<void*>(&m_storage.m_val)) T(std::move(val));
    }

    /// <summary>
    /// Constructs expected holding a copy of an error.
    /// </summary>
    /// <param name="unexp">The unexpected error wrapper.</param>
    expected(const unexpected<E>& unexp) : m_has_value(false) {
        ::new (static_cast<void*>(&m_storage.m_err)) E(unexp.error());
    }

    /// <summary>
    /// Constructs expected holding a moved error.
    /// </summary>
    /// <param name="unexp">The unexpected error wrapper.</param>
    expected(unexpected<E>&& unexp) : m_has_value(false) {
        ::new (static_cast<void*>(&m_storage.m_err)) E(std::move(unexp.error()));
    }

    /// <summary>
    /// Constructs expected holding an in-place constructed error.
    /// </summary>
    /// <typeparam name="Args">Constructor argument types for error.</typeparam>
    /// <param name="args">Forwarded arguments.</param>
    template <typename... Args>
    explicit expected(unexpect_t, Args&&... args) : m_has_value(false) {
        ::new (static_cast<void*>(&m_storage.m_err)) E(std::forward<Args>(args)...);
    }

    /// <summary>
    /// Copy constructor.
    /// </summary>
    /// <param name="other">Instance to copy.</param>
    expected(const expected& other) : m_has_value(other.m_has_value) {
        if (m_has_value) {
            ::new (static_cast<void*>(&m_storage.m_val)) T(other.m_storage.m_val);
        } else {
            ::new (static_cast<void*>(&m_storage.m_err)) E(other.m_storage.m_err);
        }
    }

    /// <summary>
    /// Move constructor.
    /// </summary>
    /// <param name="other">Instance to move.</param>
    expected(expected&& other) noexcept : m_has_value(other.m_has_value) {
        if (m_has_value) {
            ::new (static_cast<void*>(&m_storage.m_val)) T(std::move(other.m_storage.m_val));
        } else {
            ::new (static_cast<void*>(&m_storage.m_err)) E(std::move(other.m_storage.m_err));
        }
    }

    /// <summary>
    /// Destructor properly invoking active union member's destructor.
    /// </summary>
    ~expected() {
        destroy();
    }

    /// <summary>
    /// Copy assignment operator.
    /// </summary>
    /// <param name="other">Instance to copy.</param>
    /// <returns>Reference to self.</returns>
    expected& operator=(const expected& other) {
        if (this != &other) {
            destroy();
            m_has_value = other.m_has_value;
            if (m_has_value) {
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
            if (m_has_value) {
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
    expected& operator=(const T& val) {
        destroy();
        m_has_value = true;
        ::new (static_cast<void*>(&m_storage.m_val)) T(val);
        return *this;
    }

    /// <summary>
    /// Value move assignment operator.
    /// </summary>
    expected& operator=(T&& val) {
        destroy();
        m_has_value = true;
        ::new (static_cast<void*>(&m_storage.m_val)) T(std::move(val));
        return *this;
    }

    /// <summary>
    /// Unexpected assignment operator.
    /// </summary>
    expected& operator=(const unexpected<E>& unexp) {
        destroy();
        m_has_value = false;
        ::new (static_cast<void*>(&m_storage.m_err)) E(unexp.error());
        return *this;
    }

    /// <summary>
    /// Unexpected move assignment operator.
    /// </summary>
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
    /// Obtains mutable reference to the contained value or throws std::logic_error.
    /// </summary>
    /// <returns>Reference to value.</returns>
    /// <exception cref="std::logic_error">Thrown when holding error.</exception>
    T& value() & {
        if (!m_has_value) {
            throw std::logic_error("Bad expected access: object contains error");
        }
        return m_storage.m_val;
    }

    /// <summary>
    /// Obtains const reference to the contained value or throws std::logic_error.
    /// </summary>
    /// <returns>Const reference to value.</returns>
    /// <exception cref="std::logic_error">Thrown when holding error.</exception>
    const T& value() const & {
        if (!m_has_value) {
            throw std::logic_error("Bad expected access: object contains error");
        }
        return m_storage.m_val;
    }

    /// <summary>
    /// Obtains rvalue reference to the contained value or throws std::logic_error.
    /// </summary>
    /// <returns>Rvalue reference to value.</returns>
    /// <exception cref="std::logic_error">Thrown when holding error.</exception>
    T&& value() && {
        if (!m_has_value) {
            throw std::logic_error("Bad expected access: object contains error");
        }
        return std::move(m_storage.m_val);
    }

    /// <summary>
    /// Obtains const rvalue reference to the contained value or throws std::logic_error.
    /// </summary>
    /// <returns>Const rvalue reference to value.</returns>
    /// <exception cref="std::logic_error">Thrown when holding error.</exception>
    const T&& value() const && {
        if (!m_has_value) {
            throw std::logic_error("Bad expected access: object contains error");
        }
        return std::move(m_storage.m_val);
    }

    /// <summary>
    /// Obtains mutable reference to the contained error or throws std::logic_error.
    /// </summary>
    /// <returns>Reference to error.</returns>
    /// <exception cref="std::logic_error">Thrown when holding value.</exception>
    E& error() & {
        if (m_has_value) {
            throw std::logic_error("Bad expected access: object contains value");
        }
        return m_storage.m_err;
    }

    /// <summary>
    /// Obtains const reference to the contained error or throws std::logic_error.
    /// </summary>
    /// <returns>Const reference to error.</returns>
    /// <exception cref="std::logic_error">Thrown when holding value.</exception>
    const E& error() const & {
        if (m_has_value) {
            throw std::logic_error("Bad expected access: object contains value");
        }
        return m_storage.m_err;
    }

    /// <summary>
    /// Obtains rvalue reference to the contained error or throws std::logic_error.
    /// </summary>
    /// <returns>Rvalue reference to error.</returns>
    /// <exception cref="std::logic_error">Thrown when holding value.</exception>
    E&& error() && {
        if (m_has_value) {
            throw std::logic_error("Bad expected access: object contains value");
        }
        return std::move(m_storage.m_err);
    }

    /// <summary>
    /// Obtains const rvalue reference to the contained error or throws std::logic_error.
    /// </summary>
    /// <returns>Const rvalue reference to error.</returns>
    /// <exception cref="std::logic_error">Thrown when holding value.</exception>
    const E&& error() const && {
        if (m_has_value) {
            throw std::logic_error("Bad expected access: object contains value");
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
        return m_has_value ? m_storage.m_val : static_cast<T>(std::forward<U>(default_val));
    }

    /// <summary>
    /// Returns stored rvalue value or fallback default value.
    /// </summary>
    /// <typeparam name="U">Type convertible to T.</typeparam>
    /// <param name="default_val">Fallback value.</param>
    /// <returns>Stored value or fallback value.</returns>
    template <typename U>
    T value_or(U&& default_val) && {
        return m_has_value ? std::move(m_storage.m_val) : static_cast<T>(std::forward<U>(default_val));
    }

private:
    union Storage {
        T m_val;
        E m_err;
        Storage() {}
        ~Storage() {}
    } m_storage;
    bool m_has_value;

    void destroy() noexcept {
        if (m_has_value) {
            m_storage.m_val.~T();
        } else {
            m_storage.m_err.~E();
        }
    }
};

/// <summary>
/// Partial specialization of expected for void value type.
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
    expected() : m_has_value(true) {}

    /// <summary>
    /// Constructs expected from copy of error.
    /// </summary>
    /// <param name="unexp">The unexpected wrapper.</param>
    expected(const unexpected<E>& unexp) : m_has_value(false) {
        ::new (static_cast<void*>(&m_storage.m_err)) E(unexp.error());
    }

    /// <summary>
    /// Constructs expected from moved error.
    /// </summary>
    /// <param name="unexp">The unexpected wrapper.</param>
    expected(unexpected<E>&& unexp) : m_has_value(false) {
        ::new (static_cast<void*>(&m_storage.m_err)) E(std::move(unexp.error()));
    }

    /// <summary>
    /// In-place error constructor.
    /// </summary>
    /// <typeparam name="Args">Constructor argument types.</typeparam>
    /// <param name="args">Forwarded arguments.</param>
    template <typename... Args>
    explicit expected(unexpect_t, Args&&... args) : m_has_value(false) {
        ::new (static_cast<void*>(&m_storage.m_err)) E(std::forward<Args>(args)...);
    }

    /// <summary>
    /// Copy constructor.
    /// </summary>
    /// <param name="other">Instance to copy.</param>
    expected(const expected& other) : m_has_value(other.m_has_value) {
        if (!m_has_value) {
            ::new (static_cast<void*>(&m_storage.m_err)) E(other.m_storage.m_err);
        }
    }

    /// <summary>
    /// Move constructor.
    /// </summary>
    /// <param name="other">Instance to move.</param>
    expected(expected&& other) noexcept : m_has_value(other.m_has_value) {
        if (!m_has_value) {
            ::new (static_cast<void*>(&m_storage.m_err)) E(std::move(other.m_storage.m_err));
        }
    }

    /// <summary>
    /// Destructor.
    /// </summary>
    ~expected() {
        destroy();
    }

    /// <summary>
    /// Copy assignment operator.
    /// </summary>
    /// <param name="other">Instance to copy.</param>
    /// <returns>Reference to self.</returns>
    expected& operator=(const expected& other) {
        if (this != &other) {
            destroy();
            m_has_value = other.m_has_value;
            if (!m_has_value) {
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
            if (!m_has_value) {
                ::new (static_cast<void*>(&m_storage.m_err)) E(std::move(other.m_storage.m_err));
            }
        }
        return *this;
    }

    /// <summary>
    /// Unexpected assignment operator.
    /// </summary>
    expected& operator=(const unexpected<E>& unexp) {
        destroy();
        m_has_value = false;
        ::new (static_cast<void*>(&m_storage.m_err)) E(unexp.error());
        return *this;
    }

    /// <summary>
    /// Unexpected move assignment operator.
    /// </summary>
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
    /// Verifies success state or throws std::logic_error.
    /// </summary>
    /// <exception cref="std::logic_error">Thrown when holding error.</exception>
    void value() const {
        if (!m_has_value) {
            throw std::logic_error("Bad expected access: object contains error");
        }
    }

    /// <summary>
    /// Obtains mutable reference to error or throws std::logic_error.
    /// </summary>
    /// <returns>Reference to error.</returns>
    /// <exception cref="std::logic_error">Thrown when in success state.</exception>
    E& error() & {
        if (m_has_value) {
            throw std::logic_error("Bad expected access: object contains value");
        }
        return m_storage.m_err;
    }

    /// <summary>
    /// Obtains const reference to error or throws std::logic_error.
    /// </summary>
    /// <returns>Const reference to error.</returns>
    /// <exception cref="std::logic_error">Thrown when in success state.</exception>
    const E& error() const & {
        if (m_has_value) {
            throw std::logic_error("Bad expected access: object contains value");
        }
        return m_storage.m_err;
    }

    /// <summary>
    /// Obtains rvalue reference to error or throws std::logic_error.
    /// </summary>
    /// <returns>Rvalue reference to error.</returns>
    /// <exception cref="std::logic_error">Thrown when in success state.</exception>
    E&& error() && {
        if (m_has_value) {
            throw std::logic_error("Bad expected access: object contains value");
        }
        return std::move(m_storage.m_err);
    }

    /// <summary>
    /// Obtains const rvalue reference to error or throws std::logic_error.
    /// </summary>
    /// <returns>Const rvalue reference to error.</returns>
    /// <exception cref="std::logic_error">Thrown when in success state.</exception>
    const E&& error() const && {
        if (m_has_value) {
            throw std::logic_error("Bad expected access: object contains value");
        }
        return std::move(m_storage.m_err);
    }

private:
    union Storage {
        char m_dummy;
        E m_err;
        Storage() : m_dummy(0) {}
        ~Storage() {}
    } m_storage;
    bool m_has_value;

    void destroy() noexcept {
        if (!m_has_value) {
            m_storage.m_err.~E();
        }
    }
};

} // namespace detail
} // namespace compat

#endif // !COMPAT_HAS_STD_EXPECTED && !COMPAT_HAS_STD_VARIANT
