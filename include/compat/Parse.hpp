#pragma once

#include "Config.hpp"
#include "StringView.hpp"
#include "Expected.hpp"
#include "detail/SelfParse.hpp"
#include <system_error>
#include <type_traits>

namespace compat {

using from_chars_result = detail::from_chars_result;

/// <summary>
/// Parses an unsigned integer from a character buffer with specified radix base.
/// </summary>
/// <typeparam name="IntType">Unsigned integer destination type.</typeparam>
/// <param name="first">Pointer to start of character sequence.</param>
/// <param name="last">Pointer past end of character sequence.</param>
/// <param name="value">Output reference for parsed value.</param>
/// <param name="base">Radix base between 2 and 36 inclusive.</param>
/// <returns>from_chars_result with position and error state.</returns>
template <typename IntType,
          typename std::enable_if<std::is_integral<IntType>::value && std::is_unsigned<IntType>::value && !std::is_same<IntType, bool>::value, int32_t>::type = 0>
inline from_chars_result from_chars(const char* first, const char* last, IntType& value, int32_t base = 10) noexcept {
    return detail::from_chars_unsigned(first, last, value, base);
}

/// <summary>
/// Parses a signed integer from a character buffer with specified radix base.
/// </summary>
/// <typeparam name="IntType">Signed integer destination type.</typeparam>
/// <param name="first">Pointer to start of character sequence.</param>
/// <param name="last">Pointer past end of character sequence.</param>
/// <param name="value">Output reference for parsed value.</param>
/// <param name="base">Radix base between 2 and 36 inclusive.</param>
/// <returns>from_chars_result with position and error state.</returns>
template <typename IntType,
          typename std::enable_if<std::is_integral<IntType>::value && std::is_signed<IntType>::value && !std::is_same<IntType, bool>::value, int32_t>::type = 0>
inline from_chars_result from_chars(const char* first, const char* last, IntType& value, int32_t base = 10) noexcept {
    return detail::from_chars_signed(first, last, value, base);
}

/// <summary>
/// Parses a floating-point number from a character buffer without allocations or locale dependency.
/// </summary>
/// <typeparam name="FloatType">Floating-point destination type (float or double).</typeparam>
/// <param name="first">Pointer to start of character sequence.</param>
/// <param name="last">Pointer past end of character sequence.</param>
/// <param name="value">Output reference for parsed value.</param>
/// <returns>from_chars_result with position and error state.</returns>
template <typename FloatType,
          typename std::enable_if<std::is_floating_point<FloatType>::value, int32_t>::type = 0>
inline from_chars_result from_chars(const char* first, const char* last, FloatType& value) noexcept {
    return detail::from_chars_float(first, last, value);
}

/// <summary>
/// Parses a string_view into the specified type T with fail-fast validation.
/// Fails fast on invalid characters, empty string, or boundary overflow without throwing exceptions.
/// </summary>
/// <typeparam name="T">Target type to parse into.</typeparam>
/// <param name="str">Input string view.</param>
/// <returns>expected containing parsed value on success, or unexpected error description.</returns>
template <typename T>
COMPAT_NODISCARD inline compat::expected<T, compat::string_view> parse(compat::string_view str) noexcept {
    return detail::Parser<T>::parse(str);
}

} // namespace compat
