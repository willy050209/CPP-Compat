#pragma once

#include "../Config.hpp"
#include "../StringView.hpp"
#include <ostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <stdexcept>
#include <cstdint>

#if COMPAT_HAS_STD_FORMAT
#  include <format>
#endif

#if defined(_WIN32)
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#  include <io.h>
#  include <stdio.h>
#else
#  include <unistd.h>
#  include <stdio.h>
#endif

namespace compat {

#if !COMPAT_HAS_STD_FORMAT

/// <summary>
/// Forward declaration of compat::formatter extension point.
/// </summary>
/// <typeparam name="T">Type to be formatted.</typeparam>
/// <typeparam name="CharT">Character type, defaults to char.</typeparam>
template <typename T, typename CharT = char>
struct formatter;

#endif // !COMPAT_HAS_STD_FORMAT

namespace detail {

/// <summary>
/// Minimal basic_format_context for compatibility with std::formatter pattern.
/// </summary>
/// <typeparam name="OutputIt">Output iterator type.</typeparam>
/// <typeparam name="CharT">Character type.</typeparam>
template <typename OutputIt, typename CharT = char>
class basic_format_context {
public:
    using iterator = OutputIt;
    using char_type = CharT;

    /// <summary>
    /// Constructs a basic_format_context with the given output iterator.
    /// </summary>
    /// <param name="out">Output iterator.</param>
    explicit basic_format_context(OutputIt out) : out_(out) {}

    /// <summary>
    /// Returns the current output iterator.
    /// </summary>
    /// <returns>The output iterator.</returns>
    iterator out() const { return out_; }

    /// <summary>
    /// Advances the current output iterator.
    /// </summary>
    /// <param name="it">New iterator position.</param>
    void advance_to(iterator it) { out_ = it; }

private:
    OutputIt out_;
};

using format_context = basic_format_context<std::ostreambuf_iterator<char>, char>;

} // namespace detail

#if !COMPAT_HAS_STD_FORMAT

// Specializations of compat::formatter for primitive types

/// <summary>
/// Formatter specialization for std::string.
/// </summary>
template <>
struct formatter<std::string, char> {
    template <typename FormatContext>
    auto format(const std::string& val, FormatContext& ctx) const {
        auto it = ctx.out();
        for (char c : val) {
            *it++ = c;
        }
        ctx.advance_to(it);
        return it;
    }
};

/// <summary>
/// Formatter specialization for compat::string_view.
/// </summary>
template <>
struct formatter<compat::string_view, char> {
    template <typename FormatContext>
    auto format(compat::string_view val, FormatContext& ctx) const {
        auto it = ctx.out();
        for (std::size_t i = 0; i < val.size(); ++i) {
            *it++ = val[i];
        }
        ctx.advance_to(it);
        return it;
    }
};

/// <summary>
/// Formatter specialization for const char*.
/// </summary>
template <>
struct formatter<const char*, char> {
    template <typename FormatContext>
    auto format(const char* val, FormatContext& ctx) const {
        auto it = ctx.out();
        if (val != nullptr) {
            while (*val != '\0') {
                *it++ = *val++;
            }
        }
        ctx.advance_to(it);
        return it;
    }
};

/// <summary>
/// Formatter specialization for char* (non-const).
/// </summary>
template <>
struct formatter<char*, char> {
    template <typename FormatContext>
    auto format(char* val, FormatContext& ctx) const {
        const char* p = val;
        return formatter<const char*, char>{}.format(p, ctx);
    }
};

/// <summary>
/// Formatter specialization for single char.
/// </summary>
template <>
struct formatter<char, char> {
    template <typename FormatContext>
    auto format(char val, FormatContext& ctx) const {
        auto it = ctx.out();
        *it++ = val;
        ctx.advance_to(it);
        return it;
    }
};

/// <summary>
/// Formatter specialization for bool.
/// </summary>
template <>
struct formatter<bool, char> {
    template <typename FormatContext>
    auto format(bool val, FormatContext& ctx) const {
        const char* s = val ? "true" : "false";
        return formatter<const char*, char>{}.format(s, ctx);
    }
};

/// <summary>
/// Formatter specialization for const void*.
/// </summary>
template <>
struct formatter<const void*, char> {
    template <typename FormatContext>
    auto format(const void* val, FormatContext& ctx) const {
        std::ostringstream ss;
        ss << val;
        std::string s = ss.str();
        return formatter<std::string, char>{}.format(s, ctx);
    }
};

/// <summary>
/// Formatter specialization for void*.
/// </summary>
template <>
struct formatter<void*, char> {
    template <typename FormatContext>
    auto format(void* val, FormatContext& ctx) const {
        return formatter<const void*, char>{}.format(val, ctx);
    }
};

#define COMPAT_DEFINE_ARITHMETIC_FORMATTER(Type) \
template <> \
struct formatter<Type, char> { \
    template <typename FormatContext> \
    auto format(Type val, FormatContext& ctx) const { \
        std::ostringstream ss; \
        ss << val; \
        std::string s = ss.str(); \
        return formatter<std::string, char>{}.format(s, ctx); \
    } \
};

COMPAT_DEFINE_ARITHMETIC_FORMATTER(short)
COMPAT_DEFINE_ARITHMETIC_FORMATTER(unsigned short)
COMPAT_DEFINE_ARITHMETIC_FORMATTER(int)
COMPAT_DEFINE_ARITHMETIC_FORMATTER(unsigned int)
COMPAT_DEFINE_ARITHMETIC_FORMATTER(long)
COMPAT_DEFINE_ARITHMETIC_FORMATTER(unsigned long)
COMPAT_DEFINE_ARITHMETIC_FORMATTER(long long)
COMPAT_DEFINE_ARITHMETIC_FORMATTER(unsigned long long)
COMPAT_DEFINE_ARITHMETIC_FORMATTER(float)
COMPAT_DEFINE_ARITHMETIC_FORMATTER(double)
COMPAT_DEFINE_ARITHMETIC_FORMATTER(long double)

#undef COMPAT_DEFINE_ARITHMETIC_FORMATTER

/// <summary>
/// Formatter specialization for signed char (printed as integer).
/// </summary>
template <>
struct formatter<signed char, char> {
    template <typename FormatContext>
    auto format(signed char val, FormatContext& ctx) const {
        return formatter<int, char>{}.format(static_cast<int>(val), ctx);
    }
};

/// <summary>
/// Formatter specialization for unsigned char (printed as integer).
/// </summary>
template <>
struct formatter<unsigned char, char> {
    template <typename FormatContext>
    auto format(unsigned char val, FormatContext& ctx) const {
        return formatter<unsigned int, char>{}.format(static_cast<unsigned int>(val), ctx);
    }
};

#endif // !COMPAT_HAS_STD_FORMAT

namespace detail {

template <typename T>
struct format_arg_traits {
    using type = typename std::decay<T>::type;
};

template <std::size_t N>
struct format_arg_traits<char[N]> {
    using type = const char*;
};

template <std::size_t N>
struct format_arg_traits<const char[N]> {
    using type = const char*;
};

#if COMPAT_HAS_STD_FORMAT

template <typename T, typename = void>
struct has_std_formatter : std::false_type {};

template <typename T>
struct has_std_formatter<T, std::void_t<
    decltype(std::declval<std::formatter<typename format_arg_traits<T>::type, char>>()
        .format(std::declval<const typename format_arg_traits<T>::type&>(), std::declval<std::format_context&>()))
>> : std::true_type {};

template <typename T>
inline typename std::enable_if<has_std_formatter<T>::value, void>::type
StreamFormatArg(std::ostream& os, const T& arg) {
    using FormatterType = typename format_arg_traits<T>::type;
    FormatterType formatted_arg = static_cast<FormatterType>(arg);
    std::string s = std::format("{}", formatted_arg);
    os << s;
}

template <typename T>
inline typename std::enable_if<!has_std_formatter<T>::value, void>::type
StreamFormatArg(std::ostream& os, const T& arg) {
    os << arg;
}

inline void StreamFormatArg(std::ostream& os, signed char arg) {
    os << static_cast<int32_t>(arg);
}

inline void StreamFormatArg(std::ostream& os, unsigned char arg) {
    os << static_cast<uint32_t>(arg);
}

#else

template <typename T, typename = void>
struct has_compat_formatter : std::false_type {};

template <typename T>
struct has_compat_formatter<T, std::void_t<
    decltype(std::declval<formatter<typename format_arg_traits<T>::type, char>>()
        .format(std::declval<const typename format_arg_traits<T>::type&>(), std::declval<format_context&>()))
>> : std::true_type {};

template <typename T>
inline typename std::enable_if<has_compat_formatter<T>::value, void>::type
StreamFormatArg(std::ostream& os, const T& arg) {
    std::ostreambuf_iterator<char> out_it(os);
    format_context ctx(out_it);
    using FormatterType = typename format_arg_traits<T>::type;
    formatter<FormatterType, char> fmt_obj;
    FormatterType formatted_arg = static_cast<FormatterType>(arg);
    fmt_obj.format(formatted_arg, ctx);
}

template <typename T>
inline typename std::enable_if<!has_compat_formatter<T>::value, void>::type
StreamFormatArg(std::ostream& os, const T& arg) {
    os << arg;
}

inline void StreamFormatArg(std::ostream& os, signed char arg) {
    os << static_cast<int32_t>(arg);
}

inline void StreamFormatArg(std::ostream& os, unsigned char arg) {
    os << static_cast<uint32_t>(arg);
}

#endif

/// <summary>
/// Counts the number of {} placeholders in a format string (skipping escaped {{ and }}).
/// Validates balanced braces.
/// </summary>
/// <param name="fmt">Format string view.</param>
/// <returns>Number of {} replacement fields.</returns>
/// <exception cref="std::invalid_argument">Thrown on unmatched single { or }.</exception>
inline std::size_t CountAndValidatePlaceholders(compat::string_view fmt) {
    std::size_t count = 0;
    std::size_t i = 0;
    while (i < fmt.size()) {
        if (fmt[i] == '{') {
            if (i + 1 < fmt.size() && fmt[i + 1] == '{') {
                i += 2;
            } else if (i + 1 < fmt.size() && fmt[i + 1] == '}') {
                ++count;
                i += 2;
            } else {
                throw std::invalid_argument("Unmatched '{' in format string");
            }
        } else if (fmt[i] == '}') {
            if (i + 1 < fmt.size() && fmt[i + 1] == '}') {
                i += 2;
            } else {
                throw std::invalid_argument("Unmatched '}' in format string");
            }
        } else {
            ++i;
        }
    }
    return count;
}

/// <summary>
/// Base case for recursive template format string writer.
/// </summary>
/// <param name="os">Target output stream.</param>
/// <param name="fmt">Format string view.</param>
inline void WriteFormattedImpl(std::ostream& os, compat::string_view fmt) {
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
/// Recursive template format string writer substituting {} placeholders with arguments.
/// </summary>
template <typename First, typename... Rest>
inline void WriteFormattedImpl(std::ostream& os, compat::string_view fmt, const First& first, const Rest&... rest) {
    for (std::size_t i = 0; i < fmt.size(); ++i) {
        if (fmt[i] == '{') {
            if (i + 1 < fmt.size() && fmt[i + 1] == '{') {
                os << '{';
                ++i;
            } else if (i + 1 < fmt.size() && fmt[i + 1] == '}') {
                StreamFormatArg(os, first);
                WriteFormattedImpl(os, fmt.substr(i + 2), rest...);
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

/// <summary>
/// Validates placeholder count against argument count and writes formatted output to stream.
/// </summary>
/// <typeparam name="Args">Types of arguments.</typeparam>
/// <param name="os">Target output stream.</param>
/// <param name="fmt">Format string view.</param>
/// <param name="args">Arguments to format.</param>
/// <exception cref="std::invalid_argument">Thrown if placeholder count does not match argument count or on malformed braces.</exception>
template <typename... Args>
inline void WriteFormatted(std::ostream& os, compat::string_view fmt, const Args&... args) {
    constexpr std::size_t num_args = sizeof...(Args);
    std::size_t num_placeholders = CountAndValidatePlaceholders(fmt);
    if (num_placeholders < num_args) {
        throw std::invalid_argument("Too many arguments for format string");
    }
    if (num_placeholders > num_args) {
        throw std::invalid_argument("Too few arguments for format string");
    }
    WriteFormattedImpl(os, fmt, args...);
}

/// <summary>
/// Writes UTF-8 encoded string view to stdout, converting to UTF-16 via WriteConsoleW if attached to a Windows console.
/// </summary>
/// <param name="text">UTF-8 encoded string view to write.</param>
inline void WriteStdoutUtf8(compat::string_view text) {
#if defined(_WIN32)
    int stdout_fd = _fileno(stdout);
    if (stdout_fd >= 0 && _isatty(stdout_fd)) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode = 0;
        if (hConsole != INVALID_HANDLE_VALUE && hConsole != NULL && GetConsoleMode(hConsole, &mode)) {
            if (!text.empty()) {
                int wide_len = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), NULL, 0);
                if (wide_len > 0) {
                    std::wstring wide_buf(static_cast<std::size_t>(wide_len), L'\0');
                    MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), &wide_buf[0], wide_len);
                    DWORD written = 0;
                    WriteConsoleW(hConsole, wide_buf.data(), static_cast<DWORD>(wide_buf.size()), &written, NULL);
                    return;
                }
            } else {
                return;
            }
        }
    }
#endif
    if (!text.empty()) {
        std::fwrite(text.data(), 1, text.size(), stdout);
        std::fflush(stdout);
    }
}

} // namespace detail
} // namespace compat
