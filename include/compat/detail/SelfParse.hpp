#pragma once

#include "../Config.hpp"
#include "../StringView.hpp"
#include "../Expected.hpp"
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <type_traits>
#include <system_error>

namespace compat {
namespace detail {
inline namespace COMPAT_ABI_TAG {

/// <summary>
/// Result structure for from_chars operations indicating end of match and error code.
/// </summary>
struct from_chars_result {
    const char* ptr;
    std::errc ec;

    /// <summary>
    /// Explicit boolean conversion indicating whether parsing succeeded without error.
    /// </summary>
    /// <returns>True if ec is empty (success), false otherwise.</returns>
    constexpr explicit operator bool() const noexcept {
        return ec == std::errc{};
    }
};

/// <summary>
/// Parses an unsigned integer from a character buffer without heap allocations or locale dependency.
/// </summary>
/// <typeparam name="UIntType">Unsigned integral destination type.</typeparam>
/// <param name="first">Pointer to beginning of character range.</param>
/// <param name="last">Pointer past end of character range.</param>
/// <param name="value">Output reference to receive parsed value.</param>
/// <param name="base">Radix base between 2 and 36 inclusive.</param>
/// <returns>from_chars_result with updated pointer and error code.</returns>
template <typename UIntType>
inline from_chars_result from_chars_unsigned(const char* first, const char* last, UIntType& value, int32_t base = 10) noexcept {
    from_chars_result res{first, std::errc{}};
    if (COMPAT_UNLIKELY(base < 2 || base > 36 || first >= last)) {
        res.ec = std::errc::invalid_argument;
        return res;
    }

    const uint64_t max_val = static_cast<uint64_t>(std::numeric_limits<UIntType>::max());
    const uint64_t ubase = static_cast<uint64_t>(base);
    const uint64_t limit_div = max_val / ubase;
    const uint64_t limit_mod = max_val % ubase;

    const char* ptr = first;
    uint64_t result = 0;
    bool overflow = false;
    bool has_digits = false;

    if (COMPAT_LIKELY(base == 10)) {
        while (ptr < last) {
            uint8_t c = static_cast<uint8_t>(*ptr);
            uint8_t digit = static_cast<uint8_t>(c - '0');
            if (digit > 9) {
                break;
            }
            has_digits = true;
            if (COMPAT_LIKELY(!overflow)) {
                if (COMPAT_UNLIKELY(result > limit_div || (result == limit_div && digit > limit_mod))) {
                    overflow = true;
                } else {
                    result = result * 10 + digit;
                }
            }
            ++ptr;
        }
    } else {
        while (ptr < last) {
            char c = *ptr;
            int32_t digit = -1;
            if (c >= '0' && c <= '9') {
                digit = c - '0';
            } else if (c >= 'a' && c <= 'z') {
                digit = c - 'a' + 10;
            } else if (c >= 'A' && c <= 'Z') {
                digit = c - 'A' + 10;
            }

            if (digit < 0 || digit >= base) {
                break;
            }

            has_digits = true;
            uint64_t udigit = static_cast<uint64_t>(digit);
            if (COMPAT_LIKELY(!overflow)) {
                if (COMPAT_UNLIKELY(result > limit_div || (result == limit_div && udigit > limit_mod))) {
                    overflow = true;
                } else {
                    result = result * ubase + udigit;
                }
            }
            ++ptr;
        }
    }

    if (COMPAT_UNLIKELY(!has_digits)) {
        res.ec = std::errc::invalid_argument;
        res.ptr = first;
        return res;
    }

    if (COMPAT_UNLIKELY(overflow)) {
        res.ec = std::errc::result_out_of_range;
        res.ptr = ptr;
        return res;
    }

    value = static_cast<UIntType>(result);
    res.ptr = ptr;
    res.ec = std::errc{};
    return res;
}

/// <summary>
/// Parses a signed integer from a character buffer without heap allocations or locale dependency.
/// </summary>
/// <typeparam name="IntType">Signed integral destination type.</typeparam>
/// <param name="first">Pointer to beginning of character range.</param>
/// <param name="last">Pointer past end of character range.</param>
/// <param name="value">Output reference to receive parsed value.</param>
/// <param name="base">Radix base between 2 and 36 inclusive.</param>
/// <returns>from_chars_result with updated pointer and error code.</returns>
template <typename IntType>
inline from_chars_result from_chars_signed(const char* first, const char* last, IntType& value, int32_t base = 10) noexcept {
    from_chars_result res{first, std::errc{}};
    if (COMPAT_UNLIKELY(base < 2 || base > 36 || first >= last)) {
        res.ec = std::errc::invalid_argument;
        return res;
    }

    const char* ptr = first;
    bool negative = false;
    if (*ptr == '-') {
        negative = true;
        ++ptr;
        if (COMPAT_UNLIKELY(ptr >= last)) {
            res.ec = std::errc::invalid_argument;
            res.ptr = first;
            return res;
        }
    }

    const uint64_t max_mag = negative
        ? (static_cast<uint64_t>(0) - static_cast<uint64_t>(std::numeric_limits<IntType>::min()))
        : static_cast<uint64_t>(std::numeric_limits<IntType>::max());

    const uint64_t ubase = static_cast<uint64_t>(base);
    const uint64_t limit_div = max_mag / ubase;
    const uint64_t limit_mod = max_mag % ubase;

    uint64_t result = 0;
    bool overflow = false;
    bool has_digits = false;

    if (COMPAT_LIKELY(base == 10)) {
        while (ptr < last) {
            uint8_t c = static_cast<uint8_t>(*ptr);
            uint8_t digit = static_cast<uint8_t>(c - '0');
            if (digit > 9) {
                break;
            }
            has_digits = true;
            if (COMPAT_LIKELY(!overflow)) {
                if (COMPAT_UNLIKELY(result > limit_div || (result == limit_div && digit > limit_mod))) {
                    overflow = true;
                } else {
                    result = result * 10 + digit;
                }
            }
            ++ptr;
        }
    } else {
        while (ptr < last) {
            char c = *ptr;
            int32_t digit = -1;
            if (c >= '0' && c <= '9') {
                digit = c - '0';
            } else if (c >= 'a' && c <= 'z') {
                digit = c - 'a' + 10;
            } else if (c >= 'A' && c <= 'Z') {
                digit = c - 'A' + 10;
            }

            if (digit < 0 || digit >= base) {
                break;
            }

            has_digits = true;
            uint64_t udigit = static_cast<uint64_t>(digit);
            if (COMPAT_LIKELY(!overflow)) {
                if (COMPAT_UNLIKELY(result > limit_div || (result == limit_div && udigit > limit_mod))) {
                    overflow = true;
                } else {
                    result = result * ubase + udigit;
                }
            }
            ++ptr;
        }
    }

    if (COMPAT_UNLIKELY(!has_digits)) {
        res.ec = std::errc::invalid_argument;
        res.ptr = first;
        return res;
    }

    if (COMPAT_UNLIKELY(overflow)) {
        res.ec = std::errc::result_out_of_range;
        res.ptr = ptr;
        return res;
    }

    if (negative) {
        if (result == max_mag) {
            value = std::numeric_limits<IntType>::min();
        } else {
            value = static_cast<IntType>(-static_cast<int64_t>(result));
        }
    } else {
        value = static_cast<IntType>(result);
    }

    res.ptr = ptr;
    res.ec = std::errc{};
    return res;
}

/// <summary>
/// Computes positive powers of 10 up to 10^308 without external dependencies.
/// </summary>
/// <param name="exp">Non-negative exponent value.</param>
/// <returns>10^exp as a double.</returns>
inline double Pow10Positive(int32_t exp) noexcept {
    static const double kPow10[] = {
        1e1, 1e2, 1e4, 1e8, 1e16, 1e32, 1e64, 1e128, 1e256
    };
    double r = 1.0;
    for (int32_t i = 0; i < 9; ++i) {
        if (exp & (1 << i)) {
            r *= kPow10[i];
        }
    }
    return r;
}

/// <summary>
/// Case-insensitive character prefix comparison helper.
/// </summary>
/// <param name="a">Input character sequence.</param>
/// <param name="b">Target lower-case ASCII literal.</param>
/// <param name="len">Length of sequence to compare.</param>
/// <returns>True if characters match case-insensitively.</returns>
inline bool CaseInsensitiveEqual(const char* a, const char* b, std::size_t len) noexcept {
    for (std::size_t i = 0; i < len; ++i) {
        char c1 = a[i];
        char c2 = b[i];
        if (c1 >= 'A' && c1 <= 'Z') c1 = static_cast<char>(c1 + ('a' - 'A'));
        if (c2 >= 'A' && c2 <= 'Z') c2 = static_cast<char>(c2 + ('a' - 'A'));
        if (c1 != c2) return false;
    }
    return true;
}

/// <summary>
/// Parses a floating-point number from a character buffer without heap allocations or locale dependency.
/// </summary>
/// <typeparam name="FloatType">Floating-point destination type (float or double).</typeparam>
/// <param name="first">Pointer to beginning of character range.</param>
/// <param name="last">Pointer past end of character range.</param>
/// <param name="value">Output reference to receive parsed value.</param>
/// <returns>from_chars_result with updated pointer and error code.</returns>
template <typename FloatType>
inline from_chars_result from_chars_float(const char* first, const char* last, FloatType& value) noexcept {
    from_chars_result res{first, std::errc{}};
    if (COMPAT_UNLIKELY(first >= last)) {
        res.ec = std::errc::invalid_argument;
        return res;
    }

    const char* ptr = first;
    bool negative = false;
    if (ptr < last && (*ptr == '-' || *ptr == '+')) {
        negative = (*ptr == '-');
        ++ptr;
    }

    std::size_t rem = static_cast<std::size_t>(last - ptr);
    if (rem >= 8 && CaseInsensitiveEqual(ptr, "infinity", 8)) {
        ptr += 8;
        FloatType inf_val = std::numeric_limits<FloatType>::infinity();
        value = negative ? -inf_val : inf_val;
        res.ptr = ptr;
        res.ec = std::errc{};
        return res;
    }
    if (rem >= 3 && CaseInsensitiveEqual(ptr, "inf", 3)) {
        ptr += 3;
        FloatType inf_val = std::numeric_limits<FloatType>::infinity();
        value = negative ? -inf_val : inf_val;
        res.ptr = ptr;
        res.ec = std::errc{};
        return res;
    }
    if (rem >= 3 && CaseInsensitiveEqual(ptr, "nan", 3)) {
        ptr += 3;
        FloatType nan_val = std::numeric_limits<FloatType>::quiet_NaN();
        value = negative ? -nan_val : nan_val;
        res.ptr = ptr;
        res.ec = std::errc{};
        return res;
    }

    static const double kPow10Fast[23] = {
        1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9,
        1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19,
        1e20, 1e21, 1e22
    };

    double mantissa = 0.0;
    int32_t frac_digits = 0;
    int32_t extra_exp = 0;
    bool has_digits = false;

    while (ptr < last) {
        uint8_t digit = static_cast<uint8_t>(*ptr - '0');
        if (digit > 9) break;
        has_digits = true;
        if (mantissa < 1e16) {
            mantissa = mantissa * 10.0 + digit;
        } else {
            ++extra_exp;
        }
        ++ptr;
    }

    if (ptr < last && *ptr == '.') {
        ++ptr;
        while (ptr < last) {
            uint8_t digit = static_cast<uint8_t>(*ptr - '0');
            if (digit > 9) break;
            has_digits = true;
            if (mantissa < 1e16) {
                mantissa = mantissa * 10.0 + digit;
                ++frac_digits;
            }
            ++ptr;
        }
    }

    if (COMPAT_UNLIKELY(!has_digits)) {
        res.ec = std::errc::invalid_argument;
        res.ptr = first;
        return res;
    }

    int32_t exp_val = 0;
    if (ptr < last && (*ptr == 'e' || *ptr == 'E')) {
        const char* exp_start = ptr;
        const char* p = ptr + 1;
        bool exp_neg = false;
        if (p < last && (*p == '-' || *p == '+')) {
            exp_neg = (*p == '-');
            ++p;
        }
        if (p < last && *p >= '0' && *p <= '9') {
            while (p < last && *p >= '0' && *p <= '9') {
                if (exp_val < 10000) {
                    exp_val = exp_val * 10 + (*p - '0');
                }
                ++p;
            }
            if (exp_neg) {
                exp_val = -exp_val;
            }
            ptr = p;
        } else {
            ptr = exp_start;
        }
    }

    int32_t total_exp = exp_val + extra_exp - frac_digits;

    if (COMPAT_UNLIKELY(total_exp > 308 && mantissa != 0.0)) {
        res.ec = std::errc::result_out_of_range;
        res.ptr = ptr;
        return res;
    }

    double dval = mantissa;
    if (total_exp >= 0 && total_exp <= 22) {
        dval *= kPow10Fast[total_exp];
    } else if (total_exp < 0 && total_exp >= -22) {
        dval /= kPow10Fast[-total_exp];
    } else if (total_exp > 22) {
        dval *= Pow10Positive(total_exp);
    } else {
        int32_t neg_exp = -total_exp;
        if (neg_exp > 324) {
            dval = 0.0;
        } else if (neg_exp > 256) {
            dval /= 1e256;
            dval /= Pow10Positive(neg_exp - 256);
        } else {
            dval /= Pow10Positive(neg_exp);
        }
    }

    // Overflow check for double
    if (COMPAT_UNLIKELY(dval > 1.7976931348623157e+308 || dval < -1.7976931348623157e+308)) {
        res.ec = std::errc::result_out_of_range;
        res.ptr = ptr;
        return res;
    }

    // Overflow check for float
    if (std::is_same<FloatType, float>::value) {
        if (COMPAT_UNLIKELY(dval > 3.4028234663852886e+38 || dval < -3.4028234663852886e+38)) {
            res.ec = std::errc::result_out_of_range;
            res.ptr = ptr;
            return res;
        }
    }

    // Underflow check: if mantissa was non-zero but scaled value underflows to 0.0
    if (COMPAT_UNLIKELY(mantissa != 0.0 && static_cast<FloatType>(dval) == static_cast<FloatType>(0.0))) {
        res.ec = std::errc::result_out_of_range;
        res.ptr = ptr;
        return res;
    }

    if (negative) {
        dval = -dval;
    }

    value = static_cast<FloatType>(dval);
    res.ptr = ptr;
    res.ec = std::errc{};
    return res;
}

/// <summary>
/// Parses an unsigned integer from a string_view with boundary checking, delegating to from_chars_unsigned.
/// </summary>
/// <typeparam name="UIntType">Unsigned integer destination type.</typeparam>
/// <param name="str">Input string view.</param>
/// <returns>expected containing parsed value or error description string_view.</returns>
template <typename UIntType>
inline compat::expected<UIntType, compat::string_view> ParseUnsigned(compat::string_view str) noexcept {
    if (COMPAT_UNLIKELY(str.empty())) {
        return compat::unexpected<compat::string_view>("Empty input string");
    }
    if (COMPAT_UNLIKELY(str[0] == '-')) {
        return compat::unexpected<compat::string_view>("Negative sign in unsigned integer");
    }
    const char* first = str.data();
    const char* last = str.data() + str.size();
    if (COMPAT_UNLIKELY(*first == '+')) {
        ++first;
        if (COMPAT_UNLIKELY(first == last)) {
            return compat::unexpected<compat::string_view>("Sign without digits");
        }
    }
    UIntType value{};
    from_chars_result res = from_chars_unsigned(first, last, value, 10);
    if (COMPAT_UNLIKELY(res.ec == std::errc::invalid_argument)) {
        return compat::unexpected<compat::string_view>("Invalid character in integer");
    }
    if (COMPAT_UNLIKELY(res.ec == std::errc::result_out_of_range)) {
        return compat::unexpected<compat::string_view>("Integer overflow");
    }
    if (COMPAT_UNLIKELY(res.ptr != last)) {
        return compat::unexpected<compat::string_view>("Invalid character in integer");
    }
    return value;
}

/// <summary>
/// Parses a signed integer from a string_view with boundary checking, delegating to from_chars_signed.
/// </summary>
/// <typeparam name="IntType">Signed integer destination type.</typeparam>
/// <param name="str">Input string view.</param>
/// <returns>expected containing parsed value or error description string_view.</returns>
template <typename IntType>
inline compat::expected<IntType, compat::string_view> ParseSigned(compat::string_view str) noexcept {
    if (COMPAT_UNLIKELY(str.empty())) {
        return compat::unexpected<compat::string_view>("Empty input string");
    }
    const char* first = str.data();
    const char* last = str.data() + str.size();
    if (COMPAT_UNLIKELY(*first == '+')) {
        ++first;
        if (COMPAT_UNLIKELY(first == last)) {
            return compat::unexpected<compat::string_view>("Sign without digits");
        }
    }
    IntType value{};
    from_chars_result res = from_chars_signed(first, last, value, 10);
    if (COMPAT_UNLIKELY(res.ec == std::errc::invalid_argument)) {
        return compat::unexpected<compat::string_view>("Invalid character in integer");
    }
    if (COMPAT_UNLIKELY(res.ec == std::errc::result_out_of_range)) {
        return compat::unexpected<compat::string_view>("Integer overflow");
    }
    if (COMPAT_UNLIKELY(res.ptr != last)) {
        return compat::unexpected<compat::string_view>("Invalid character in integer");
    }
    return value;
}

/// <summary>
/// Parses a floating-point number from a string_view with boundary checking, delegating to from_chars_float.
/// </summary>
/// <typeparam name="FloatType">Floating-point destination type.</typeparam>
/// <param name="str">Input string view.</param>
/// <returns>expected containing parsed value or error description string_view.</returns>
template <typename FloatType>
inline compat::expected<FloatType, compat::string_view> ParseFloating(compat::string_view str) noexcept {
    if (COMPAT_UNLIKELY(str.empty())) {
        return compat::unexpected<compat::string_view>("Empty input string");
    }
    const char* first = str.data();
    const char* last = str.data() + str.size();
    FloatType value{};
    from_chars_result res = from_chars_float(first, last, value);
    if (COMPAT_UNLIKELY(res.ec == std::errc::invalid_argument)) {
        return compat::unexpected<compat::string_view>("Invalid character in floating point number");
    }
    if (COMPAT_UNLIKELY(res.ec == std::errc::result_out_of_range)) {
        return compat::unexpected<compat::string_view>("Floating point overflow");
    }
    if (COMPAT_UNLIKELY(res.ptr != last)) {
        return compat::unexpected<compat::string_view>("Invalid character in floating point number");
    }
    return value;
}

/// <summary>
/// Primary parser template structure with SFINAE support.
/// </summary>
template <typename T, typename Enable = void>
struct Parser;

template <typename T>
struct Parser<T, typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value && !std::is_same<T, bool>::value>::type> {
    static compat::expected<T, compat::string_view> parse(compat::string_view str) noexcept {
        return ParseSigned<T>(str);
    }
};

template <typename T>
struct Parser<T, typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value && !std::is_same<T, bool>::value>::type> {
    static compat::expected<T, compat::string_view> parse(compat::string_view str) noexcept {
        return ParseUnsigned<T>(str);
    }
};

template <typename T>
struct Parser<T, typename std::enable_if<std::is_floating_point<T>::value>::type> {
    static compat::expected<T, compat::string_view> parse(compat::string_view str) noexcept {
        return ParseFloating<T>(str);
    }
};

template <>
struct Parser<bool> {
    static compat::expected<bool, compat::string_view> parse(compat::string_view str) noexcept {
        if (COMPAT_UNLIKELY(str.empty())) {
            return compat::unexpected<compat::string_view>("Empty input string");
        }
        if (str == "true" || str == "True" || str == "1") {
            return true;
        }
        if (str == "false" || str == "False" || str == "0") {
            return false;
        }
        return compat::unexpected<compat::string_view>("Invalid boolean value");
    }
};

template <>
struct Parser<std::string> {
    static compat::expected<std::string, compat::string_view> parse(compat::string_view str) noexcept {
        if (COMPAT_UNLIKELY(str.empty())) {
            return compat::unexpected<compat::string_view>("Empty input string");
        }
        return std::string(str.data(), str.size());
    }
};

template <>
struct Parser<compat::string_view> {
    static compat::expected<compat::string_view, compat::string_view> parse(compat::string_view str) noexcept {
        if (COMPAT_UNLIKELY(str.empty())) {
            return compat::unexpected<compat::string_view>("Empty input string");
        }
        return str;
    }
};

} // namespace COMPAT_ABI_TAG
} // namespace detail
} // namespace compat
