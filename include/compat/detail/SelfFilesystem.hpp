#pragma once

#include "../Config.hpp"

#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <system_error>
#include <stdexcept>
#include <algorithm>
#include <utility>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>

#if defined(_WIN32)
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#else
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <dirent.h>
#  include <unistd.h>
#  include <sys/statvfs.h>
#  include <errno.h>
#endif

namespace compat {
namespace filesystem {

// ============================================================================
// 1. 列舉與位元遮罩 (Enumerations & Bitmasks)
// ============================================================================

/// <summary>
/// 檔案型態列舉。
/// </summary>
enum class file_type {
    none = 0,
    not_found = -1,
    regular = 1,
    directory = 2,
    symlink = 3,
    block = 4,
    character = 5,
    fifo = 6,
    socket = 7,
    unknown = 8
};

/// <summary>
/// 檔案存取權限位元遮罩。
/// </summary>
enum class perms : uint32_t {
    none = 0,
    owner_read = 0400,
    owner_write = 0200,
    owner_exec = 0100,
    owner_all = 0700,
    group_read = 0040,
    group_write = 0020,
    group_exec = 0010,
    group_all = 0070,
    others_read = 0004,
    others_write = 0002,
    others_exec = 0001,
    others_all = 0007,
    all = 0777,
    set_uid = 04000,
    set_gid = 02000,
    sticky_bit = 01000,
    mask = 07777,
    unknown = 0xFFFF
};

constexpr perms operator&(perms a, perms b) noexcept {
    return static_cast<perms>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}
constexpr perms operator|(perms a, perms b) noexcept {
    return static_cast<perms>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
constexpr perms operator^(perms a, perms b) noexcept {
    return static_cast<perms>(static_cast<uint32_t>(a) ^ static_cast<uint32_t>(b));
}
constexpr perms operator~(perms a) noexcept {
    return static_cast<perms>(~static_cast<uint32_t>(a));
}
inline perms& operator&=(perms& a, perms b) noexcept {
    a = a & b;
    return a;
}
inline perms& operator|=(perms& a, perms b) noexcept {
    a = a | b;
    return a;
}
inline perms& operator^=(perms& a, perms b) noexcept {
    a = a ^ b;
    return a;
}

/// <summary>
/// 權限操作選項。
/// </summary>
enum class perm_options : uint32_t {
    replace = 1,
    add = 2,
    remove = 4,
    nofollow = 8
};

constexpr perm_options operator&(perm_options a, perm_options b) noexcept {
    return static_cast<perm_options>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}
constexpr perm_options operator|(perm_options a, perm_options b) noexcept {
    return static_cast<perm_options>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
constexpr perm_options operator^(perm_options a, perm_options b) noexcept {
    return static_cast<perm_options>(static_cast<uint32_t>(a) ^ static_cast<uint32_t>(b));
}
constexpr perm_options operator~(perm_options a) noexcept {
    return static_cast<perm_options>(~static_cast<uint32_t>(a));
}
inline perm_options& operator&=(perm_options& a, perm_options b) noexcept {
    a = a & b;
    return a;
}
inline perm_options& operator|=(perm_options& a, perm_options b) noexcept {
    a = a | b;
    return a;
}
inline perm_options& operator^=(perm_options& a, perm_options b) noexcept {
    a = a ^ b;
    return a;
}

/// <summary>
/// 複製行為選項。
/// </summary>
enum class copy_options : uint32_t {
    none = 0,
    skip_existing = 1,
    overwrite_existing = 2,
    update_existing = 4,
    recursive = 8,
    copy_symlinks = 16,
    skip_symlinks = 32,
    directories_only = 64,
    create_symlinks = 128,
    create_hard_links = 256
};

constexpr copy_options operator&(copy_options a, copy_options b) noexcept {
    return static_cast<copy_options>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}
constexpr copy_options operator|(copy_options a, copy_options b) noexcept {
    return static_cast<copy_options>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
constexpr copy_options operator^(copy_options a, copy_options b) noexcept {
    return static_cast<copy_options>(static_cast<uint32_t>(a) ^ static_cast<uint32_t>(b));
}
constexpr copy_options operator~(copy_options a) noexcept {
    return static_cast<copy_options>(~static_cast<uint32_t>(a));
}
inline copy_options& operator&=(copy_options& a, copy_options b) noexcept {
    a = a & b;
    return a;
}
inline copy_options& operator|=(copy_options& a, copy_options b) noexcept {
    a = a | b;
    return a;
}
inline copy_options& operator^=(copy_options& a, copy_options b) noexcept {
    a = a ^ b;
    return a;
}

/// <summary>
/// 目錄走訪選項。
/// </summary>
enum class directory_options : uint32_t {
    none = 0,
    follow_directory_symlink = 1,
    skip_permission_denied = 2
};

constexpr directory_options operator&(directory_options a, directory_options b) noexcept {
    return static_cast<directory_options>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}
constexpr directory_options operator|(directory_options a, directory_options b) noexcept {
    return static_cast<directory_options>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
constexpr directory_options operator^(directory_options a, directory_options b) noexcept {
    return static_cast<directory_options>(static_cast<uint32_t>(a) ^ static_cast<uint32_t>(b));
}
constexpr directory_options operator~(directory_options a) noexcept {
    return static_cast<directory_options>(~static_cast<uint32_t>(a));
}
inline directory_options& operator&=(directory_options& a, directory_options b) noexcept {
    a = a & b;
    return a;
}
inline directory_options& operator|=(directory_options& a, directory_options b) noexcept {
    a = a | b;
    return a;
}
inline directory_options& operator^=(directory_options& a, directory_options b) noexcept {
    a = a ^ b;
    return a;
}

/// <summary>
/// 檔案修改時間型別別名。
/// </summary>
using file_time_type = std::chrono::time_point<std::chrono::system_clock>;

/// <summary>
/// 磁碟空間容量資訊。
/// </summary>
struct space_info {
    uintmax_t capacity{0};
    uintmax_t free{0};
    uintmax_t available{0};
};

/// <summary>
/// 檔案狀態資訊（型態與權限）。
/// </summary>
class file_status {
public:
    constexpr file_status() noexcept : m_type(file_type::none), m_perms(perms::unknown) {}
    explicit constexpr file_status(file_type type, perms prms = perms::unknown) noexcept
        : m_type(type), m_perms(prms) {}

    file_status(const file_status&) noexcept = default;
    file_status(file_status&&) noexcept = default;
    file_status& operator=(const file_status&) noexcept = default;
    file_status& operator=(file_status&&) noexcept = default;

    [[nodiscard]] constexpr file_type type() const noexcept { return m_type; }
    void type(file_type t) noexcept { m_type = t; }

    [[nodiscard]] constexpr perms permissions() const noexcept { return m_perms; }
    void permissions(perms p) noexcept { m_perms = p; }

    [[nodiscard]] constexpr bool operator==(const file_status& rhs) const noexcept {
        return m_type == rhs.m_type && m_perms == rhs.m_perms;
    }
    [[nodiscard]] constexpr bool operator!=(const file_status& rhs) const noexcept {
        return !(*this == rhs);
    }

private:
    file_type m_type;
    perms m_perms;
};

// ============================================================================
// 2. 內部輔助字串轉換函式
// ============================================================================

namespace detail_fs {

#if defined(_WIN32)
inline std::wstring utf8_to_wstring(const std::string& str) {
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0);
    if (size_needed <= 0) {
        // Fallback to ANSI code page
        size_needed = MultiByteToWideChar(CP_ACP, 0, str.data(), static_cast<int>(str.size()), NULL, 0);
        if (size_needed <= 0) return std::wstring(str.begin(), str.end());
        std::wstring wstr(size_needed, 0);
        MultiByteToWideChar(CP_ACP, 0, str.data(), static_cast<int>(str.size()), &wstr[0], size_needed);
        return wstr;
    }
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), &wstr[0], size_needed);
    return wstr;
}

inline std::string wstring_to_utf8(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), NULL, 0, NULL, NULL);
    if (size_needed <= 0) {
        std::string res;
        res.reserve(wstr.size());
        for (wchar_t wc : wstr) res.push_back(static_cast<char>(wc & 0xFF));
        return res;
    }
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), &str[0], size_needed, NULL, NULL);
    return str;
}
#else
inline std::wstring utf8_to_wstring(const std::string& str) {
    return std::wstring(str.begin(), str.end());
}
inline std::string wstring_to_utf8(const std::wstring& wstr) {
    return std::string(wstr.begin(), wstr.end());
}
#endif

} // namespace detail_fs

// Forward declaration of path
class path;

// ============================================================================
// 3. filesystem_error 例外類別
// ============================================================================

/// <summary>
/// 檔案系統操作發生錯誤時拋出之標準例外。
/// </summary>
class filesystem_error : public std::system_error {
public:
    filesystem_error(const std::string& what_arg, std::error_code ec);
    filesystem_error(const std::string& what_arg, const path& p1, std::error_code ec);
    filesystem_error(const std::string& what_arg, const path& p1, const path& p2, std::error_code ec);

    [[nodiscard]] const path& path1() const noexcept;
    [[nodiscard]] const path& path2() const noexcept;
    [[nodiscard]] const char* what() const noexcept override;

private:
    std::shared_ptr<path> m_p1;
    std::shared_ptr<path> m_p2;
    std::string m_what;
};

// ============================================================================
// 4. path 類別
// ============================================================================

/// <summary>
/// 檔案系統路徑物件。
/// </summary>
class path {
public:
#if defined(_WIN32)
    using value_type = wchar_t;
    static constexpr value_type preferred_separator = L'\\';
#else
    using value_type = char;
    static constexpr value_type preferred_separator = '/';
#endif
    using string_type = std::basic_string<value_type>;

    enum format {
        native_format,
        generic_format,
        auto_format
    };

    path() noexcept = default;
    path(const path& p) = default;
    path(path&& p) noexcept = default;
    path& operator=(const path& p) = default;
    path& operator=(path&& p) noexcept = default;
    ~path() = default;

    path(const string_type& s, format = auto_format) : m_pathname(s) {}
    path(string_type&& s, format = auto_format) noexcept : m_pathname(std::move(s)) {}
    path(const value_type* s, format = auto_format) : m_pathname(s ? s : string_type()) {}

#if defined(_WIN32)
    path(const std::string& s, format = auto_format) : m_pathname(detail_fs::utf8_to_wstring(s)) {}
    path(const char* s, format = auto_format) : m_pathname(s ? detail_fs::utf8_to_wstring(s) : string_type()) {}
#else
    path(const std::wstring& ws, format = auto_format) : m_pathname(detail_fs::wstring_to_utf8(ws)) {}
    path(const wchar_t* ws, format = auto_format) : m_pathname(ws ? detail_fs::wstring_to_utf8(ws) : string_type()) {}
#endif

    template <class InputIt>
    path(InputIt first, InputIt last, format = auto_format) {
        string_type s(first, last);
        m_pathname = std::move(s);
    }

    path& operator=(const string_type& s) {
        m_pathname = s;
        return *this;
    }
    path& operator=(const value_type* s) {
        m_pathname = s ? s : string_type();
        return *this;
    }
#if defined(_WIN32)
    path& operator=(const std::string& s) {
        m_pathname = detail_fs::utf8_to_wstring(s);
        return *this;
    }
    path& operator=(const char* s) {
        m_pathname = s ? detail_fs::utf8_to_wstring(s) : string_type();
        return *this;
    }
#else
    path& operator=(const std::wstring& ws) {
        m_pathname = detail_fs::wstring_to_utf8(ws);
        return *this;
    }
    path& operator=(const wchar_t* ws) {
        m_pathname = ws ? detail_fs::wstring_to_utf8(ws) : string_type();
        return *this;
    }
#endif

    path& assign(const string_type& s) { return *this = s; }
    template <class InputIt>
    path& assign(InputIt first, InputIt last) {
        m_pathname.assign(first, last);
        return *this;
    }

    // --- Appending (/) ---
    path& operator/=(const path& p) {
        if (p.is_absolute() || (p.has_root_name() && p.root_name() != this->root_name())) {
            *this = p;
            return *this;
        }
        if (p.has_root_directory()) {
            *this = path(this->root_name().native() + p.native());
            return *this;
        }
        if (this->has_filename() || (!this->has_root_directory() && this->is_absolute())) {
            m_pathname += preferred_separator;
        }
        m_pathname += p.native();
        return *this;
    }

    template <class Source>
    path& operator/=(const Source& source) {
        return *this /= path(source);
    }

    template <class Source>
    path& append(const Source& source) {
        return *this /= path(source);
    }

    // --- Concatenation (+=) ---
    path& operator+=(const path& p) {
        m_pathname += p.native();
        return *this;
    }
    path& operator+=(const string_type& s) {
        m_pathname += s;
        return *this;
    }
    path& operator+=(const value_type* s) {
        if (s) m_pathname += s;
        return *this;
    }
    path& operator+=(value_type c) {
        m_pathname += c;
        return *this;
    }
#if defined(_WIN32)
    path& operator+=(const std::string& s) {
        m_pathname += detail_fs::utf8_to_wstring(s);
        return *this;
    }
    path& operator+=(const char* s) {
        if (s) m_pathname += detail_fs::utf8_to_wstring(s);
        return *this;
    }
    path& operator+=(char c) {
        char buf[2] = {c, 0};
        m_pathname += detail_fs::utf8_to_wstring(buf);
        return *this;
    }
#endif

    template <class Source>
    path& concat(const Source& source) {
        return *this += source;
    }

    // --- Modifiers ---
    void clear() noexcept { m_pathname.clear(); }

    path& make_preferred() {
#if defined(_WIN32)
        std::replace(m_pathname.begin(), m_pathname.end(), L'/', L'\\');
#endif
        return *this;
    }

    path& remove_filename() {
        size_t idx = find_last_separator();
        if (idx == string_type::npos) {
            if (has_root_name()) {
                // Keep root name
            } else {
                m_pathname.clear();
            }
        } else {
            m_pathname.erase(idx + 1);
        }
        return *this;
    }

    path& replace_filename(const path& replacement) {
        remove_filename();
        return *this /= replacement;
    }

    path& replace_extension(const path& replacement = path()) {
        size_t dot_idx = find_extension_dot();
        if (dot_idx != string_type::npos) {
            m_pathname.erase(dot_idx);
        }
        if (!replacement.empty()) {
            if (replacement.native()[0] != (value_type)'.') {
                m_pathname += (value_type)'.';
            }
            m_pathname += replacement.native();
        }
        return *this;
    }

    void swap(path& rhs) noexcept { m_pathname.swap(rhs.m_pathname); }

    // --- Observers ---
    [[nodiscard]] const string_type& native() const noexcept { return m_pathname; }
    [[nodiscard]] const value_type* c_str() const noexcept { return m_pathname.c_str(); }
    operator string_type() const { return m_pathname; }

    [[nodiscard]] std::string string() const {
#if defined(_WIN32)
        return detail_fs::wstring_to_utf8(m_pathname);
#else
        return m_pathname;
#endif
    }

    [[nodiscard]] std::wstring wstring() const {
#if defined(_WIN32)
        return m_pathname;
#else
        return detail_fs::utf8_to_wstring(m_pathname);
#endif
    }

    [[nodiscard]] std::string u8string() const { return string(); }

    [[nodiscard]] std::string generic_string() const {
        std::string s = string();
        std::replace(s.begin(), s.end(), '\\', '/');
        return s;
    }

    [[nodiscard]] std::wstring generic_wstring() const {
        std::wstring ws = wstring();
        std::replace(ws.begin(), ws.end(), L'\\', L'/');
        return ws;
    }

    [[nodiscard]] std::string generic_u8string() const { return generic_string(); }

    // --- Decomposition ---
    [[nodiscard]] path root_name() const {
#if defined(_WIN32)
        if (m_pathname.size() >= 2 && m_pathname[1] == L':') {
            return path(m_pathname.substr(0, 2));
        }
        if (m_pathname.size() >= 2 && is_separator(m_pathname[0]) && is_separator(m_pathname[1])) {
            // UNC path //server
            size_t next_sep = m_pathname.find_first_of(L"/\\", 2);
            if (next_sep != string_type::npos) {
                return path(m_pathname.substr(0, next_sep));
            }
            return *this;
        }
#endif
        return path();
    }

    [[nodiscard]] path root_directory() const {
        size_t rn_len = root_name().native().size();
        if (m_pathname.size() > rn_len && is_separator(m_pathname[rn_len])) {
            return path(string_type(1, preferred_separator));
        }
        return path();
    }

    [[nodiscard]] path root_path() const {
        return path(root_name().native() + root_directory().native());
    }

    [[nodiscard]] path relative_path() const {
        size_t prefix_len = root_path().native().size();
        if (m_pathname.size() > prefix_len) {
            return path(m_pathname.substr(prefix_len));
        }
        return path();
    }

    [[nodiscard]] path parent_path() const {
        size_t idx = find_last_separator();
        if (idx == string_type::npos) {
            return path();
        }
        if (idx == 0) {
            return path(m_pathname.substr(0, 1));
        }
        return path(m_pathname.substr(0, idx));
    }

    [[nodiscard]] path filename() const {
        size_t idx = find_last_separator();
        if (idx == string_type::npos) {
            return *this;
        }
        return path(m_pathname.substr(idx + 1));
    }

    [[nodiscard]] path stem() const {
        path fn = filename();
        size_t dot_idx = fn.find_extension_dot();
        if (dot_idx == string_type::npos) {
            return fn;
        }
        return path(fn.native().substr(0, dot_idx));
    }

    [[nodiscard]] path extension() const {
        path fn = filename();
        size_t dot_idx = fn.find_extension_dot();
        if (dot_idx == string_type::npos) {
            return path();
        }
        return path(fn.native().substr(dot_idx));
    }

    // --- Queries ---
    [[nodiscard]] bool empty() const noexcept { return m_pathname.empty(); }
    [[nodiscard]] bool has_root_path() const { return !root_path().empty(); }
    [[nodiscard]] bool has_root_name() const { return !root_name().empty(); }
    [[nodiscard]] bool has_root_directory() const { return !root_directory().empty(); }
    [[nodiscard]] bool has_relative_path() const { return !relative_path().empty(); }
    [[nodiscard]] bool has_parent_path() const { return !parent_path().empty(); }
    [[nodiscard]] bool has_filename() const { return !filename().empty(); }
    [[nodiscard]] bool has_stem() const { return !stem().empty(); }
    [[nodiscard]] bool has_extension() const { return !extension().empty(); }

    [[nodiscard]] bool is_absolute() const {
#if defined(_WIN32)
        return has_root_name() && has_root_directory();
#else
        return has_root_directory();
#endif
    }
    [[nodiscard]] bool is_relative() const { return !is_absolute(); }

    // --- Lexical normalization ---
    [[nodiscard]] path lexically_normal() const {
        if (empty()) return path();

        // 拆解為元件
        std::vector<string_type> elements;
        string_type cur;
        bool has_root = false;

        string_type r_name = root_name().native();
        string_type r_dir = root_directory().native();
        size_t offset = r_name.size() + r_dir.size();
        if (!r_name.empty() || !r_dir.empty()) {
            has_root = true;
        }

        bool ends_with_slash_or_dot = false;
        if (!m_pathname.empty()) {
            if (is_separator(m_pathname.back())) {
                ends_with_slash_or_dot = true;
            } else if (m_pathname.back() == (value_type)'.') {
                size_t n = m_pathname.size();
                if (n == 1 || is_separator(m_pathname[n - 2])) {
                    ends_with_slash_or_dot = true;
                }
            }
        }

        for (size_t i = offset; i < m_pathname.size(); ++i) {
            if (is_separator(m_pathname[i])) {
                if (!cur.empty()) {
                    elements.push_back(cur);
                    cur.clear();
                }
            } else {
                cur += m_pathname[i];
            }
        }
        if (!cur.empty()) {
            elements.push_back(cur);
        }

        std::vector<string_type> normalized;
        for (const auto& elem : elements) {
            if (elem == string_type(1, (value_type)'.')) {
                // Ignore single dot
                continue;
            } else if (elem == string_type(2, (value_type)'.')) {
                if (!normalized.empty() && normalized.back() != string_type(2, (value_type)'.')) {
                    normalized.pop_back();
                } else if (!has_root) {
                    normalized.push_back(elem);
                }
            } else {
                normalized.push_back(elem);
            }
        }

        string_type res = r_name + r_dir;
        for (size_t i = 0; i < normalized.size(); ++i) {
            if (i > 0 || !res.empty()) {
                if (res.empty() || !is_separator(res.back())) {
                    res += preferred_separator;
                }
            }
            res += normalized[i];
        }

        if (res.empty()) {
            res = string_type(1, (value_type)'.');
        } else if (ends_with_slash_or_dot && !is_separator(res.back())) {
            res += preferred_separator;
        }

        return path(res);
    }

    // --- Iterators ---
    class iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = path;
        using difference_type = std::ptrdiff_t;
        using pointer = const path*;
        using reference = const path&;

        iterator() = default;
        iterator(const path* p, size_t pos, const std::vector<path>& elems)
            : m_path(p), m_pos(pos), m_elems(elems) {}

        reference operator*() const { return m_elems[m_pos]; }
        pointer operator->() const { return &m_elems[m_pos]; }

        iterator& operator++() {
            ++m_pos;
            return *this;
        }
        iterator operator++(int) {
            iterator tmp = *this;
            ++m_pos;
            return tmp;
        }
        iterator& operator--() {
            --m_pos;
            return *this;
        }
        iterator operator--(int) {
            iterator tmp = *this;
            --m_pos;
            return tmp;
        }

        bool operator==(const iterator& rhs) const noexcept {
            return m_pos == rhs.m_pos && m_path == rhs.m_path;
        }
        bool operator!=(const iterator& rhs) const noexcept {
            return !(*this == rhs);
        }

    private:
        const path* m_path{nullptr};
        size_t m_pos{0};
        std::vector<path> m_elems;
    };

    using const_iterator = iterator;

    [[nodiscard]] iterator begin() const {
        std::vector<path> elems = decompose_elements();
        return iterator(this, 0, elems);
    }

    [[nodiscard]] iterator end() const {
        std::vector<path> elems = decompose_elements();
        return iterator(this, elems.size(), elems);
    }

    // --- Comparison ---
    [[nodiscard]] int compare(const path& p) const noexcept {
        return m_pathname.compare(p.m_pathname);
    }

    [[nodiscard]] bool operator==(const path& p) const noexcept {
        return generic_string() == p.generic_string();
    }
    [[nodiscard]] bool operator!=(const path& p) const noexcept {
        return !(*this == p);
    }
    [[nodiscard]] bool operator<(const path& p) const noexcept {
        return generic_string() < p.generic_string();
    }
    [[nodiscard]] bool operator<=(const path& p) const noexcept {
        return !(p < *this);
    }
    [[nodiscard]] bool operator>(const path& p) const noexcept {
        return p < *this;
    }
    [[nodiscard]] bool operator>=(const path& p) const noexcept {
        return !(*this < p);
    }

private:
    string_type m_pathname;

    static bool is_separator(value_type c) noexcept {
        return c == (value_type)'/' || c == (value_type)'\\';
    }

    size_t find_last_separator() const noexcept {
        for (size_t i = m_pathname.size(); i > 0; --i) {
            if (is_separator(m_pathname[i - 1])) {
                return i - 1;
            }
        }
        return string_type::npos;
    }

    size_t find_extension_dot() const noexcept {
        size_t last_sep = find_last_separator();
        size_t start = (last_sep == string_type::npos) ? 0 : last_sep + 1;
        if (start >= m_pathname.size()) return string_type::npos;

        // Skip leading dot in filename (e.g., .bashrc is not an extension)
        size_t dot = string_type::npos;
        for (size_t i = m_pathname.size(); i > start + 1; --i) {
            if (m_pathname[i - 1] == (value_type)'.') {
                dot = i - 1;
                break;
            }
        }
        return dot;
    }

    std::vector<path> decompose_elements() const {
        std::vector<path> elems;
        if (empty()) return elems;

        path rn = root_name();
        path rd = root_directory();
        if (!rn.empty()) elems.push_back(rn);
        if (!rd.empty()) elems.push_back(rd);

        size_t offset = rn.native().size() + rd.native().size();
        string_type cur;
        for (size_t i = offset; i < m_pathname.size(); ++i) {
            if (is_separator(m_pathname[i])) {
                if (!cur.empty()) {
                    elems.push_back(path(cur));
                    cur.clear();
                }
            } else {
                cur += m_pathname[i];
            }
        }
        if (!cur.empty()) {
            elems.push_back(path(cur));
        }
        return elems;
    }
};

// Non-member operators for path
inline path operator/(const path& lhs, const path& rhs) {
    path res = lhs;
    res /= rhs;
    return res;
}

inline void swap(path& lhs, path& rhs) noexcept {
    lhs.swap(rhs);
}

inline size_t hash_value(const path& p) noexcept {
    return std::hash<std::string>{}(p.generic_string());
}

inline std::ostream& operator<<(std::ostream& os, const path& p) {
    os << p.string();
    return os;
}

inline std::istream& operator>>(std::istream& is, path& p) {
    std::string s;
    if (is >> s) {
        p = s;
    }
    return is;
}

// filesystem_error implementation
inline filesystem_error::filesystem_error(const std::string& what_arg, std::error_code ec)
    : std::system_error(ec, what_arg), m_p1(std::make_shared<path>()), m_p2(std::make_shared<path>()) {
    m_what = what_arg + ": " + ec.message();
}

inline filesystem_error::filesystem_error(const std::string& what_arg, const path& p1, std::error_code ec)
    : std::system_error(ec, what_arg), m_p1(std::make_shared<path>(p1)), m_p2(std::make_shared<path>()) {
    m_what = what_arg + " [" + p1.string() + "]: " + ec.message();
}

inline filesystem_error::filesystem_error(const std::string& what_arg, const path& p1, const path& p2, std::error_code ec)
    : std::system_error(ec, what_arg), m_p1(std::make_shared<path>(p1)), m_p2(std::make_shared<path>(p2)) {
    m_what = what_arg + " [" + p1.string() + ", " + p2.string() + "]: " + ec.message();
}

inline const path& filesystem_error::path1() const noexcept { return *m_p1; }
inline const path& filesystem_error::path2() const noexcept { return *m_p2; }
inline const char* filesystem_error::what() const noexcept { return m_what.c_str(); }

// ============================================================================
// 5. 檔案操作函式宣告與實作 (Operations)
// ============================================================================

// Forward declaration of directory_entry
class directory_entry;

// Status & existence
inline file_status status(const path& p, std::error_code& ec) noexcept {
    ec.clear();
#if defined(_WIN32)
    DWORD attrs = GetFileAttributesW(p.wstring().c_str());
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        DWORD err = GetLastError();
        if (err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND) {
            return file_status(file_type::not_found);
        }
        ec = std::make_error_code(static_cast<std::errc>(err));
        return file_status(file_type::none);
    }
    perms prms = perms::owner_read | perms::group_read | perms::others_read;
    if (!(attrs & FILE_ATTRIBUTE_READONLY)) {
        prms |= perms::owner_write | perms::group_write | perms::others_write;
    }
    if (attrs & FILE_ATTRIBUTE_DIRECTORY) {
        return file_status(file_type::directory, prms);
    }
    return file_status(file_type::regular, prms);
#else
    struct stat st;
    if (::stat(p.c_str(), &st) != 0) {
        if (errno == ENOENT || errno == ENOTDIR) {
            return file_status(file_type::not_found);
        }
        ec = std::make_error_code(static_cast<std::errc>(errno));
        return file_status(file_type::none);
    }
    perms prms = static_cast<perms>(st.st_mode & 07777);
    if (S_ISREG(st.st_mode)) return file_status(file_type::regular, prms);
    if (S_ISDIR(st.st_mode)) return file_status(file_type::directory, prms);
    if (S_ISLNK(st.st_mode)) return file_status(file_type::symlink, prms);
    if (S_ISCHR(st.st_mode)) return file_status(file_type::character, prms);
    if (S_ISBLK(st.st_mode)) return file_status(file_type::block, prms);
    if (S_ISFIFO(st.st_mode)) return file_status(file_type::fifo, prms);
    if (S_ISSOCK(st.st_mode)) return file_status(file_type::socket, prms);
    return file_status(file_type::unknown, prms);
#endif
}

inline file_status status(const path& p) {
    std::error_code ec;
    file_status st = status(p, ec);
    if (ec) {
        throw filesystem_error("status failed", p, ec);
    }
    return st;
}

inline file_status symlink_status(const path& p, std::error_code& ec) noexcept {
    return status(p, ec);
}

inline file_status symlink_status(const path& p) {
    return status(p);
}

inline bool status_known(file_status s) noexcept {
    return s.type() != file_type::none;
}

inline bool exists(file_status s) noexcept {
    return status_known(s) && s.type() != file_type::not_found;
}

inline bool exists(const path& p, std::error_code& ec) noexcept {
    return exists(status(p, ec));
}

inline bool exists(const path& p) {
    return exists(status(p));
}

inline bool is_regular_file(file_status s) noexcept {
    return s.type() == file_type::regular;
}
inline bool is_regular_file(const path& p, std::error_code& ec) noexcept {
    return is_regular_file(status(p, ec));
}
inline bool is_regular_file(const path& p) {
    return is_regular_file(status(p));
}

inline bool is_directory(file_status s) noexcept {
    return s.type() == file_type::directory;
}
inline bool is_directory(const path& p, std::error_code& ec) noexcept {
    return is_directory(status(p, ec));
}
inline bool is_directory(const path& p) {
    return is_directory(status(p));
}

inline bool is_symlink(file_status s) noexcept {
    return s.type() == file_type::symlink;
}
inline bool is_symlink(const path& p, std::error_code& ec) noexcept {
    return is_symlink(symlink_status(p, ec));
}
inline bool is_symlink(const path& p) {
    return is_symlink(symlink_status(p));
}

inline bool is_block_file(file_status s) noexcept { return s.type() == file_type::block; }
inline bool is_block_file(const path& p, std::error_code& ec) noexcept { return is_block_file(status(p, ec)); }
inline bool is_block_file(const path& p) { return is_block_file(status(p)); }

inline bool is_character_file(file_status s) noexcept { return s.type() == file_type::character; }
inline bool is_character_file(const path& p, std::error_code& ec) noexcept { return is_character_file(status(p, ec)); }
inline bool is_character_file(const path& p) { return is_character_file(status(p)); }

inline bool is_fifo(file_status s) noexcept { return s.type() == file_type::fifo; }
inline bool is_fifo(const path& p, std::error_code& ec) noexcept { return is_fifo(status(p, ec)); }
inline bool is_fifo(const path& p) { return is_fifo(status(p)); }

inline bool is_socket(file_status s) noexcept { return s.type() == file_type::socket; }
inline bool is_socket(const path& p, std::error_code& ec) noexcept { return is_socket(status(p, ec)); }
inline bool is_socket(const path& p) { return is_socket(status(p)); }

inline bool is_other(file_status s) noexcept {
    return exists(s) && !is_regular_file(s) && !is_directory(s) && !is_symlink(s);
}
inline bool is_other(const path& p, std::error_code& ec) noexcept { return is_other(status(p, ec)); }
inline bool is_other(const path& p) { return is_other(status(p)); }

// Current path & temp directory
inline path current_path(std::error_code& ec) {
    ec.clear();
#if defined(_WIN32)
    wchar_t buf[MAX_PATH];
    DWORD len = GetCurrentDirectoryW(MAX_PATH, buf);
    if (len == 0) {
        ec = std::make_error_code(std::errc::bad_file_descriptor);
        return path();
    }
    return path(buf);
#else
    char buf[1024];
    if (::getcwd(buf, sizeof(buf)) == NULL) {
        ec = std::make_error_code(static_cast<std::errc>(errno));
        return path();
    }
    return path(buf);
#endif
}

inline path current_path() {
    std::error_code ec;
    path p = current_path(ec);
    if (ec) throw filesystem_error("current_path failed", ec);
    return p;
}

inline void current_path(const path& p, std::error_code& ec) noexcept {
    ec.clear();
#if defined(_WIN32)
    if (!SetCurrentDirectoryW(p.wstring().c_str())) {
        ec = std::make_error_code(std::errc::invalid_argument);
    }
#else
    if (::chdir(p.c_str()) != 0) {
        ec = std::make_error_code(static_cast<std::errc>(errno));
    }
#endif
}

inline void current_path(const path& p) {
    std::error_code ec;
    current_path(p, ec);
    if (ec) throw filesystem_error("current_path(p) failed", p, ec);
}

inline path temp_directory_path(std::error_code& ec) {
    ec.clear();
#if defined(_WIN32)
    wchar_t buf[MAX_PATH];
    DWORD len = GetTempPathW(MAX_PATH, buf);
    if (len == 0) {
        ec = std::make_error_code(std::errc::no_such_file_or_directory);
        return path();
    }
    path p(buf);
    p.make_preferred();
    return p;
#else
    const char* tmp = std::getenv("TMPDIR");
    if (!tmp) tmp = std::getenv("TMP");
    if (!tmp) tmp = std::getenv("TEMP");
    if (!tmp) tmp = "/tmp";
    return path(tmp);
#endif
}

inline path temp_directory_path() {
    std::error_code ec;
    path p = temp_directory_path(ec);
    if (ec) throw filesystem_error("temp_directory_path failed", ec);
    return p;
}

// File size
inline uintmax_t file_size(const path& p, std::error_code& ec) noexcept {
    ec.clear();
#if defined(_WIN32)
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesExW(p.wstring().c_str(), GetFileExInfoStandard, &fad)) {
        ec = std::make_error_code(std::errc::no_such_file_or_directory);
        return static_cast<uintmax_t>(-1);
    }
    if (fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        ec = std::make_error_code(std::errc::is_a_directory);
        return static_cast<uintmax_t>(-1);
    }
    LARGE_INTEGER size;
    size.LowPart = fad.nFileSizeLow;
    size.HighPart = fad.nFileSizeHigh;
    return static_cast<uintmax_t>(size.QuadPart);
#else
    struct stat st;
    if (::stat(p.c_str(), &st) != 0) {
        ec = std::make_error_code(static_cast<std::errc>(errno));
        return static_cast<uintmax_t>(-1);
    }
    if (S_ISDIR(st.st_mode)) {
        ec = std::make_error_code(std::errc::is_a_directory);
        return static_cast<uintmax_t>(-1);
    }
    return static_cast<uintmax_t>(st.st_size);
#endif
}

inline uintmax_t file_size(const path& p) {
    std::error_code ec;
    uintmax_t sz = file_size(p, ec);
    if (ec) throw filesystem_error("file_size failed", p, ec);
    return sz;
}

// Directory creation
inline bool create_directory(const path& p, std::error_code& ec) noexcept {
    ec.clear();
#if defined(_WIN32)
    if (CreateDirectoryW(p.wstring().c_str(), NULL)) {
        return true;
    }
    DWORD err = GetLastError();
    if (err == ERROR_ALREADY_EXISTS) {
        return false;
    }
    ec = std::make_error_code(static_cast<std::errc>(err));
    return false;
#else
    if (::mkdir(p.c_str(), 0755) == 0) {
        return true;
    }
    if (errno == EEXIST) {
        return false;
    }
    ec = std::make_error_code(static_cast<std::errc>(errno));
    return false;
#endif
}

inline bool create_directory(const path& p) {
    std::error_code ec;
    bool res = create_directory(p, ec);
    if (ec) throw filesystem_error("create_directory failed", p, ec);
    return res;
}

inline bool create_directories(const path& p, std::error_code& ec) noexcept {
    ec.clear();
    if (p.empty() || exists(p, ec)) {
        return false;
    }
    path parent = p.parent_path();
    if (!parent.empty() && !exists(parent, ec)) {
        create_directories(parent, ec);
        if (ec) return false;
    }
    return create_directory(p, ec);
}

inline bool create_directories(const path& p) {
    std::error_code ec;
    bool res = create_directories(p, ec);
    if (ec) throw filesystem_error("create_directories failed", p, ec);
    return res;
}

// Removal
inline bool remove(const path& p, std::error_code& ec) noexcept {
    ec.clear();
#if defined(_WIN32)
    DWORD attrs = GetFileAttributesW(p.wstring().c_str());
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        return false;
    }
    if (attrs & FILE_ATTRIBUTE_DIRECTORY) {
        if (RemoveDirectoryW(p.wstring().c_str())) return true;
    } else {
        if (DeleteFileW(p.wstring().c_str())) return true;
    }
    ec = std::make_error_code(static_cast<std::errc>(GetLastError()));
    return false;
#else
    if (::unlink(p.c_str()) == 0) return true;
    if (errno == EISDIR || errno == EPERM) {
        if (::rmdir(p.c_str()) == 0) return true;
    }
    if (errno == ENOENT) return false;
    ec = std::make_error_code(static_cast<std::errc>(errno));
    return false;
#endif
}

inline bool remove(const path& p) {
    std::error_code ec;
    bool res = remove(p, ec);
    if (ec) throw filesystem_error("remove failed", p, ec);
    return res;
}

// Forward declaration for remove_all
inline uintmax_t remove_all(const path& p, std::error_code& ec);

inline uintmax_t remove_all(const path& p) {
    std::error_code ec;
    uintmax_t count = remove_all(p, ec);
    if (ec) throw filesystem_error("remove_all failed", p, ec);
    return count;
}

// Rename
inline void rename(const path& old_p, const path& new_p, std::error_code& ec) noexcept {
    ec.clear();
#if defined(_WIN32)
    if (!MoveFileExW(old_p.wstring().c_str(), new_p.wstring().c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED)) {
        ec = std::make_error_code(static_cast<std::errc>(GetLastError()));
    }
#else
    if (::rename(old_p.c_str(), new_p.c_str()) != 0) {
        ec = std::make_error_code(static_cast<std::errc>(errno));
    }
#endif
}

inline void rename(const path& old_p, const path& new_p) {
    std::error_code ec;
    rename(old_p, new_p, ec);
    if (ec) throw filesystem_error("rename failed", old_p, new_p, ec);
}

// Copy file
inline bool copy_file(const path& from, const path& to, copy_options options, std::error_code& ec) noexcept {
    ec.clear();
#if defined(_WIN32)
    BOOL fail_if_exists = ((options & copy_options::overwrite_existing) == copy_options::none);
    if (CopyFileW(from.wstring().c_str(), to.wstring().c_str(), fail_if_exists)) {
        return true;
    }
    ec = std::make_error_code(static_cast<std::errc>(GetLastError()));
    return false;
#else
    std::ifstream src(from.string(), std::ios::binary);
    if (!src) {
        ec = std::make_error_code(std::errc::no_such_file_or_directory);
        return false;
    }
    if (exists(to, ec) && ((options & copy_options::overwrite_existing) == copy_options::none)) {
        ec = std::make_error_code(std::errc::file_exists);
        return false;
    }
    std::ofstream dst(to.string(), std::ios::binary);
    if (!dst) {
        ec = std::make_error_code(std::errc::permission_denied);
        return false;
    }
    dst << src.rdbuf();
    return true;
#endif
}

inline bool copy_file(const path& from, const path& to, copy_options options = copy_options::none) {
    std::error_code ec;
    bool res = copy_file(from, to, options, ec);
    if (ec) throw filesystem_error("copy_file failed", from, to, ec);
    return res;
}

inline bool copy_file(const path& from, const path& to, std::error_code& ec) noexcept {
    return copy_file(from, to, copy_options::none, ec);
}

// Space
inline space_info space(const path& p, std::error_code& ec) noexcept {
    ec.clear();
    space_info info;
#if defined(_WIN32)
    ULARGE_INTEGER free_bytes_avail, total_bytes, free_bytes;
    if (GetDiskFreeSpaceExW(p.wstring().c_str(), &free_bytes_avail, &total_bytes, &free_bytes)) {
        info.capacity = total_bytes.QuadPart;
        info.free = free_bytes.QuadPart;
        info.available = free_bytes_avail.QuadPart;
    } else {
        ec = std::make_error_code(static_cast<std::errc>(GetLastError()));
    }
#else
    struct statvfs st;
    if (::statvfs(p.c_str(), &st) == 0) {
        info.capacity = static_cast<uintmax_t>(st.f_blocks) * st.f_frsize;
        info.free = static_cast<uintmax_t>(st.f_bfree) * st.f_frsize;
        info.available = static_cast<uintmax_t>(st.f_bavail) * st.f_frsize;
    } else {
        ec = std::make_error_code(static_cast<std::errc>(errno));
    }
#endif
    return info;
}

inline space_info space(const path& p) {
    std::error_code ec;
    space_info res = space(p, ec);
    if (ec) throw filesystem_error("space failed", p, ec);
    return res;
}

// Equivalent
inline bool equivalent(const path& p1, const path& p2, std::error_code& ec) noexcept {
    ec.clear();
    return p1.lexically_normal() == p2.lexically_normal();
}

inline bool equivalent(const path& p1, const path& p2) {
    std::error_code ec;
    bool res = equivalent(p1, p2, ec);
    if (ec) throw filesystem_error("equivalent failed", p1, p2, ec);
    return res;
}

// Absolute
inline path absolute(const path& p, std::error_code& ec) {
    ec.clear();
    if (p.is_absolute()) return p;
    return current_path(ec) / p;
}

inline path absolute(const path& p) {
    std::error_code ec;
    path res = absolute(p, ec);
    if (ec) throw filesystem_error("absolute failed", p, ec);
    return res;
}

// Canonical
inline path canonical(const path& p, std::error_code& ec) {
    ec.clear();
    path abs_p = absolute(p, ec);
    if (!exists(abs_p, ec)) {
        ec = std::make_error_code(std::errc::no_such_file_or_directory);
        return path();
    }
    return abs_p.lexically_normal();
}

inline path canonical(const path& p) {
    std::error_code ec;
    path res = canonical(p, ec);
    if (ec) throw filesystem_error("canonical failed", p, ec);
    return res;
}

// ============================================================================
// 6. directory_entry 與 directory_iterator
// ============================================================================

/// <summary>
/// 表示目錄項目的類別。
/// </summary>
class directory_entry {
public:
    directory_entry() noexcept = default;
    directory_entry(const directory_entry&) = default;
    directory_entry(directory_entry&&) noexcept = default;
    directory_entry& operator=(const directory_entry&) = default;
    directory_entry& operator=(directory_entry&&) noexcept = default;

    explicit directory_entry(const path& p) : m_path(p) {
        refresh();
    }
    directory_entry(const path& p, std::error_code& ec) : m_path(p) {
        refresh(ec);
    }

    void assign(const path& p) {
        m_path = p;
        refresh();
    }
    void replace_filename(const path& p) {
        m_path.replace_filename(p);
        refresh();
    }
    void refresh() {
        std::error_code ec;
        refresh(ec);
    }
    void refresh(std::error_code& ec) noexcept {
        m_status = filesystem::status(m_path, ec);
    }

    [[nodiscard]] const path& path() const noexcept { return m_path; }
    operator const filesystem::path&() const noexcept { return m_path; }

    [[nodiscard]] bool exists() const { return filesystem::exists(m_status); }
    [[nodiscard]] bool exists(std::error_code&) const noexcept { return filesystem::exists(m_status); }

    [[nodiscard]] bool is_block_file() const { return filesystem::is_block_file(m_status); }
    [[nodiscard]] bool is_character_file() const { return filesystem::is_character_file(m_status); }
    [[nodiscard]] bool is_directory() const { return filesystem::is_directory(m_status); }
    [[nodiscard]] bool is_fifo() const { return filesystem::is_fifo(m_status); }
    [[nodiscard]] bool is_other() const { return filesystem::is_other(m_status); }
    [[nodiscard]] bool is_regular_file() const { return filesystem::is_regular_file(m_status); }
    [[nodiscard]] bool is_socket() const { return filesystem::is_socket(m_status); }
    [[nodiscard]] bool is_symlink() const { return filesystem::is_symlink(m_status); }

    [[nodiscard]] uintmax_t file_size() const { return filesystem::file_size(m_path); }
    [[nodiscard]] uintmax_t file_size(std::error_code& ec) const noexcept { return filesystem::file_size(m_path, ec); }

    [[nodiscard]] file_status status() const { return m_status; }
    [[nodiscard]] file_status status(std::error_code&) const noexcept { return m_status; }

    [[nodiscard]] bool operator==(const directory_entry& rhs) const noexcept { return m_path == rhs.m_path; }
    [[nodiscard]] bool operator!=(const directory_entry& rhs) const noexcept { return m_path != rhs.m_path; }
    [[nodiscard]] bool operator<(const directory_entry& rhs) const noexcept { return m_path < rhs.m_path; }

private:
    filesystem::path m_path;
    file_status m_status;
};

// ============================================================================
// 7. directory_iterator
// ============================================================================

namespace detail_fs {

struct DirIterImpl {
    path dir_path;
    std::vector<directory_entry> entries;
    size_t index{0};

    DirIterImpl() = default;

    explicit DirIterImpl(const path& p, directory_options, std::error_code& ec) : dir_path(p), index(0) {
        ec.clear();
#if defined(_WIN32)
        std::wstring search_pattern = (p / "*").wstring();
        WIN32_FIND_DATAW fd;
        HANDLE hFind = FindFirstFileW(search_pattern.c_str(), &fd);
        if (hFind == INVALID_HANDLE_VALUE) {
            DWORD err = GetLastError();
            if (err != ERROR_FILE_NOT_FOUND) {
                ec = std::make_error_code(static_cast<std::errc>(err));
            }
            return;
        }
        do {
            std::wstring name = fd.cFileName;
            if (name == L"." || name == L"..") continue;
            entries.push_back(directory_entry(p / name));
        } while (FindNextFileW(hFind, &fd));
        FindClose(hFind);
#else
        DIR* dir = ::opendir(p.c_str());
        if (!dir) {
            ec = std::make_error_code(static_cast<std::errc>(errno));
            return;
        }
        struct dirent* entry;
        while ((entry = ::readdir(dir)) != NULL) {
            std::string name = entry->d_name;
            if (name == "." || name == "..") continue;
            entries.push_back(directory_entry(p / name));
        }
        ::closedir(dir);
#endif
    }
};

} // namespace detail_fs

/// <summary>
/// 目錄內容單層走訪疊代器。
/// </summary>
class directory_iterator {
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = directory_entry;
    using difference_type = std::ptrdiff_t;
    using pointer = const directory_entry*;
    using reference = const directory_entry&;

    directory_iterator() noexcept : m_impl(nullptr) {}

    explicit directory_iterator(const path& p)
        : directory_iterator(p, directory_options::none) {}

    directory_iterator(const path& p, directory_options options) {
        std::error_code ec;
        m_impl = std::make_shared<detail_fs::DirIterImpl>(p, options, ec);
        if (ec) {
            m_impl.reset();
            throw filesystem_error("directory_iterator failed", p, ec);
        }
        if (m_impl->entries.empty()) {
            m_impl.reset();
        }
    }

    directory_iterator(const path& p, std::error_code& ec) noexcept
        : directory_iterator(p, directory_options::none, ec) {}

    directory_iterator(const path& p, directory_options options, std::error_code& ec) noexcept {
        m_impl = std::make_shared<detail_fs::DirIterImpl>(p, options, ec);
        if (ec || m_impl->entries.empty()) {
            m_impl.reset();
        }
    }

    [[nodiscard]] reference operator*() const { return m_impl->entries[m_impl->index]; }
    [[nodiscard]] pointer operator->() const { return &m_impl->entries[m_impl->index]; }

    directory_iterator& operator++() {
        if (m_impl) {
            ++m_impl->index;
            if (m_impl->index >= m_impl->entries.size()) {
                m_impl.reset();
            }
        }
        return *this;
    }

    directory_iterator& increment(std::error_code& ec) {
        ec.clear();
        return ++(*this);
    }

    [[nodiscard]] bool operator==(const directory_iterator& rhs) const noexcept {
        if (!m_impl && !rhs.m_impl) return true;
        if (!m_impl || !rhs.m_impl) return false;
        return m_impl == rhs.m_impl && m_impl->index == rhs.m_impl->index;
    }

    [[nodiscard]] bool operator!=(const directory_iterator& rhs) const noexcept {
        return !(*this == rhs);
    }

private:
    std::shared_ptr<detail_fs::DirIterImpl> m_impl;
};

inline directory_iterator begin(directory_iterator iter) noexcept { return iter; }
inline directory_iterator end(const directory_iterator&) noexcept { return directory_iterator(); }

// ============================================================================
// 8. recursive_directory_iterator
// ============================================================================

/// <summary>
/// 遞迴目錄樹走訪疊代器。
/// </summary>
class recursive_directory_iterator {
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = directory_entry;
    using difference_type = std::ptrdiff_t;
    using pointer = const directory_entry*;
    using reference = const directory_entry&;

    recursive_directory_iterator() noexcept : m_options(directory_options::none), m_recursion_pending(true) {}

    explicit recursive_directory_iterator(const path& p)
        : recursive_directory_iterator(p, directory_options::none) {}

    recursive_directory_iterator(const path& p, directory_options options)
        : m_options(options), m_recursion_pending(true) {
        directory_iterator it(p, options);
        if (it != directory_iterator()) {
            m_stack.push_back(it);
        }
    }

    recursive_directory_iterator(const path& p, std::error_code& ec) noexcept
        : recursive_directory_iterator(p, directory_options::none, ec) {}

    recursive_directory_iterator(const path& p, directory_options options, std::error_code& ec) noexcept
        : m_options(options), m_recursion_pending(true) {
        directory_iterator it(p, options, ec);
        if (!ec && it != directory_iterator()) {
            m_stack.push_back(it);
        }
    }

    [[nodiscard]] reference operator*() const { return *m_stack.back(); }
    [[nodiscard]] pointer operator->() const { return &*m_stack.back(); }

    [[nodiscard]] int depth() const noexcept { return static_cast<int>(m_stack.size()) - 1; }
    [[nodiscard]] bool recursion_pending() const noexcept { return m_recursion_pending; }
    void disable_recursion_pending() noexcept { m_recursion_pending = false; }
    [[nodiscard]] directory_options options() const noexcept { return m_options; }

    recursive_directory_iterator& operator++() {
        if (m_stack.empty()) return *this;

        const directory_entry& cur = *m_stack.back();
        if (m_recursion_pending && cur.is_directory()) {
            std::error_code ec;
            directory_iterator sub_it(cur.path(), m_options, ec);
            if (!ec && sub_it != directory_iterator()) {
                m_stack.push_back(sub_it);
                m_recursion_pending = true;
                return *this;
            }
        }

        m_recursion_pending = true;
        while (!m_stack.empty()) {
            ++m_stack.back();
            if (m_stack.back() != directory_iterator()) {
                return *this;
            }
            m_stack.pop_back();
        }
        return *this;
    }

    void pop() {
        if (!m_stack.empty()) {
            m_stack.pop_back();
        }
    }

    [[nodiscard]] bool operator==(const recursive_directory_iterator& rhs) const noexcept {
        if (m_stack.empty() && rhs.m_stack.empty()) return true;
        if (m_stack.empty() || rhs.m_stack.empty()) return false;
        return m_stack == rhs.m_stack;
    }

    [[nodiscard]] bool operator!=(const recursive_directory_iterator& rhs) const noexcept {
        return !(*this == rhs);
    }

private:
    std::vector<directory_iterator> m_stack;
    directory_options m_options;
    bool m_recursion_pending;
};

inline recursive_directory_iterator begin(recursive_directory_iterator iter) noexcept { return iter; }
inline recursive_directory_iterator end(const recursive_directory_iterator&) noexcept { return recursive_directory_iterator(); }

// Helper: is_empty
inline bool is_empty(const path& p, std::error_code& ec) {
    file_status st = status(p, ec);
    if (ec) return false;
    if (is_directory(st)) {
        directory_iterator it(p, ec);
        return it == directory_iterator();
    }
    return file_size(p, ec) == 0;
}

inline bool is_empty(const path& p) {
    std::error_code ec;
    bool res = is_empty(p, ec);
    if (ec) throw filesystem_error("is_empty failed", p, ec);
    return res;
}

// remove_all full definition
inline uintmax_t remove_all(const path& p, std::error_code& ec) {
    ec.clear();
    file_status st = status(p, ec);
    if (!exists(st)) return 0;

    uintmax_t count = 0;
    if (is_directory(st)) {
        directory_iterator it(p, ec);
        while (it != directory_iterator()) {
            count += remove_all(it->path(), ec);
            ++it;
        }
    }
    if (remove(p, ec)) {
        ++count;
    }
    return count;
}

} // namespace filesystem
} // namespace compat
