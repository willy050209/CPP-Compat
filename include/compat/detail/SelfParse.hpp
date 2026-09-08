#pragma once

#include "../StringView.hpp"
#include "../Expected.hpp"
#include <cstddef>
#include <cstdint>
#include <limits>
#include <cmath>
#include <string>
#include <type_traits>

namespace compat {
namespace detail {

/// <summary>
/// Parses an unsigned integer from a string_view with boundary checking.
/// </summary>
template <typename UIntType>
inline compat::expected<UIntType, compat::string_view> ParseUnsigned(compat::string_view str) noexcept {
    if (str.empty()) {
        return compat::unexpected<compat::string_view>("Empty input string");
    }
    std::size_t i = 0;
    if (str[0] == '+') {
        i = 1;
        if (i >= str.size()) {
            return compat::unexpected<compat::string_view>("Sign without digits");
        }
    } else if (str[0] == '-') {
        return compat::unexpected<compat::string_view>("Negative sign in unsigned integer");
    }

    uint64_t result = 0;
    const uint64_t max_val = static_cast<uint64_t>(std::numeric_limits<UIntType>::max());
    const uint64_t limit_div_10 = max_val / 10;
    const uint64_t limit_mod_10 = max_val % 10;

    for (; i < str.size(); ++i) {
        char c = str[i];
        if (c < '0' || c > '9') {
            return compat::unexpected<compat::string_view>("Invalid character in integer");
        }
        uint64_t digit = static_cast<uint64_t>(c - '0');
        if (result > limit_div_10 || (result == limit_div_10 && digit > limit_mod_10)) {
            return compat::unexpected<compat::string_view>("Integer overflow");
        }
        result = result * 10 + digit;
    }
    return static_cast<UIntType>(result);
}

/// <summary>
/// Parses a signed integer from a string_view with boundary checking.
/// </summary>
template <typename IntType>
inline compat::expected<IntType, compat::string_view> ParseSigned(compat::string_view str) noexcept {
    if (str.empty()) {
        return compat::unexpected<compat::string_view>("Empty input string");
    }
    std::size_t i = 0;
    bool negative = false;
    if (str[0] == '-') {
        negative = true;
        i = 1;
    } else if (str[0] == '+') {
        i = 1;
    }
    if (i >= str.size()) {
        return compat::unexpected<compat::string_view>("Sign without digits");
    }

    const uint64_t max_magnitude = negative
        ? (static_cast<uint64_t>(0) - static_cast<uint64_t>(std::numeric_limits<IntType>::min()))
        : static_cast<uint64_t>(std::numeric_limits<IntType>::max());

    const uint64_t limit_div_10 = max_magnitude / 10;
    const uint64_t limit_mod_10 = max_magnitude % 10;

    uint64_t result = 0;
    for (; i < str.size(); ++i) {
        char c = str[i];
        if (c < '0' || c > '9') {
            return compat::unexpected<compat::string_view>("Invalid character in integer");
        }
        uint64_t digit = static_cast<uint64_t>(c - '0');
        if (result > limit_div_10 || (result == limit_div_10 && digit > limit_mod_10)) {
            return compat::unexpected<compat::string_view>("Integer overflow");
        }
        result = result * 10 + digit;
    }

    if (negative) {
        if (result == max_magnitude) {
            return std::numeric_limits<IntType>::min();
        }
        return static_cast<IntType>(-static_cast<int64_t>(result));
    }
    return static_cast<IntType>(result);
}

/// <summary>
/// Parses a floating-point number from a string_view with boundary checking.
/// </summary>
template <typename FloatType>
inline compat::expected<FloatType, compat::string_view> ParseFloating(compat::string_view str) noexcept {
    if (str.empty()) {
        return compat::unexpected<compat::string_view>("Empty input string");
    }
    std::size_t i = 0;
    bool negative = false;
    if (str[0] == '-') {
        negative = true;
        i = 1;
    } else if (str[0] == '+') {
        i = 1;
    }
    if (i >= str.size()) {
        return compat::unexpected<compat::string_view>("Sign without digits");
    }

    compat::string_view remaining = str.substr(i);
    if (remaining == "nan" || remaining == "NaN" || remaining == "NAN") {
        FloatType val = std::numeric_limits<FloatType>::quiet_NaN();
        return negative ? -val : val;
    }
    if (remaining == "inf" || remaining == "Inf" || remaining == "infinity" || remaining == "Infinity") {
        FloatType val = std::numeric_limits<FloatType>::infinity();
        return negative ? -val : val;
    }

    double value = 0.0;
    bool has_digits = false;

    while (i < str.size() && str[i] >= '0' && str[i] <= '9') {
        has_digits = true;
        value = value * 10.0 + (str[i] - '0');
        ++i;
    }

    if (i < str.size() && str[i] == '.') {
        ++i;
        double factor = 0.1;
        while (i < str.size() && str[i] >= '0' && str[i] <= '9') {
            has_digits = true;
            value += (str[i] - '0') * factor;
            factor *= 0.1;
            ++i;
        }
    }

    if (!has_digits) {
        return compat::unexpected<compat::string_view>("No digits in floating point number");
    }

    if (i < str.size() && (str[i] == 'e' || str[i] == 'E')) {
        ++i;
        if (i >= str.size()) {
            return compat::unexpected<compat::string_view>("Missing exponent digits");
        }
        bool exp_negative = false;
        if (str[i] == '-') {
            exp_negative = true;
            ++i;
        } else if (str[i] == '+') {
            ++i;
        }
        if (i >= str.size() || str[i] < '0' || str[i] > '9') {
            return compat::unexpected<compat::string_view>("Missing exponent digits");
        }
        int32_t exp_val = 0;
        while (i < str.size() && str[i] >= '0' && str[i] <= '9') {
            if (exp_val < 1000) {
                exp_val = exp_val * 10 + (str[i] - '0');
            }
            ++i;
        }
        if (exp_negative) {
            exp_val = -exp_val;
        }
        value = value * std::pow(10.0, static_cast<double>(exp_val));
    }

    if (i < str.size()) {
        return compat::unexpected<compat::string_view>("Invalid character in floating point number");
    }

    if (negative) {
        value = -value;
    }

    if (std::isinf(value)) {
        return compat::unexpected<compat::string_view>("Floating point overflow");
    }

    if (std::is_same<FloatType, float>::value) {
        if (value > 3.402823466e+38 || value < -3.402823466e+38) {
            return compat::unexpected<compat::string_view>("Floating point overflow");
        }
    }

    return static_cast<FloatType>(value);
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
        if (str.empty()) {
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
        if (str.empty()) {
            return compat::unexpected<compat::string_view>("Empty input string");
        }
        return std::string(str.data(), str.size());
    }
};

template <>
struct Parser<compat::string_view> {
    static compat::expected<compat::string_view, compat::string_view> parse(compat::string_view str) noexcept {
        if (str.empty()) {
            return compat::unexpected<compat::string_view>("Empty input string");
        }
        return str;
    }
};

} // namespace detail
} // namespace compat
