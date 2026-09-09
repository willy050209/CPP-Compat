#pragma once

#include "SelfPrint.hpp"
#include <string>
#include <sstream>

namespace compat {

// compat::formatter is declared in SelfPrint.hpp and can be specialized here or in user code.

namespace detail {

/// <summary>
/// Formats arguments into a std::string according to the format string.
/// </summary>
/// <typeparam name="Args">Types of arguments to format.</typeparam>
/// <param name="fmt">Format string view containing {} placeholders.</param>
/// <param name="args">Arguments to substitute.</param>
/// <returns>Formatted std::string.</returns>
template <typename... Args>
inline std::string FormatToString(compat::string_view fmt, const Args&... args) {
    std::ostringstream oss;
    WriteFormatted(oss, fmt, args...);
    return oss.str();
}

} // namespace detail
} // namespace compat
