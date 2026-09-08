#pragma once

#include "../StringView.hpp"
#include <ostream>
#include <stdexcept>
#include <cstdint>

namespace compat {
namespace detail {

/// <summary>
/// Streams a generic argument to an output stream.
/// </summary>
template <typename T>
inline void StreamFormatArg(std::ostream& os, const T& arg) {
    os << arg;
}

/// <summary>
/// Streams int8_t as an integer number rather than ASCII character.
/// </summary>
inline void StreamFormatArg(std::ostream& os, int8_t arg) {
    os << static_cast<int32_t>(arg);
}

/// <summary>
/// Streams uint8_t as an integer number rather than ASCII character.
/// </summary>
inline void StreamFormatArg(std::ostream& os, uint8_t arg) {
    os << static_cast<uint32_t>(arg);
}

/// <summary>
/// Base case for recursive template format string parser.
/// </summary>
/// <param name="os">Target output stream.</param>
/// <param name="fmt">Format string view.</param>
/// <exception cref="std::invalid_argument">Thrown if unmatched placeholder remains.</exception>
inline void WriteFormatted(std::ostream& os, compat::string_view fmt) {
    for (std::size_t i = 0; i < fmt.size(); ++i) {
        if (fmt[i] == '{') {
            if (i + 1 < fmt.size() && fmt[i + 1] == '{') {
                os << '{';
                ++i;
            } else if (i + 1 < fmt.size() && fmt[i + 1] == '}') {
                throw std::invalid_argument("Too few arguments for format string");
            } else {
                os << fmt[i];
            }
        } else if (fmt[i] == '}' && i + 1 < fmt.size() && fmt[i + 1] == '}') {
            os << '}';
            ++i;
        } else {
            os << fmt[i];
        }
    }
}

/// <summary>
/// Recursive template format string parser substituting {} placeholders with arguments.
/// </summary>
/// <typeparam name="First">Type of head argument.</typeparam>
/// <typeparam name="Rest">Types of tail arguments.</typeparam>
/// <param name="os">Target output stream.</param>
/// <param name="fmt">Format string view.</param>
/// <param name="first">Head argument.</param>
/// <param name="rest">Tail arguments.</param>
/// <exception cref="std::invalid_argument">Thrown if excess arguments are supplied.</exception>
template <typename First, typename... Rest>
inline void WriteFormatted(std::ostream& os, compat::string_view fmt, const First& first, const Rest&... rest) {
    for (std::size_t i = 0; i < fmt.size(); ++i) {
        if (fmt[i] == '{') {
            if (i + 1 < fmt.size() && fmt[i + 1] == '{') {
                os << '{';
                ++i;
            } else if (i + 1 < fmt.size() && fmt[i + 1] == '}') {
                StreamFormatArg(os, first);
                WriteFormatted(os, fmt.substr(i + 2), rest...);
                return;
            } else {
                os << fmt[i];
            }
        } else if (fmt[i] == '}' && i + 1 < fmt.size() && fmt[i + 1] == '}') {
            os << '}';
            ++i;
        } else {
            os << fmt[i];
        }
    }
    throw std::invalid_argument("Too many arguments for format string");
}

} // namespace detail
} // namespace compat
