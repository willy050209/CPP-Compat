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
/// Writes formatted text to the specified output stream using single-write buffered output.
/// </summary>
/// <typeparam name="Args">Types of arguments to format.</typeparam>
/// <param name="os">Target output stream.</param>
/// <param name="fmt">Format string view.</param>
/// <param name="args">Arguments to substitute.</param>
template <typename... Args>
inline void print(std::ostream& os, compat::string_view fmt, const Args&... args) {
    detail::stack_buffer<512> buf;
    detail::WriteFormattedBuffer(buf, fmt, args...);
    os.write(buf.data(), static_cast<std::streamsize>(buf.size()));
}

/// <summary>
/// Writes formatted text to standard output using UTF-8 console output when available.
/// </summary>
/// <typeparam name="Args">Types of arguments to format.</typeparam>
/// <param name="fmt">Format string view.</param>
/// <param name="args">Arguments to substitute.</param>
template <typename... Args>
inline void print(compat::string_view fmt, const Args&... args) {
    detail::stack_buffer<512> buf;
    detail::WriteFormattedBuffer(buf, fmt, args...);
    detail::WriteStdoutUtf8(buf.view());
}

/// <summary>
/// Writes formatted text followed by a newline to the specified output stream using single-write buffered output.
/// </summary>
/// <typeparam name="Args">Types of arguments to format.</typeparam>
/// <param name="os">Target output stream.</param>
/// <param name="fmt">Format string view.</param>
/// <param name="args">Arguments to substitute.</param>
template <typename... Args>
inline void println(std::ostream& os, compat::string_view fmt, const Args&... args) {
    detail::stack_buffer<512> buf;
    detail::WriteFormattedBuffer(buf, fmt, args...);
    buf.push_back('\n');
    os.write(buf.data(), static_cast<std::streamsize>(buf.size()));
}

/// <summary>
/// Writes formatted text followed by a newline to standard output using UTF-8 console output when available.
/// </summary>
/// <typeparam name="Args">Types of arguments to format.</typeparam>
/// <param name="fmt">Format string view.</param>
/// <param name="args">Arguments to substitute.</param>
template <typename... Args>
inline void println(compat::string_view fmt, const Args&... args) {
    detail::stack_buffer<512> buf;
    detail::WriteFormattedBuffer(buf, fmt, args...);
    buf.push_back('\n');
    detail::WriteStdoutUtf8(buf.view());
}

/// <summary>
/// Writes a single newline character to the specified output stream.
/// </summary>
/// <param name="os">Target output stream.</param>
inline void println(std::ostream& os) {
    os << '\n';
}

/// <summary>
/// Writes a single newline character to standard output.
/// </summary>
inline void println() {
    detail::WriteStdoutUtf8("\n");
}

} // namespace compat
#endif
