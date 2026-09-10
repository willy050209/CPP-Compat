#pragma once

// Feature detection and configuration header for compat library.

#include <cstdlib>

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

// Constexpr support for C++14+
#if (COMPAT_CPLUSPLUS >= COMPAT_CXX_14)
#  define COMPAT_CONSTEXPR_14 constexpr
#else
#  define COMPAT_CONSTEXPR_14 inline
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
#  define COMPAT_HAS_STD_EXPECTED    0
#  define COMPAT_HAS_STD_PRINT       0
#  define COMPAT_HAS_STD_FORMAT      0
#  define COMPAT_HAS_STD_STRING_VIEW 0
#  define COMPAT_HAS_STD_VARIANT     0

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
#  elif (COMPAT_CPLUSPLUS >= COMPAT_CXX_23) && defined(__has_include)
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
#  elif (COMPAT_CPLUSPLUS >= COMPAT_CXX_23) && defined(__has_include)
#    if __has_include(<print>)
#      define COMPAT_HAS_STD_PRINT 1
#    else
#      define COMPAT_HAS_STD_PRINT 0
#    endif
#  else
#    define COMPAT_HAS_STD_PRINT 0
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

} // namespace detail
} // namespace compat

