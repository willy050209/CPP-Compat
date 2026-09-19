#pragma once

#include "Config.hpp"

#if (COMPAT_CPLUSPLUS >= COMPAT_CXX_17) && defined(__has_include)
#  if __has_include(<optional>)
#    include <optional>
#    define COMPAT_HAS_STD_OPTIONAL 1
#  else
#    define COMPAT_HAS_STD_OPTIONAL 0
#  endif
#else
#  define COMPAT_HAS_STD_OPTIONAL 0
#endif

#include <utility>
#include <type_traits>
#include <exception>
#include <new>

#if COMPAT_HAS_STD_OPTIONAL && !defined(COMPAT_FORCE_SELF_IMPLEMENTATION)
namespace compat {
    using std::optional;
    using std::nullopt;
    using std::nullopt_t;
    using std::make_optional;
    using std::bad_optional_access;
}
#else
namespace compat {

    struct nullopt_t {
        explicit constexpr nullopt_t(int) {}
    };
    constexpr nullopt_t nullopt{0};

    class bad_optional_access : public std::exception {
    public:
        const char* what() const noexcept override { return "bad optional access"; }
    };

    template <typename T>
    class optional {
        bool has_val_;
        typename std::aligned_storage<sizeof(T), alignof(T)>::type storage_;

        T* ptr() noexcept { return reinterpret_cast<T*>(&storage_); }
        const T* ptr() const noexcept { return reinterpret_cast<const T*>(&storage_); }

        void destroy() noexcept {
            if (has_val_) {
                ptr()->~T();
                has_val_ = false;
            }
        }
    public:
        constexpr optional() noexcept : has_val_(false), storage_{} {}
        constexpr optional(nullopt_t) noexcept : has_val_(false), storage_{} {}

        optional(const T& val) : has_val_(true) {
            ::new (static_cast<void*>(&storage_)) T(val);
        }

        optional(T&& val) : has_val_(true) {
            ::new (static_cast<void*>(&storage_)) T(std::move(val));
        }

        optional(const optional& other) : has_val_(other.has_val_) {
            if (other.has_val_) {
                ::new (static_cast<void*>(&storage_)) T(*other.ptr());
            }
        }

        optional(optional&& other) noexcept(std::is_nothrow_move_constructible<T>::value) : has_val_(other.has_val_) {
            if (other.has_val_) {
                ::new (static_cast<void*>(&storage_)) T(std::move(*other.ptr()));
            }
        }

        ~optional() { destroy(); }

        optional& operator=(nullopt_t) noexcept {
            destroy();
            return *this;
        }

        optional& operator=(const optional& other) {
            if (this != &other) {
                if (has_val_ && other.has_val_) {
                    *ptr() = *other.ptr();
                } else if (has_val_) {
                    destroy();
                } else if (other.has_val_) {
                    ::new (static_cast<void*>(&storage_)) T(*other.ptr());
                    has_val_ = true;
                }
            }
            return *this;
        }

        optional& operator=(optional&& other) noexcept(std::is_nothrow_move_assignable<T>::value && std::is_nothrow_move_constructible<T>::value) {
            if (this != &other) {
                if (has_val_ && other.has_val_) {
                    *ptr() = std::move(*other.ptr());
                } else if (has_val_) {
                    destroy();
                } else if (other.has_val_) {
                    ::new (static_cast<void*>(&storage_)) T(std::move(*other.ptr()));
                    has_val_ = true;
                }
            }
            return *this;
        }

        template <typename U = T, typename = typename std::enable_if<!std::is_same<typename std::decay<U>::type, optional>::value>::type>
        optional& operator=(U&& val) {
            if (has_val_) {
                *ptr() = std::forward<U>(val);
            } else {
                ::new (static_cast<void*>(&storage_)) T(std::forward<U>(val));
                has_val_ = true;
            }
            return *this;
        }

        constexpr explicit operator bool() const noexcept { return has_val_; }
        constexpr bool has_value() const noexcept { return has_val_; }

        T& operator*() & noexcept { return *ptr(); }
        const T& operator*() const & noexcept { return *ptr(); }
        T&& operator*() && noexcept { return std::move(*ptr()); }
        const T&& operator*() const && noexcept { return std::move(*ptr()); }

        T* operator->() noexcept { return ptr(); }
        const T* operator->() const noexcept { return ptr(); }

        T& value() & {
            if (!has_val_) throw bad_optional_access();
            return *ptr();
        }

        const T& value() const & {
            if (!has_val_) throw bad_optional_access();
            return *ptr();
        }

        template <typename U>
        T value_or(U&& default_value) const & {
            return has_val_ ? *ptr() : static_cast<T>(std::forward<U>(default_value));
        }

        template <typename U>
        T value_or(U&& default_value) && {
            return has_val_ ? std::move(*ptr()) : static_cast<T>(std::forward<U>(default_value));
        }
    };

    template <typename T>
    inline optional<typename std::decay<T>::type> make_optional(T&& value) {
        return optional<typename std::decay<T>::type>(std::forward<T>(value));
    }

    template <typename T, typename U>
    inline bool operator==(const optional<T>& opt, const U& val) {
        return opt.has_value() && (*opt == val);
    }

    template <typename T, typename U>
    inline bool operator==(const U& val, const optional<T>& opt) {
        return opt.has_value() && (val == *opt);
    }

    template <typename T>
    inline bool operator==(const optional<T>& opt, nullopt_t) noexcept {
        return !opt.has_value();
    }

    template <typename T>
    inline bool operator==(nullopt_t, const optional<T>& opt) noexcept {
        return !opt.has_value();
    }

    template <typename T>
    inline bool operator!=(const optional<T>& opt, nullopt_t) noexcept {
        return opt.has_value();
    }

    template <typename T>
    inline bool operator!=(nullopt_t, const optional<T>& opt) noexcept {
        return opt.has_value();
    }

} // namespace compat
#endif
