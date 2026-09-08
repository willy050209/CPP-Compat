#pragma once

#include "Config.hpp"
#include "StringView.hpp"
#include "detail/SelfPrint.hpp"
#include <iostream>

#if COMPAT_HAS_STD_PRINT
#  include <print>
namespace compat {
    using std::print;
    using std::println;
}
#else
namespace compat {

/// <summary>
/// Writes formatted text to the specified output stream.
/// </summary>
template <typename... Args>
inline void print(std::ostream& os, compat::string_view fmt, const Args&... args) {
    detail::WriteFormatted(os, fmt, args...);
}

/// <summary>
/// Writes formatted text to standard output (std::cout).
/// </summary>
template <typename... Args>
inline void print(compat::string_view fmt, const Args&... args) {
    detail::WriteFormatted(std::cout, fmt, args...);
}

/// <summary>
/// Writes formatted text followed by a newline to the specified output stream.
/// </summary>
template <typename... Args>
inline void println(std::ostream& os, compat::string_view fmt, const Args&... args) {
    detail::WriteFormatted(os, fmt, args...);
    os << '\n';
}

/// <summary>
/// Writes formatted text followed by a newline to standard output (std::cout).
/// </summary>
template <typename... Args>
inline void println(compat::string_view fmt, const Args&... args) {
    detail::WriteFormatted(std::cout, fmt, args...);
    std::cout << '\n';
}

/// <summary>
/// Writes a single newline character to the specified output stream.
/// </summary>
inline void println(std::ostream& os) {
    os << '\n';
}

/// <summary>
/// Writes a single newline character to standard output (std::cout).
/// </summary>
inline void println() {
    std::cout << '\n';
}

} // namespace compat
#endif
