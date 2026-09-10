#pragma once

#include "SelfPrint.hpp"
#include <string>

namespace compat {

// compat::formatter is declared in SelfPrint.hpp and can be specialized here or in user code.

namespace detail {

/// <summary>
/// Formats arguments into a std::string using zero-heap-allocation stack_buffer.
/// </summary>
/// <typeparam name="Args">Types of arguments to format.</typeparam>
/// <param name="fmt">Format string view containing {} placeholders.</param>
/// <param name="args">Arguments to substitute.</param>
/// <returns>Formatted std::string.</returns>
template <typename... Args>
inline std::string FormatToString(compat::string_view fmt, const Args&... args) {
    stack_buffer<512> buf;
    WriteFormattedBuffer(buf, fmt, args...);
    return buf.str();
}

} // namespace detail
} // namespace compat
