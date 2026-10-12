#pragma once

#include "Config.hpp"

#include <type_traits>
#include <utility>
#include <functional>

#if COMPAT_HAS_STD_PROPAGATE_CONST && !defined(COMPAT_FORCE_SELF_IMPLEMENTATION)
#  include <experimental/propagate_const>

namespace compat {
    using std::experimental::propagate_const;
    using std::experimental::get_underlying;
} // namespace compat

#else

namespace compat {

    template <typename T>
    class propagate_const;

    template <typename T>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 const T&
    get_underlying(const propagate_const<T>& pt) noexcept;

    template <typename T>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 T&
    get_underlying(propagate_const<T>& pt) noexcept;

    namespace detail {
        namespace propagate_const_detail {

            template <typename T>
            struct get_element_type {
            private:
                template <typename U>
                static auto test(int) -> typename std::remove_reference<decltype(*std::declval<U&>())>::type;

                template <typename>
                static void test(...);

            public:
                using type = decltype(test<T>(0));
            };

            template <typename T>
            struct get_element_type<T*> {
                using type = T;
            };

            template <typename U>
            struct is_propagate_const_helper : std::false_type {};

            template <typename U>
            struct is_propagate_const_helper<propagate_const<U>> : std::true_type {};

            // 條件式隱含轉換至裸指標基底類別 (對齊 ISO Library Fundamentals TS)
            template <typename T, typename Elem, bool = std::is_convertible<const T, const Elem*>::value>
            struct conv_c {};

            template <typename T, typename Elem>
            struct conv_c<T, Elem, true> {
                COMPAT_CONSTEXPR_14 operator const Elem*() const;
            };

            template <typename T, typename Elem, bool = std::is_convertible<T, Elem*>::value>
            struct conv_nc {};

            template <typename T, typename Elem>
            struct conv_nc<T, Elem, true> {
                COMPAT_CONSTEXPR_14 operator Elem*();
            };

            template <typename T, typename Elem>
            struct conv_base : conv_c<T, Elem>, conv_nc<T, Elem> {};

            template <typename T, typename Elem>
            struct conv_base<T*, Elem> {
                COMPAT_CONSTEXPR_14 operator const T*() const noexcept;
                COMPAT_CONSTEXPR_14 operator T*() noexcept;
            };

        } // namespace propagate_const_detail
    } // namespace detail

    /// <summary>
    /// ISO C++ Library Fundamentals TS 常數傳播指標包裝器 (對齊 std::experimental::propagate_const)。
    /// 當此包裝器或宿主物件處於 const 存取路徑時，會強制將 const 屬性傳播至底層所指向之物件 (Deep Const)。
    /// 向下相容至 C++11。
    /// </summary>
    /// <typeparam name="T">指標或類指標型別 (如 U*, std::unique_ptr&lt;U&gt;, std::shared_ptr&lt;U&gt;)。</typeparam>
    template <typename T>
    class propagate_const : public detail::propagate_const_detail::conv_base<
                                T, typename detail::propagate_const_detail::get_element_type<T>::type> {
    public:
        using element_type = typename detail::propagate_const_detail::get_element_type<T>::type;

        static_assert(std::is_object<typename std::remove_pointer<T>::type>::value &&
                      !std::is_array<T>::value &&
                      (std::is_class<T>::value || std::is_pointer<T>::value),
                      "propagate_const requires a class or a pointer to an object type");

    private:
        template <typename U>
        friend class propagate_const;

        template <typename U>
        friend COMPAT_CONSTEXPR_14 const U&
        compat::get_underlying(const propagate_const<U>& pt) noexcept;

        template <typename U>
        friend COMPAT_CONSTEXPR_14 U&
        compat::get_underlying(propagate_const<U>& pt) noexcept;

        template <typename U, typename Elem, bool>
        friend struct detail::propagate_const_detail::conv_c;

        template <typename U, typename Elem, bool>
        friend struct detail::propagate_const_detail::conv_nc;

        template <typename U, typename Elem>
        friend struct detail::propagate_const_detail::conv_base;

        template <typename U>
        static COMPAT_CONSTEXPR_14 element_type* to_raw_pointer(U* u) noexcept { return u; }

        template <typename U>
        static COMPAT_CONSTEXPR_14 const element_type* to_raw_pointer(const U* u) noexcept { return u; }

        template <typename U>
        static COMPAT_CONSTEXPR_14 element_type* to_raw_pointer(U& u) { return u.get(); }

        template <typename U>
        static COMPAT_CONSTEXPR_14 const element_type* to_raw_pointer(const U& u) { return u.get(); }

    public:
        // --------------------------------------------------------------------
        // 建構子 (Constructors)
        // --------------------------------------------------------------------

        /// <summary>
        /// 預設建構子，值初始化 (Value-initialize) 被包裝之指標物件 (指標歸零為 nullptr)。
        /// </summary>
        template <typename Dummy = T,
                  typename = typename std::enable_if<std::is_default_constructible<Dummy>::value>::type>
        COMPAT_CONSTEXPR_14 propagate_const() : m_t{} {}

        /// <summary>
        /// 移動建構子。
        /// </summary>
        constexpr propagate_const(propagate_const&&) = default;

        /// <summary>
        /// 複製建構子顯式刪除 (對齊 ISO TS 規範)。
        /// </summary>
        propagate_const(const propagate_const&) = delete;

        /// <summary>
        /// 相容 propagate_const&lt;U&gt; 移動建構子 (隱含轉換)。
        /// </summary>
        template <typename U,
                  typename = typename std::enable_if<
                      std::is_constructible<T, U&&>::value &&
                      std::is_convertible<U&&, T>::value
                  >::type>
        COMPAT_CONSTEXPR_14 propagate_const(propagate_const<U>&& pu)
            : m_t(std::move(compat::get_underlying(pu))) {}

        /// <summary>
        /// 相容 propagate_const&lt;U&gt; 移動建構子 (顯式轉換)。
        /// </summary>
        template <typename U,
                  typename = typename std::enable_if<
                      std::is_constructible<T, U&&>::value &&
                      !std::is_convertible<U&&, T>::value
                  >::type,
                  int = 0>
        explicit COMPAT_CONSTEXPR_14 propagate_const(propagate_const<U>&& pu)
            : m_t(std::move(compat::get_underlying(pu))) {}

        /// <summary>
        /// 轉發建構子 (隱含轉換)。
        /// </summary>
        template <typename U,
                  typename = typename std::enable_if<
                      std::is_constructible<T, U&&>::value &&
                      std::is_convertible<U&&, T>::value &&
                      !detail::propagate_const_detail::is_propagate_const_helper<typename std::decay<U>::type>::value
                  >::type>
        COMPAT_CONSTEXPR_14 propagate_const(U&& u)
            : m_t(std::forward<U>(u)) {}

        /// <summary>
        /// 轉發建構子 (顯式轉換)。
        /// </summary>
        template <typename U,
                  typename = typename std::enable_if<
                      std::is_constructible<T, U&&>::value &&
                      !std::is_convertible<U&&, T>::value &&
                      !detail::propagate_const_detail::is_propagate_const_helper<typename std::decay<U>::type>::value
                  >::type,
                  int = 0>
        explicit COMPAT_CONSTEXPR_14 propagate_const(U&& u)
            : m_t(std::forward<U>(u)) {}

        // --------------------------------------------------------------------
        // 賦值運算子 (Assignment Operators)
        // --------------------------------------------------------------------

        /// <summary>
        /// 複製賦值運算子顯式刪除 (對齊 ISO TS 規範)。
        /// </summary>
        propagate_const& operator=(const propagate_const&) = delete;

        /// <summary>
        /// 移動賦值運算子。
        /// </summary>
        COMPAT_CONSTEXPR_14 propagate_const& operator=(propagate_const&&) = default;

        /// <summary>
        /// 相容 propagate_const&lt;U&gt; 移動賦值運算子。
        /// </summary>
        template <typename U,
                  typename = typename std::enable_if<std::is_convertible<U&&, T>::value>::type>
        COMPAT_CONSTEXPR_14 propagate_const& operator=(propagate_const<U>&& pu) {
            m_t = std::move(compat::get_underlying(pu));
            return *this;
        }

        /// <summary>
        /// 轉發賦值運算子。
        /// </summary>
        template <typename U,
                  typename = typename std::enable_if<
                      std::is_convertible<U&&, T>::value &&
                      !detail::propagate_const_detail::is_propagate_const_helper<typename std::decay<U>::type>::value
                  >::type>
        COMPAT_CONSTEXPR_14 propagate_const& operator=(U&& u) {
            m_t = std::forward<U>(u);
            return *this;
        }

        // --------------------------------------------------------------------
        // 觀察器 (Observers)
        // --------------------------------------------------------------------

        /// <summary>
        /// 檢查被包裝之指標是否非空。
        /// </summary>
        COMPAT_NODISCARD explicit COMPAT_CONSTEXPR_14 operator bool() const noexcept(noexcept(static_cast<bool>(std::declval<const T&>()))) {
            return static_cast<bool>(m_t);
        }

        /// <summary>
        /// 邏輯非運算子，檢查被包裝之指標是否為空。
        /// </summary>
        COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator!() const noexcept(noexcept(!static_cast<bool>(std::declval<const T&>()))) {
            return !static_cast<bool>(m_t);
        }

        /// <summary>
        /// 取得底層指向物件之常數指標 (唯讀存取)。
        /// </summary>
        COMPAT_NODISCARD COMPAT_CONSTEXPR_14 const element_type* operator->() const {
            return get();
        }

        /// <summary>
        /// 取得底層指向物件之非常數指標 (可變更存取)。
        /// </summary>
        COMPAT_NODISCARD COMPAT_CONSTEXPR_14 element_type* operator->() {
            return get();
        }

        /// <summary>
        /// 解參考取得底層物件之常數引用 (唯讀存取)。
        /// </summary>
        COMPAT_NODISCARD COMPAT_CONSTEXPR_14 const element_type& operator*() const {
            return *get();
        }

        /// <summary>
        /// 解參考取得底層物件之非常數引用 (可變更存取)。
        /// </summary>
        COMPAT_NODISCARD COMPAT_CONSTEXPR_14 element_type& operator*() {
            return *get();
        }

        /// <summary>
        /// 取得裸指標 (唯讀常數存取)。
        /// </summary>
        COMPAT_NODISCARD COMPAT_CONSTEXPR_14 const element_type* get() const {
            return to_raw_pointer(m_t);
        }

        /// <summary>
        /// 取得裸指標 (非常數存取)。
        /// </summary>
        COMPAT_NODISCARD COMPAT_CONSTEXPR_14 element_type* get() {
            return to_raw_pointer(m_t);
        }

        // --------------------------------------------------------------------
        // 修改器 (Modifiers)
        // --------------------------------------------------------------------

        /// <summary>
        /// 與另一個 propagate_const 交換底層指標。
        /// </summary>
        COMPAT_CONSTEXPR_14 void swap(propagate_const& pt) noexcept(noexcept(std::swap(std::declval<T&>(), std::declval<T&>()))) {
            using std::swap;
            swap(m_t, compat::get_underlying(pt));
        }

    private:
        T m_t;
    };

    // ------------------------------------------------------------------------
    // 條件式隱含轉換至裸指標運算子實作
    // ------------------------------------------------------------------------

    namespace detail {
        namespace propagate_const_detail {

            template <typename T, typename Elem>
            COMPAT_CONSTEXPR_14
            conv_base<T*, Elem>::operator const T*() const noexcept {
                return static_cast<const propagate_const<T*>*>(this)->get();
            }

            template <typename T, typename Elem>
            COMPAT_CONSTEXPR_14
            conv_base<T*, Elem>::operator T*() noexcept {
                return static_cast<propagate_const<T*>*>(this)->get();
            }

            template <typename T, typename Elem>
            COMPAT_CONSTEXPR_14
            conv_c<T, Elem, true>::operator const Elem*() const {
                return static_cast<const propagate_const<T>*>(this)->get();
            }

            template <typename T, typename Elem>
            COMPAT_CONSTEXPR_14
            conv_nc<T, Elem, true>::operator Elem*() {
                return static_cast<propagate_const<T>*>(this)->get();
            }

        } // namespace propagate_const_detail
    } // namespace detail

    // ------------------------------------------------------------------------
    // 非成員函式 (Non-member Functions)
    // ------------------------------------------------------------------------

    /// <summary>
    /// 存取 propagate_const 底層包裝之常數指標物件。
    /// </summary>
    template <typename T>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 const T&
    get_underlying(const propagate_const<T>& pt) noexcept {
        return pt.m_t;
    }

    /// <summary>
    /// 存取 propagate_const 底層包裝之非常數指標物件。
    /// </summary>
    template <typename T>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 T&
    get_underlying(propagate_const<T>& pt) noexcept {
        return pt.m_t;
    }

    /// <summary>
    /// 特化交換函式。
    /// </summary>
    template <typename T>
    COMPAT_CONSTEXPR_14 void swap(propagate_const<T>& p1, propagate_const<T>& p2)
        noexcept(noexcept(p1.swap(p2))) {
        p1.swap(p2);
    }

    // ------------------------------------------------------------------------
    // 關聯性比較運算子 (Relational Operators)
    // ------------------------------------------------------------------------

    // 與 nullptr_t 比較
    template <typename T>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator==(const propagate_const<T>& pt, std::nullptr_t) {
        return compat::get_underlying(pt) == nullptr;
    }

    template <typename T>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator==(std::nullptr_t, const propagate_const<T>& pt) {
        return nullptr == compat::get_underlying(pt);
    }

    template <typename T>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator!=(const propagate_const<T>& pt, std::nullptr_t) {
        return compat::get_underlying(pt) != nullptr;
    }

    template <typename T>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator!=(std::nullptr_t, const propagate_const<T>& pt) {
        return nullptr != compat::get_underlying(pt);
    }

    // 兩 propagate_const 互比
    template <typename T, typename U>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator==(const propagate_const<T>& pt, const propagate_const<U>& pu) {
        return compat::get_underlying(pt) == compat::get_underlying(pu);
    }

    template <typename T, typename U>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator!=(const propagate_const<T>& pt, const propagate_const<U>& pu) {
        return compat::get_underlying(pt) != compat::get_underlying(pu);
    }

    template <typename T, typename U>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator<(const propagate_const<T>& pt, const propagate_const<U>& pu) {
        return compat::get_underlying(pt) < compat::get_underlying(pu);
    }

    template <typename T, typename U>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator>(const propagate_const<T>& pt, const propagate_const<U>& pu) {
        return compat::get_underlying(pt) > compat::get_underlying(pu);
    }

    template <typename T, typename U>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator<=(const propagate_const<T>& pt, const propagate_const<U>& pu) {
        return compat::get_underlying(pt) <= compat::get_underlying(pu);
    }

    template <typename T, typename U>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator>=(const propagate_const<T>& pt, const propagate_const<U>& pu) {
        return compat::get_underlying(pt) >= compat::get_underlying(pu);
    }

    // propagate_const 與底層指標/相容型別比較
    template <typename T, typename U,
              typename = typename std::enable_if<!detail::propagate_const_detail::is_propagate_const_helper<typename std::decay<U>::type>::value>::type>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator==(const propagate_const<T>& pt, const U& u) {
        return compat::get_underlying(pt) == u;
    }

    template <typename T, typename U,
              typename = typename std::enable_if<!detail::propagate_const_detail::is_propagate_const_helper<typename std::decay<U>::type>::value>::type>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator!=(const propagate_const<T>& pt, const U& u) {
        return compat::get_underlying(pt) != u;
    }

    template <typename T, typename U,
              typename = typename std::enable_if<!detail::propagate_const_detail::is_propagate_const_helper<typename std::decay<U>::type>::value>::type>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator<(const propagate_const<T>& pt, const U& u) {
        return compat::get_underlying(pt) < u;
    }

    template <typename T, typename U,
              typename = typename std::enable_if<!detail::propagate_const_detail::is_propagate_const_helper<typename std::decay<U>::type>::value>::type>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator>(const propagate_const<T>& pt, const U& u) {
        return compat::get_underlying(pt) > u;
    }

    template <typename T, typename U,
              typename = typename std::enable_if<!detail::propagate_const_detail::is_propagate_const_helper<typename std::decay<U>::type>::value>::type>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator<=(const propagate_const<T>& pt, const U& u) {
        return compat::get_underlying(pt) <= u;
    }

    template <typename T, typename U,
              typename = typename std::enable_if<!detail::propagate_const_detail::is_propagate_const_helper<typename std::decay<U>::type>::value>::type>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator>=(const propagate_const<T>& pt, const U& u) {
        return compat::get_underlying(pt) >= u;
    }

    template <typename T, typename U,
              typename = typename std::enable_if<!detail::propagate_const_detail::is_propagate_const_helper<typename std::decay<T>::type>::value>::type>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator==(const T& t, const propagate_const<U>& pu) {
        return t == compat::get_underlying(pu);
    }

    template <typename T, typename U,
              typename = typename std::enable_if<!detail::propagate_const_detail::is_propagate_const_helper<typename std::decay<T>::type>::value>::type>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator!=(const T& t, const propagate_const<U>& pu) {
        return t != compat::get_underlying(pu);
    }

    template <typename T, typename U,
              typename = typename std::enable_if<!detail::propagate_const_detail::is_propagate_const_helper<typename std::decay<T>::type>::value>::type>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator<(const T& t, const propagate_const<U>& pu) {
        return t < compat::get_underlying(pu);
    }

    template <typename T, typename U,
              typename = typename std::enable_if<!detail::propagate_const_detail::is_propagate_const_helper<typename std::decay<T>::type>::value>::type>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator>(const T& t, const propagate_const<U>& pu) {
        return t > compat::get_underlying(pu);
    }

    template <typename T, typename U,
              typename = typename std::enable_if<!detail::propagate_const_detail::is_propagate_const_helper<typename std::decay<T>::type>::value>::type>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator<=(const T& t, const propagate_const<U>& pu) {
        return t <= compat::get_underlying(pu);
    }

    template <typename T, typename U,
              typename = typename std::enable_if<!detail::propagate_const_detail::is_propagate_const_helper<typename std::decay<T>::type>::value>::type>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool operator>=(const T& t, const propagate_const<U>& pu) {
        return t >= compat::get_underlying(pu);
    }

#if defined(__cpp_impl_three_way_comparison) && (__cpp_impl_three_way_comparison >= 201907L)
#  if defined(__has_include)
#    if __has_include(<compare>)
#      include <compare>
    template <typename T, typename U>
    COMPAT_NODISCARD constexpr auto operator<=>(const propagate_const<T>& pt, const propagate_const<U>& pu)
        -> decltype(compat::get_underlying(pt) <=> compat::get_underlying(pu)) {
        return compat::get_underlying(pt) <=> compat::get_underlying(pu);
    }

    template <typename T, typename U,
              typename = typename std::enable_if<!detail::propagate_const_detail::is_propagate_const_helper<typename std::decay<U>::type>::value>::type>
    COMPAT_NODISCARD constexpr auto operator<=>(const propagate_const<T>& pt, const U& u)
        -> decltype(compat::get_underlying(pt) <=> u) {
        return compat::get_underlying(pt) <=> u;
    }
#    endif
#  endif
#endif

} // namespace compat

// ----------------------------------------------------------------------------
// std 特化 (std::hash 與比較函式物件)
// ----------------------------------------------------------------------------

namespace std {

    template <typename T>
    struct hash<compat::propagate_const<T>> {
        using result_type = std::size_t;
        using argument_type = compat::propagate_const<T>;

        std::size_t operator()(const compat::propagate_const<T>& p) const
            noexcept(noexcept(std::hash<T>{}(compat::get_underlying(p)))) {
            return std::hash<T>{}(compat::get_underlying(p));
        }
    };

    template <typename T>
    struct equal_to<compat::propagate_const<T>> {
        typedef compat::propagate_const<T> first_argument_type;
        typedef compat::propagate_const<T> second_argument_type;
        typedef bool result_type;

        COMPAT_CONSTEXPR_14 bool operator()(const compat::propagate_const<T>& x,
                                            const compat::propagate_const<T>& y) const {
            return std::equal_to<T>{}(compat::get_underlying(x), compat::get_underlying(y));
        }
    };

    template <typename T>
    struct not_equal_to<compat::propagate_const<T>> {
        typedef compat::propagate_const<T> first_argument_type;
        typedef compat::propagate_const<T> second_argument_type;
        typedef bool result_type;

        COMPAT_CONSTEXPR_14 bool operator()(const compat::propagate_const<T>& x,
                                            const compat::propagate_const<T>& y) const {
            return std::not_equal_to<T>{}(compat::get_underlying(x), compat::get_underlying(y));
        }
    };

    template <typename T>
    struct less<compat::propagate_const<T>> {
        typedef compat::propagate_const<T> first_argument_type;
        typedef compat::propagate_const<T> second_argument_type;
        typedef bool result_type;

        COMPAT_CONSTEXPR_14 bool operator()(const compat::propagate_const<T>& x,
                                            const compat::propagate_const<T>& y) const {
            return std::less<T>{}(compat::get_underlying(x), compat::get_underlying(y));
        }
    };

    template <typename T>
    struct greater<compat::propagate_const<T>> {
        typedef compat::propagate_const<T> first_argument_type;
        typedef compat::propagate_const<T> second_argument_type;
        typedef bool result_type;

        COMPAT_CONSTEXPR_14 bool operator()(const compat::propagate_const<T>& x,
                                            const compat::propagate_const<T>& y) const {
            return std::greater<T>{}(compat::get_underlying(x), compat::get_underlying(y));
        }
    };

    template <typename T>
    struct less_equal<compat::propagate_const<T>> {
        typedef compat::propagate_const<T> first_argument_type;
        typedef compat::propagate_const<T> second_argument_type;
        typedef bool result_type;

        COMPAT_CONSTEXPR_14 bool operator()(const compat::propagate_const<T>& x,
                                            const compat::propagate_const<T>& y) const {
            return std::less_equal<T>{}(compat::get_underlying(x), compat::get_underlying(y));
        }
    };

    template <typename T>
    struct greater_equal<compat::propagate_const<T>> {
        typedef compat::propagate_const<T> first_argument_type;
        typedef compat::propagate_const<T> second_argument_type;
        typedef bool result_type;

        COMPAT_CONSTEXPR_14 bool operator()(const compat::propagate_const<T>& x,
                                            const compat::propagate_const<T>& y) const {
            return std::greater_equal<T>{}(compat::get_underlying(x), compat::get_underlying(y));
        }
    };

} // namespace std

#endif
