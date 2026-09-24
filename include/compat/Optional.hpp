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

    namespace detail {

        template <typename T, bool IsTriviallyDestructible = std::is_trivially_destructible<T>::value>
        struct optional_storage_base {
            union Storage {
                char dummy_;
                T value_;
                Storage() noexcept : dummy_{} {}
                ~Storage() noexcept {}
            } storage_;
            bool has_val_;

            optional_storage_base() noexcept : storage_{}, has_val_(false) {}
            explicit optional_storage_base(bool engaged) noexcept : storage_{}, has_val_(engaged) {}

            ~optional_storage_base() {
                destroy();
            }

            void destroy() noexcept {
                if (has_val_) {
                    storage_.value_.~T();
                    has_val_ = false;
                }
            }
        };

        template <typename T>
        struct optional_storage_base<T, true> {
            union Storage {
                char dummy_;
                T value_;
                Storage() noexcept : dummy_{} {}
                ~Storage() = default;
            } storage_;
            bool has_val_;

            optional_storage_base() noexcept : storage_{}, has_val_(false) {}
            explicit optional_storage_base(bool engaged) noexcept : storage_{}, has_val_(engaged) {}
            ~optional_storage_base() = default;

            void destroy() noexcept {
                has_val_ = false;
            }
        };

        template <typename T, bool CanCopy = std::is_copy_constructible<T>::value>
        struct optional_copy_ctor_base : optional_storage_base<T> {
            using optional_storage_base<T>::optional_storage_base;
            optional_copy_ctor_base() = default;
            optional_copy_ctor_base(const optional_copy_ctor_base& other) : optional_storage_base<T>(false) {
                if (other.has_val_) {
                    ::new (static_cast<void*>(&this->storage_.value_)) T(other.storage_.value_);
                    this->has_val_ = true;
                }
            }
            optional_copy_ctor_base(optional_copy_ctor_base&&) = default;
            optional_copy_ctor_base& operator=(const optional_copy_ctor_base&) = default;
            optional_copy_ctor_base& operator=(optional_copy_ctor_base&&) = default;
        };

        template <typename T>
        struct optional_copy_ctor_base<T, false> : optional_storage_base<T> {
            using optional_storage_base<T>::optional_storage_base;
            optional_copy_ctor_base() = default;
            optional_copy_ctor_base(const optional_copy_ctor_base&) = delete;
            optional_copy_ctor_base(optional_copy_ctor_base&&) = default;
            optional_copy_ctor_base& operator=(const optional_copy_ctor_base&) = default;
            optional_copy_ctor_base& operator=(optional_copy_ctor_base&&) = default;
        };

        template <typename T, bool CanMove = std::is_move_constructible<T>::value>
        struct optional_move_ctor_base : optional_copy_ctor_base<T> {
            using optional_copy_ctor_base<T>::optional_copy_ctor_base;
            optional_move_ctor_base() = default;
            optional_move_ctor_base(const optional_move_ctor_base&) = default;
            optional_move_ctor_base(optional_move_ctor_base&& other) noexcept(std::is_nothrow_move_constructible<T>::value) : optional_copy_ctor_base<T>(false) {
                if (other.has_val_) {
                    ::new (static_cast<void*>(&this->storage_.value_)) T(std::move(other.storage_.value_));
                    this->has_val_ = true;
                }
            }
            optional_move_ctor_base& operator=(const optional_move_ctor_base&) = default;
            optional_move_ctor_base& operator=(optional_move_ctor_base&&) = default;
        };

        template <typename T>
        struct optional_move_ctor_base<T, false> : optional_copy_ctor_base<T> {
            using optional_copy_ctor_base<T>::optional_copy_ctor_base;
            optional_move_ctor_base() = default;
            optional_move_ctor_base(const optional_move_ctor_base&) = default;
            optional_move_ctor_base(optional_move_ctor_base&&) = delete;
            optional_move_ctor_base& operator=(const optional_move_ctor_base&) = default;
            optional_move_ctor_base& operator=(optional_move_ctor_base&&) = default;
        };

        template <typename T, bool CanCopyAssign = std::is_copy_constructible<T>::value && std::is_copy_assignable<T>::value>
        struct optional_copy_assign_base : optional_move_ctor_base<T> {
            using optional_move_ctor_base<T>::optional_move_ctor_base;
            optional_copy_assign_base() = default;
            optional_copy_assign_base(const optional_copy_assign_base&) = default;
            optional_copy_assign_base(optional_copy_assign_base&&) = default;
            optional_copy_assign_base& operator=(const optional_copy_assign_base& other) {
                if (this != &other) {
                    if (this->has_val_ && other.has_val_) {
                        this->storage_.value_ = other.storage_.value_;
                    } else if (this->has_val_) {
                        this->destroy();
                    } else if (other.has_val_) {
                        ::new (static_cast<void*>(&this->storage_.value_)) T(other.storage_.value_);
                        this->has_val_ = true;
                    }
                }
                return *this;
            }
            optional_copy_assign_base& operator=(optional_copy_assign_base&&) = default;
        };

        template <typename T>
        struct optional_copy_assign_base<T, false> : optional_move_ctor_base<T> {
            using optional_move_ctor_base<T>::optional_move_ctor_base;
            optional_copy_assign_base() = default;
            optional_copy_assign_base(const optional_copy_assign_base&) = default;
            optional_copy_assign_base(optional_copy_assign_base&&) = default;
            optional_copy_assign_base& operator=(const optional_copy_assign_base&) = delete;
            optional_copy_assign_base& operator=(optional_copy_assign_base&&) = default;
        };

        template <typename T, bool CanMoveAssign = std::is_move_constructible<T>::value && std::is_move_assignable<T>::value>
        struct optional_move_assign_base : optional_copy_assign_base<T> {
            using optional_copy_assign_base<T>::optional_copy_assign_base;
            optional_move_assign_base() = default;
            optional_move_assign_base(const optional_move_assign_base&) = default;
            optional_move_assign_base(optional_move_assign_base&&) = default;
            optional_move_assign_base& operator=(const optional_move_assign_base&) = default;
            optional_move_assign_base& operator=(optional_move_assign_base&& other) noexcept(std::is_nothrow_move_assignable<T>::value && std::is_nothrow_move_constructible<T>::value) {
                if (this != &other) {
                    if (this->has_val_ && other.has_val_) {
                        this->storage_.value_ = std::move(other.storage_.value_);
                    } else if (this->has_val_) {
                        this->destroy();
                    } else if (other.has_val_) {
                        ::new (static_cast<void*>(&this->storage_.value_)) T(std::move(other.storage_.value_));
                        this->has_val_ = true;
                    }
                }
                return *this;
            }
        };

        template <typename T>
        struct optional_move_assign_base<T, false> : optional_copy_assign_base<T> {
            using optional_copy_assign_base<T>::optional_copy_assign_base;
            optional_move_assign_base() = default;
            optional_move_assign_base(const optional_move_assign_base&) = default;
            optional_move_assign_base(optional_move_assign_base&&) = default;
            optional_move_assign_base& operator=(const optional_move_assign_base&) = default;
            optional_move_assign_base& operator=(optional_move_assign_base&&) = delete;
        };

    } // namespace detail

    template <typename T>
    class optional : public detail::optional_move_assign_base<T> {
        using Base = detail::optional_move_assign_base<T>;

        T* ptr() noexcept { return reinterpret_cast<T*>(&this->storage_.value_); }
        const T* ptr() const noexcept { return reinterpret_cast<const T*>(&this->storage_.value_); }

    public:
        using value_type = T;

        constexpr optional() noexcept : Base(false) {}
        constexpr optional(nullopt_t) noexcept : Base(false) {}

        optional(const T& val) : Base(false) {
            ::new (static_cast<void*>(&this->storage_.value_)) T(val);
            this->has_val_ = true;
        }

        optional(T&& val) : Base(false) {
            ::new (static_cast<void*>(&this->storage_.value_)) T(std::move(val));
            this->has_val_ = true;
        }

        template <typename... Args>
        explicit optional(std::piecewise_construct_t, Args&&... args) : Base(false) {
            ::new (static_cast<void*>(&this->storage_.value_)) T(std::forward<Args>(args)...);
            this->has_val_ = true;
        }

        optional(const optional&) = default;
        optional(optional&&) = default;
        optional& operator=(const optional&) = default;
        optional& operator=(optional&&) = default;

        ~optional() = default;

        optional& operator=(nullopt_t) noexcept {
            this->destroy();
            return *this;
        }

        template <typename U = T, typename = typename std::enable_if<!std::is_same<typename std::decay<U>::type, optional>::value && std::is_constructible<T, U>::value && std::is_assignable<T&, U>::value>::type>
        optional& operator=(U&& val) {
            if (this->has_val_) {
                *ptr() = std::forward<U>(val);
            } else {
                ::new (static_cast<void*>(&this->storage_.value_)) T(std::forward<U>(val));
                this->has_val_ = true;
            }
            return *this;
        }

        void reset() noexcept {
            this->destroy();
        }

        template <typename... Args>
        T& emplace(Args&&... args) {
            this->destroy();
            ::new (static_cast<void*>(&this->storage_.value_)) T(std::forward<Args>(args)...);
            this->has_val_ = true;
            return *ptr();
        }

        constexpr explicit operator bool() const noexcept { return this->has_val_; }
        constexpr bool has_value() const noexcept { return this->has_val_; }

        T& operator*() & noexcept { return *ptr(); }
        const T& operator*() const & noexcept { return *ptr(); }
        T&& operator*() && noexcept { return std::move(*ptr()); }
        const T&& operator*() const && noexcept { return std::move(*ptr()); }

        T* operator->() noexcept { return ptr(); }
        const T* operator->() const noexcept { return ptr(); }

        T& value() & {
            if (!this->has_val_) {
                COMPAT_THROW_OR_ABORT(bad_optional_access());
            }
            return *ptr();
        }

        const T& value() const & {
            if (!this->has_val_) {
                COMPAT_THROW_OR_ABORT(bad_optional_access());
            }
            return *ptr();
        }

        template <typename U>
        T value_or(U&& default_value) const & {
            return this->has_val_ ? *ptr() : static_cast<T>(std::forward<U>(default_value));
        }

        template <typename U>
        T value_or(U&& default_value) && {
            return this->has_val_ ? std::move(*ptr()) : static_cast<T>(std::forward<U>(default_value));
        }
    };

    template <typename T>
    inline optional<typename std::decay<T>::type> make_optional(T&& value) {
        return optional<typename std::decay<T>::type>(std::forward<T>(value));
    }

    template <typename T, typename... Args>
    inline optional<T> make_optional(Args&&... args) {
        return optional<T>(std::piecewise_construct, std::forward<Args>(args)...);
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

    template <typename T, typename U>
    inline bool operator==(const optional<T>& lhs, const optional<U>& rhs) {
        if (lhs.has_value() != rhs.has_value()) return false;
        if (!lhs.has_value()) return true;
        return *lhs == *rhs;
    }

    template <typename T, typename U>
    inline bool operator!=(const optional<T>& lhs, const optional<U>& rhs) {
        return !(lhs == rhs);
    }

} // namespace compat
#endif
