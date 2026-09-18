#pragma once

#include "../Config.hpp"

#include <cstddef>
#include <cstdint>
#include <utility>
#include <type_traits>
#include <iterator>
#include <tuple>
#include <array>
#include <new>
#include <cstdlib>
#include <cassert>
#include <memory>

namespace compat {
namespace detail {
namespace self_ranges {

    /// <summary>
    /// 編譯期型別連接判斷 (對齊 C++17 std::conjunction)。
    /// </summary>
    template <typename... B>
    struct conjunction : std::true_type {};
    template <typename B1>
    struct conjunction<B1> : B1 {};
    template <typename B1, typename... Bn>
    struct conjunction<B1, Bn...> : std::conditional<bool(B1::value), conjunction<Bn...>, B1>::type {};

    /// <summary>
    /// 編譯期型別析取判斷 (對齊 C++17 std::disjunction)。
    /// </summary>
    template <typename... B>
    struct disjunction : std::false_type {};
    template <typename B1>
    struct disjunction<B1> : B1 {};
    template <typename B1, typename... Bn>
    struct disjunction<B1, Bn...> : std::conditional<bool(B1::value), B1, disjunction<Bn...>>::type {};

    /// <summary>
    /// 編譯期型別否定判斷 (對齊 C++17 std::negation)。
    /// </summary>
    template <typename B>
    struct negation : std::integral_constant<bool, !bool(B::value)> {};

    /// <summary>
    /// 移除 cv 與引用修飾 (對齊 C++20 std::remove_cvref)。
    /// </summary>
    template <typename T>
    struct remove_cvref {
        using type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
    };

    template <typename T>
    using remove_cvref_t = typename remove_cvref<T>::type;

    /// <summary>
    /// CPO 靜態常數單例儲存樣板。
    /// </summary>
    template <typename T>
    struct static_const {
        static constexpr T value{};
    };

    template <typename T>
    constexpr T static_const<T>::value;

    // 通用引用推導輔助
    template <typename T, typename U>
    struct simple_common_ref {
    private:
        template <typename X, typename Y>
        static auto test(int) -> decltype(true ? std::declval<X>() : std::declval<Y>());
        template <typename X, typename Y>
        static auto test(...) -> typename std::common_type<typename std::decay<X>::type, typename std::decay<Y>::type>::type;
    public:
        using type = decltype(test<T, U>(0));
    };

    /// <summary>
    /// 多元型別共用引用型別萃取 (對齊 C++20 std::common_reference)。
    /// </summary>
    template <typename... Ts>
    struct common_reference;

    template <typename T>
    struct common_reference<T> {
        using type = T;
    };

    template <typename T, typename U>
    struct common_reference<T, U> : simple_common_ref<T, U> {};

    template <typename T1, typename T2, typename... Rest>
    struct common_reference<T1, T2, Rest...>
        : common_reference<typename common_reference<T1, T2>::type, Rest...> {};

    template <typename... Ts>
    using common_reference_t = typename common_reference<Ts...>::type;

    template <typename... Ts>
    using common_type_t = typename std::common_type<Ts...>::type;

    namespace ranges {

        /// <summary>
        /// 預設哨兵型別 (對齊 C++20 std::ranges::default_sentinel_t)。
        /// </summary>
        struct default_sentinel_t {};

#if (COMPAT_CPLUSPLUS >= COMPAT_CXX_17)
        inline constexpr default_sentinel_t default_sentinel{};
#else
        constexpr default_sentinel_t default_sentinel{};
#endif

        /// <summary>
        /// 懸空迭代器佔位型別 (對齊 C++20 std::ranges::dangling)。
        /// </summary>
        struct dangling {
            constexpr dangling() noexcept = default;
            template <typename... Args>
            constexpr dangling(Args&&...) noexcept {}
        };

        /// <summary>
        /// 子區間尺寸分類列舉 (對齊 C++20 std::ranges::subrange_kind)。
        /// </summary>
        enum class subrange_kind : bool { unsized, sized };

        /// <summary>
        /// View 基礎標記結構體 (對齊 C++20 std::ranges::view_base)。
        /// </summary>
        struct view_base {};

        /// <summary>
        /// 借用區間特性萃取輔助。
        /// </summary>
        template <typename R>
        struct enable_borrowed_range_helper : std::false_type {};

        template <typename R>
        struct enable_borrowed_range_helper<R&> : std::true_type {};

#if (COMPAT_CPLUSPLUS >= COMPAT_CXX_14)
        template <typename R>
        COMPAT_CONSTEXPR_14 bool enable_borrowed_range = enable_borrowed_range_helper<R>::value;
#endif

        /// <summary>
        /// View 啟用特性萃取輔助。
        /// </summary>
        template <typename R>
        struct enable_view_helper : std::is_base_of<view_base, R> {};

#if (COMPAT_CPLUSPLUS >= COMPAT_CXX_14)
        template <typename R>
        COMPAT_CONSTEXPR_14 bool enable_view = enable_view_helper<R>::value;
#endif

        // --- Customization Point Objects (CPOs) Implementation Details ---
        namespace detail {

            // ADL 隔離毒丸
            void begin();
            void end();
            void rbegin();
            void rend();
            void size();
            void empty();
            void data();

            template <typename T>
            struct has_member_begin {
            private:
                template <typename U>
                static auto test(int) -> decltype(std::declval<U&>().begin(), std::true_type{});
                template <typename>
                static std::false_type test(...);
            public:
                static constexpr bool value = decltype(test<T>(0))::value;
            };

            template <typename T>
            struct has_adl_begin {
            private:
                template <typename U>
                static auto test(int) -> decltype(begin(std::declval<U&>()), std::true_type{});
                template <typename>
                static std::false_type test(...);
            public:
                static constexpr bool value = decltype(test<T>(0))::value;
            };

            template <typename T>
            struct has_member_end {
            private:
                template <typename U>
                static auto test(int) -> decltype(std::declval<U&>().end(), std::true_type{});
                template <typename>
                static std::false_type test(...);
            public:
                static constexpr bool value = decltype(test<T>(0))::value;
            };

            template <typename T>
            struct has_adl_end {
            private:
                template <typename U>
                static auto test(int) -> decltype(end(std::declval<U&>()), std::true_type{});
                template <typename>
                static std::false_type test(...);
            public:
                static constexpr bool value = decltype(test<T>(0))::value;
            };

            template <typename T>
            struct has_member_size {
            private:
                template <typename U>
                static auto test(int) -> decltype(std::declval<U&>().size(), std::true_type{});
                template <typename>
                static std::false_type test(...);
            public:
                static constexpr bool value = decltype(test<T>(0))::value;
            };

            template <typename T>
            struct has_member_empty {
            private:
                template <typename U>
                static auto test(int) -> decltype(std::declval<U&>().empty(), std::true_type{});
                template <typename>
                static std::false_type test(...);
            public:
                static constexpr bool value = decltype(test<T>(0))::value;
            };

            template <typename T>
            struct has_member_data {
            private:
                template <typename U>
                static auto test(int) -> decltype(std::declval<U&>().data(), std::true_type{});
                template <typename>
                static std::false_type test(...);
            public:
                static constexpr bool value = decltype(test<T>(0))::value;
            };

            struct begin_fn {
                template <typename T, size_t N>
                constexpr T* operator()(T (&arr)[N]) const noexcept {
                    return arr + 0;
                }

                template <typename T, typename = typename std::enable_if<!std::is_array<typename std::remove_reference<T>::type>::value && has_member_begin<T>::value>::type>
                constexpr auto operator()(T&& t) const -> decltype(t.begin()) {
                    return t.begin();
                }

                template <typename T, typename = typename std::enable_if<!std::is_array<typename std::remove_reference<T>::type>::value && !has_member_begin<T>::value && has_adl_begin<T>::value>::type, int = 0>
                constexpr auto operator()(T&& t) const -> decltype(begin(t)) {
                    return begin(t);
                }
            };

            struct end_fn {
                template <typename T, size_t N>
                constexpr T* operator()(T (&arr)[N]) const noexcept {
                    return arr + N;
                }

                template <typename T, typename = typename std::enable_if<!std::is_array<typename std::remove_reference<T>::type>::value && has_member_end<T>::value>::type>
                constexpr auto operator()(T&& t) const -> decltype(t.end()) {
                    return t.end();
                }

                template <typename T, typename = typename std::enable_if<!std::is_array<typename std::remove_reference<T>::type>::value && !has_member_end<T>::value && has_adl_end<T>::value>::type, int = 0>
                constexpr auto operator()(T&& t) const -> decltype(end(t)) {
                    return end(t);
                }
            };

            struct cbegin_fn {
                template <typename T>
                constexpr auto operator()(const T& t) const -> decltype(begin_fn{}(t)) {
                    return begin_fn{}(t);
                }
            };

            struct cend_fn {
                template <typename T>
                constexpr auto operator()(const T& t) const -> decltype(end_fn{}(t)) {
                    return end_fn{}(t);
                }
            };

            struct size_fn {
                template <typename T, size_t N>
                constexpr size_t operator()(T (&)[N]) const noexcept {
                    return N;
                }

                template <typename T, typename = typename std::enable_if<!std::is_array<typename std::remove_reference<T>::type>::value && has_member_size<T>::value>::type>
                constexpr auto operator()(T&& t) const -> decltype(t.size()) {
                    return t.size();
                }

                template <typename T, typename = typename std::enable_if<!std::is_array<typename std::remove_reference<T>::type>::value && !has_member_size<T>::value>::type,
                          typename Diff = decltype(end_fn{}(std::declval<T&>()) - begin_fn{}(std::declval<T&>()))>
                constexpr size_t operator()(T&& t) const {
                    return static_cast<size_t>(end_fn{}(t) - begin_fn{}(t));
                }
            };

            struct ssize_fn {
                template <typename T>
                constexpr auto operator()(T&& t) const -> std::ptrdiff_t {
                    return static_cast<std::ptrdiff_t>(size_fn{}(std::forward<T>(t)));
                }
            };

            struct empty_fn {
                template <typename T, typename = typename std::enable_if<has_member_empty<T>::value>::type>
                constexpr auto operator()(T&& t) const -> decltype(bool(t.empty())) {
                    return bool(t.empty());
                }

                template <typename T, typename = typename std::enable_if<!has_member_empty<T>::value && has_member_size<T>::value>::type, int = 0>
                constexpr bool operator()(T&& t) const {
                    return size_fn{}(t) == 0;
                }

                template <typename T, typename = typename std::enable_if<!has_member_empty<T>::value && !has_member_size<T>::value>::type, int = 0, int = 0>
                constexpr bool operator()(T&& t) const {
                    return begin_fn{}(t) == end_fn{}(t);
                }
            };

            struct data_fn {
                template <typename T, typename = typename std::enable_if<has_member_data<T>::value>::type>
                constexpr auto operator()(T&& t) const -> decltype(t.data()) {
                    return t.data();
                }

                template <typename T, typename = typename std::enable_if<!has_member_data<T>::value>::type, int = 0>
                constexpr auto operator()(T&& t) const -> decltype(std::addressof(*begin_fn{}(t))) {
                    return begin_fn{}(t) == end_fn{}(t) ? nullptr : std::addressof(*begin_fn{}(t));
                }
            };

            struct cdata_fn {
                template <typename T>
                constexpr auto operator()(const T& t) const -> decltype(data_fn{}(t)) {
                    return data_fn{}(t);
                }
            };

            struct rbegin_fn {
                template <typename T>
                constexpr auto operator()(T&& t) const -> std::reverse_iterator<decltype(end_fn{}(t))> {
                    return std::reverse_iterator<decltype(end_fn{}(t))>(end_fn{}(t));
                }
            };

            struct rend_fn {
                template <typename T>
                constexpr auto operator()(T&& t) const -> std::reverse_iterator<decltype(begin_fn{}(t))> {
                    return std::reverse_iterator<decltype(begin_fn{}(t))>(begin_fn{}(t));
                }
            };

            struct crbegin_fn {
                template <typename T>
                constexpr auto operator()(const T& t) const -> decltype(rbegin_fn{}(t)) {
                    return rbegin_fn{}(t);
                }
            };

            struct crend_fn {
                template <typename T>
                constexpr auto operator()(const T& t) const -> decltype(rend_fn{}(t)) {
                    return rend_fn{}(t);
                }
            };

        } // namespace detail

#if (COMPAT_CPLUSPLUS >= COMPAT_CXX_17)
        inline constexpr detail::begin_fn begin{};
        inline constexpr detail::end_fn end{};
        inline constexpr detail::cbegin_fn cbegin{};
        inline constexpr detail::cend_fn cend{};
        inline constexpr detail::rbegin_fn rbegin{};
        inline constexpr detail::rend_fn rend{};
        inline constexpr detail::crbegin_fn crbegin{};
        inline constexpr detail::crend_fn crend{};
        inline constexpr detail::size_fn size{};
        inline constexpr detail::ssize_fn ssize{};
        inline constexpr detail::empty_fn empty{};
        inline constexpr detail::data_fn data{};
        inline constexpr detail::cdata_fn cdata{};
#else
        namespace {
            constexpr const detail::begin_fn& begin = static_const<detail::begin_fn>::value;
            constexpr const detail::end_fn& end = static_const<detail::end_fn>::value;
            constexpr const detail::cbegin_fn& cbegin = static_const<detail::cbegin_fn>::value;
            constexpr const detail::cend_fn& cend = static_const<detail::cend_fn>::value;
            constexpr const detail::rbegin_fn& rbegin = static_const<detail::rbegin_fn>::value;
            constexpr const detail::rend_fn& rend = static_const<detail::rend_fn>::value;
            constexpr const detail::crbegin_fn& crbegin = static_const<detail::crbegin_fn>::value;
            constexpr const detail::crend_fn& crend = static_const<detail::crend_fn>::value;
            constexpr const detail::size_fn& size = static_const<detail::size_fn>::value;
            constexpr const detail::ssize_fn& ssize = static_const<detail::ssize_fn>::value;
            constexpr const detail::empty_fn& empty = static_const<detail::empty_fn>::value;
            constexpr const detail::data_fn& data = static_const<detail::data_fn>::value;
            constexpr const detail::cdata_fn& cdata = static_const<detail::cdata_fn>::value;
        }
#endif

        // --- Range Type Traits & Concepts ---
        template <typename R>
        using iterator_t = decltype(detail::begin_fn{}(std::declval<R&>()));

        template <typename R>
        using sentinel_t = decltype(detail::end_fn{}(std::declval<R&>()));

        template <typename R>
        using range_difference_t = typename std::iterator_traits<iterator_t<R>>::difference_type;

        template <typename R>
        using range_value_t = typename std::iterator_traits<iterator_t<R>>::value_type;

        template <typename R>
        using range_reference_t = decltype(*detail::begin_fn{}(std::declval<R&>()));

        template <typename R>
        using range_rvalue_reference_t = decltype(std::move(*detail::begin_fn{}(std::declval<R&>())));

        template <typename R>
        using range_size_t = decltype(detail::size_fn{}(std::declval<R&>()));

        namespace detail {
            template <typename R>
            struct is_range {
            private:
                template <typename U>
                static auto test(int) -> decltype(begin_fn{}(std::declval<U&>()), end_fn{}(std::declval<U&>()), std::true_type{});
                template <typename>
                static std::false_type test(...);
            public:
                static constexpr bool value = decltype(test<R>(0))::value;
            };

            template <typename R>
            struct is_sized_range {
            private:
                template <typename U>
                static auto test(int) -> decltype(size_fn{}(std::declval<U&>()), std::true_type{});
                template <typename>
                static std::false_type test(...);
            public:
                static constexpr bool value = is_range<R>::value && decltype(test<R>(0))::value;
            };

            template <typename R>
            struct is_common_range {
            private:
                template <typename U>
                static auto test(int) -> typename std::is_same<iterator_t<U>, sentinel_t<U>>::type;
                template <typename>
                static std::false_type test(...);
            public:
                static constexpr bool value = is_range<R>::value && decltype(test<R>(0))::value;
            };

            template <typename R>
            struct is_input_range {
            private:
                template <typename U>
                static auto test(int) -> typename std::is_base_of<
                    std::input_iterator_tag,
                    typename std::iterator_traits<iterator_t<U>>::iterator_category
                >::type;
                template <typename>
                static std::false_type test(...);
            public:
                static constexpr bool value = is_range<R>::value && decltype(test<R>(0))::value;
            };

            template <typename R>
            struct is_forward_range {
            private:
                template <typename U>
                static auto test(int) -> typename std::is_base_of<
                    std::forward_iterator_tag,
                    typename std::iterator_traits<iterator_t<U>>::iterator_category
                >::type;
                template <typename>
                static std::false_type test(...);
            public:
                static constexpr bool value = is_input_range<R>::value && decltype(test<R>(0))::value;
            };

            template <typename R>
            struct is_bidirectional_range {
            private:
                template <typename U>
                static auto test(int) -> typename std::is_base_of<
                    std::bidirectional_iterator_tag,
                    typename std::iterator_traits<iterator_t<U>>::iterator_category
                >::type;
                template <typename>
                static std::false_type test(...);
            public:
                static constexpr bool value = is_forward_range<R>::value && decltype(test<R>(0))::value;
            };

            template <typename R>
            struct is_random_access_range {
            private:
                template <typename U>
                static auto test(int) -> typename std::is_base_of<
                    std::random_access_iterator_tag,
                    typename std::iterator_traits<iterator_t<U>>::iterator_category
                >::type;
                template <typename>
                static std::false_type test(...);
            public:
                static constexpr bool value = is_bidirectional_range<R>::value && decltype(test<R>(0))::value;
            };

            template <typename R>
            struct is_constant_range {
            private:
                template <typename U>
                static auto test(int) -> typename std::integral_constant<bool,
                    (std::is_const<typename std::remove_reference<range_reference_t<U>>::type>::value ||
                     !std::is_lvalue_reference<range_reference_t<U>>::value)
                >::type;
                template <typename>
                static std::false_type test(...);
            public:
                static constexpr bool value = is_input_range<R>::value && decltype(test<R>(0))::value;
            };
        } // namespace detail

        template <typename R>
        struct range : std::integral_constant<bool, detail::is_range<R>::value> {};

        template <typename R>
        struct sized_range : std::integral_constant<bool, detail::is_sized_range<R>::value> {};

        template <typename R>
        struct common_range : std::integral_constant<bool, detail::is_common_range<R>::value> {};

        template <typename R>
        struct input_range : std::integral_constant<bool, detail::is_input_range<R>::value> {};

        template <typename R>
        struct forward_range : std::integral_constant<bool, detail::is_forward_range<R>::value> {};

        template <typename R>
        struct bidirectional_range : std::integral_constant<bool, detail::is_bidirectional_range<R>::value> {};

        template <typename R>
        struct random_access_range : std::integral_constant<bool, detail::is_random_access_range<R>::value> {};

        /// <summary>
        /// ISO C++26 常數區間概念特性萃取 (對齊 P2728R6 std::ranges::constant_range)。
        /// </summary>
        template <typename R>
        struct constant_range : std::integral_constant<bool, detail::is_constant_range<R>::value> {};

        template <typename R>
        struct borrowed_range : std::integral_constant<bool,
            range<R>::value && (std::is_lvalue_reference<R>::value || enable_borrowed_range_helper<remove_cvref_t<R>>::value)
        > {};

        template <typename R>
        struct view : std::integral_constant<bool,
            range<R>::value && enable_view_helper<remove_cvref_t<R>>::value
        > {};

        template <typename R>
        struct viewable_range : std::integral_constant<bool,
            range<R>::value && (borrowed_range<R>::value || view<remove_cvref_t<R>>::value)
        > {};

        // --- Pipeline Machinery & Adaptor Closures ---
        template <typename Derived>
        struct range_adaptor_closure;

        namespace detail {
            template <typename F, typename G>
            struct pipeline_composition : range_adaptor_closure<pipeline_composition<F, G>> {
                F f_;
                G g_;

                pipeline_composition(F f, G g) : f_(std::move(f)), g_(std::move(g)) {}

                template <typename R>
                auto operator()(R&& r) const -> decltype(g_(f_(std::forward<R>(r)))) {
                    return g_(f_(std::forward<R>(r)));
                }
            };
        } // namespace detail

        /// <summary>
        /// 區間適配器標籤結構體。
        /// </summary>
        struct range_adaptor_closure_tag {};

        /// <summary>
        /// 區間適配器閉包基類 (提供 operator| 管道運算能力)。
        /// </summary>
        template <typename Derived>
        struct range_adaptor_closure : public range_adaptor_closure_tag {
            template <typename Range,
                      typename = typename std::enable_if<range<typename std::remove_reference<Range>::type>::value>::type>
            friend auto operator|(Range&& r, const Derived& closure)
                -> decltype(closure(std::forward<Range>(r))) {
                return closure(std::forward<Range>(r));
            }

            template <typename OtherDerived,
                      typename = typename std::enable_if<std::is_base_of<range_adaptor_closure_tag, OtherDerived>::value>::type>
            friend detail::pipeline_composition<Derived, OtherDerived> operator|(const Derived& lhs, const OtherDerived& rhs) {
                return detail::pipeline_composition<Derived, OtherDerived>(lhs, rhs);
            }

            template <typename OtherClosure,
                      typename = typename std::enable_if<!std::is_base_of<range_adaptor_closure_tag, OtherClosure>::value &&
                                                         !range<typename std::remove_reference<OtherClosure>::type>::value>::type>
            friend detail::pipeline_composition<OtherClosure, Derived> operator|(const OtherClosure& lhs, const Derived& rhs) {
                return detail::pipeline_composition<OtherClosure, Derived>(lhs, rhs);
            }
        };

        // --- view_interface ---
        /// <summary>
        /// View CRTP 介面基類 (對齊 C++20 std::ranges::view_interface)。
        /// </summary>
        template <typename Derived>
        class view_interface : public view_base {
        private:
            Derived& derived() noexcept { return static_cast<Derived&>(*this); }
            const Derived& derived() const noexcept { return static_cast<const Derived&>(*this); }

        public:
            template <typename D = Derived>
            auto empty() -> decltype(detail::begin_fn{}(std::declval<D&>()) == detail::end_fn{}(std::declval<D&>())) {
                return detail::begin_fn{}(derived()) == detail::end_fn{}(derived());
            }

            template <typename D = Derived>
            auto empty() const -> decltype(detail::begin_fn{}(std::declval<const D&>()) == detail::end_fn{}(std::declval<const D&>())) {
                return detail::begin_fn{}(derived()) == detail::end_fn{}(derived());
            }

            template <typename D = Derived>
            explicit operator bool() { return !derived().empty(); }

            template <typename D = Derived>
            explicit operator bool() const { return !derived().empty(); }

            template <typename D = Derived>
            auto data() -> decltype(detail::begin_fn{}(std::declval<D&>())) {
                return detail::begin_fn{}(derived());
            }

            template <typename D = Derived>
            auto data() const -> decltype(detail::begin_fn{}(std::declval<const D&>())) {
                return detail::begin_fn{}(derived());
            }

            template <typename D = Derived>
            auto size() -> decltype(static_cast<size_t>(detail::end_fn{}(std::declval<D&>()) - detail::begin_fn{}(std::declval<D&>()))) {
                return static_cast<size_t>(detail::end_fn{}(derived()) - detail::begin_fn{}(derived()));
            }

            template <typename D = Derived>
            auto size() const -> decltype(static_cast<size_t>(detail::end_fn{}(std::declval<const D&>()) - detail::begin_fn{}(std::declval<const D&>()))) {
                return static_cast<size_t>(detail::end_fn{}(derived()) - detail::begin_fn{}(derived()));
            }

            template <typename D = Derived>
            auto front() -> decltype(*detail::begin_fn{}(std::declval<D&>())) {
                return *detail::begin_fn{}(derived());
            }

            template <typename D = Derived>
            auto front() const -> decltype(*detail::begin_fn{}(std::declval<const D&>())) {
                return *detail::begin_fn{}(derived());
            }

            template <typename D = Derived>
            auto back() -> decltype(*std::prev(detail::end_fn{}(std::declval<D&>()))) {
                return *std::prev(detail::end_fn{}(derived()));
            }

            template <typename D = Derived>
            auto back() const -> decltype(*std::prev(detail::end_fn{}(std::declval<const D&>()))) {
                return *std::prev(detail::end_fn{}(derived()));
            }

            template <typename D = Derived>
            auto operator[](std::ptrdiff_t n) -> decltype(detail::begin_fn{}(std::declval<D&>())[n]) {
                return detail::begin_fn{}(derived())[n];
            }

            template <typename D = Derived>
            auto operator[](std::ptrdiff_t n) const -> decltype(detail::begin_fn{}(std::declval<const D&>())[n]) {
                return detail::begin_fn{}(derived())[n];
            }
        };

        // --- subrange ---
        /// <summary>
        /// 迭代器哨兵配對子區間 View (對齊 C++20 std::ranges::subrange)。
        /// </summary>
        template <typename I, typename S = I, subrange_kind K = subrange_kind::unsized>
        class subrange : public view_interface<subrange<I, S, K>> {
        private:
            I begin_{};
            S end_{};

        public:
            subrange() = default;
            subrange(I b, S e) : begin_(std::move(b)), end_(std::move(e)) {}

            I begin() const { return begin_; }
            S end() const { return end_; }

            bool empty() const { return begin_ == end_; }

            template <typename II = I, typename SS = S>
            auto size() const -> decltype(static_cast<size_t>(std::declval<const SS&>() - std::declval<const II&>())) {
                return static_cast<size_t>(end_ - begin_);
            }
        };

        template <typename I, typename S, subrange_kind K>
        struct enable_borrowed_range_helper<subrange<I, S, K>> : std::true_type {};

#if (COMPAT_CPLUSPLUS >= COMPAT_CXX_14)
        template <typename I, typename S, subrange_kind K>
        COMPAT_CONSTEXPR_14 bool enable_borrowed_range<subrange<I, S, K>> = true;
#endif

        // --- owning_view ---
        /// <summary>
        /// 容器持有 View (對齊 C++20 std::ranges::owning_view)。
        /// </summary>
        template <typename R>
        class owning_view : public view_interface<owning_view<R>> {
        private:
            R r_{};

        public:
            owning_view() = default;
            explicit owning_view(R&& r) : r_(std::move(r)) {}
            owning_view(owning_view&&) = default;
            owning_view& operator=(owning_view&&) = default;
            owning_view(const owning_view&) = default;
            owning_view& operator=(const owning_view&) = default;

            R& base() & noexcept { return r_; }
            const R& base() const & noexcept { return r_; }
            R&& base() && noexcept { return std::move(r_); }
            const R&& base() const && noexcept { return std::move(r_); }

            auto begin() -> decltype(ranges::begin(r_)) { return ranges::begin(r_); }
            auto begin() const -> decltype(ranges::begin(r_)) { return ranges::begin(r_); }
            auto end() -> decltype(ranges::end(r_)) { return ranges::end(r_); }
            auto end() const -> decltype(ranges::end(r_)) { return ranges::end(r_); }
            auto size() -> decltype(ranges::size(r_)) { return ranges::size(r_); }
            auto size() const -> decltype(ranges::size(r_)) { return ranges::size(r_); }
            auto empty() -> decltype(ranges::empty(r_)) { return ranges::empty(r_); }
            auto empty() const -> decltype(ranges::empty(r_)) { return ranges::empty(r_); }
            auto data() -> decltype(ranges::data(r_)) { return ranges::data(r_); }
            auto data() const -> decltype(ranges::data(r_)) { return ranges::data(r_); }
        };

        template <typename R>
        struct enable_borrowed_range_helper<owning_view<R>> : enable_borrowed_range_helper<R> {};

#if (COMPAT_CPLUSPLUS >= COMPAT_CXX_14)
        template <typename R>
        COMPAT_CONSTEXPR_14 bool enable_borrowed_range<owning_view<R>> = enable_borrowed_range<R>;
#endif

    } // namespace ranges

    // --- views Namespace Machinery ---
    namespace views {

        namespace detail {
            struct all_fn : ranges::range_adaptor_closure<all_fn> {
                template <typename R, typename Decayed = typename std::decay<R>::type,
                          typename = typename std::enable_if<ranges::view<Decayed>::value>::type>
                Decayed operator()(R&& r) const {
                    return std::forward<R>(r);
                }

                template <typename R, typename Decayed = typename std::decay<R>::type,
                          typename = typename std::enable_if<!ranges::view<Decayed>::value && std::is_lvalue_reference<R>::value>::type>
                ranges::subrange<ranges::iterator_t<R>, ranges::sentinel_t<R>> operator()(R&& r) const {
                    return ranges::subrange<ranges::iterator_t<R>, ranges::sentinel_t<R>>(ranges::begin(r), ranges::end(r));
                }

                template <typename R, typename Decayed = typename std::decay<R>::type,
                          typename = typename std::enable_if<!ranges::view<Decayed>::value && !std::is_lvalue_reference<R>::value>::type,
                          int = 0>
                ranges::owning_view<Decayed> operator()(R&& r) const {
                    return ranges::owning_view<Decayed>(std::move(r));
                }
            };
        } // namespace detail

#if (COMPAT_CPLUSPLUS >= COMPAT_CXX_17)
        inline constexpr detail::all_fn all{};
#else
        namespace {
            constexpr const detail::all_fn& all = static_const<detail::all_fn>::value;
        }
#endif

        template <typename R>
        using all_t = decltype(all(std::declval<R>()));

    } // namespace views

    namespace ranges {

        // --- views::as_const Implementation (ISO C++23 / P2278R4) ---
        template <typename V>
        class as_const_view : public view_interface<as_const_view<V>> {
        private:
            V base_{};

            template <bool IsConst>
            class iterator_impl {
            private:
                using BaseIter = typename std::conditional<IsConst,
                    iterator_t<const V>,
                    iterator_t<V>
                >::type;
                BaseIter current_{};

            public:
                using difference_type = range_difference_t<V>;
                using value_type = range_value_t<V>;
                using reference = typename std::conditional<
                    std::is_reference<range_reference_t<V>>::value,
                    typename std::add_const<typename std::remove_reference<range_reference_t<V>>::type>::type&,
                    range_reference_t<V>
                >::type;
                using pointer = typename std::add_pointer<reference>::type;
                using iterator_category = typename std::iterator_traits<BaseIter>::iterator_category;

                iterator_impl() = default;
                explicit iterator_impl(BaseIter it) : current_(std::move(it)) {}

                reference operator*() const { return *current_; }
                BaseIter base() const { return current_; }

                iterator_impl& operator++() { ++current_; return *this; }
                iterator_impl operator++(int) { iterator_impl tmp = *this; ++(*this); return tmp; }
                iterator_impl& operator--() { --current_; return *this; }
                iterator_impl operator--(int) { iterator_impl tmp = *this; --(*this); return tmp; }

                iterator_impl& operator+=(difference_type n) { current_ += n; return *this; }
                iterator_impl& operator-=(difference_type n) { current_ -= n; return *this; }
                reference operator[](difference_type n) const { return current_[n]; }

                friend bool operator==(const iterator_impl& x, const iterator_impl& y) { return x.current_ == y.current_; }
                friend bool operator!=(const iterator_impl& x, const iterator_impl& y) { return !(x == y); }
                friend bool operator<(const iterator_impl& x, const iterator_impl& y) { return x.current_ < y.current_; }
                friend bool operator>(const iterator_impl& x, const iterator_impl& y) { return y < x; }
                friend bool operator<=(const iterator_impl& x, const iterator_impl& y) { return !(y < x); }
                friend bool operator>=(const iterator_impl& x, const iterator_impl& y) { return !(x < y); }
                friend difference_type operator-(const iterator_impl& x, const iterator_impl& y) { return x.current_ - y.current_; }
                friend iterator_impl operator+(iterator_impl x, difference_type n) { return x += n; }
                friend iterator_impl operator+(difference_type n, iterator_impl x) { return x += n; }
                friend iterator_impl operator-(iterator_impl x, difference_type n) { return x -= n; }
            };

        public:
            as_const_view() = default;
            explicit as_const_view(V base) : base_(std::move(base)) {}

            V base() const & { return base_; }
            V base() && { return std::move(base_); }

            auto begin() -> iterator_impl<false> { return iterator_impl<false>(ranges::begin(base_)); }
            auto begin() const -> iterator_impl<true> { return iterator_impl<true>(ranges::begin(base_)); }
            auto end() -> iterator_impl<false> { return iterator_impl<false>(ranges::end(base_)); }
            auto end() const -> iterator_impl<true> { return iterator_impl<true>(ranges::end(base_)); }
            auto size() -> decltype(ranges::size(base_)) { return ranges::size(base_); }
            auto size() const -> decltype(ranges::size(base_)) { return ranges::size(base_); }
        };

    } // namespace ranges

    namespace views {

        namespace detail {
            struct as_const_fn : ranges::range_adaptor_closure<as_const_fn> {
                template <typename R, typename = typename std::enable_if<ranges::constant_range<R>::value>::type>
                auto operator()(R&& r) const -> decltype(views::all(std::forward<R>(r))) {
                    return views::all(std::forward<R>(r));
                }

                template <typename R, typename = typename std::enable_if<!ranges::constant_range<R>::value>::type, int = 0>
                auto operator()(R&& r) const -> ranges::as_const_view<all_t<R>> {
                    return ranges::as_const_view<all_t<R>>(views::all(std::forward<R>(r)));
                }
            };
        } // namespace detail

#if (COMPAT_CPLUSPLUS >= COMPAT_CXX_17)
        inline constexpr detail::as_const_fn as_const{};
#else
        namespace {
            constexpr const detail::as_const_fn& as_const = static_const<detail::as_const_fn>::value;
        }
#endif

    } // namespace views

    // --- views::cache_latest Implementation (ISO C++26 / P3138R5) ---
    namespace ranges {
        namespace detail {

            /// <summary>
            /// 快取槽 (泛型版：專供 PRValue 暫存物件，單槽數值儲存保障生命週期，ISO non-propagating-cache 合規)。
            /// </summary>
            template <typename Ref, bool IsLvalue = std::is_lvalue_reference<Ref>::value>
            struct cache_slot {
                using val_t = typename std::decay<Ref>::type;
                bool has_value_{false};
                alignas(val_t) unsigned char storage_[sizeof(val_t)];

                cache_slot() = default;
                cache_slot(const cache_slot&) noexcept : has_value_(false) {}
                cache_slot& operator=(const cache_slot&) noexcept { reset(); return *this; }

                cache_slot(cache_slot&& other) noexcept : has_value_(false) {
                    other.reset();
                }
                cache_slot& operator=(cache_slot&& other) noexcept {
                    reset();
                    other.reset();
                    return *this;
                }

                void reset() noexcept {
                    if (has_value_) {
                        reinterpret_cast<val_t*>(storage_)->~val_t();
                        has_value_ = false;
                    }
                }

                template <typename F>
                val_t& emplace_deref(F&& f) {
                    if (!has_value_) {
                        ::new (static_cast<void*>(storage_)) val_t(f());
                        has_value_ = true;
                    }
#if (COMPAT_CPLUSPLUS >= COMPAT_CXX_17)
                    return *std::launder(reinterpret_cast<val_t*>(storage_));
#else
                    return *reinterpret_cast<val_t*>(storage_);
#endif
                }

                ~cache_slot() { reset(); }
            };

            /// <summary>
            /// 快取槽 (左值引用特化版：快取容器底層裸指標，支援 Move-Only，零深拷貝)。
            /// </summary>
            template <typename Ref>
            struct cache_slot<Ref, true> {
                using raw_t = typename std::remove_reference<Ref>::type;
                raw_t* ptr_{nullptr};

                cache_slot() = default;
                cache_slot(const cache_slot&) noexcept : ptr_(nullptr) {}
                cache_slot& operator=(const cache_slot&) noexcept { ptr_ = nullptr; return *this; }
                cache_slot(cache_slot&& other) noexcept : ptr_(nullptr) { other.ptr_ = nullptr; }
                cache_slot& operator=(cache_slot&& other) noexcept {
                    ptr_ = nullptr;
                    other.ptr_ = nullptr;
                    return *this;
                }

                void reset() noexcept { ptr_ = nullptr; }

                template <typename F>
                Ref emplace_deref(F&& f) {
                    if (!ptr_) {
                        ptr_ = std::addressof(f());
                    }
                    return *ptr_;
                }
            };

        } // namespace detail

        /// <summary>
        /// ISO C++26 最新元素快取 View (對齊 P3138R5 std::views::cache_latest)。
        /// </summary>
        template <typename V>
        class cache_latest_view : public view_interface<cache_latest_view<V>> {
            friend class iterator;
        private:
            V base_{};
            mutable detail::cache_slot<range_reference_t<V>> cache_{};

        public:
            cache_latest_view() = default;
            explicit cache_latest_view(V base) : base_(std::move(base)) {}

            V base() const & { return base_; }
            V base() && { return std::move(base_); }
            auto base_end() const -> decltype(ranges::end(base_)) { return ranges::end(base_); }

            class iterator {
            private:
                cache_latest_view* parent_{nullptr};
                iterator_t<V> current_{};

            public:
                using difference_type = range_difference_t<V>;
                using value_type = range_value_t<V>;
                using reference = typename std::conditional<
                    std::is_lvalue_reference<range_reference_t<V>>::value,
                    range_reference_t<V>,
                    typename std::decay<range_reference_t<V>>::type&
                >::type;
                using iterator_category = std::input_iterator_tag;

                iterator() = default;
                iterator(cache_latest_view* p, iterator_t<V> cur)
                    : parent_(p), current_(std::move(cur)) {}

                reference operator*() const {
                    COMPAT_ASSERT(parent_ != nullptr);
                    return parent_->cache_.emplace_deref([this]() -> range_reference_t<V> {
                        return *this->current_;
                    });
                }

                template <bool Enable = std::is_lvalue_reference<range_reference_t<V>>::value,
                          typename = typename std::enable_if<Enable>::type>
                auto operator->() const -> typename std::add_pointer<range_reference_t<V>>::type {
                    return std::addressof(**this);
                }

                iterator& operator++() {
                    COMPAT_ASSERT(parent_ != nullptr);
                    parent_->cache_.reset();
                    ++current_;
                    return *this;
                }

                void operator++(int) {
                    ++(*this);
                }

                friend bool operator==(const iterator& x, default_sentinel_t) {
                    return x.current_ == x.parent_->base_end();
                }
                friend bool operator==(default_sentinel_t, const iterator& x) {
                    return x == default_sentinel;
                }
                friend bool operator!=(const iterator& x, default_sentinel_t s) {
                    return !(x == s);
                }
                friend bool operator!=(default_sentinel_t s, const iterator& x) {
                    return !(x == s);
                }

                friend bool operator==(const iterator& x, const iterator& y) {
                    return x.current_ == y.current_;
                }
                friend bool operator!=(const iterator& x, const iterator& y) {
                    return !(x == y);
                }
            };

            iterator begin() {
                cache_.reset();
                return iterator(this, ranges::begin(base_));
            }

            default_sentinel_t end() const noexcept {
                return default_sentinel_t{};
            }

            auto size() -> decltype(ranges::size(base_)) { return ranges::size(base_); }
            auto size() const -> decltype(ranges::size(base_)) { return ranges::size(base_); }
        };

    } // namespace ranges

    namespace views {

        namespace detail {
            struct cache_latest_fn : ranges::range_adaptor_closure<cache_latest_fn> {
                template <typename R>
                ranges::cache_latest_view<all_t<R>> operator()(R&& r) const {
                    return ranges::cache_latest_view<all_t<R>>(views::all(std::forward<R>(r)));
                }
            };
        } // namespace detail

#if (COMPAT_CPLUSPLUS >= COMPAT_CXX_17)
        inline constexpr detail::cache_latest_fn cache_latest{};
#else
        namespace {
            constexpr const detail::cache_latest_fn& cache_latest = static_const<detail::cache_latest_fn>::value;
        }
#endif

    } // namespace views

    // --- views::concat Implementation (ISO C++26 / P2542R8 / N4984 - Plan 36) ---
    namespace ranges {
        namespace detail {

            /// <summary>
            /// 條件式前綴長度儲存基類 (純 Sized 驅動)。
            /// </summary>
            template <bool EnableSized, size_t N>
            struct concat_size_storage {
                using diff_t = std::ptrdiff_t;
                template <typename Tuple>
                void init_prefix_sizes(const Tuple&) noexcept {}
            };

            template <size_t N>
            struct concat_size_storage<true, N> {
                using diff_t = std::ptrdiff_t;
                std::array<size_t, N + 1> prefix_sizes_{};

                template <typename Tuple>
                void init_prefix_sizes(const Tuple& views) {
                    prefix_sizes_[0] = 0;
                    init_prefix_sizes_impl(views, compat::detail::make_index_sequence<N>{});
                }

                diff_t prefix_size(size_t idx) const noexcept {
                    return static_cast<diff_t>(prefix_sizes_[idx]);
                }

            private:
                template <typename Tuple, size_t... Is>
                void init_prefix_sizes_impl(const Tuple& views, compat::detail::index_sequence<Is...>) {
                    size_t acc = 0;
                    int dummy[] = { 0, (prefix_sizes_[Is + 1] = (acc += ranges::size(std::get<Is>(views))), 0)... };
                    (void)dummy;
                }
            };

            template <typename Tuple, typename Seq>
            struct all_common_except_last_impl;

            template <typename Tuple, size_t... Is>
            struct all_common_except_last_impl<Tuple, compat::detail::index_sequence<Is...>> {
                static constexpr bool value = conjunction<
                    ranges::common_range<typename std::tuple_element<Is, Tuple>::type>...
                >::value;
            };

            template <typename... Views>
            struct all_common_except_last {
                static constexpr size_t N = sizeof...(Views);
                static constexpr bool value = (N <= 1) ? true :
                    all_common_except_last_impl<std::tuple<Views...>, compat::detail::make_index_sequence<N - 1>>::value;
            };

            template <bool AllConst, typename... Views>
            struct concat_const_types {
                using const_reference = void;
                using iter_tuple = std::tuple<>;
            };

            template <typename... Views>
            struct concat_const_types<true, Views...> {
                using const_reference = common_reference_t<range_reference_t<const Views>...>;
                using iter_tuple = std::tuple<iterator_t<const Views>...>;
            };

            template <size_t N_Views, typename ConcatView, typename IterTuple, typename Ref>
            class iterator_storage_base {
            public:
                using diff_t = std::ptrdiff_t;
                using reference = Ref;

            protected:
                ConcatView* parent_{nullptr};
                size_t active_index_{N_Views};
                IterTuple iters_{};

            public:
                iterator_storage_base() = default;
                iterator_storage_base(ConcatView* p, size_t idx, IterTuple it)
                    : parent_(p), active_index_(idx), iters_(std::move(it)) {}

                size_t active_index() const noexcept { return active_index_; }
                ConcatView* parent() const noexcept { return parent_; }
                IterTuple& iters() noexcept { return iters_; }
                const IterTuple& iters() const noexcept { return iters_; }
            };

            template <size_t N_Views, bool EnableBidi, typename Iterator, typename ConcatView, typename IterTuple, typename Ref>
            struct iterator_bidirectional_base
                : iterator_storage_base<N_Views, ConcatView, IterTuple, Ref> {
                using Base = iterator_storage_base<N_Views, ConcatView, IterTuple, Ref>;
                using Base::Base;
            };

            template <size_t N_Views, typename Iterator, typename ConcatView, typename IterTuple, typename Ref>
            struct iterator_bidirectional_base<N_Views, true, Iterator, ConcatView, IterTuple, Ref>
                : iterator_storage_base<N_Views, ConcatView, IterTuple, Ref> {
                using Base = iterator_storage_base<N_Views, ConcatView, IterTuple, Ref>;
                using Base::Base;

                Iterator& operator--() {
                    static_cast<Iterator*>(this)->step_backward();
                    return *static_cast<Iterator*>(this);
                }
                Iterator operator--(int) {
                    Iterator tmp = *static_cast<Iterator*>(this);
                    --(*this);
                    return tmp;
                }
            };

            template <size_t N_Views, bool EnableRandomAccess, typename Iterator, typename ConcatView, typename IterTuple, typename Ref>
            struct iterator_random_access_base
                : iterator_bidirectional_base<N_Views, ConcatView::IsBidi, Iterator, ConcatView, IterTuple, Ref> {
                using Base = iterator_bidirectional_base<N_Views, ConcatView::IsBidi, Iterator, ConcatView, IterTuple, Ref>;
                using Base::Base;
            };

            template <size_t N_Views, typename Iterator, typename ConcatView, typename IterTuple, typename Ref>
            struct iterator_random_access_base<N_Views, true, Iterator, ConcatView, IterTuple, Ref>
                : iterator_bidirectional_base<N_Views, true, Iterator, ConcatView, IterTuple, Ref> {
                using Base = iterator_bidirectional_base<N_Views, true, Iterator, ConcatView, IterTuple, Ref>;
                using Base::Base;

            public:
                using diff_t = std::ptrdiff_t;

                diff_t current_offset() const {
                    return current_offset_impl(compat::detail::make_index_sequence<N_Views>{});
                }

            protected:
                template <typename It1, typename It2>
                static diff_t subtract(const It1& lhs, const It2& rhs) {
                    const diff_t off_lhs = (lhs.active_index() < N_Views) ? lhs.current_offset() : 0;
                    const diff_t off_rhs = (rhs.active_index() < N_Views) ? rhs.current_offset() : 0;
                    return (lhs.parent()->prefix_size(lhs.active_index()) + off_lhs) -
                           (rhs.parent()->prefix_size(rhs.active_index()) + off_rhs);
                }

            private:
                template <size_t... Is>
                diff_t current_offset_impl(compat::detail::index_sequence<Is...>) const {
                    diff_t res = 0;
                    int dummy[] = { 0, (this->active_index_ == Is ? (res = static_cast<diff_t>(std::get<Is>(this->iters_) - ranges::begin(std::get<Is>(this->parent_->base_views()))), 0) : 0)... };
                    (void)dummy;
                    return res;
                }

            public:
                friend diff_t operator-(const Iterator& it, default_sentinel_t) {
                    const diff_t cur_pos = (it.active_index() < N_Views)
                        ? (it.parent()->prefix_size(it.active_index()) + it.current_offset())
                        : static_cast<diff_t>(it.parent()->prefix_size(N_Views));
                    const diff_t total_size = static_cast<diff_t>(it.parent()->prefix_size(N_Views));
                    return cur_pos - total_size;
                }

                friend diff_t operator-(default_sentinel_t s, const Iterator& it) {
                    return -(it - s);
                }

                Iterator& operator+=(diff_t n) {
                    static_cast<Iterator*>(this)->advance_random(n);
                    return *static_cast<Iterator*>(this);
                }
                Iterator& operator-=(diff_t n) { return *this += (-n); }
                friend Iterator operator+(Iterator it, diff_t n) { return it += n; }
                friend Iterator operator+(diff_t n, Iterator it) { return it += n; }
                friend Iterator operator-(Iterator it, diff_t n) { return it -= n; }

                Ref operator[](diff_t n) const {
                    const diff_t cur_pos = (this->active_index() < N_Views)
                        ? (this->parent()->prefix_size(this->active_index()) + this->current_offset())
                        : static_cast<diff_t>(this->parent()->prefix_size(N_Views));
                    const diff_t target_pos = cur_pos + n;
                    COMPAT_ASSERT(target_pos >= 0 && target_pos < static_cast<diff_t>(this->parent()->prefix_size(N_Views)));
                    return (*this->parent())[target_pos];
                }

                friend bool operator<(const Iterator& lhs, const Iterator& rhs) { return subtract(lhs, rhs) < 0; }
                friend bool operator>(const Iterator& lhs, const Iterator& rhs) { return rhs < lhs; }
                friend bool operator<=(const Iterator& lhs, const Iterator& rhs) { return !(rhs < lhs); }
                friend bool operator>=(const Iterator& lhs, const Iterator& rhs) { return !(lhs < rhs); }
            };

            template <typename Concat>
            using concat_ref_t = typename std::conditional<
                std::is_const<typename std::remove_reference<Concat>::type>::value,
                typename std::decay<Concat>::type::const_reference,
                typename std::decay<Concat>::type::reference
            >::type;

            template <typename Concat, size_t I>
            inline concat_ref_t<Concat> subscript_at(Concat& c, std::ptrdiff_t off) {
                return static_cast<concat_ref_t<Concat>>((ranges::begin(std::get<I>(c.base_views())))[off]);
            }

            template <size_t N_Views>
            struct subscript_ladder {
                template <typename Concat, typename Diff>
                static concat_ref_t<Concat> apply(Concat&& c, Diff target_abs) {
                    const size_t idx = c.find_index_for_offset(target_abs);
                    const std::ptrdiff_t off = target_abs - c.prefix_size(idx);
                    return dispatch_switch(c, idx, off);
                }

            private:
                template <typename Concat>
                static concat_ref_t<Concat> dispatch_switch(Concat& c, size_t idx, std::ptrdiff_t off) {
                    switch (idx) {
                        case 0: return subscript_at<Concat, 0>(c, off);
                        case 1: if (N_Views > 1) return subscript_at<Concat, (N_Views > 1 ? 1 : 0)>(c, off); break;
                        case 2: if (N_Views > 2) return subscript_at<Concat, (N_Views > 2 ? 2 : 0)>(c, off); break;
                        case 3: if (N_Views > 3) return subscript_at<Concat, (N_Views > 3 ? 3 : 0)>(c, off); break;
                        case 4: if (N_Views > 4) return subscript_at<Concat, (N_Views > 4 ? 4 : 0)>(c, off); break;
                        case 5: if (N_Views > 5) return subscript_at<Concat, (N_Views > 5 ? 5 : 0)>(c, off); break;
                        case 6: if (N_Views > 6) return subscript_at<Concat, (N_Views > 6 ? 6 : 0)>(c, off); break;
                        case 7: if (N_Views > 7) return subscript_at<Concat, (N_Views > 7 ? 7 : 0)>(c, off); break;
                        case 8: if (N_Views > 8) return subscript_at<Concat, (N_Views > 8 ? 8 : 0)>(c, off); break;
                        case 9: if (N_Views > 9) return subscript_at<Concat, (N_Views > 9 ? 9 : 0)>(c, off); break;
                        case 10: if (N_Views > 10) return subscript_at<Concat, (N_Views > 10 ? 10 : 0)>(c, off); break;
                        case 11: if (N_Views > 11) return subscript_at<Concat, (N_Views > 11 ? 11 : 0)>(c, off); break;
                        case 12: if (N_Views > 12) return subscript_at<Concat, (N_Views > 12 ? 12 : 0)>(c, off); break;
                        case 13: if (N_Views > 13) return subscript_at<Concat, (N_Views > 13 ? 13 : 0)>(c, off); break;
                        case 14: if (N_Views > 14) return subscript_at<Concat, (N_Views > 14 ? 14 : 0)>(c, off); break;
                        case 15: if (N_Views > 15) return subscript_at<Concat, (N_Views > 15 ? 15 : 0)>(c, off); break;
                        default: compat::detail::compat_unreachable_abort();
                    }
                    compat::detail::compat_unreachable_abort();
                }
            };

            template <>
            struct subscript_ladder<1> {
                template <typename Concat, typename Diff>
                static concat_ref_t<Concat> apply(Concat&& c, Diff n) {
                    return subscript_at<Concat, 0>(c, static_cast<std::ptrdiff_t>(n));
                }
            };

            template <>
            struct subscript_ladder<2> {
                template <typename Concat, typename Diff>
                static concat_ref_t<Concat> apply(Concat&& c, Diff n) {
                    if (n < c.prefix_size(1)) return subscript_at<Concat, 0>(c, static_cast<std::ptrdiff_t>(n));
                    return subscript_at<Concat, 1>(c, static_cast<std::ptrdiff_t>(n - c.prefix_size(1)));
                }
            };

            template <>
            struct subscript_ladder<3> {
                template <typename Concat, typename Diff>
                static concat_ref_t<Concat> apply(Concat&& c, Diff n) {
                    if (n < c.prefix_size(1)) return subscript_at<Concat, 0>(c, static_cast<std::ptrdiff_t>(n));
                    if (n < c.prefix_size(2)) return subscript_at<Concat, 1>(c, static_cast<std::ptrdiff_t>(n - c.prefix_size(1)));
                    return subscript_at<Concat, 2>(c, static_cast<std::ptrdiff_t>(n - c.prefix_size(2)));
                }
            };

            template <>
            struct subscript_ladder<4> {
                template <typename Concat, typename Diff>
                static concat_ref_t<Concat> apply(Concat&& c, Diff n) {
                    if (n < c.prefix_size(1)) return subscript_at<Concat, 0>(c, static_cast<std::ptrdiff_t>(n));
                    if (n < c.prefix_size(2)) return subscript_at<Concat, 1>(c, static_cast<std::ptrdiff_t>(n - c.prefix_size(1)));
                    if (n < c.prefix_size(3)) return subscript_at<Concat, 2>(c, static_cast<std::ptrdiff_t>(n - c.prefix_size(2)));
                    return subscript_at<Concat, 3>(c, static_cast<std::ptrdiff_t>(n - c.prefix_size(3)));
                }
            };

            template <size_t I, typename ViewTuple, typename IterTuple>
            typename std::enable_if<common_range<typename std::tuple_element<I, ViewTuple>::type>::value>::type
            assign_end_if_common(ViewTuple& views, IterTuple& iters) {
                std::get<I>(iters) = ranges::end(std::get<I>(views));
            }

            template <size_t I, typename ViewTuple, typename IterTuple>
            typename std::enable_if<!common_range<typename std::tuple_element<I, ViewTuple>::type>::value>::type
            assign_end_if_common(ViewTuple&, IterTuple&) {}

        } // namespace detail

        /// <summary>
        /// ISO C++26 串接檢視本體 (對齊 P2542R8 / N4984 std::views::concat)。
        /// </summary>
        template <typename... Views>
        class concat_view
            : public view_interface<concat_view<Views...>>,
              public detail::concat_size_storage<
                  conjunction<ranges::sized_range<Views>...>::value,
                  sizeof...(Views)
              > {
        public:
            using diff_t = std::ptrdiff_t;
            static constexpr size_t N = sizeof...(Views);
            static constexpr bool AllSized = conjunction<ranges::sized_range<Views>...>::value;
            static constexpr bool AllCommon = conjunction<ranges::common_range<Views>...>::value;
            static constexpr bool AllBidi = conjunction<ranges::bidirectional_range<Views>...>::value;
            static constexpr bool AllRandom = conjunction<ranges::random_access_range<Views>...>::value;

            static constexpr bool IsBidi = AllBidi && detail::all_common_except_last<Views...>::value;
            static constexpr bool IsRandomAndSized = AllRandom && AllSized;
            static constexpr bool IsCommon = AllCommon;
            static constexpr bool AllConstRange = conjunction<ranges::range<const Views>...>::value;

            using value_type = common_type_t<ranges::range_value_t<Views>...>;
            using reference = common_reference_t<range_reference_t<Views>...>;
            using const_reference = typename detail::concat_const_types<AllConstRange, Views...>::const_reference;

        private:
            template <size_t> friend struct detail::subscript_ladder;
            std::tuple<Views...> views_{};

        public:
            concat_view() = default;
            explicit concat_view(Views... views)
                : views_(std::move(views)...) {
                this->init_prefix_sizes(this->views_);
            }

            std::tuple<Views...>& base_views() noexcept { return views_; }
            const std::tuple<Views...>& base_views() const noexcept { return views_; }

            template <bool Enable = AllSized, typename = typename std::enable_if<Enable>::type>
            constexpr auto size() const noexcept -> decltype(this->prefix_size(N)) {
                return this->prefix_size(N);
            }

            template <bool IsConst>
            class basic_iterator
                : public detail::iterator_random_access_base<
                      sizeof...(Views),
                      IsRandomAndSized,
                      basic_iterator<IsConst>,
                      typename std::conditional<IsConst, const concat_view, concat_view>::type,
                      typename std::conditional<IsConst,
                          typename detail::concat_const_types<AllConstRange, Views...>::iter_tuple,
                          std::tuple<iterator_t<Views>...>
                      >::type,
                      typename std::conditional<IsConst, const_reference, typename concat_view::reference>::type
                  > {
                template <bool> friend class basic_iterator;
                using parent_view_t = typename std::conditional<IsConst, const concat_view, concat_view>::type;
                using parent_view_ptr = parent_view_t*;

                using Base = detail::iterator_random_access_base<
                    sizeof...(Views),
                    IsRandomAndSized,
                    basic_iterator<IsConst>,
                    parent_view_t,
                    typename std::conditional<IsConst,
                        typename detail::concat_const_types<AllConstRange, Views...>::iter_tuple,
                        std::tuple<iterator_t<Views>...>
                    >::type,
                    typename std::conditional<IsConst, const_reference, typename concat_view::reference>::type
                >;

            public:
                using difference_type = std::ptrdiff_t;
                using value_type = typename concat_view::value_type;
                using reference = typename std::conditional<IsConst, const_reference, typename concat_view::reference>::type;
                using pointer = typename std::conditional<
                    std::is_lvalue_reference<reference>::value,
                    typename std::add_pointer<reference>::type,
                    void
                >::type;

                using iterator_concept = typename std::conditional<
                    IsRandomAndSized,
                    std::random_access_iterator_tag,
                    typename std::conditional<
                        IsBidi,
                        std::bidirectional_iterator_tag,
                        std::forward_iterator_tag
                    >::type
                >::type;

                using iterator_category = typename std::conditional<
                    IsRandomAndSized,
                    std::random_access_iterator_tag,
                    typename std::conditional<
                        IsBidi,
                        std::bidirectional_iterator_tag,
                        std::input_iterator_tag
                    >::type
                >::type;

                basic_iterator() = default;
                basic_iterator(parent_view_ptr p, size_t idx)
                    : Base(p, idx, {}) {
                    if (this->parent_ && this->active_index_ < N) {
                        init_active_iter(this->active_index_);
                        satisfy();
                    }
                }

                // 支援從 non-const iterator 轉換至 const iterator
                template <bool OtherConst, typename = typename std::enable_if<IsConst && !OtherConst>::type>
                basic_iterator(const basic_iterator<OtherConst>& other)
                    : Base(other.parent(), other.active_index(), other.iters()) {}

                reference operator*() const {
                    return deref_impl(compat::detail::make_index_sequence<N>{});
                }

                template <bool Enable = std::is_lvalue_reference<reference>::value,
                          typename = typename std::enable_if<Enable>::type>
                pointer operator->() const {
                    return std::addressof(**this);
                }

                void satisfy() {
                    while (this->active_index_ < N) {
                        if (!is_active_at_end(this->active_index_)) {
                            break;
                        }
                        ++this->active_index_;
                        if (this->active_index_ < N) {
                            init_active_iter(this->active_index_);
                        }
                    }
                }

                basic_iterator& operator++() {
                    step_forward(this->active_index_);
                    satisfy();
                    return *this;
                }

                basic_iterator operator++(int) {
                    basic_iterator tmp = *this;
                    ++(*this);
                    return tmp;
                }

                void step_backward() {
                    if (this->active_index_ == N) {
                        retreat_to_last_non_empty();
                        return;
                    }
                    if (!is_active_at_begin(this->active_index_)) {
                        step_backward_single(this->active_index_);
                        return;
                    }
                    retreat_to_prev_non_empty();
                }

                void advance_random(diff_t n) {
                    const diff_t cur_pos = (this->active_index_ < N)
                        ? (this->parent_->prefix_size(this->active_index_) + this->current_offset())
                        : static_cast<diff_t>(this->parent_->prefix_size(N));
                    const diff_t target_pos = cur_pos + n;
                    COMPAT_ASSERT(target_pos >= 0 && target_pos <= static_cast<diff_t>(this->parent_->prefix_size(N)));

                    if (target_pos == static_cast<diff_t>(this->parent_->prefix_size(N))) {
                        this->active_index_ = N;
                        return;
                    }

                    const size_t target_idx = this->parent_->find_index_for_offset(target_pos);
                    this->active_index_ = target_idx;
                    init_active_iter(this->active_index_);
                    const diff_t off_in_sub = target_pos - this->parent_->prefix_size(this->active_index_);
                    advance_active_sub(this->active_index_, off_in_sub);
                    satisfy();
                }

                friend bool operator==(const basic_iterator& lhs, const basic_iterator& rhs) {
                    if (lhs.active_index() != rhs.active_index()) return false;
                    if (lhs.active_index() == N) return true;
                    return lhs.equals_impl(rhs, compat::detail::make_index_sequence<N>{});
                }

                friend bool operator!=(const basic_iterator& lhs, const basic_iterator& rhs) {
                    return !(lhs == rhs);
                }

                template <bool OtherConst, typename = typename std::enable_if<IsConst != OtherConst>::type>
                friend bool operator==(const basic_iterator& lhs, const basic_iterator<OtherConst>& rhs) {
                    if (lhs.active_index() != rhs.active_index()) return false;
                    if (lhs.active_index() == N) return true;
                    return lhs.equals_impl(rhs, compat::detail::make_index_sequence<N>{});
                }

                template <bool OtherConst, typename = typename std::enable_if<IsConst != OtherConst>::type>
                friend bool operator!=(const basic_iterator& lhs, const basic_iterator<OtherConst>& rhs) {
                    return !(lhs == rhs);
                }

                friend bool operator==(const basic_iterator& it, default_sentinel_t) { return it.active_index() == N; }
                friend bool operator==(default_sentinel_t s, const basic_iterator& it) { return it == s; }
                friend bool operator!=(const basic_iterator& it, default_sentinel_t s) { return !(it == s); }
                friend bool operator!=(default_sentinel_t s, const basic_iterator& it) { return !(it == s); }

                template <bool Dummy = IsRandomAndSized, typename = typename std::enable_if<Dummy>::type>
                friend diff_t operator-(const basic_iterator& lhs, const basic_iterator& rhs) {
                    return lhs.subtract(lhs, rhs);
                }

                template <bool OtherConst, bool Dummy = IsRandomAndSized,
                          typename = typename std::enable_if<Dummy && (IsConst != OtherConst)>::type>
                friend diff_t operator-(const basic_iterator& lhs, const basic_iterator<OtherConst>& rhs) {
                    return lhs.subtract(lhs, rhs);
                }

            private:
                template <size_t... Is>
                reference deref_impl(compat::detail::index_sequence<Is...>) const {
                    return deref_helper<0>(this->active_index_);
                }

                template <size_t I = 0>
                typename std::enable_if<(I < N), reference>::type
                deref_helper(size_t idx) const {
                    if (idx == I) return static_cast<reference>(*std::get<I>(this->iters_));
                    return deref_helper<I + 1>(idx);
                }

                template <size_t I>
                typename std::enable_if<(I == N), reference>::type
                deref_helper(size_t) const {
                    compat::detail::compat_unreachable_abort();
                }

                void init_active_iter(size_t idx) {
                    init_iter_impl(idx, compat::detail::make_index_sequence<N>{});
                }

                template <size_t... Is>
                void init_iter_impl(size_t idx, compat::detail::index_sequence<Is...>) {
                    int dummy[] = { 0, (idx == Is ? (std::get<Is>(this->iters_) = ranges::begin(std::get<Is>(this->parent_->base_views())), 0) : 0)... };
                    (void)dummy;
                }

                bool is_active_at_end(size_t idx) const {
                    return is_at_end_impl(idx, compat::detail::make_index_sequence<N>{});
                }

                template <size_t... Is>
                bool is_at_end_impl(size_t idx, compat::detail::index_sequence<Is...>) const {
                    bool res = false;
                    int dummy[] = { 0, (idx == Is ? (res = (std::get<Is>(this->iters_) == ranges::end(std::get<Is>(this->parent_->base_views()))), 0) : 0)... };
                    (void)dummy;
                    return res;
                }

                bool is_active_at_begin(size_t idx) const {
                    return is_at_begin_impl(idx, compat::detail::make_index_sequence<N>{});
                }

                template <size_t... Is>
                bool is_at_begin_impl(size_t idx, compat::detail::index_sequence<Is...>) const {
                    bool res = false;
                    int dummy[] = { 0, (idx == Is ? (res = (std::get<Is>(this->iters_) == ranges::begin(std::get<Is>(this->parent_->base_views()))), 0) : 0)... };
                    (void)dummy;
                    return res;
                }

                void step_forward(size_t idx) {
                    step_forward_impl(idx, compat::detail::make_index_sequence<N>{});
                }

                template <size_t... Is>
                void step_forward_impl(size_t idx, compat::detail::index_sequence<Is...>) {
                    int dummy[] = { 0, (idx == Is ? (++std::get<Is>(this->iters_), 0) : 0)... };
                    (void)dummy;
                }

                void step_backward_single(size_t idx) {
                    step_backward_impl(idx, compat::detail::make_index_sequence<N>{});
                }

                template <size_t... Is>
                void step_backward_impl(size_t idx, compat::detail::index_sequence<Is...>) {
                    int dummy[] = { 0, (idx == Is ? (--std::get<Is>(this->iters_), 0) : 0)... };
                    (void)dummy;
                }

                void advance_active_sub(size_t idx, diff_t off) {
                    advance_sub_impl(idx, off, compat::detail::make_index_sequence<N>{});
                }

                template <size_t... Is>
                void advance_sub_impl(size_t idx, diff_t off, compat::detail::index_sequence<Is...>) {
                    int dummy[] = { 0, (idx == Is ? (std::get<Is>(this->iters_) += off, 0) : 0)... };
                    (void)dummy;
                }

                void retreat_to_last_non_empty() {
                    size_t k = N;
                    while (k > 0) {
                        --k;
                        init_active_iter(k);
                        if (!is_active_at_end(k)) {
                            init_iter_to_end(k);
                            step_backward_single(k);
                            this->active_index_ = k;
                            return;
                        }
                    }
                    this->active_index_ = N;
                }

                void retreat_to_prev_non_empty() {
                    size_t k = this->active_index_;
                    while (k > 0) {
                        --k;
                        init_active_iter(k);
                        if (!is_active_at_end(k)) {
                            init_iter_to_end(k);
                            step_backward_single(k);
                            this->active_index_ = k;
                            return;
                        }
                    }
                }

                void init_iter_to_end(size_t idx) {
                    init_iter_end_impl(idx, compat::detail::make_index_sequence<N>{});
                }

                template <size_t... Is>
                void init_iter_end_impl(size_t idx, compat::detail::index_sequence<Is...>) {
                    int dummy[] = { 0, (idx == Is ? (detail::assign_end_if_common<Is>(this->parent_->base_views(), this->iters_), 0) : 0)... };
                    (void)dummy;
                }

                template <bool OtherConst, size_t... Is>
                bool equals_impl(const basic_iterator<OtherConst>& rhs, compat::detail::index_sequence<Is...>) const {
                    bool res = false;
                    int dummy[] = { 0, (this->active_index() == Is ? (res = (std::get<Is>(this->iters()) == std::get<Is>(rhs.iters())), 0) : 0)... };
                    (void)dummy;
                    return res;
                }
            };

            basic_iterator<false> begin() {
                return basic_iterator<false>(this, 0);
            }

            template <bool Enable = AllConstRange, typename = typename std::enable_if<Enable>::type>
            basic_iterator<true> begin() const {
                return basic_iterator<true>(this, 0);
            }

            template <bool Enable = IsCommon, typename = typename std::enable_if<Enable>::type>
            basic_iterator<false> end() {
                return basic_iterator<false>(this, N);
            }

            template <bool Enable = IsCommon && AllConstRange, typename = typename std::enable_if<Enable>::type>
            basic_iterator<true> end() const {
                return basic_iterator<true>(this, N);
            }

            template <bool Enable = !IsCommon, typename = typename std::enable_if<Enable>::type>
            default_sentinel_t end() const noexcept {
                return default_sentinel_t{};
            }

            size_t find_index_for_offset(diff_t target_abs) const noexcept {
                size_t low = 0;
                size_t high = N;
                while (low < high) {
                    const size_t mid = low + (high - low) / 2;
                    if (static_cast<diff_t>(this->prefix_size(mid + 1)) <= target_abs) {
                        low = mid + 1;
                    } else {
                        high = mid;
                    }
                }
                return low < N ? low : (N - 1);
            }

            template <bool Enable = IsRandomAndSized,
                      typename = typename std::enable_if<Enable>::type>
            reference operator[](diff_t n) {
                return detail::subscript_ladder<N>::apply(*this, n);
            }

            template <bool Enable = IsRandomAndSized && AllConstRange,
                      typename = typename std::enable_if<Enable>::type>
            const_reference operator[](diff_t n) const {
                return detail::subscript_ladder<N>::apply(*this, n);
            }
        };

        template <typename... Views>
        struct enable_borrowed_range_helper<concat_view<Views...>> : std::false_type {};

#if (COMPAT_CPLUSPLUS >= COMPAT_CXX_14)
        template <typename... Views>
        COMPAT_CONSTEXPR_14 bool enable_borrowed_range<concat_view<Views...>> = false;
#endif

    } // namespace ranges

    namespace views {

        namespace detail {
            struct concat_fn {
                template <typename... R,
                          typename = typename std::enable_if<(sizeof...(R) > 0)>::type>
                ranges::concat_view<all_t<R>...> operator()(R&&... ranges) const {
                    return ranges::concat_view<all_t<R>...>(views::all(std::forward<R>(ranges))...);
                }
            };
        } // namespace detail

#if (COMPAT_CPLUSPLUS >= COMPAT_CXX_17)
        inline constexpr detail::concat_fn concat{};
#else
        namespace {
            constexpr const detail::concat_fn& concat = static_const<detail::concat_fn>::value;
        }
#endif

    } // namespace views

} // namespace self_ranges
} // namespace detail
} // namespace compat
