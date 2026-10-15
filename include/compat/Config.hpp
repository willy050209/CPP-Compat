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
#define COMPAT_CXX_VERSION COMPAT_CPLUSPLUS
#define COMPAT_CXX_11 201103L
#define COMPAT_CXX_14 201402L
#define COMPAT_CXX_17 201703L
#define COMPAT_CXX_20 202002L
#define COMPAT_CXX_23 202302L
#define COMPAT_CXX_26 202602L

// Global ABI lock toggle: forces self-contained C++11 fallback shims across all standards for ABI stability
#if !defined(COMPAT_FORCE_SELF_IMPLEMENTATION)
#  if defined(COMPAT_FORCE_FALLBACK)
#    define COMPAT_FORCE_SELF_IMPLEMENTATION COMPAT_FORCE_FALLBACK
#  else
#    define COMPAT_FORCE_SELF_IMPLEMENTATION 0
#  endif
#endif

#if !defined(COMPAT_FORCE_FALLBACK)
#  define COMPAT_FORCE_FALLBACK COMPAT_FORCE_SELF_IMPLEMENTATION
#endif

// Mutual exclusion check
#if (COMPAT_FORCE_SELF_IMPLEMENTATION) && defined(COMPAT_FORCE_STD_IMPLEMENTATION)
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

// Constexpr support for C++11 and C++14+
#define COMPAT_CONSTEXPR constexpr

#if (COMPAT_CPLUSPLUS >= COMPAT_CXX_14)
#  define COMPAT_CXX14_CONSTEXPR constexpr
#  define COMPAT_CONSTEXPR_14    constexpr
#else
#  define COMPAT_CXX14_CONSTEXPR inline
#  define COMPAT_CONSTEXPR_14    inline
#endif

// Constexpr support for C++17+
#if (COMPAT_CPLUSPLUS >= COMPAT_CXX_17)
#  define COMPAT_CONSTEXPR_17 constexpr
#else
#  define COMPAT_CONSTEXPR_17 inline
#endif

// Constexpr support for C++20+
#if (COMPAT_CPLUSPLUS >= COMPAT_CXX_20)
#  define COMPAT_CONSTEXPR_20 constexpr
#else
#  define COMPAT_CONSTEXPR_20 inline
#endif

// ABI tagging and inline namespace support
#ifndef COMPAT_SELF_NS
#  define COMPAT_SELF_NS self_v1
#endif

#ifndef COMPAT_STD_NS
#  define COMPAT_STD_NS std_v1
#endif

#ifndef COMPAT_ABI_TAG
#  define COMPAT_ABI_TAG abi_v1
#endif

namespace compat {
namespace detail {

    /// <summary>
    /// C++11 向下相容之靜態常數樣板（Static Const Idiom），確保多個編譯單元取址時具有唯一實體。
    /// </summary>
    template <typename T>
    struct static_const {
        static constexpr T value{};
    };

    template <typename T>
    constexpr T static_const<T>::value;

} // namespace detail
} // namespace compat

// Inline variable support (C++17+ uses inline constexpr, C++14/11 binds to unique static_const reference)
#if (COMPAT_CPLUSPLUS >= COMPAT_CXX_17)
#  define COMPAT_INLINE_VARIABLE(type, name) inline constexpr type name{}
#  define COMPAT_INLINE_VAR inline
#else
#  define COMPAT_INLINE_VARIABLE(type, name) \
       namespace { constexpr const type& name = ::compat::detail::static_const<type>::value; }
#  define COMPAT_INLINE_VAR static
#endif

// Probe standard library version header if available
#if defined(__has_include)
#  if __has_include(<version>)
#    include <version>
#  endif
#endif

// ============================================================================
// Feature detection and Component-level switches
// ============================================================================
#if (COMPAT_FORCE_SELF_IMPLEMENTATION)

// Forced self-implementation mode: disable all native C++ standard features
#  define COMPAT_USE_STD_OPTIONAL           0
#  define COMPAT_USE_STD_STRING_VIEW        0
#  define COMPAT_USE_STD_VARIANT            0
#  define COMPAT_USE_STD_EXPECTED           0
#  define COMPAT_USE_STD_FORMAT             0
#  define COMPAT_USE_STD_PRINT              0
#  define COMPAT_USE_STD_SPAN               0
#  define COMPAT_USE_STD_SHARED_MUTEX       0
#  define COMPAT_USE_STD_SHARED_TIMED_MUTEX 0
#  define COMPAT_USE_STD_FILESYSTEM         0
#  define COMPAT_USE_STD_RANGES             0
#  define COMPAT_USE_STD_VIEWS_CONCAT       0
#  define COMPAT_USE_STD_VIEWS_CACHE_LATEST 0
#  define COMPAT_USE_STD_VIEWS_AS_CONST     0
#  define COMPAT_USE_STD_CONSTANT_RANGE     0
#  define COMPAT_USE_STD_RANGES_CONTAINS    0
#  define COMPAT_USE_STD_RANGES_STARTS_WITH 0
#  define COMPAT_USE_STD_RANGES_FOLD        0
#  define COMPAT_USE_STD_RANGES_TO          0
#  define COMPAT_USE_STD_RANGES_FIND_LAST   0
#  define COMPAT_USE_STD_RANGES_IOTA        0
#  define COMPAT_USE_STD_RANGES_SHIFT       0
#  define COMPAT_USE_STD_RANGES_GENERATE_RANDOM 0
#  define COMPAT_USE_STD_CLAMP              0
#  define COMPAT_USE_STD_PROPAGATE_CONST    0

#else

// 1. Optional (C++17+)
#  if !defined(COMPAT_FORCE_SELF_OPTIONAL) && (COMPAT_CPLUSPLUS >= COMPAT_CXX_17) && defined(__has_include)
#    if __has_include(<optional>)
#      define COMPAT_USE_STD_OPTIONAL 1
#    else
#      define COMPAT_USE_STD_OPTIONAL 0
#    endif
#  else
#    define COMPAT_USE_STD_OPTIONAL 0
#  endif

// 2. StringView (C++17+)
#  if !defined(COMPAT_FORCE_SELF_STRING_VIEW) && (COMPAT_CPLUSPLUS >= COMPAT_CXX_17) && (defined(__cpp_lib_string_view) || defined(__has_include))
#    if defined(__cpp_lib_string_view) && (__cpp_lib_string_view >= 201606L)
#      define COMPAT_USE_STD_STRING_VIEW 1
#    elif defined(__has_include) && __has_include(<string_view>)
#      define COMPAT_USE_STD_STRING_VIEW 1
#    else
#      define COMPAT_USE_STD_STRING_VIEW 0
#    endif
#  else
#    define COMPAT_USE_STD_STRING_VIEW 0
#  endif

// 3. Variant (C++17+)
#  if !defined(COMPAT_FORCE_SELF_VARIANT) && (COMPAT_CPLUSPLUS >= COMPAT_CXX_17) && (defined(__cpp_lib_variant) || defined(__has_include))
#    if defined(__cpp_lib_variant) && (__cpp_lib_variant >= 201606L)
#      define COMPAT_USE_STD_VARIANT 1
#    elif defined(__has_include) && __has_include(<variant>)
#      define COMPAT_USE_STD_VARIANT 1
#    else
#      define COMPAT_USE_STD_VARIANT 0
#    endif
#  else
#    define COMPAT_USE_STD_VARIANT 0
#  endif

// 4. Format (C++20+)
// Note: libstdc++ prior to GCC 14 does not implement P1868R2 (Unicode East Asian Width / UAX #11)
#  if !defined(COMPAT_FORCE_SELF_FORMAT) && (COMPAT_CPLUSPLUS >= COMPAT_CXX_20) && (defined(__cpp_lib_format) || defined(__has_include))
#    if defined(__cpp_lib_format) && (__cpp_lib_format >= 201907L)
#      if defined(_GLIBCXX_RELEASE) && (_GLIBCXX_RELEASE < 14)
#        define COMPAT_USE_STD_FORMAT 0
#      else
#        define COMPAT_USE_STD_FORMAT 1
#      endif
#    elif defined(__has_include) && __has_include(<format>) && (!defined(_GLIBCXX_RELEASE) || (_GLIBCXX_RELEASE >= 14))
#      define COMPAT_USE_STD_FORMAT 1
#    else
#      define COMPAT_USE_STD_FORMAT 0
#    endif
#  else
#    define COMPAT_USE_STD_FORMAT 0
#  endif

// 5. Expected (C++23+)
#  if !defined(COMPAT_FORCE_SELF_EXPECTED) && (COMPAT_CPLUSPLUS >= COMPAT_CXX_23) && (defined(__cpp_lib_expected) || defined(__has_include))
#    if defined(__cpp_lib_expected) && (__cpp_lib_expected >= 202202L)
#      define COMPAT_USE_STD_EXPECTED 1
#    elif defined(_MSC_VER) && defined(__has_include) && __has_include(<expected>)
#      define COMPAT_USE_STD_EXPECTED 1
#    else
#      define COMPAT_USE_STD_EXPECTED 0
#    endif
#  else
#    define COMPAT_USE_STD_EXPECTED 0
#  endif

// 6. Print (C++23+)
// Note: MinGW-w64 libstdc++ defines __cpp_lib_print but fails to link std::__open_terminal /
// std::__write_to_terminal on Windows (GCC Bugzilla #113834). Use SelfPrint on MinGW.
#  if !defined(COMPAT_FORCE_SELF_PRINT) && (COMPAT_CPLUSPLUS >= COMPAT_CXX_23) && (defined(__cpp_lib_print) || defined(__has_include))
#    if defined(__cpp_lib_print) && (__cpp_lib_print >= 202207L)
#      if defined(__MINGW32__) || defined(__MINGW64__)
#        define COMPAT_USE_STD_PRINT 0
#      else
#        define COMPAT_USE_STD_PRINT 1
#      endif
#    elif defined(_MSC_VER) && defined(__has_include) && __has_include(<print>)
#      define COMPAT_USE_STD_PRINT 1
#    else
#      define COMPAT_USE_STD_PRINT 0
#    endif
#  else
#    define COMPAT_USE_STD_PRINT 0
#  endif

// 7. Ranges (C++20+)
#  if !defined(COMPAT_FORCE_SELF_RANGES) && (COMPAT_CPLUSPLUS >= COMPAT_CXX_20) && (defined(__cpp_lib_ranges) || defined(__has_include))
#    if defined(__cpp_lib_ranges) && (__cpp_lib_ranges >= 201911L)
#      define COMPAT_USE_STD_RANGES 1
#    elif defined(__has_include) && __has_include(<ranges>)
#      define COMPAT_USE_STD_RANGES 1
#    else
#      define COMPAT_USE_STD_RANGES 0
#    endif
#  else
#    define COMPAT_USE_STD_RANGES 0
#  endif

// 8. Views as_const, concat, cache_latest, etc.
#  if COMPAT_USE_STD_RANGES && defined(__cpp_lib_ranges_as_const) && (__cpp_lib_ranges_as_const >= 202207L)
#    define COMPAT_USE_STD_VIEWS_AS_CONST 1
#  else
#    define COMPAT_USE_STD_VIEWS_AS_CONST 0
#  endif

#  if COMPAT_USE_STD_RANGES && defined(__cpp_lib_ranges_constant_range) && (__cpp_lib_ranges_constant_range >= 202302L)
#    define COMPAT_USE_STD_CONSTANT_RANGE 1
#  else
#    define COMPAT_USE_STD_CONSTANT_RANGE 0
#  endif

#  if COMPAT_USE_STD_RANGES && defined(__cpp_lib_ranges_concat) && (__cpp_lib_ranges_concat >= 202403L)
#    define COMPAT_USE_STD_VIEWS_CONCAT 1
#  else
#    define COMPAT_USE_STD_VIEWS_CONCAT 0
#  endif

#  if COMPAT_USE_STD_RANGES && defined(__cpp_lib_ranges_cache_latest) && (__cpp_lib_ranges_cache_latest >= 202406L)
#    define COMPAT_USE_STD_VIEWS_CACHE_LATEST 1
#  else
#    define COMPAT_USE_STD_VIEWS_CACHE_LATEST 0
#  endif

#  if COMPAT_USE_STD_RANGES && defined(__cpp_lib_ranges_contains) && (__cpp_lib_ranges_contains >= 202207L)
#    define COMPAT_USE_STD_RANGES_CONTAINS 1
#  elif defined(_MSC_VER) && (COMPAT_CPLUSPLUS >= COMPAT_CXX_23) && COMPAT_USE_STD_RANGES
#    define COMPAT_USE_STD_RANGES_CONTAINS 1
#  else
#    define COMPAT_USE_STD_RANGES_CONTAINS 0
#  endif

#  if COMPAT_USE_STD_RANGES && defined(__cpp_lib_ranges_starts_ends_with) && (__cpp_lib_ranges_starts_ends_with >= 202106L)
#    define COMPAT_USE_STD_RANGES_STARTS_WITH 1
#  elif defined(_MSC_VER) && (COMPAT_CPLUSPLUS >= COMPAT_CXX_23) && COMPAT_USE_STD_RANGES
#    define COMPAT_USE_STD_RANGES_STARTS_WITH 1
#  else
#    define COMPAT_USE_STD_RANGES_STARTS_WITH 0
#  endif

#  if COMPAT_USE_STD_RANGES && defined(__cpp_lib_ranges_fold) && (__cpp_lib_ranges_fold >= 202207L)
#    define COMPAT_USE_STD_RANGES_FOLD 1
#  elif defined(_MSC_VER) && (COMPAT_CPLUSPLUS >= COMPAT_CXX_23) && COMPAT_USE_STD_RANGES
#    define COMPAT_USE_STD_RANGES_FOLD 1
#  else
#    define COMPAT_USE_STD_RANGES_FOLD 0
#  endif

#  if COMPAT_USE_STD_RANGES && defined(__cpp_lib_ranges_to_container) && (__cpp_lib_ranges_to_container >= 202202L) && !defined(__GLIBCXX__)
#    define COMPAT_USE_STD_RANGES_TO 1
#  elif defined(_MSC_VER) && (COMPAT_CPLUSPLUS >= COMPAT_CXX_23) && COMPAT_USE_STD_RANGES
#    define COMPAT_USE_STD_RANGES_TO 1
#  else
#    define COMPAT_USE_STD_RANGES_TO 0
#  endif

#  if COMPAT_USE_STD_RANGES && defined(__cpp_lib_ranges_find_last) && (__cpp_lib_ranges_find_last >= 202207L)
#    define COMPAT_USE_STD_RANGES_FIND_LAST 1
#  elif defined(_MSC_VER) && (COMPAT_CPLUSPLUS >= COMPAT_CXX_23) && COMPAT_USE_STD_RANGES
#    define COMPAT_USE_STD_RANGES_FIND_LAST 1
#  else
#    define COMPAT_USE_STD_RANGES_FIND_LAST 0
#  endif

#  if COMPAT_USE_STD_RANGES && defined(__cpp_lib_ranges_iota) && (__cpp_lib_ranges_iota >= 202202L)
#    define COMPAT_USE_STD_RANGES_IOTA 1
#  elif defined(_MSC_VER) && (COMPAT_CPLUSPLUS >= COMPAT_CXX_23) && COMPAT_USE_STD_RANGES
#    define COMPAT_USE_STD_RANGES_IOTA 1
#  else
#    define COMPAT_USE_STD_RANGES_IOTA 0
#  endif

#  if COMPAT_USE_STD_RANGES && defined(__cpp_lib_ranges_shift) && (__cpp_lib_ranges_shift >= 202202L)
#    define COMPAT_USE_STD_RANGES_SHIFT 1
#  elif defined(_MSC_VER) && (COMPAT_CPLUSPLUS >= COMPAT_CXX_23) && COMPAT_USE_STD_RANGES
#    define COMPAT_USE_STD_RANGES_SHIFT 1
#  else
#    define COMPAT_USE_STD_RANGES_SHIFT 0
#  endif

#  if COMPAT_USE_STD_RANGES && defined(__cpp_lib_ranges_generate_random) && (__cpp_lib_ranges_generate_random >= 202403L)
#    define COMPAT_USE_STD_RANGES_GENERATE_RANDOM 1
#  else
#    define COMPAT_USE_STD_RANGES_GENERATE_RANDOM 0
#  endif

// 9. Filesystem (C++17+)
#  if !defined(COMPAT_FORCE_SELF_FILESYSTEM) && (COMPAT_CPLUSPLUS >= COMPAT_CXX_17) && (defined(__cpp_lib_filesystem) || defined(__has_include))
#    if defined(__cpp_lib_filesystem) && (__cpp_lib_filesystem >= 201703L)
#      define COMPAT_USE_STD_FILESYSTEM 1
#    elif defined(__has_include) && __has_include(<filesystem>)
#      define COMPAT_USE_STD_FILESYSTEM 1
#    else
#      define COMPAT_USE_STD_FILESYSTEM 0
#    endif
#  else
#    define COMPAT_USE_STD_FILESYSTEM 0
#  endif

// 10. Clamp (C++17+)
#  if (COMPAT_CPLUSPLUS >= COMPAT_CXX_17) && (defined(__cpp_lib_clamp) || defined(__has_include))
#    define COMPAT_USE_STD_CLAMP 1
#  else
#    define COMPAT_USE_STD_CLAMP 0
#  endif

// 11. Span (C++20+)
#  if !defined(COMPAT_FORCE_SELF_SPAN) && (COMPAT_CPLUSPLUS >= COMPAT_CXX_20) && (defined(__cpp_lib_span) || defined(__has_include))
#    if defined(__cpp_lib_span) && (__cpp_lib_span >= 202002L)
#      define COMPAT_USE_STD_SPAN 1
#    elif defined(__has_include) && __has_include(<span>)
#      define COMPAT_USE_STD_SPAN 1
#    else
#      define COMPAT_USE_STD_SPAN 0
#    endif
#  else
#    define COMPAT_USE_STD_SPAN 0
#  endif

// 12. PropagateConst (TS v2/v3)
#  if defined(COMPAT_FORCE_STD_IMPLEMENTATION) && defined(__has_include)
#    if __has_include(<experimental/propagate_const>) && (COMPAT_CPLUSPLUS >= COMPAT_CXX_14)
#      define COMPAT_USE_STD_PROPAGATE_CONST 1
#    else
#      define COMPAT_USE_STD_PROPAGATE_CONST 0
#    endif
#  else
#    define COMPAT_USE_STD_PROPAGATE_CONST 0
#  endif

// 13. SharedMutex (C++17+) & SharedTimedMutex (C++14+)
#  if !defined(COMPAT_FORCE_SELF_SHARED_MUTEX) && (COMPAT_CPLUSPLUS >= COMPAT_CXX_17) && (defined(__cpp_lib_shared_mutex) || defined(__has_include))
#    if defined(__cpp_lib_shared_mutex) && (__cpp_lib_shared_mutex >= 201505L)
#      define COMPAT_USE_STD_SHARED_MUTEX 1
#    elif (COMPAT_CPLUSPLUS >= COMPAT_CXX_17)
#      define COMPAT_USE_STD_SHARED_MUTEX 1
#    elif defined(_MSC_VER) && (COMPAT_CPLUSPLUS >= COMPAT_CXX_14)
#      define COMPAT_USE_STD_SHARED_MUTEX 1
#    else
#      define COMPAT_USE_STD_SHARED_MUTEX 0
#    endif
#  else
#    define COMPAT_USE_STD_SHARED_MUTEX 0
#  endif

#  if !defined(COMPAT_FORCE_SELF_SHARED_MUTEX) && (COMPAT_CPLUSPLUS >= COMPAT_CXX_14) && (defined(__cpp_lib_shared_timed_mutex) || defined(__has_include))
#    if defined(__cpp_lib_shared_timed_mutex) && (__cpp_lib_shared_timed_mutex >= 201402L)
#      define COMPAT_USE_STD_SHARED_TIMED_MUTEX 1
#    elif (COMPAT_CPLUSPLUS >= COMPAT_CXX_14)
#      define COMPAT_USE_STD_SHARED_TIMED_MUTEX 1
#    else
#      define COMPAT_USE_STD_SHARED_TIMED_MUTEX 0
#    endif
#  else
#    define COMPAT_USE_STD_SHARED_TIMED_MUTEX 0
#  endif

#endif // !COMPAT_FORCE_SELF_IMPLEMENTATION

// Backwards-compatible aliases from COMPAT_USE_STD_* to COMPAT_HAS_STD_*
#define COMPAT_HAS_STD_OPTIONAL               COMPAT_USE_STD_OPTIONAL
#define COMPAT_HAS_STD_STRING_VIEW            COMPAT_USE_STD_STRING_VIEW
#define COMPAT_HAS_STD_VARIANT                COMPAT_USE_STD_VARIANT
#define COMPAT_HAS_STD_EXPECTED               COMPAT_USE_STD_EXPECTED
#define COMPAT_HAS_STD_FORMAT                 COMPAT_USE_STD_FORMAT
#define COMPAT_HAS_STD_PRINT                  COMPAT_USE_STD_PRINT
#define COMPAT_HAS_STD_SPAN                   COMPAT_USE_STD_SPAN
#define COMPAT_HAS_STD_SHARED_MUTEX           COMPAT_USE_STD_SHARED_MUTEX
#define COMPAT_HAS_STD_SHARED_TIMED_MUTEX     COMPAT_USE_STD_SHARED_TIMED_MUTEX
#define COMPAT_HAS_STD_FILESYSTEM             COMPAT_USE_STD_FILESYSTEM
#define COMPAT_HAS_STD_RANGES                 COMPAT_USE_STD_RANGES
#define COMPAT_HAS_STD_VIEWS_CONCAT           COMPAT_USE_STD_VIEWS_CONCAT
#define COMPAT_HAS_STD_VIEWS_CACHE_LATEST     COMPAT_USE_STD_VIEWS_CACHE_LATEST
#define COMPAT_HAS_STD_VIEWS_AS_CONST         COMPAT_USE_STD_VIEWS_AS_CONST
#define COMPAT_HAS_STD_CONSTANT_RANGE         COMPAT_USE_STD_CONSTANT_RANGE
#define COMPAT_HAS_STD_RANGES_CONTAINS        COMPAT_USE_STD_RANGES_CONTAINS
#define COMPAT_HAS_STD_RANGES_STARTS_WITH     COMPAT_USE_STD_RANGES_STARTS_WITH
#define COMPAT_HAS_STD_RANGES_FOLD            COMPAT_USE_STD_RANGES_FOLD
#define COMPAT_HAS_STD_RANGES_TO              COMPAT_USE_STD_RANGES_TO
#define COMPAT_HAS_STD_RANGES_FIND_LAST       COMPAT_USE_STD_RANGES_FIND_LAST
#define COMPAT_HAS_STD_RANGES_IOTA            COMPAT_USE_STD_RANGES_IOTA
#define COMPAT_HAS_STD_RANGES_SHIFT           COMPAT_USE_STD_RANGES_SHIFT
#define COMPAT_HAS_STD_RANGES_GENERATE_RANDOM COMPAT_USE_STD_RANGES_GENERATE_RANDOM
#define COMPAT_HAS_STD_CLAMP                  COMPAT_USE_STD_CLAMP
#define COMPAT_HAS_STD_PROPAGATE_CONST        COMPAT_USE_STD_PROPAGATE_CONST

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

