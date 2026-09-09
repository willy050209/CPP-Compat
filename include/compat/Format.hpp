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
