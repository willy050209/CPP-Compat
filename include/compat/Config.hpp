#pragma once

// Feature detection and configuration header for compat library.

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

// Probe standard library version header if available
#if defined(__has_include)
#  if __has_include(<version>)
#    include <version>
#  endif
#endif

#if defined(COMPAT_FORCE_FALLBACK)

// Forced fallback mode: disable all native C++ standard features
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

#endif // !defined(COMPAT_FORCE_FALLBACK)

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
