#pragma once

#include "Config.hpp"
#include "StringView.hpp"
#include "detail/SelfPrint.hpp"

#if COMPAT_HAS_STD_FORMAT
#  include <format>
namespace compat {
    using std::format;
    using std::formatter;
}
#else
#  include "detail/SelfFormat.hpp"
namespace compat {

/// <summary>
/// Formats arguments into a std::string according to the format string.
/// </summary>
/// <typeparam name="Args">Types of arguments to format.</typeparam>
/// <param name="fmt">Format string containing {} placeholders.</param>
/// <param name="args">Arguments to substitute into the format string.</param>
/// <returns>Formatted std::string.</returns>
template <typename... Args>
inline std::string format(compat::string_view fmt, const Args&... args) {
    return detail::FormatToString(fmt, args...);
}

} // namespace compat
#endif

namespace compat {

/// <summary>
/// Converts an arithmetic or formattable value to std::string using the compat formatting engine.
/// Provides a drop-in replacement for std::to_string with zero locale dependence and cross-platform consistency.
/// </summary>
/// <typeparam name="T">Type of value to convert.</typeparam>
/// <param name="value">The value to convert.</param>
/// <returns>Formatted std::string representation.</returns>
template <typename T>
COMPAT_NODISCARD inline std::string to_string(const T& value) {
#if COMPAT_HAS_STD_FORMAT
    return std::format("{}", value);
#else
    return detail::FormatToString("{}", value);
#endif
}

} // namespace compat

