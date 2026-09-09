#pragma once

#include "../Config.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <ostream>
#include <stdexcept>
#include <algorithm>
#include <functional>

namespace compat {
namespace detail {
inline namespace COMPAT_ABI_TAG {

namespace string_view_helper {
#if (COMPAT_CPLUSPLUS >= COMPAT_CXX_14)
    inline constexpr std::size_t ConstexprStrlen(const char* s) noexcept {
        if (!s) return 0;
        std::size_t len = 0;
        while (s[len] != '\0') ++len;
        return len;
    }
#else
    inline constexpr std::size_t ConstexprStrlen(const char* s) noexcept {
        return (!s || *s == '\0') ? 0 : (1 + ConstexprStrlen(s + 1));
    }
#endif

    /// <summary>
    /// Constexpr-compatible lexicographical byte comparison helper for C++14 and later.
    /// Uses std::memcmp at runtime for maximum SIMD throughput and constexpr loop at compile time.
    /// </summary>
    /// <param name="s1">First memory buffer.</param>
    /// <param name="s2">Second memory buffer.</param>
    /// <param name="n">Number of bytes to compare.</param>
    /// <returns>Negative if s1 &lt; s2, 0 if equal, positive if s1 &gt; s2.</returns>
    inline COMPAT_CONSTEXPR_14 int32_t ConstexprMemcmp(const char* s1, const char* s2, std::size_t n) noexcept {
#if defined(__cpp_lib_is_constant_evaluated) && (__cpp_lib_is_constant_evaluated >= 201811L)
        if (std::is_constant_evaluated()) {
            for (std::size_t i = 0; i < n; ++i) {
                unsigned char c1 = static_cast<unsigned char>(s1[i]);
                unsigned char c2 = static_cast<unsigned char>(s2[i]);
                if (c1 < c2) return -1;
                if (c1 > c2) return 1;
            }
            return 0;
        }
#elif defined(__has_builtin)
#  if __has_builtin(__builtin_is_constant_evaluated)
        if (__builtin_is_constant_evaluated()) {
            for (std::size_t i = 0; i < n; ++i) {
                unsigned char c1 = static_cast<unsigned char>(s1[i]);
                unsigned char c2 = static_cast<unsigned char>(s2[i]);
                if (c1 < c2) return -1;
                if (c1 > c2) return 1;
            }
            return 0;
        }
#  endif
#endif
        return (n > 0) ? std::memcmp(s1, s2, n) : 0;
    }
} // namespace string_view_helper

/// <summary>
/// Lightweight non-owning view of a character buffer, compatible with C++11 and later.
/// </summary>
class string_view {
public:
    using value_type = char;
    using pointer = char*;
    using const_pointer = const char*;
    using reference = char&;
    using const_reference = const char&;
    using const_iterator = const char*;
    using iterator = const_iterator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    static constexpr size_type npos = static_cast<size_type>(-1);

    /// <summary>
    /// Constructs an empty string_view.
    /// </summary>
    constexpr string_view() noexcept
        : m_data(nullptr), m_size(0) {}

    /// <summary>
    /// Constructs a string_view from a null-terminated C-style string.
    /// </summary>
    /// <param name="str">Null-terminated string pointer.</param>
    constexpr string_view(const char* str) noexcept
        : m_data(str), m_size(str ? string_view_helper::ConstexprStrlen(str) : 0) {}

    /// <summary>
    /// Constructs a string_view from a character buffer and length.
    /// </summary>
    /// <param name="data">Pointer to character buffer.</param>
    /// <param name="size">Number of characters in the buffer.</param>
    constexpr string_view(const char* data, size_type size) noexcept
        : m_data(data), m_size(size) {}

    /// <summary>
    /// Constructs a string_view from a std::string.
    /// </summary>
    /// <param name="str">Source std::string.</param>
    string_view(const std::string& str) noexcept
        : m_data(str.data()), m_size(str.size()) {}

    /// <summary>
    /// Returns a pointer to the beginning of the viewed buffer.
    /// </summary>
    /// <returns>Pointer to character data.</returns>
    constexpr const_pointer data() const noexcept {
        return m_data;
    }

    /// <summary>
    /// Returns the number of characters in the view.
    /// </summary>
    /// <returns>Number of characters.</returns>
    constexpr size_type size() const noexcept {
        return m_size;
    }

    /// <summary>
    /// Returns the number of characters in the view (alias for size).
    /// </summary>
    /// <returns>Number of characters.</returns>
    constexpr size_type length() const noexcept {
        return m_size;
    }

    /// <summary>
    /// Checks if the view contains no characters.
    /// </summary>
    /// <returns>True if empty, false otherwise.</returns>
    constexpr bool empty() const noexcept {
        return m_size == 0;
    }

    /// <summary>
    /// Returns a const reference to the character at the given index without bounds checking.
    /// </summary>
    /// <param name="pos">Zero-based character index.</param>
    /// <returns>Const reference to character.</returns>
    constexpr const_reference operator[](size_type pos) const noexcept {
        return m_data[pos];
    }

    /// <summary>
    /// Returns a const reference to the character at the given index with bounds checking.
    /// </summary>
    /// <param name="pos">Zero-based character index.</param>
    /// <returns>Const reference to character.</returns>
    /// <exception cref="std::out_of_range">Thrown if pos is greater than or equal to size().</exception>
    COMPAT_CONSTEXPR_14 const_reference at(size_type pos) const {
        if (pos >= m_size) {
            COMPAT_THROW_OR_ABORT(std::out_of_range("compat::string_view::at out of range"));
        }
        return m_data[pos];
    }

    /// <summary>
    /// Returns a const reference to the first character.
    /// </summary>
    /// <returns>Const reference to first character.</returns>
    constexpr const_reference front() const noexcept {
        return m_data[0];
    }

    /// <summary>
    /// Returns a const reference to the last character.
    /// </summary>
    /// <returns>Const reference to last character.</returns>
    constexpr const_reference back() const noexcept {
        return m_data[m_size - 1];
    }

    /// <summary>
    /// Returns an iterator to the beginning of the view.
    /// </summary>
    /// <returns>Const iterator pointing to start.</returns>
    constexpr const_iterator begin() const noexcept {
        return m_data;
    }

    /// <summary>
    /// Returns an iterator to the end of the view.
    /// </summary>
    /// <returns>Const iterator pointing past the end.</returns>
    constexpr const_iterator end() const noexcept {
        return m_data + m_size;
    }

    /// <summary>
    /// Returns a const iterator to the beginning of the view.
    /// </summary>
    /// <returns>Const iterator pointing to start.</returns>
    constexpr const_iterator cbegin() const noexcept {
        return m_data;
    }

    /// <summary>
    /// Returns a const iterator to the end of the view.
    /// </summary>
    /// <returns>Const iterator pointing past the end.</returns>
    constexpr const_iterator cend() const noexcept {
        return m_data + m_size;
    }

    /// <summary>
    /// Shrinks the view from the front by n characters.
    /// </summary>
    /// <param name="n">Number of characters to remove.</param>
    COMPAT_CONSTEXPR_14 void remove_prefix(size_type n) noexcept {
        m_data += n;
        m_size -= n;
    }

    /// <summary>
    /// Shrinks the view from the back by n characters.
    /// </summary>
    /// <param name="n">Number of characters to remove.</param>
    COMPAT_CONSTEXPR_14 void remove_suffix(size_type n) noexcept {
        m_size -= n;
    }

    /// <summary>
    /// Swaps this string_view with another.
    /// </summary>
    /// <param name="s">Another string_view.</param>
    COMPAT_CONSTEXPR_14 void swap(string_view& s) noexcept {
        const char* d = m_data;
        m_data = s.m_data;
        s.m_data = d;
        size_type sz = m_size;
        m_size = s.m_size;
        s.m_size = sz;
    }

    /// <summary>
    /// Returns a subview starting at pos of length count.
    /// </summary>
    /// <param name="pos">Start offset.</param>
    /// <param name="count">Requested length or npos for entire remainder.</param>
    /// <returns>A new string_view covering the requested substring.</returns>
    /// <exception cref="std::out_of_range">Thrown if pos is greater than size().</exception>
    COMPAT_CONSTEXPR_14 string_view substr(size_type pos = 0, size_type count = npos) const {
        if (pos > m_size) {
            COMPAT_THROW_OR_ABORT(std::out_of_range("compat::string_view::substr out of range"));
        }
        size_type rcount = (count > m_size - pos) ? (m_size - pos) : count;
        return string_view(m_data + pos, rcount);
    }

    /// <summary>
    /// Finds the first occurrence of another string_view.
    /// </summary>
    /// <param name="v">Substring to find.</param>
    /// <param name="pos">Offset to begin search.</param>
    /// <returns>Position of match, or npos if not found.</returns>
    COMPAT_CONSTEXPR_14 size_type find(string_view v, size_type pos = 0) const noexcept {
        if (v.m_size == 0) {
            return pos <= m_size ? pos : npos;
        }
        if (pos >= m_size || v.m_size > m_size - pos) {
            return npos;
        }
        for (size_type i = pos; i <= m_size - v.m_size; ++i) {
            if (string_view_helper::ConstexprMemcmp(m_data + i, v.m_data, v.m_size) == 0) {
                return i;
            }
        }
        return npos;
    }

    /// <summary>
    /// Finds the first occurrence of a character.
    /// </summary>
    /// <param name="c">Character to find.</param>
    /// <param name="pos">Offset to begin search.</param>
    /// <returns>Position of match, or npos if not found.</returns>
    COMPAT_CONSTEXPR_14 size_type find(char c, size_type pos = 0) const noexcept {
        for (size_type i = pos; i < m_size; ++i) {
            if (m_data[i] == c) {
                return i;
            }
        }
        return npos;
    }

    /// <summary>
    /// Finds the first occurrence of a buffer.
    /// </summary>
    /// <param name="s">Pointer to character buffer.</param>
    /// <param name="pos">Offset to begin search.</param>
    /// <param name="count">Length of buffer.</param>
    /// <returns>Position of match, or npos if not found.</returns>
    COMPAT_CONSTEXPR_14 size_type find(const char* s, size_type pos, size_type count) const noexcept {
        return find(string_view(s, count), pos);
    }

    /// <summary>
    /// Finds the first occurrence of a null-terminated string.
    /// </summary>
    /// <param name="s">Null-terminated string.</param>
    /// <param name="pos">Offset to begin search.</param>
    /// <returns>Position of match, or npos if not found.</returns>
    COMPAT_CONSTEXPR_14 size_type find(const char* s, size_type pos = 0) const noexcept {
        return s ? find(string_view(s), pos) : npos;
    }

    /// <summary>
    /// Finds the last occurrence of another string_view.
    /// </summary>
    /// <param name="v">Substring to find.</param>
    /// <param name="pos">Offset at which to begin searching backwards.</param>
    /// <returns>Position of match, or npos if not found.</returns>
    COMPAT_CONSTEXPR_14 size_type rfind(string_view v, size_type pos = npos) const noexcept {
        if (v.m_size == 0) {
            return pos < m_size ? pos : m_size;
        }
        if (m_size < v.m_size) {
            return npos;
        }
        size_type cur = (pos < m_size - v.m_size) ? pos : (m_size - v.m_size);
        while (true) {
            if (string_view_helper::ConstexprMemcmp(m_data + cur, v.m_data, v.m_size) == 0) {
                return cur;
            }
            if (cur == 0) {
                break;
            }
            --cur;
        }
        return npos;
    }

    /// <summary>
    /// Finds the last occurrence of a character.
    /// </summary>
    /// <param name="c">Character to find.</param>
    /// <param name="pos">Offset at which to begin searching backwards.</param>
    /// <returns>Position of match, or npos if not found.</returns>
    COMPAT_CONSTEXPR_14 size_type rfind(char c, size_type pos = npos) const noexcept {
        if (m_size == 0) {
            return npos;
        }
        size_type cur = (pos < m_size) ? pos : (m_size - 1);
        while (true) {
            if (m_data[cur] == c) {
                return cur;
            }
            if (cur == 0) {
                break;
            }
            --cur;
        }
        return npos;
    }

    /// <summary>
    /// Finds the last occurrence of a character buffer.
    /// </summary>
    /// <param name="s">Pointer to character buffer.</param>
    /// <param name="pos">Offset at which to begin searching backwards.</param>
    /// <param name="count">Length of buffer.</param>
    /// <returns>Position of match, or npos if not found.</returns>
    COMPAT_CONSTEXPR_14 size_type rfind(const char* s, size_type pos, size_type count) const noexcept {
        return rfind(string_view(s, count), pos);
    }

    /// <summary>
    /// Finds the last occurrence of a null-terminated string.
    /// </summary>
    /// <param name="s">Null-terminated string.</param>
    /// <param name="pos">Offset at which to begin searching backwards.</param>
    /// <returns>Position of match, or npos if not found.</returns>
    COMPAT_CONSTEXPR_14 size_type rfind(const char* s, size_type pos = npos) const noexcept {
        return s ? rfind(string_view(s), pos) : npos;
    }

    /// <summary>
    /// Compares two string_views lexicographically.
    /// </summary>
    /// <param name="other">The other string_view to compare.</param>
    /// <returns>Negative if *this &lt; other, 0 if equal, positive if *this &gt; other.</returns>
    COMPAT_CONSTEXPR_14 int32_t compare(string_view other) const noexcept {
        size_type rlen = (m_size < other.m_size) ? m_size : other.m_size;
        int32_t res = (rlen > 0) ? string_view_helper::ConstexprMemcmp(m_data, other.m_data, rlen) : 0;
        if (res != 0) {
            return res < 0 ? -1 : 1;
        }
        if (m_size < other.m_size) return -1;
        if (m_size > other.m_size) return 1;
        return 0;
    }

    /// <summary>
    /// Checks if the view starts with the given prefix.
    /// </summary>
    /// <param name="sv">Prefix to check.</param>
    /// <returns>True if view starts with prefix, false otherwise.</returns>
    COMPAT_CONSTEXPR_14 bool starts_with(string_view sv) const noexcept {
        return m_size >= sv.m_size && string_view_helper::ConstexprMemcmp(m_data, sv.m_data, sv.m_size) == 0;
    }

    /// <summary>
    /// Checks if the view starts with the given character.
    /// </summary>
    /// <param name="c">Prefix character to check.</param>
    /// <returns>True if view starts with prefix character, false otherwise.</returns>
    COMPAT_CONSTEXPR_14 bool starts_with(char c) const noexcept {
        return !empty() && front() == c;
    }

    /// <summary>
    /// Checks if the view ends with the given suffix.
    /// </summary>
    /// <param name="sv">Suffix to check.</param>
    /// <returns>True if view ends with suffix, false otherwise.</returns>
    COMPAT_CONSTEXPR_14 bool ends_with(string_view sv) const noexcept {
        return m_size >= sv.m_size && string_view_helper::ConstexprMemcmp(m_data + (m_size - sv.m_size), sv.m_data, sv.m_size) == 0;
    }

    /// <summary>
    /// Checks if the view ends with the given character.
    /// </summary>
    /// <param name="c">Suffix character to check.</param>
    /// <returns>True if view ends with suffix character, false otherwise.</returns>
    COMPAT_CONSTEXPR_14 bool ends_with(char c) const noexcept {
        return !empty() && back() == c;
    }

    /// <summary>
    /// Explicitly converts the view to a std::string copy.
    /// </summary>
    /// <returns>A new std::string copy.</returns>
    explicit operator std::string() const {
        return std::string(m_data ? m_data : "", m_size);
    }

    /// <summary>
    /// Converts the view to a std::string copy.
    /// </summary>
    /// <returns>A new std::string copy.</returns>
    std::string to_string() const {
        return std::string(m_data ? m_data : "", m_size);
    }

private:
    const char* m_data;
    size_type m_size;
};

/// <summary>
/// Equality comparison between two string_views.
/// </summary>
COMPAT_CONSTEXPR_14 inline bool operator==(string_view lhs, string_view rhs) noexcept {
    return lhs.size() == rhs.size() &&
           (lhs.size() == 0 || string_view_helper::ConstexprMemcmp(lhs.data(), rhs.data(), lhs.size()) == 0);
}

/// <summary>
/// Inequality comparison between two string_views.
/// </summary>
COMPAT_CONSTEXPR_14 inline bool operator!=(string_view lhs, string_view rhs) noexcept {
    return !(lhs == rhs);
}

/// <summary>
/// Less-than comparison between two string_views.
/// </summary>
COMPAT_CONSTEXPR_14 inline bool operator<(string_view lhs, string_view rhs) noexcept {
    return lhs.compare(rhs) < 0;
}

/// <summary>
/// Less-than-or-equal comparison between two string_views.
/// </summary>
COMPAT_CONSTEXPR_14 inline bool operator<=(string_view lhs, string_view rhs) noexcept {
    return lhs.compare(rhs) <= 0;
}

/// <summary>
/// Greater-than comparison between two string_views.
/// </summary>
COMPAT_CONSTEXPR_14 inline bool operator>(string_view lhs, string_view rhs) noexcept {
    return lhs.compare(rhs) > 0;
}

/// <summary>
/// Greater-than-or-equal comparison between two string_views.
/// </summary>
COMPAT_CONSTEXPR_14 inline bool operator>=(string_view lhs, string_view rhs) noexcept {
    return lhs.compare(rhs) >= 0;
}

/// <summary>
/// Equality comparison between string_view and C-style string.
/// </summary>
COMPAT_CONSTEXPR_14 inline bool operator==(string_view lhs, const char* rhs) noexcept {
    return lhs == string_view(rhs);
}

/// <summary>
/// Equality comparison between C-style string and string_view.
/// </summary>
COMPAT_CONSTEXPR_14 inline bool operator==(const char* lhs, string_view rhs) noexcept {
    return string_view(lhs) == rhs;
}

/// <summary>
/// Inequality comparison between string_view and C-style string.
/// </summary>
COMPAT_CONSTEXPR_14 inline bool operator!=(string_view lhs, const char* rhs) noexcept {
    return !(lhs == rhs);
}

/// <summary>
/// Inequality comparison between C-style string and string_view.
/// </summary>
COMPAT_CONSTEXPR_14 inline bool operator!=(const char* lhs, string_view rhs) noexcept {
    return !(lhs == rhs);
}

/// <summary>
/// Equality comparison between string_view and std::string.
/// </summary>
inline bool operator==(string_view lhs, const std::string& rhs) noexcept {
    return lhs == string_view(rhs);
}

/// <summary>
/// Equality comparison between std::string and string_view.
/// </summary>
inline bool operator==(const std::string& lhs, string_view rhs) noexcept {
    return string_view(lhs) == rhs;
}

/// <summary>
/// Inequality comparison between string_view and std::string.
/// </summary>
inline bool operator!=(string_view lhs, const std::string& rhs) noexcept {
    return !(lhs == rhs);
}

/// <summary>
/// Inequality comparison between std::string and string_view.
/// </summary>
inline bool operator!=(const std::string& lhs, string_view rhs) noexcept {
    return !(lhs == rhs);
}

/// <summary>
/// Writes string_view contents to an output stream.
/// </summary>
/// <param name="os">Target output stream.</param>
/// <param name="sv">Source string_view.</param>
/// <returns>Reference to the output stream.</returns>
inline std::ostream& operator<<(std::ostream& os, string_view sv) {
    if (sv.data() && sv.size() > 0) {
        os.write(sv.data(), static_cast<std::streamsize>(sv.size()));
    }
    return os;
}

} // namespace COMPAT_ABI_TAG
} // namespace detail
} // namespace compat

namespace std {
    /// <summary>
    /// std::hash specialization for compat::detail::string_view using FNV-1a.
    /// </summary>
    template <>
    struct hash<compat::detail::string_view> {
        std::size_t operator()(compat::detail::string_view sv) const noexcept {
            std::size_t hash_val = 2166136261u;
            for (std::size_t i = 0; i < sv.size(); ++i) {
                hash_val ^= static_cast<std::size_t>(static_cast<unsigned char>(sv[i]));
                hash_val *= 16777619u;
            }
            return hash_val;
        }
    };
} // namespace std
