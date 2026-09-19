#pragma once

#include "../Config.hpp"
#include "../StringView.hpp"
#include <ostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <stdexcept>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cwchar>

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
/// Ensures Windows console input/output code page is set to UTF-8 and enables virtual terminal processing.
/// </summary>
inline void EnsureConsoleUtf8() noexcept {
#if defined(_WIN32)
    static const bool initialized = []() noexcept {
        ::SetConsoleOutputCP(CP_UTF8);
        ::SetConsoleCP(CP_UTF8);
        HANDLE hOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE && hOut != NULL) {
            DWORD mode = 0;
            if (::GetConsoleMode(hOut, &mode)) {
                ::SetConsoleMode(hOut, mode | 0x0004 /* ENABLE_VIRTUAL_TERMINAL_PROCESSING */);
            }
        }
        HANDLE hErr = ::GetStdHandle(STD_ERROR_HANDLE);
        if (hErr != INVALID_HANDLE_VALUE && hErr != NULL) {
            DWORD mode = 0;
            if (::GetConsoleMode(hErr, &mode)) {
                ::SetConsoleMode(hErr, mode | 0x0004 /* ENABLE_VIRTUAL_TERMINAL_PROCESSING */);
            }
        }
        return true;
    }();
    (void)initialized;
#endif
}

#if defined(_WIN32)
namespace {
    struct ConsoleUtf8AutoInit {
        ConsoleUtf8AutoInit() noexcept {
            EnsureConsoleUtf8();
        }
    };
    static const ConsoleUtf8AutoInit g_console_utf8_auto_init;
}
#endif

/// <summary>
/// Validates whether a character sequence is strictly valid UTF-8.
/// </summary>
/// <param name="s">String view to validate.</param>
/// <returns>True if sequence is valid UTF-8, false otherwise.</returns>
inline bool IsValidUtf8(compat::string_view s) noexcept {
    const unsigned char* p = reinterpret_cast<const unsigned char*>(s.data());
    const unsigned char* end = p + s.size();
    while (p < end) {
        if (*p <= 0x7FU) {
            ++p;
        } else if (*p >= 0xC2U && *p <= 0xDFU) {
            if (p + 1 >= end || (p[1] & 0xC0U) != 0x80U) return false;
            p += 2;
        } else if (*p >= 0xE0U && *p <= 0xEFU) {
            if (p + 2 >= end || (p[1] & 0xC0U) != 0x80U || (p[2] & 0xC0U) != 0x80U) return false;
            if (*p == 0xE0U && p[1] < 0xA0U) return false;
            if (*p == 0xEDU && p[1] > 0x9FU) return false;
            p += 3;
        } else if (*p >= 0xF0U && *p <= 0xF4U) {
            if (p + 3 >= end || (p[1] & 0xC0U) != 0x80U || (p[2] & 0xC0U) != 0x80U || (p[3] & 0xC0U) != 0x80U) return false;
            if (*p == 0xF0U && p[1] < 0x90U) return false;
            if (*p == 0xF4U && p[1] > 0x8FU) return false;
            p += 4;
        } else {
            return false;
        }
    }
    return true;
}

#if defined(_WIN32)
/// <summary>
/// Converts an ANSI string encoded in the active system code page (CP_ACP) to a UTF-8 std::string.
/// </summary>
/// <param name="s">String view in CP_ACP encoding.</param>
/// <returns>UTF-8 encoded std::string.</returns>
inline std::string AcpToUtf8(compat::string_view s) {
    if (s.empty()) return std::string();
    int wlen = MultiByteToWideChar(CP_ACP, 0, s.data(), static_cast<int>(s.size()), NULL, 0);
    if (wlen <= 0) return std::string(s.data(), s.size());
    std::wstring wstr(static_cast<std::size_t>(wlen), L'\0');
    MultiByteToWideChar(CP_ACP, 0, s.data(), static_cast<int>(s.size()), &wstr[0], wlen);
    int u8len = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wlen, NULL, 0, NULL, NULL);
    if (u8len <= 0) return std::string(s.data(), s.size());
    std::string u8str(static_cast<std::size_t>(u8len), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wlen, &u8str[0], u8len, NULL, NULL);
    return u8str;
}
#endif

/// <summary>
/// Small-buffer-optimized stack buffer with fallback to heap allocation for formatted output.
/// </summary>
/// <typeparam name="Capacity">Stack buffer capacity in bytes (default 512).</typeparam>
template <std::size_t Capacity = 512>
class stack_buffer {
public:
    /// <summary>
    /// Constructs an empty stack_buffer.
    /// </summary>
    stack_buffer() noexcept : size_(0), heap_ptr_(nullptr), capacity_(Capacity) {}

    /// <summary>
    /// Destructor releasing heap allocation if buffer expanded beyond stack capacity.
    /// </summary>
    ~stack_buffer() {
        if (heap_ptr_ != nullptr) {
            delete[] heap_ptr_;
        }
    }

    stack_buffer(const stack_buffer&) = delete;
    stack_buffer& operator=(const stack_buffer&) = delete;

    /// <summary>
    /// Move constructor transferring heap buffer ownership or copying stack contents.
    /// </summary>
    /// <param name="other">Source buffer to move from.</param>
    stack_buffer(stack_buffer&& other) noexcept
        : size_(other.size_), heap_ptr_(other.heap_ptr_), capacity_(other.capacity_) {
        if (other.heap_ptr_ == nullptr) {
            std::memcpy(stack_buf_, other.stack_buf_, other.size_);
        }
        other.heap_ptr_ = nullptr;
        other.size_ = 0;
        other.capacity_ = Capacity;
    }

    /// <summary>
    /// Move assignment operator transferring heap buffer ownership or copying stack contents.
    /// </summary>
    /// <param name="other">Source buffer to move from.</param>
    /// <returns>Reference to this buffer.</returns>
    stack_buffer& operator=(stack_buffer&& other) noexcept {
        if (this != &other) {
            if (heap_ptr_ != nullptr) {
                delete[] heap_ptr_;
            }
            size_ = other.size_;
            heap_ptr_ = other.heap_ptr_;
            capacity_ = other.capacity_;
            if (other.heap_ptr_ == nullptr) {
                std::memcpy(stack_buf_, other.stack_buf_, other.size_);
            }
            other.heap_ptr_ = nullptr;
            other.size_ = 0;
            other.capacity_ = Capacity;
        }
        return *this;
    }

    /// <summary>
    /// Appends a single character to the buffer, growing capacity if needed.
    /// </summary>
    /// <param name="c">Character to append.</param>
    COMPAT_ALWAYS_INLINE void push_back(char c) {
        if (COMPAT_UNLIKELY(size_ >= capacity_)) {
            grow(size_ + 1);
        }
        char* p = heap_ptr_ ? heap_ptr_ : stack_buf_;
        p[size_++] = c;
    }

    /// <summary>
    /// Appends a contiguous sequence of characters to the buffer.
    /// </summary>
    /// <param name="data">Pointer to character sequence.</param>
    /// <param name="len">Number of characters to append.</param>
    COMPAT_ALWAYS_INLINE void append(const char* data, std::size_t len) {
        if (COMPAT_UNLIKELY(len == 0)) {
            return;
        }
        if (COMPAT_UNLIKELY(size_ + len > capacity_)) {
            grow(size_ + len);
        }
        char* p = heap_ptr_ ? heap_ptr_ : stack_buf_;
        std::memcpy(p + size_, data, len);
        size_ += len;
    }

    /// <summary>
    /// Returns pointer to constant buffer data.
    /// </summary>
    /// <returns>Pointer to data.</returns>
    COMPAT_ALWAYS_INLINE const char* data() const noexcept {
        return heap_ptr_ ? heap_ptr_ : stack_buf_;
    }

    /// <summary>
    /// Returns pointer to mutable buffer data.
    /// </summary>
    /// <returns>Pointer to data.</returns>
    COMPAT_ALWAYS_INLINE char* data() noexcept {
        return heap_ptr_ ? heap_ptr_ : stack_buf_;
    }

    /// <summary>
    /// Returns the current number of characters in the buffer.
    /// </summary>
    /// <returns>Character count.</returns>
    COMPAT_ALWAYS_INLINE std::size_t size() const noexcept {
        return size_;
    }

    /// <summary>
    /// Checks whether the buffer is empty.
    /// </summary>
    /// <returns>True if size is zero, false otherwise.</returns>
    COMPAT_ALWAYS_INLINE bool empty() const noexcept {
        return size_ == 0;
    }

    /// <summary>
    /// Returns a string_view spanning the buffer contents.
    /// </summary>
    /// <returns>compat::string_view representing the buffer.</returns>
    COMPAT_ALWAYS_INLINE compat::string_view view() const noexcept {
        return compat::string_view(data(), size_);
    }

    /// <summary>
    /// Constructs a std::string from the buffer contents.
    /// </summary>
    /// <returns>std::string copy.</returns>
    std::string str() const {
        return std::string(data(), size_);
    }

    /// <summary>
    /// Clears buffer contents without deallocating heap storage.
    /// </summary>
    COMPAT_ALWAYS_INLINE void clear() noexcept {
        size_ = 0;
    }

private:
    void grow(std::size_t min_capacity) {
        std::size_t new_cap = capacity_ * 2;
        if (new_cap < min_capacity) {
            new_cap = min_capacity;
        }
        char* new_ptr = new char[new_cap];
        char* old_ptr = heap_ptr_ ? heap_ptr_ : stack_buf_;
        std::memcpy(new_ptr, old_ptr, size_);
        if (heap_ptr_ != nullptr) {
            delete[] heap_ptr_;
        }
        heap_ptr_ = new_ptr;
        capacity_ = new_cap;
    }

    char stack_buf_[Capacity];
    std::size_t size_;
    char* heap_ptr_;
    std::size_t capacity_;
};

/// <summary>
/// Encodes a 32-bit Unicode code point to UTF-8 bytes and appends to stack_buffer.
/// </summary>
/// <typeparam name="Capacity">Stack buffer capacity.</typeparam>
/// <param name="buf">Target stack buffer.</param>
/// <param name="cp">Unicode code point.</param>
template <std::size_t Capacity>
inline void AppendUtf8CodePoint(stack_buffer<Capacity>& buf, uint32_t cp) {
    if (cp <= 0x7FU) {
        buf.push_back(static_cast<char>(cp));
    } else if (cp <= 0x7FFU) {
        buf.push_back(static_cast<char>(0xC0U | ((cp >> 6) & 0x1FU)));
        buf.push_back(static_cast<char>(0x80U | (cp & 0x3FU)));
    } else if (cp <= 0xFFFFU) {
        if (cp >= 0xD800U && cp <= 0xDFFFU) {
            cp = 0xFFFDU;
        }
        buf.push_back(static_cast<char>(0xE0U | ((cp >> 12) & 0x0FU)));
        buf.push_back(static_cast<char>(0x80U | ((cp >> 6) & 0x3FU)));
        buf.push_back(static_cast<char>(0x80U | (cp & 0x3FU)));
    } else if (cp <= 0x10FFFFU) {
        buf.push_back(static_cast<char>(0xF0U | ((cp >> 18) & 0x07U)));
        buf.push_back(static_cast<char>(0x80U | ((cp >> 12) & 0x3FU)));
        buf.push_back(static_cast<char>(0x80U | ((cp >> 6) & 0x3FU)));
        buf.push_back(static_cast<char>(0x80U | (cp & 0x3FU)));
    } else {
        buf.push_back(static_cast<char>(0xEFU));
        buf.push_back(static_cast<char>(0xBFU));
        buf.push_back(static_cast<char>(0xBDU));
    }
}

/// <summary>
/// Converts UTF-16 code units to UTF-8 and appends to stack_buffer.
/// </summary>
/// <typeparam name="Capacity">Stack buffer capacity.</typeparam>
/// <typeparam name="Char16It">Iterator or pointer to 16-bit characters.</typeparam>
/// <param name="buf">Target stack buffer.</param>
/// <param name="begin">Start of UTF-16 character sequence.</param>
/// <param name="end">End of UTF-16 character sequence.</param>
template <std::size_t Capacity, typename Char16It>
inline void AppendUtf16ToBuffer(stack_buffer<Capacity>& buf, Char16It begin, Char16It end) {
    while (begin != end) {
        uint32_t c1 = static_cast<uint16_t>(*begin++);
        if (c1 >= 0xD800U && c1 <= 0xDBFFU) {
            if (begin != end) {
                uint32_t c2 = static_cast<uint16_t>(*begin);
                if (c2 >= 0xDC00U && c2 <= 0xDFFFU) {
                    ++begin;
                    uint32_t cp = 0x10000U + (((c1 - 0xD800U) << 10) | (c2 - 0xDC00U));
                    AppendUtf8CodePoint(buf, cp);
                    continue;
                }
            }
            AppendUtf8CodePoint(buf, 0xFFFDU);
        } else if (c1 >= 0xDC00U && c1 <= 0xDFFFU) {
            AppendUtf8CodePoint(buf, 0xFFFDU);
        } else {
            AppendUtf8CodePoint(buf, c1);
        }
    }
}

/// <summary>
/// Converts UTF-32 code units to UTF-8 and appends to stack_buffer.
/// </summary>
/// <typeparam name="Capacity">Stack buffer capacity.</typeparam>
/// <typeparam name="Char32It">Iterator or pointer to 32-bit characters.</typeparam>
/// <param name="buf">Target stack buffer.</param>
/// <param name="begin">Start of UTF-32 character sequence.</param>
/// <param name="end">End of UTF-32 character sequence.</param>
template <std::size_t Capacity, typename Char32It>
inline void AppendUtf32ToBuffer(stack_buffer<Capacity>& buf, Char32It begin, Char32It end) {
    while (begin != end) {
        AppendUtf8CodePoint(buf, static_cast<uint32_t>(*begin++));
    }
}

/// <summary>
/// Output iterator adapter appending characters directly into a stack_buffer.
/// </summary>
/// <typeparam name="Buffer">Buffer type supporting push_back.</typeparam>
template <typename Buffer>
class buffer_appender {
public:
    using iterator_category = std::output_iterator_tag;
    using value_type = void;
    using difference_type = void;
    using pointer = void;
    using reference = void;

    /// <summary>
    /// Constructs a buffer_appender bound to the target buffer.
    /// </summary>
    /// <param name="buf">Target buffer reference.</param>
    COMPAT_ALWAYS_INLINE explicit buffer_appender(Buffer& buf) noexcept : buf_(&buf) {}

    /// <summary>
    /// Appends a character to the bound buffer.
    /// </summary>
    /// <param name="c">Character to write.</param>
    /// <returns>Reference to this iterator.</returns>
    COMPAT_ALWAYS_INLINE buffer_appender& operator=(char c) {
        buf_->push_back(c);
        return *this;
    }

    /// <summary>
    /// Dereference operator returning this iterator.
    /// </summary>
    /// <returns>Reference to this iterator.</returns>
    COMPAT_ALWAYS_INLINE buffer_appender& operator*() noexcept { return *this; }

    /// <summary>
    /// Prefix increment operator.
    /// </summary>
    /// <returns>Reference to this iterator.</returns>
    COMPAT_ALWAYS_INLINE buffer_appender& operator++() noexcept { return *this; }

    /// <summary>
    /// Postfix increment operator.
    /// </summary>
    /// <returns>Copy of this iterator.</returns>
    COMPAT_ALWAYS_INLINE buffer_appender operator++(int) noexcept { return *this; }

private:
    Buffer* buf_;
};

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

using format_context = basic_format_context<buffer_appender<stack_buffer<512>>, char>;

/// <summary>
/// Minimal basic_format_parse_context for compatibility with std::formatter pattern.
/// </summary>
/// <typeparam name="CharT">Character type.</typeparam>
template <typename CharT = char>
class basic_format_parse_context {
public:
    using char_type = CharT;
    using const_iterator = const CharT*;
    using iterator = const CharT*;

    /// <summary>
    /// Constructs a basic_format_parse_context with the given format string_view.
    /// </summary>
    /// <param name="fmt">Format string_view.</param>
    constexpr explicit basic_format_parse_context(compat::string_view fmt) noexcept
        : begin_(fmt.data()), end_(fmt.data() + fmt.size()) {}

    /// <summary>
    /// Returns iterator to the beginning of the format specification.
    /// </summary>
    /// <returns>Iterator to beginning.</returns>
    constexpr const_iterator begin() const noexcept { return begin_; }

    /// <summary>
    /// Returns iterator to the end of the format specification.
    /// </summary>
    /// <returns>Iterator to end.</returns>
    constexpr const_iterator end() const noexcept { return end_; }

    /// <summary>
    /// Advances iterator to the given position.
    /// </summary>
    /// <param name="it">New iterator position.</param>
    void advance_to(const_iterator it) noexcept { begin_ = it; }

private:
    const_iterator begin_;
    const_iterator end_;
};

using format_parse_context = basic_format_parse_context<char>;

#if (COMPAT_CPLUSPLUS >= COMPAT_CXX_17)
alignas(64) inline constexpr char DigitsLut[200] = {
#else
alignas(64) static const char DigitsLut[200] = {
#endif
    '0', '0', '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '6', '0', '7', '0', '8', '0', '9',
    '1', '0', '1', '1', '1', '2', '1', '3', '1', '4', '1', '5', '1', '6', '1', '7', '1', '8', '1', '9',
    '2', '0', '2', '1', '2', '2', '2', '3', '2', '4', '2', '5', '2', '6', '2', '7', '2', '8', '2', '9',
    '3', '0', '3', '1', '3', '2', '3', '3', '3', '4', '3', '5', '3', '6', '3', '7', '3', '8', '3', '9',
    '4', '0', '4', '1', '4', '2', '4', '3', '4', '4', '4', '5', '4', '6', '4', '7', '4', '8', '4', '9',
    '5', '0', '5', '1', '5', '2', '5', '3', '5', '4', '5', '5', '5', '6', '5', '7', '5', '8', '5', '9',
    '6', '0', '6', '1', '6', '2', '6', '3', '6', '4', '6', '5', '6', '6', '6', '7', '6', '8', '6', '9',
    '7', '0', '7', '1', '7', '2', '7', '3', '7', '4', '7', '5', '7', '6', '7', '7', '7', '8', '7', '9',
    '8', '0', '8', '1', '8', '2', '8', '3', '8', '4', '8', '5', '8', '6', '8', '7', '8', '8', '8', '9',
    '9', '0', '9', '1', '9', '2', '9', '3', '9', '4', '9', '5', '9', '6', '9', '7', '9', '8', '9', '9'
};

/// <summary>
/// Formats an unsigned 64-bit integer into a buffer using Radix-100 table lookup from back to front.
/// </summary>
/// <param name="end_ptr">Pointer to end of destination buffer.</param>
/// <param name="val">Unsigned 64-bit value to format.</param>
/// <returns>Pointer to first character of formatted string.</returns>
COMPAT_ALWAYS_INLINE char* FormatUIntToBuffer(char* end_ptr, uint64_t val) noexcept {
    while (val >= 100) {
        uint32_t rem = static_cast<uint32_t>(val % 100);
        val /= 100;
        end_ptr -= 2;
        end_ptr[0] = DigitsLut[rem * 2];
        end_ptr[1] = DigitsLut[rem * 2 + 1];
    }
    if (val >= 10) {
        end_ptr -= 2;
        end_ptr[0] = DigitsLut[val * 2];
        end_ptr[1] = DigitsLut[val * 2 + 1];
    } else {
        *--end_ptr = static_cast<char>('0' + val);
    }
    return end_ptr;
}

/// <summary>
/// Formats a signed 64-bit integer into a buffer using Radix-100 table lookup from back to front.
/// </summary>
/// <param name="end_ptr">Pointer to end of destination buffer.</param>
/// <param name="val">Signed 64-bit value to format.</param>
/// <returns>Pointer to first character of formatted string.</returns>
COMPAT_ALWAYS_INLINE char* FormatIntToBuffer(char* end_ptr, int64_t val) noexcept {
    uint64_t uval;
    bool negative = false;
    if (val < 0) {
        negative = true;
        uval = static_cast<uint64_t>(-(val + 1)) + 1;
    } else {
        uval = static_cast<uint64_t>(val);
    }
    char* p = FormatUIntToBuffer(end_ptr, uval);
    if (negative) {
        *--p = '-';
    }
    return p;
}

} // namespace detail

#if !COMPAT_HAS_STD_FORMAT

// Specializations of compat::formatter for primitive types

/// <summary>
/// Formatter specialization for std::string.
/// </summary>
template <>
struct formatter<std::string, char> {
    template <typename FormatContext>
    auto format(const std::string& val, FormatContext& ctx) const -> decltype(ctx.out()) {
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
    auto format(compat::string_view val, FormatContext& ctx) const -> decltype(ctx.out()) {
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
    auto format(const char* val, FormatContext& ctx) const -> decltype(ctx.out()) {
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
    auto format(char* val, FormatContext& ctx) const -> decltype(ctx.out()) {
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
    auto format(char val, FormatContext& ctx) const -> decltype(ctx.out()) {
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
    auto format(bool val, FormatContext& ctx) const -> decltype(ctx.out()) {
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
    auto format(const void* val, FormatContext& ctx) const -> decltype(ctx.out()) {
        char buf[32];
        int len = std::snprintf(buf, sizeof(buf), "%p", val);
        auto it = ctx.out();
        for (int i = 0; i < len; ++i) {
            *it++ = buf[i];
        }
        ctx.advance_to(it);
        return it;
    }
};

/// <summary>
/// Formatter specialization for void*.
/// </summary>
template <>
struct formatter<void*, char> {
    template <typename FormatContext>
    auto format(void* val, FormatContext& ctx) const -> decltype(ctx.out()) {
        return formatter<const void*, char>{}.format(val, ctx);
    }
};

#define COMPAT_DEFINE_UINT_FORMATTER(Type) template <> struct formatter<Type, char> {     template <typename FormatContext>     auto format(Type val, FormatContext& ctx) const -> decltype(ctx.out()) {         char buf[32];         char* end = buf + sizeof(buf);         char* start = detail::FormatUIntToBuffer(end, static_cast<uint64_t>(val));         auto it = ctx.out();         for (char* p = start; p < end; ++p) {             *it++ = *p;         }         ctx.advance_to(it);         return it;     } };

#define COMPAT_DEFINE_SINT_FORMATTER(Type) template <> struct formatter<Type, char> {     template <typename FormatContext>     auto format(Type val, FormatContext& ctx) const -> decltype(ctx.out()) {         char buf[32];         char* end = buf + sizeof(buf);         char* start = detail::FormatIntToBuffer(end, static_cast<int64_t>(val));         auto it = ctx.out();         for (char* p = start; p < end; ++p) {             *it++ = *p;         }         ctx.advance_to(it);         return it;     } };

COMPAT_DEFINE_SINT_FORMATTER(short)
COMPAT_DEFINE_UINT_FORMATTER(unsigned short)
COMPAT_DEFINE_SINT_FORMATTER(int)
COMPAT_DEFINE_UINT_FORMATTER(unsigned int)
COMPAT_DEFINE_SINT_FORMATTER(long)
COMPAT_DEFINE_UINT_FORMATTER(unsigned long)
COMPAT_DEFINE_SINT_FORMATTER(long long)
COMPAT_DEFINE_UINT_FORMATTER(unsigned long long)

#undef COMPAT_DEFINE_UINT_FORMATTER
#undef COMPAT_DEFINE_SINT_FORMATTER

/// <summary>
/// Formatter specialization for float.
/// </summary>
template <>
struct formatter<float, char> {
    template <typename FormatContext>
    auto format(float val, FormatContext& ctx) const -> decltype(ctx.out()) {
        char buf[64];
        int len = std::snprintf(buf, sizeof(buf), "%g", static_cast<double>(val));
        auto it = ctx.out();
        for (int i = 0; i < len; ++i) {
            *it++ = buf[i];
        }
        ctx.advance_to(it);
        return it;
    }
};

/// <summary>
/// Formatter specialization for double.
/// </summary>
template <>
struct formatter<double, char> {
    template <typename FormatContext>
    auto format(double val, FormatContext& ctx) const -> decltype(ctx.out()) {
        char buf[64];
        int len = std::snprintf(buf, sizeof(buf), "%g", val);
        auto it = ctx.out();
        for (int i = 0; i < len; ++i) {
            *it++ = buf[i];
        }
        ctx.advance_to(it);
        return it;
    }
};

/// <summary>
/// Formatter specialization for long double.
/// </summary>
template <>
struct formatter<long double, char> {
    template <typename FormatContext>
    auto format(long double val, FormatContext& ctx) const -> decltype(ctx.out()) {
        char buf[64];
        int len = std::snprintf(buf, sizeof(buf), "%Lg", val);
        auto it = ctx.out();
        for (int i = 0; i < len; ++i) {
            *it++ = buf[i];
        }
        ctx.advance_to(it);
        return it;
    }
};

/// <summary>
/// Formatter specialization for signed char (printed as integer).
/// </summary>
template <>
struct formatter<signed char, char> {
    template <typename FormatContext>
    auto format(signed char val, FormatContext& ctx) const -> decltype(ctx.out()) {
        return formatter<int, char>{}.format(static_cast<int>(val), ctx);
    }
};

/// <summary>
/// Formatter specialization for unsigned char (printed as integer).
/// </summary>
template <>
struct formatter<unsigned char, char> {
    template <typename FormatContext>
    auto format(unsigned char val, FormatContext& ctx) const -> decltype(ctx.out()) {
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
struct has_std_formatter<T, compat::detail::void_t<
    decltype(std::declval<std::formatter<typename format_arg_traits<T>::type, char>>()
        .format(std::declval<const typename format_arg_traits<T>::type&>(), std::declval<std::format_context&>()))
>> : std::true_type {};

template <typename T>
inline typename std::enable_if<has_std_formatter<T>::value, void>::type
FormatArgToBuffer(stack_buffer<512>& buf, const T& arg) {
    using FormatterType = typename format_arg_traits<T>::type;
    FormatterType formatted_arg = static_cast<FormatterType>(arg);
    std::string s = std::format("{}", formatted_arg);
    buf.append(s.data(), s.size());
}

template <typename T>
inline typename std::enable_if<!has_std_formatter<T>::value, void>::type
FormatArgToBuffer(stack_buffer<512>& buf, const T& arg) {
    std::ostringstream oss;
    oss << arg;
    std::string s = oss.str();
    buf.append(s.data(), s.size());
}

template <typename T>
inline typename std::enable_if<has_std_formatter<T>::value, void>::type
FormatArgToBufferWithSpec(stack_buffer<512>& buf, const T& arg, compat::string_view spec) {
    using FormatterType = typename format_arg_traits<T>::type;
    FormatterType formatted_arg = static_cast<FormatterType>(arg);
    std::string s;
    if (spec.empty()) {
        s = std::format("{}", formatted_arg);
    } else {
        std::string fmt_str = "{" + std::string(spec.data(), spec.size()) + "}";
        s = std::vformat(fmt_str, std::make_format_args(formatted_arg));
    }
    buf.append(s.data(), s.size());
}

template <typename T>
inline typename std::enable_if<!has_std_formatter<T>::value, void>::type
FormatArgToBufferWithSpec(stack_buffer<512>& buf, const T& arg, compat::string_view /*spec*/) {
    FormatArgToBuffer(buf, arg);
}

#else

template <typename T, typename = void>
struct has_compat_formatter : std::false_type {};

template <typename T>
struct has_compat_formatter<T, compat::detail::void_t<
    decltype(std::declval<formatter<typename format_arg_traits<T>::type, char>>()
        .format(std::declval<const typename format_arg_traits<T>::type&>(), std::declval<format_context&>()))
>> : std::true_type {};

template <typename F, typename PC, typename = void>
struct has_parse_member : std::false_type {};

template <typename F, typename PC>
struct has_parse_member<F, PC, compat::detail::void_t<
    decltype(std::declval<F&>().parse(std::declval<PC&>()))
>> : std::true_type {};

template <typename F, typename PC>
inline typename std::enable_if<has_parse_member<F, PC>::value, void>::type
CallFormatterParse(F& fmt_obj, PC& pctx) {
    fmt_obj.parse(pctx);
}

template <typename F, typename PC>
inline typename std::enable_if<!has_parse_member<F, PC>::value, void>::type
CallFormatterParse(F&, PC&) {}

// Forward declarations for concrete FormatArgToBuffer overloads
COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, const std::string& val);
COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, compat::string_view val);
COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, const char* val);
COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, char* val);
COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, char val);
COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, wchar_t val);
COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, const wchar_t* val);
COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, wchar_t* val);
COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, const std::wstring& val);
COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, char16_t val);
COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, const char16_t* val);
COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, char16_t* val);
COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, const std::u16string& val);
COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, char32_t val);
COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, const char32_t* val);
COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, char32_t* val);
COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, const std::u32string& val);
#if defined(__cpp_char8_t)
COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, char8_t val);
COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, const char8_t* val);
COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, char8_t* val);
COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, const std::u8string& val);
#endif

template <std::size_t N>
COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, const wchar_t (&val)[N]) {
    const wchar_t* p = val;
    std::size_t len = (N > 0 && val[N - 1] == L'\0') ? N - 1 : N;
#if defined(_WIN32)
    AppendUtf16ToBuffer(buf, p, p + len);
#else
    AppendUtf32ToBuffer(buf, p, p + len);
#endif
}

template <std::size_t N>
COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, wchar_t (&val)[N]) {
    FormatArgToBuffer(buf, static_cast<const wchar_t (&)[N]>(val));
}

template <std::size_t N>
COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, const char (&val)[N]) {
    std::size_t len = (N > 0 && val[N - 1] == '\0') ? N - 1 : N;
    buf.append(val, len);
}

template <std::size_t N>
COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, char (&val)[N]) {
    FormatArgToBuffer(buf, static_cast<const char (&)[N]>(val));
}

template <typename T>
inline typename std::enable_if<has_compat_formatter<T>::value, void>::type
FormatArgToBuffer(stack_buffer<512>& buf, const T& arg) {
    buffer_appender<stack_buffer<512>> app(buf);
    basic_format_context<buffer_appender<stack_buffer<512>>, char> ctx(app);
    using FormatterType = typename format_arg_traits<T>::type;
    formatter<FormatterType, char> fmt_obj;
    FormatterType formatted_arg = static_cast<FormatterType>(arg);
    fmt_obj.format(formatted_arg, ctx);
}

template <typename T>
inline typename std::enable_if<!has_compat_formatter<T>::value, void>::type
FormatArgToBuffer(stack_buffer<512>& buf, const T& arg) {
    std::ostringstream oss;
    oss << arg;
    std::string s = oss.str();
    buf.append(s.data(), s.size());
}

template <typename T>
inline typename std::enable_if<has_compat_formatter<T>::value, void>::type
FormatArgToBufferWithSpec(stack_buffer<512>& buf, const T& arg, compat::string_view spec) {
    buffer_appender<stack_buffer<512>> app(buf);
    basic_format_context<buffer_appender<stack_buffer<512>>, char> ctx(app);
    using FormatterType = typename format_arg_traits<T>::type;
    formatter<FormatterType, char> fmt_obj;
    if (!spec.empty()) {
        format_parse_context pctx(spec);
        CallFormatterParse(fmt_obj, pctx);
    }
    FormatterType formatted_arg = static_cast<FormatterType>(arg);
    fmt_obj.format(formatted_arg, ctx);
}

inline void FormatArgToBufferWithSpec(stack_buffer<512>& buf, const std::wstring& arg, compat::string_view) {
    FormatArgToBuffer(buf, arg);
}
inline void FormatArgToBufferWithSpec(stack_buffer<512>& buf, const wchar_t* arg, compat::string_view) {
    FormatArgToBuffer(buf, arg);
}
inline void FormatArgToBufferWithSpec(stack_buffer<512>& buf, wchar_t* arg, compat::string_view) {
    FormatArgToBuffer(buf, arg);
}
inline void FormatArgToBufferWithSpec(stack_buffer<512>& buf, wchar_t arg, compat::string_view) {
    FormatArgToBuffer(buf, arg);
}
template <std::size_t N>
inline void FormatArgToBufferWithSpec(stack_buffer<512>& buf, const wchar_t (&arg)[N], compat::string_view) {
    FormatArgToBuffer(buf, arg);
}
template <std::size_t N>
inline void FormatArgToBufferWithSpec(stack_buffer<512>& buf, wchar_t (&arg)[N], compat::string_view) {
    FormatArgToBuffer(buf, arg);
}
inline void FormatArgToBufferWithSpec(stack_buffer<512>& buf, const std::u16string& arg, compat::string_view) {
    FormatArgToBuffer(buf, arg);
}
inline void FormatArgToBufferWithSpec(stack_buffer<512>& buf, const char16_t* arg, compat::string_view) {
    FormatArgToBuffer(buf, arg);
}
inline void FormatArgToBufferWithSpec(stack_buffer<512>& buf, char16_t arg, compat::string_view) {
    FormatArgToBuffer(buf, arg);
}
inline void FormatArgToBufferWithSpec(stack_buffer<512>& buf, const std::u32string& arg, compat::string_view) {
    FormatArgToBuffer(buf, arg);
}
inline void FormatArgToBufferWithSpec(stack_buffer<512>& buf, const char32_t* arg, compat::string_view) {
    FormatArgToBuffer(buf, arg);
}
inline void FormatArgToBufferWithSpec(stack_buffer<512>& buf, char32_t arg, compat::string_view) {
    FormatArgToBuffer(buf, arg);
}
#if defined(__cpp_char8_t)
inline void FormatArgToBufferWithSpec(stack_buffer<512>& buf, const std::u8string& arg, compat::string_view) {
    FormatArgToBuffer(buf, arg);
}
inline void FormatArgToBufferWithSpec(stack_buffer<512>& buf, const char8_t* arg, compat::string_view) {
    FormatArgToBuffer(buf, arg);
}
inline void FormatArgToBufferWithSpec(stack_buffer<512>& buf, char8_t arg, compat::string_view) {
    FormatArgToBuffer(buf, arg);
}
#endif

template <typename T>
inline typename std::enable_if<!has_compat_formatter<T>::value, void>::type
FormatArgToBufferWithSpec(stack_buffer<512>& buf, const T& arg, compat::string_view spec) {
    if (spec.empty()) {
        FormatArgToBuffer(buf, arg);
    } else {
        std::ostringstream oss;
        oss << arg;
        std::string s = oss.str();
        buf.append(s.data(), s.size());
    }
}

#endif

// Fast-path overloads for common standard types to bypass std::format/format_context abstraction overhead
COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, const std::string& val) {
    buf.append(val.data(), val.size());
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, compat::string_view val) {
    buf.append(val.data(), val.size());
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, const char* val) {
    if (val != nullptr) {
        buf.append(val, std::strlen(val));
    }
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, char* val) {
    if (val != nullptr) {
        buf.append(val, std::strlen(val));
    }
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, char val) {
    buf.push_back(val);
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, wchar_t val) {
#if defined(_WIN32)
    uint16_t u = static_cast<uint16_t>(val);
    AppendUtf8CodePoint(buf, u);
#else
    AppendUtf8CodePoint(buf, static_cast<uint32_t>(val));
#endif
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, const wchar_t* val) {
    if (val != nullptr) {
        std::size_t len = std::wcslen(val);
#if defined(_WIN32)
        AppendUtf16ToBuffer(buf, val, val + len);
#else
        AppendUtf32ToBuffer(buf, val, val + len);
#endif
    }
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, wchar_t* val) {
    FormatArgToBuffer(buf, static_cast<const wchar_t*>(val));
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, const std::wstring& val) {
#if defined(_WIN32)
    AppendUtf16ToBuffer(buf, val.data(), val.data() + val.size());
#else
    AppendUtf32ToBuffer(buf, val.data(), val.data() + val.size());
#endif
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, char16_t val) {
    AppendUtf8CodePoint(buf, static_cast<uint16_t>(val));
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, const char16_t* val) {
    if (val != nullptr) {
        const char16_t* p = val;
        while (*p) ++p;
        AppendUtf16ToBuffer(buf, val, p);
    }
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, char16_t* val) {
    FormatArgToBuffer(buf, static_cast<const char16_t*>(val));
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, const std::u16string& val) {
    AppendUtf16ToBuffer(buf, val.data(), val.data() + val.size());
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, char32_t val) {
    AppendUtf8CodePoint(buf, static_cast<uint32_t>(val));
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, const char32_t* val) {
    if (val != nullptr) {
        const char32_t* p = val;
        while (*p) ++p;
        AppendUtf32ToBuffer(buf, val, p);
    }
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, char32_t* val) {
    FormatArgToBuffer(buf, static_cast<const char32_t*>(val));
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, const std::u32string& val) {
    AppendUtf32ToBuffer(buf, val.data(), val.data() + val.size());
}

#if defined(__cpp_char8_t)
COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, char8_t val) {
    buf.push_back(static_cast<char>(val));
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, const char8_t* val) {
    if (val != nullptr) {
        const char8_t* p = val;
        while (*p) ++p;
        buf.append(reinterpret_cast<const char*>(val), static_cast<std::size_t>(p - val));
    }
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, char8_t* val) {
    FormatArgToBuffer(buf, static_cast<const char8_t*>(val));
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, const std::u8string& val) {
    buf.append(reinterpret_cast<const char*>(val.data()), val.size());
}
#endif

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, bool val) {
    if (val) {
        buf.append("true", 4);
    } else {
        buf.append("false", 5);
    }
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, signed char arg) {
    char sbuf[32];
    char* end = sbuf + sizeof(sbuf);
    char* start = FormatIntToBuffer(end, static_cast<int64_t>(arg));
    buf.append(start, static_cast<std::size_t>(end - start));
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, unsigned char arg) {
    char sbuf[32];
    char* end = sbuf + sizeof(sbuf);
    char* start = FormatUIntToBuffer(end, static_cast<uint64_t>(arg));
    buf.append(start, static_cast<std::size_t>(end - start));
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, short val) {
    char sbuf[32];
    char* end = sbuf + sizeof(sbuf);
    char* start = FormatIntToBuffer(end, static_cast<int64_t>(val));
    buf.append(start, static_cast<std::size_t>(end - start));
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, unsigned short val) {
    char sbuf[32];
    char* end = sbuf + sizeof(sbuf);
    char* start = FormatUIntToBuffer(end, static_cast<uint64_t>(val));
    buf.append(start, static_cast<std::size_t>(end - start));
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, int val) {
    char sbuf[32];
    char* end = sbuf + sizeof(sbuf);
    char* start = FormatIntToBuffer(end, static_cast<int64_t>(val));
    buf.append(start, static_cast<std::size_t>(end - start));
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, unsigned int val) {
    char sbuf[32];
    char* end = sbuf + sizeof(sbuf);
    char* start = FormatUIntToBuffer(end, static_cast<uint64_t>(val));
    buf.append(start, static_cast<std::size_t>(end - start));
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, long val) {
    char sbuf[32];
    char* end = sbuf + sizeof(sbuf);
    char* start = FormatIntToBuffer(end, static_cast<int64_t>(val));
    buf.append(start, static_cast<std::size_t>(end - start));
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, unsigned long val) {
    char sbuf[32];
    char* end = sbuf + sizeof(sbuf);
    char* start = FormatUIntToBuffer(end, static_cast<uint64_t>(val));
    buf.append(start, static_cast<std::size_t>(end - start));
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, long long val) {
    char sbuf[32];
    char* end = sbuf + sizeof(sbuf);
    char* start = FormatIntToBuffer(end, static_cast<int64_t>(val));
    buf.append(start, static_cast<std::size_t>(end - start));
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, unsigned long long val) {
    char sbuf[32];
    char* end = sbuf + sizeof(sbuf);
    char* start = FormatUIntToBuffer(end, val);
    buf.append(start, static_cast<std::size_t>(end - start));
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, float val) {
    char sbuf[64];
    int len = std::snprintf(sbuf, sizeof(sbuf), "%g", static_cast<double>(val));
    if (len > 0) {
        buf.append(sbuf, static_cast<std::size_t>(len));
    }
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, double val) {
    char sbuf[64];
    int len = std::snprintf(sbuf, sizeof(sbuf), "%g", val);
    if (len > 0) {
        buf.append(sbuf, static_cast<std::size_t>(len));
    }
}

COMPAT_ALWAYS_INLINE void FormatArgToBuffer(stack_buffer<512>& buf, long double val) {
    char sbuf[64];
    int len = std::snprintf(sbuf, sizeof(sbuf), "%Lg", val);
    if (len > 0) {
        buf.append(sbuf, static_cast<std::size_t>(len));
    }
}

/// <summary>
/// Formats an argument to an output stream via stack buffer to minimize stream operations.
/// </summary>
/// <typeparam name="T">Argument type.</typeparam>
/// <param name="os">Target output stream.</param>
/// <param name="arg">Argument to format.</param>
template <typename T>
inline void StreamFormatArg(std::ostream& os, const T& arg) {
    stack_buffer<512> buf;
    FormatArgToBuffer(buf, arg);
    os.write(buf.data(), static_cast<std::streamsize>(buf.size()));
}

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
            } else {
                std::size_t close = i + 1;
                while (close < fmt.size() && fmt[close] != '}') {
                    if (fmt[close] == '{') {
                        throw std::invalid_argument("Unmatched '{' in format string");
                    }
                    ++close;
                }
                if (close >= fmt.size()) {
                    throw std::invalid_argument("Unmatched '{' in format string");
                }
                ++count;
                i = close + 1;
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
/// Base case for recursive template format string writer into stack_buffer.
/// </summary>
/// <param name="buf">Target stack buffer.</param>
/// <param name="fmt">Format string view.</param>
inline void WriteFormattedBufferImpl(stack_buffer<512>& buf, compat::string_view fmt) {
    std::size_t i = 0;
    std::size_t start = 0;
    while (i < fmt.size()) {
        if (fmt[i] == '{') {
            if (i + 1 < fmt.size() && fmt[i + 1] == '{') {
                if (i > start) {
                    buf.append(fmt.data() + start, i - start);
                }
                buf.push_back('{');
                i += 2;
                start = i;
            } else {
                std::size_t close = i + 1;
                while (close < fmt.size() && fmt[close] != '}') {
                    ++close;
                }
                if (close < fmt.size()) {
                    throw std::invalid_argument("Too few arguments for format string");
                } else {
                    ++i;
                }
            }
        } else if (fmt[i] == '}') {
            if (i + 1 < fmt.size() && fmt[i + 1] == '}') {
                if (i > start) {
                    buf.append(fmt.data() + start, i - start);
                }
                buf.push_back('}');
                i += 2;
                start = i;
            } else {
                ++i;
            }
        } else {
            ++i;
        }
    }
    if (i > start) {
        buf.append(fmt.data() + start, i - start);
    }
}

/// <summary>
/// Recursive template format string writer substituting {} placeholders into stack_buffer.
/// </summary>
/// <typeparam name="First">First argument type.</typeparam>
/// <typeparam name="Rest">Remaining argument types.</typeparam>
/// <param name="buf">Target stack buffer.</param>
/// <param name="fmt">Format string view.</param>
/// <param name="first">First argument.</param>
/// <param name="rest">Remaining arguments.</param>
template <typename First, typename... Rest>
inline void WriteFormattedBufferImpl(stack_buffer<512>& buf, compat::string_view fmt, const First& first, const Rest&... rest) {
    std::size_t i = 0;
    std::size_t start = 0;
    while (i < fmt.size()) {
        if (fmt[i] == '{') {
            if (i + 1 < fmt.size() && fmt[i + 1] == '{') {
                if (i > start) {
                    buf.append(fmt.data() + start, i - start);
                }
                buf.push_back('{');
                i += 2;
                start = i;
            } else {
                std::size_t close = i + 1;
                while (close < fmt.size() && fmt[close] != '}') {
                    ++close;
                }
                if (close < fmt.size()) {
                    if (i > start) {
                        buf.append(fmt.data() + start, i - start);
                    }
                    compat::string_view spec = (close > i + 1) ? fmt.substr(i + 1, close - (i + 1)) : compat::string_view{};
                    FormatArgToBufferWithSpec(buf, first, spec);
                    WriteFormattedBufferImpl(buf, fmt.substr(close + 1), rest...);
                    return;
                } else {
                    ++i;
                }
            }
        } else if (fmt[i] == '}') {
            if (i + 1 < fmt.size() && fmt[i + 1] == '}') {
                if (i > start) {
                    buf.append(fmt.data() + start, i - start);
                }
                buf.push_back('}');
                i += 2;
                start = i;
            } else {
                ++i;
            }
        } else {
            ++i;
        }
    }
    throw std::invalid_argument("Too many arguments for format string");
}

/// <summary>
/// Validates placeholder count against argument count and writes formatted output to stack buffer.
/// </summary>
/// <typeparam name="Args">Types of arguments.</typeparam>
/// <param name="buf">Target stack buffer.</param>
/// <param name="fmt">Format string view.</param>
/// <param name="args">Arguments to format.</param>
/// <exception cref="std::invalid_argument">Thrown if placeholder count does not match argument count or on malformed braces.</exception>
template <typename... Args>
inline void WriteFormattedBuffer(stack_buffer<512>& buf, compat::string_view fmt, const Args&... args) {
#if defined(_WIN32)
    std::string converted_fmt;
    if (COMPAT_UNLIKELY(!IsValidUtf8(fmt))) {
        converted_fmt = AcpToUtf8(fmt);
        fmt = compat::string_view(converted_fmt.data(), converted_fmt.size());
    }
#endif
    constexpr std::size_t num_args = sizeof...(Args);
    std::size_t num_placeholders = CountAndValidatePlaceholders(fmt);
    if (num_placeholders < num_args) {
        throw std::invalid_argument("Too many arguments for format string");
    }
    if (num_placeholders > num_args) {
        throw std::invalid_argument("Too few arguments for format string");
    }
    WriteFormattedBufferImpl(buf, fmt, args...);
}

/// <summary>
/// Validates placeholder count against argument count and writes formatted output to stream via stack_buffer.
/// </summary>
/// <typeparam name="Args">Types of arguments.</typeparam>
/// <param name="os">Target output stream.</param>
/// <param name="fmt">Format string view.</param>
/// <param name="args">Arguments to format.</param>
/// <exception cref="std::invalid_argument">Thrown if placeholder count does not match argument count or on malformed braces.</exception>
template <typename... Args>
inline void WriteFormatted(std::ostream& os, compat::string_view fmt, const Args&... args) {
    stack_buffer<512> buf;
    WriteFormattedBuffer(buf, fmt, args...);
    os.write(buf.data(), static_cast<std::streamsize>(buf.size()));
}

/// <summary>
/// Writes UTF-8 encoded string view to specified FILE* stream, converting to UTF-16 via WriteConsoleW if attached to a Windows console.
/// </summary>
/// <param name="stream">Target FILE* stream.</param>
/// <param name="text">UTF-8 encoded string view to write.</param>
/// <exception cref="std::invalid_argument">Thrown if stream is nullptr.</exception>
inline void WriteFileUtf8(std::FILE* stream, compat::string_view text) {
    if (COMPAT_UNLIKELY(stream == nullptr)) {
        COMPAT_THROW_OR_ABORT(std::invalid_argument("Target FILE* stream cannot be null"));
        return;
    }
    if (text.empty()) {
        return;
    }
#if defined(_WIN32)
    EnsureConsoleUtf8();
    int fd = _fileno(stream);
    if (fd >= 0 && _isatty(fd)) {
        intptr_t osfh = _get_osfhandle(fd);
        if (osfh != -1) {
            HANDLE hConsole = reinterpret_cast<HANDLE>(osfh);
            DWORD mode = 0;
            if (hConsole != INVALID_HANDLE_VALUE && hConsole != NULL && GetConsoleMode(hConsole, &mode)) {
#ifndef MB_ERR_INVALID_CHARS
#  define MB_ERR_INVALID_CHARS 0x00000008
#endif
                UINT cp_used = CP_UTF8;
                int wide_len = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(), static_cast<int>(text.size()), NULL, 0);
                if (wide_len <= 0) {
                    wide_len = MultiByteToWideChar(CP_ACP, 0, text.data(), static_cast<int>(text.size()), NULL, 0);
                    cp_used = CP_ACP;
                }
                if (wide_len > 0) {
                    wchar_t stack_wbuf[256];
                    wchar_t* wptr = stack_wbuf;
                    std::wstring heap_wbuf;
                    if (static_cast<std::size_t>(wide_len) > 256) {
                        heap_wbuf.resize(static_cast<std::size_t>(wide_len));
                        wptr = &heap_wbuf[0];
                    }
                    if (MultiByteToWideChar(cp_used, (cp_used == CP_UTF8 ? MB_ERR_INVALID_CHARS : 0), text.data(), static_cast<int>(text.size()), wptr, wide_len) <= 0) {
                        if (cp_used == CP_UTF8) {
                            wide_len = MultiByteToWideChar(CP_ACP, 0, text.data(), static_cast<int>(text.size()), wptr, wide_len);
                        }
                    }
                    if (wide_len > 0) {
                        DWORD written = 0;
                        WriteConsoleW(hConsole, wptr, static_cast<DWORD>(wide_len), &written, NULL);
                        return;
                    }
                }
            }
        }
    }
#endif
    std::fwrite(text.data(), 1, text.size(), stream);
    if (stream == stdout || stream == stderr) {
        std::fflush(stream);
    }
}

/// <summary>
/// Writes UTF-8 encoded string view to stdout, converting to UTF-16 via WriteConsoleW if attached to a Windows console.
/// </summary>
/// <param name="text">UTF-8 encoded string view to write.</param>
inline void WriteStdoutUtf8(compat::string_view text) {
    WriteFileUtf8(stdout, text);
}

} // namespace detail
} // namespace compat
