#include "test_helpers.hpp"
#include <compat/StringView.hpp>
#include <sstream>
#include <string>
#include <stdexcept>
#include <unordered_map>
#include <cstdint>

#if (COMPAT_CPLUSPLUS >= COMPAT_CXX_14) || defined(COMPAT_CXX14)
/// <summary>
/// Compile-time verification of compat::string_view constexpr operations in C++14 and later.
/// </summary>
/// <returns>True if all constexpr checks succeed.</returns>
constexpr bool TestConstexprStringView() {
    compat::string_view sv = "constexpr_test_string";
    if (sv.size() != 21) return false;
    if (sv.empty()) return false;
    if (sv[0] != 'c') return false;
    if (sv.front() != 'c') return false;
    if (sv.back() != 'g') return false;
    if (sv.substr(0, 9) != "constexpr") return false;
    if (sv.find("test") != 10) return false;
    if (sv.rfind('t') != 16) return false;
    if (!sv.starts_with("const")) return false;
    if (!sv.ends_with("string")) return false;
    if (sv.compare("constexpr_test_string") != 0) return false;
    return true;
}

static_assert(TestConstexprStringView(), "compat::string_view constexpr tests failed");
#endif

/// <summary>
/// 測試 compat::string_view 之完整操作。
/// 包含建構式、大小、下標存取、邊界檢查 (at)、子字串 (substr)、搜尋 (find, rfind)、運算子與雜湊映射。
/// </summary>
void run_test_string_view() {
    std::cout << "[TEST] Running test_string_view..." << std::endl;

    // 1. 建構式與基本屬性測試
    compat::string_view sv_empty;
    TEST_ASSERT(sv_empty.empty());
    TEST_ASSERT(sv_empty.size() == 0);
    TEST_ASSERT(sv_empty.length() == 0);

    const char* raw_str = "Hello, Modern C++!";
    compat::string_view sv1(raw_str);
    TEST_ASSERT(!sv1.empty());
    TEST_ASSERT(sv1.size() == 18);
    TEST_ASSERT(sv1.data() == raw_str);

    compat::string_view sv_sub(raw_str, 5);
    TEST_ASSERT(sv_sub.size() == 5);
    TEST_ASSERT(sv_sub == "Hello");

    std::string std_str = "StandardString";
    compat::string_view sv_from_std(std_str);
    TEST_ASSERT(sv_from_std.size() == 14);
    TEST_ASSERT(sv_from_std == "StandardString");

    // 2. 元素存取與邊界測試
    TEST_ASSERT(sv1[0] == 'H');
    TEST_ASSERT(sv1[7] == 'M');
    TEST_ASSERT(sv1.front() == 'H');
    TEST_ASSERT(sv1.back() == '!');
    TEST_ASSERT(sv1.at(0) == 'H');
    TEST_ASSERT(sv1.at(17) == '!');
    TEST_ASSERT_THROWS(sv1.at(18), std::out_of_range);
    TEST_ASSERT_THROWS(sv1.at(100), std::out_of_range);

    // 3. 子字串 (substr)
    compat::string_view sub1 = sv1.substr(7, 6);
    TEST_ASSERT(sub1 == "Modern");
    compat::string_view sub_tail = sv1.substr(7);
    TEST_ASSERT(sub_tail == "Modern C++!");
    TEST_ASSERT_THROWS(sv1.substr(100), std::out_of_range);

    // 4. 搜尋 (find 與 rfind)
    TEST_ASSERT(sv1.find("Modern") == 7);
    TEST_ASSERT(sv1.find("C++") == 14);
    TEST_ASSERT(sv1.find("NotFound") == compat::string_view::npos);
    TEST_ASSERT(sv1.find('M') == 7);
    TEST_ASSERT(sv1.find('z') == compat::string_view::npos);

    compat::string_view sv_rfind("one two three two one");
    TEST_ASSERT(sv_rfind.rfind("two") == 14);
    TEST_ASSERT(sv_rfind.rfind("two", 13) == 4);
    TEST_ASSERT(sv_rfind.rfind("one") == 18);
    TEST_ASSERT(sv_rfind.rfind('o') == 18);
    TEST_ASSERT(sv_rfind.rfind('z') == compat::string_view::npos);
    TEST_ASSERT(sv_rfind.rfind("not_found") == compat::string_view::npos);

    // 5. 前後綴縮減 (remove_prefix / remove_suffix)
    compat::string_view sv_trim = sv1;
    sv_trim.remove_prefix(7);
    TEST_ASSERT(sv_trim == "Modern C++!");
    sv_trim.remove_suffix(1);
    TEST_ASSERT(sv_trim == "Modern C++");

    // 6. 比較運算子
    compat::string_view a = "abc";
    compat::string_view b = "abc";
    compat::string_view c = "def";
    TEST_ASSERT(a == b);
    TEST_ASSERT(a != c);
    TEST_ASSERT(a < c);
    TEST_ASSERT(a <= c);
    TEST_ASSERT(c > a);
    TEST_ASSERT(c >= a);

    // 7. 雜湊支援與 std::unordered_map 測試
    std::unordered_map<compat::string_view, int32_t> map;
    map["apple"] = 10;
    map["banana"] = 20;
    map["cherry"] = 30;
    TEST_ASSERT(map.size() == 3);
    TEST_ASSERT(map["apple"] == 10);
    TEST_ASSERT(map["banana"] == 20);
    TEST_ASSERT(map["cherry"] == 30);
    TEST_ASSERT(map.find("apple") != map.end());
    TEST_ASSERT(map.find("durian") == map.end());

    // 8. 串流輸出運算子 operator<<
    std::ostringstream oss;
    oss << sv1;
    TEST_ASSERT(oss.str() == "Hello, Modern C++!");

    std::cout << "[PASS] test_string_view passed." << std::endl;
}
