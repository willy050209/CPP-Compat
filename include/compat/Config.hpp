#pragma once

// Feature detection and configuration header for compat library.

#include <cstdlib>
#include <cstddef>
#include <utility>

#if defined(_MSVC_LANG)
#  define COMPAT_CPLUSPLUS _MSVC_LANG
#else
#  define COMPAT_CPLUSPLUS __cplusplus
#endif

// C++ standard level constants
#define COMPAT_CXX_11 201103L
#define COMPAT_CXX_14 201402L
#define COMPAT_CXX_17 201703L
#define COMPAT_CXX_20 202002L
#define COMPAT_CXX_23 202302L
#define COMPAT_CXX_26 202602L

// Backwards-compatible alias handling for forcing self/fallback implementation
#if defined(COMPAT_FORCE_FALLBACK) && !defined(COMPAT_FORCE_SELF_IMPLEMENTATION)
#  define COMPAT_FORCE_SELF_IMPLEMENTATION 1
#endif
#if defined(COMPAT_FORCE_SELF_IMPLEMENTATION) && !defined(COMPAT_FORCE_FALLBACK)
#  define COMPAT_FORCE_FALLBACK 1
#endif

// Mutual exclusion check
#if defined(COMPAT_FORCE_SELF_IMPLEMENTATION) && defined(COMPAT_FORCE_STD_IMPLEMENTATION)
#  error "Cannot force both self and std implementations simultaneously"
#endif

// Exception handling and throw-or-abort abstraction
#if defined(__cpp_exceptions) || defined(__EXCEPTIONS) || (defined(_MSC_VER) && defined(_CPPUNWIND))
#  define COMPAT_HAS_EXCEPTIONS 1
#  define COMPAT_THROW_OR_ABORT(ex) throw (ex)
#else
#  define COMPAT_HAS_EXCEPTIONS 0
#  define COMPAT_THROW_OR_ABORT(ex) std::abort()
#endif

#ifndef COMPAT_ASSERT
#  if defined(_DEBUG) || !defined(NDEBUG)
#    include <cassert>
#    define COMPAT_ASSERT(expr) assert(expr)
#  else
#    define COMPAT_ASSERT(expr) ((void)0)
#  endif
#endif

namespace compat {
namespace detail {

    /// <summary>
    /// 跨平台終止函式，優先執行 std::abort()，後置 __builtin_unreachable() 杜絕編譯器死碼消除。
    /// </summary>
    [[noreturn]] inline void compat_unreachable_abort() {
        std::abort();
#if defined(__GNUC__) || defined(__clang__)
        __builtin_unreachable();
#endif
    }

} // namespace detail
} // namespace compat

#if defined(_MSC_VER)
#  define COMPAT_UNREACHABLE() __assume(0)
#elif defined(__GNUC__) || defined(__clang__)
#  define COMPAT_UNREACHABLE() __builtin_unreachable()
#else
#  define COMPAT_UNREACHABLE() ::compat::detail::compat_unreachable_abort()
#endif

// Microarchitecture optimization macros
#if defined(_MSC_VER)
#  define COMPAT_ALWAYS_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#  define COMPAT_ALWAYS_INLINE __attribute__((always_inline)) inline
#else
#  define COMPAT_ALWAYS_INLINE inline
#endif

#if defined(__GNUC__) || defined(__clang__)
#  define COMPAT_LIKELY(x)   __builtin_expect(!!(x), 1)
#  define COMPAT_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#  define COMPAT_LIKELY(x)   (x)
#  define COMPAT_UNLIKELY(x) (x)
#endif

// Constexpr support for C++14+
#if (COMPAT_CPLUSPLUS >= COMPAT_CXX_14)
#  define COMPAT_CONSTEXPR_14 constexpr
#else
#  define COMPAT_CONSTEXPR_14 inline
#endif

// Constexpr support for C++20+
#if (COMPAT_CPLUSPLUS >= COMPAT_CXX_20)
#  define COMPAT_CONSTEXPR_20 constexpr
#else
#  define COMPAT_CONSTEXPR_20 inline
#endif


// ABI tagging and inline namespace support for fallback implementations
#ifndef COMPAT_ABI_TAG
#  define COMPAT_ABI_TAG abi_v1
#endif

// Probe standard library version header if available
#if defined(__has_include)
#  if __has_include(<version>)
#    include <version>
#  endif
#endif

#if defined(COMPAT_FORCE_SELF_IMPLEMENTATION)

// Forced self-implementation mode: disable all native C++ standard features
#  define COMPAT_HAS_STD_EXPECTED           0
#  define COMPAT_HAS_STD_PRINT              0
#  define COMPAT_HAS_STD_FORMAT             0
#  define COMPAT_HAS_STD_STRING_VIEW        0
#  define COMPAT_HAS_STD_VARIANT            0
#  define COMPAT_HAS_STD_RANGES             0
#  define COMPAT_HAS_STD_VIEWS_CONCAT       0
#  define COMPAT_HAS_STD_VIEWS_CACHE_LATEST 0
#  define COMPAT_HAS_STD_VIEWS_AS_CONST     0
#  define COMPAT_HAS_STD_CONSTANT_RANGE     0
#  define COMPAT_HAS_STD_RANGES_CONTAINS    0
#  define COMPAT_HAS_STD_RANGES_STARTS_WITH 0
#  define COMPAT_HAS_STD_RANGES_FOLD            0
#  define COMPAT_HAS_STD_RANGES_TO              0
#  define COMPAT_HAS_STD_RANGES_FIND_LAST       0
#  define COMPAT_HAS_STD_RANGES_IOTA            0
#  define COMPAT_HAS_STD_RANGES_SHIFT           0
#  define COMPAT_HAS_STD_RANGES_GENERATE_RANDOM 0

#else

// Feature detection: std::string_view (C++17+)
#  if defined(__cpp_lib_string_view) && (__cpp_lib_string_view >= 201606L)
#    define COMPAT_HAS_STD_STRING_VIEW 1
#  elif (COMPAT_CPLUSPLUS >= COMPAT_CXX_17)
#    define COMPAT_HAS_STD_STRING_VIEW 1
#  else
#    define COMPAT_HAS_STD_STRING_VIEW 0
#  endif

// Feature detection: std::variant (C++17+)
#  if defined(__cpp_lib_variant) && (__cpp_lib_variant >= 201606L)
#    define COMPAT_HAS_STD_VARIANT 1
#  elif (COMPAT_CPLUSPLUS >= COMPAT_CXX_17)
#    define COMPAT_HAS_STD_VARIANT 1
#  else
#    define COMPAT_HAS_STD_VARIANT 0
#  endif

// Feature detection: std::format (C++20+)
#  if defined(__cpp_lib_format) && (__cpp_lib_format >= 201907L)
#    define COMPAT_HAS_STD_FORMAT 1
#  elif (COMPAT_CPLUSPLUS >= COMPAT_CXX_20) && defined(__has_include)
#    if __has_include(<format>)
#      define COMPAT_HAS_STD_FORMAT 1
#    else
#      define COMPAT_HAS_STD_FORMAT 0
#    endif
#  else
#    define COMPAT_HAS_STD_FORMAT 0
#  endif

// Feature detection: std::expected (C++23+)
#  if defined(__cpp_lib_expected) && (__cpp_lib_expected >= 202202L)
#    define COMPAT_HAS_STD_EXPECTED 1
#  elif defined(_MSC_VER) && (COMPAT_CPLUSPLUS >= COMPAT_CXX_23) && defined(__has_include)
#    if __has_include(<expected>)
#      define COMPAT_HAS_STD_EXPECTED 1
#    else
#      define COMPAT_HAS_STD_EXPECTED 0
#    endif
#  else
#    define COMPAT_HAS_STD_EXPECTED 0
#  endif

// Feature detection: std::print (C++23+)
#  if defined(__cpp_lib_print) && (__cpp_lib_print >= 202207L)
#    define COMPAT_HAS_STD_PRINT 1
#  elif defined(_MSC_VER) && (COMPAT_CPLUSPLUS >= COMPAT_CXX_23) && defined(__has_include)
#    if __has_include(<print>)
#      define COMPAT_HAS_STD_PRINT 1
#    else
#      define COMPAT_HAS_STD_PRINT 0
#    endif
#  else
#    define COMPAT_HAS_STD_PRINT 0
#  endif

// Feature detection: std::ranges (C++20+)
#  if defined(__cpp_lib_ranges) && (__cpp_lib_ranges >= 201911L)
#    define COMPAT_HAS_STD_RANGES 1
#  elif (COMPAT_CPLUSPLUS >= COMPAT_CXX_20) && defined(__has_include)
#    if __has_include(<ranges>)
#      define COMPAT_HAS_STD_RANGES 1
#    else
#      define COMPAT_HAS_STD_RANGES 0
#    endif
#  else
#    define COMPAT_HAS_STD_RANGES 0
#  endif

// Feature detection: std::views::as_const (C++23+)
#  if defined(__cpp_lib_ranges_as_const) && (__cpp_lib_ranges_as_const >= 202207L)
#    define COMPAT_HAS_STD_VIEWS_AS_CONST 1
#  elif defined(__cpp_lib_ranges_as_const)
#    define COMPAT_HAS_STD_VIEWS_AS_CONST 1
#  else
#    define COMPAT_HAS_STD_VIEWS_AS_CONST 0
#  endif

// Feature detection: std::ranges::constant_range (C++26 / P2728R6)
#  if defined(__cpp_lib_ranges_constant_range) && (__cpp_lib_ranges_constant_range >= 202302L)
#    define COMPAT_HAS_STD_CONSTANT_RANGE 1
#  else
#    define COMPAT_HAS_STD_CONSTANT_RANGE 0
#  endif

// Feature detection: std::views::concat (C++26 / P2542R8)
#  if defined(__cpp_lib_ranges_concat) && (__cpp_lib_ranges_concat >= 202403L)
#    define COMPAT_HAS_STD_VIEWS_CONCAT 1
#  else
#    define COMPAT_HAS_STD_VIEWS_CONCAT 0
#  endif

// Feature detection: std::views::cache_latest (C++26 / P3138R5)
#  if defined(__cpp_lib_ranges_cache_latest) && (__cpp_lib_ranges_cache_latest >= 202406L)
#    define COMPAT_HAS_STD_VIEWS_CACHE_LATEST 1
#  else
#    define COMPAT_HAS_STD_VIEWS_CACHE_LATEST 0
#  endif

// Feature detection: std::ranges::contains (C++23+)
#  if defined(__cpp_lib_ranges_contains) && (__cpp_lib_ranges_contains >= 202207L)
#    define COMPAT_HAS_STD_RANGES_CONTAINS 1
#  elif defined(_MSC_VER) && (COMPAT_CPLUSPLUS >= COMPAT_CXX_23) && COMPAT_HAS_STD_RANGES
#    define COMPAT_HAS_STD_RANGES_CONTAINS 1
#  else
#    define COMPAT_HAS_STD_RANGES_CONTAINS 0
#  endif

// Feature detection: std::ranges::starts_with / ends_with (C++23+)
#  if defined(__cpp_lib_ranges_starts_ends_with) && (__cpp_lib_ranges_starts_ends_with >= 202106L)
#    define COMPAT_HAS_STD_RANGES_STARTS_WITH 1
#  elif defined(_MSC_VER) && (COMPAT_CPLUSPLUS >= COMPAT_CXX_23) && COMPAT_HAS_STD_RANGES
#    define COMPAT_HAS_STD_RANGES_STARTS_WITH 1
#  else
#    define COMPAT_HAS_STD_RANGES_STARTS_WITH 0
#  endif

// Feature detection: std::ranges::fold_left etc. (C++23+)
#  if defined(__cpp_lib_ranges_fold) && (__cpp_lib_ranges_fold >= 202207L)
#    define COMPAT_HAS_STD_RANGES_FOLD 1
#  elif defined(_MSC_VER) && (COMPAT_CPLUSPLUS >= COMPAT_CXX_23) && COMPAT_HAS_STD_RANGES
#    define COMPAT_HAS_STD_RANGES_FOLD 1
#  else
#    define COMPAT_HAS_STD_RANGES_FOLD 0
#  endif

// Feature detection: std::ranges::to (C++23 / P1206R7)
// Note: GCC 14 libstdc++ has a known bug (Bugzilla 115200) in std::ranges::to calling
// __c.emplace(__c.end(), *__it) instead of emplace_hint for associative containers.
#  if defined(__cpp_lib_ranges_to_container) && (__cpp_lib_ranges_to_container >= 202202L) && !defined(__GLIBCXX__)
#    define COMPAT_HAS_STD_RANGES_TO 1
#  elif defined(_MSC_VER) && (COMPAT_CPLUSPLUS >= COMPAT_CXX_23) && COMPAT_HAS_STD_RANGES
#    define COMPAT_HAS_STD_RANGES_TO 1
#  else
#    define COMPAT_HAS_STD_RANGES_TO 0
#  endif

// Feature detection: std::ranges::find_last (C++23+)
#  if defined(__cpp_lib_ranges_find_last) && (__cpp_lib_ranges_find_last >= 202207L)
#    define COMPAT_HAS_STD_RANGES_FIND_LAST 1
#  elif defined(_MSC_VER) && (COMPAT_CPLUSPLUS >= COMPAT_CXX_23) && COMPAT_HAS_STD_RANGES
#    define COMPAT_HAS_STD_RANGES_FIND_LAST 1
#  else
#    define COMPAT_HAS_STD_RANGES_FIND_LAST 0
#  endif

// Feature detection: std::ranges::iota (C++23+)
#  if defined(__cpp_lib_ranges_iota) && (__cpp_lib_ranges_iota >= 202202L)
#    define COMPAT_HAS_STD_RANGES_IOTA 1
#  elif defined(_MSC_VER) && (COMPAT_CPLUSPLUS >= COMPAT_CXX_23) && COMPAT_HAS_STD_RANGES
#    define COMPAT_HAS_STD_RANGES_IOTA 1
#  else
#    define COMPAT_HAS_STD_RANGES_IOTA 0
#  endif

// Feature detection: std::ranges::shift_left / shift_right (C++23+)
#  if defined(__cpp_lib_ranges_shift) && (__cpp_lib_ranges_shift >= 202202L)
#    define COMPAT_HAS_STD_RANGES_SHIFT 1
#  elif defined(_MSC_VER) && (COMPAT_CPLUSPLUS >= COMPAT_CXX_23) && COMPAT_HAS_STD_RANGES
#    define COMPAT_HAS_STD_RANGES_SHIFT 1
#  else
#    define COMPAT_HAS_STD_RANGES_SHIFT 0
#  endif

// Feature detection: std::ranges::generate_random (C++26+)
#  if defined(__cpp_lib_ranges_generate_random) && (__cpp_lib_ranges_generate_random >= 202403L)
#    define COMPAT_HAS_STD_RANGES_GENERATE_RANDOM 1
#  else
#    define COMPAT_HAS_STD_RANGES_GENERATE_RANDOM 0
#  endif

#endif // !defined(COMPAT_FORCE_SELF_IMPLEMENTATION)

// Check if forced std implementation is supported by standard library
#if defined(COMPAT_FORCE_STD_IMPLEMENTATION)
#  if !COMPAT_HAS_STD_STRING_VIEW
#    error "Standard C++ library does not support requested modern features"
#  endif
#endif

// Attribute support
#if defined(__has_cpp_attribute)
#  if __has_cpp_attribute(nodiscard)
#    define COMPAT_NODISCARD [[nodiscard]]
#  else
#    define COMPAT_NODISCARD
#  endif
#elif (COMPAT_CPLUSPLUS >= COMPAT_CXX_17)
#  define COMPAT_NODISCARD [[nodiscard]]
#else
#  define COMPAT_NODISCARD
#endif

namespace compat {
namespace detail {

    /// <summary>
    /// C++11 相容之 void_t 輔助模板型別（相當於 C++17 std::void_t）。
    /// </summary>
    template <typename... Ts>
    struct make_void {
        using type = void;
    };

    /// <summary>
    /// 用於 SFINAE 的 void 映射別名模板。
    /// </summary>
    template <typename... Ts>
    using void_t = typename make_void<Ts...>::type;

    /// <summary>
    /// C++11 相容之整數序列結構體（對齊 C++14 std::integer_sequence）。
    /// </summary>
    template <typename T, T... Ints>
    struct integer_sequence {
        using value_type = T;
        static constexpr std::size_t size() noexcept { return sizeof...(Ints); }
    };

    /// <summary>
    /// C++11 相容之索引序列別名（對齊 C++14 std::index_sequence）。
    /// </summary>
    template <std::size_t... Ints>
    using index_sequence = integer_sequence<std::size_t, Ints...>;

    namespace detail_seq {
        template <typename T, std::size_t N, T... Ints>
        struct make_int_seq_impl : make_int_seq_impl<T, N - 1, static_cast<T>(N - 1), Ints...> {};

        template <typename T, T... Ints>
        struct make_int_seq_impl<T, 0, Ints...> {
            using type = integer_sequence<T, Ints...>;
        };
    } // namespace detail_seq

    /// <summary>
    /// 建立指定長度索引序列之輔助別名（對齊 C++14 std::make_index_sequence）。
    /// </summary>
    template <std::size_t N>
    using make_index_sequence = typename detail_seq::make_int_seq_impl<std::size_t, N>::type;

    /// <summary>
    /// 依據引數包長度建立索引序列之輔助別名。
    /// </summary>
    template <typename... Ts>
    using index_sequence_for = make_index_sequence<sizeof...(Ts)>;

} // namespace detail
} // namespace compat

