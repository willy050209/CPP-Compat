#include "test_helpers.hpp"
#include <compat/Format.hpp>
#include <compat/BigInt.hpp>
#include <compat/Decimal.hpp>
#include <string>
#include <cstdint>
#include <stdexcept>
#include <unordered_set>

#if COMPAT_HAS_STD_FORMAT
#  include <format>
#endif

/// <summary>
/// A 2D point structure for testing custom formatter specialization.
/// </summary>
struct Point {
    int32_t x;
    int32_t y;
};

// Specialize formatter for Point
#if COMPAT_HAS_STD_FORMAT
template <>
struct std::formatter<Point> : std::formatter<std::string> {
    auto format(const Point& p, std::format_context& ctx) const {
        return std::formatter<std::string>::format(
            "(" + std::to_string(p.x) + ", " + std::to_string(p.y) + ")", ctx);
    }
};
#else
template <>
struct compat::formatter<Point, char> {
    template <typename FormatContext>
    auto format(const Point& p, FormatContext& ctx) const -> decltype(ctx.out()) {
        std::string s = "(" + std::to_string(p.x) + ", " + std::to_string(p.y) + ")";
        auto it = ctx.out();
        for (char c : s) {
            *it++ = c;
        }
        ctx.advance_to(it);
        return it;
    }
};
#endif

/// <summary>
/// 測試 compat::format 字串與佔位符替換功能。
/// 支援各種類型引數、多佔位符替換與空字串格式化。
/// </summary>
void run_test_format() {
    std::cout << "[TEST] Running test_format..." << std::endl;

    // 1. 無佔位符字串
    std::string s1 = compat::format("Hello, World!");
    TEST_ASSERT(s1 == "Hello, World!");

    // 2. 單一佔位符替換
    std::string s2 = compat::format("Hello, {}!", "CPP-Compat");
    TEST_ASSERT(s2 == "Hello, CPP-Compat!");

    // 3. 多型態多佔位符替換 (整數、浮點數、字串、布林)
    int32_t val = 42;
    const char* status = "OK";
    std::string s3 = compat::format("Answer: {}, Status: {}, Code: {}", val, status, 200);
    TEST_ASSERT(s3 == "Answer: 42, Status: OK, Code: 200");

    // 4. 空格式字串
    std::string s4 = compat::format("");
    TEST_ASSERT(s4 == "");

    // 5. 連續佔位符
    std::string s5 = compat::format("{}-{}-{}", 2026, 9, 8);
    TEST_ASSERT(s5 == "2026-9-8");

    // 6. 字串型態 (std::string 與 string_view)
    std::string std_str = "Standard";
    compat::string_view sv = "View";
    std::string s6 = compat::format("Types: {} and {}", std_str, sv);
    TEST_ASSERT(s6 == "Types: Standard and View");

    // 7. 自訂結構體 Formatter 擴充點測試
    Point pt{12, 34};
    std::string s7 = compat::format("Point is {}", pt);
    TEST_ASSERT(s7 == "Point is (12, 34)");

    // 8. 雙大括號跳脫 {{ 與 }}
    std::string s8 = compat::format("Escaped: {{ and }} and value: {}", 100);
    TEST_ASSERT(s8 == "Escaped: { and } and value: 100");

    std::string s9 = compat::format("{{literal braces}}");
    TEST_ASSERT(s9 == "{literal braces}");

    // 9. 格式化字串引數數量不符與格式錯誤例外測試 (Fallback 模式下之執行期檢查)
#if COMPAT_HAS_EXCEPTIONS
#  if !COMPAT_HAS_STD_FORMAT
    std::string too_few_args_fmt = "Needs two: {} and {}";
    TEST_ASSERT_THROWS(compat::format(too_few_args_fmt, 1), std::exception);

    std::string too_many_args_fmt = "Needs one: {}";
    TEST_ASSERT_THROWS(compat::format(too_many_args_fmt, 1, 2), std::exception);

    std::string unmatched_open_fmt = "Unmatched { brace";
    TEST_ASSERT_THROWS(compat::format(unmatched_open_fmt), std::exception);

    std::string unmatched_close_fmt = "Unmatched } brace";
    TEST_ASSERT_THROWS(compat::format(unmatched_close_fmt), std::exception);
#  else
    // C++20 原生 std::format 於編譯期檢查字串常數，執行期動態字串透過 std::vformat 驗證
    std::string too_few_args_fmt = "Needs two: {} and {}";
    int32_t one = 1;
    TEST_ASSERT_THROWS(std::vformat(too_few_args_fmt, std::make_format_args(one)), std::format_error);
#  endif
#endif

    // 10. compat::bigint 格式化輸出
    std::string s_bi1 = compat::format("val: {}", compat::bigint(123456789));
    TEST_ASSERT(s_bi1 == "val: 123456789");

    std::string s_bi2 = compat::format("negative: {}", compat::bigint(-987654321));
    TEST_ASSERT(s_bi2 == "negative: -987654321");

    std::string s_bi3 = compat::format("zero: {}", compat::bigint(0));
    TEST_ASSERT(s_bi3 == "zero: 0");

    std::string s_bi4 = compat::format("large: {}", compat::bigint("123456789012345678901234567890"));
    TEST_ASSERT(s_bi4 == "large: 123456789012345678901234567890");

    // 11. compat::decimal 格式化輸出 ({}, {:.Nf}, {:.N})
    std::string s_dec1 = compat::format("pi: {:.2f}", compat::decimal("3.14159"));
    TEST_ASSERT(s_dec1 == "pi: 3.14");

    std::string s_dec2 = compat::format("pi: {:.4f}", compat::decimal("3.14159"));
    TEST_ASSERT(s_dec2 == "pi: 3.1416");

    std::string s_dec3 = compat::format("val: {:.2f}", compat::decimal("123.456"));
    TEST_ASSERT(s_dec3 == "val: 123.46");

    std::string s_dec4 = compat::format("val: {:.2}", compat::decimal("123.456"));
    TEST_ASSERT(s_dec4 == "val: 123.46");

    std::string s_dec5 = compat::format("pad: {:.2f}", compat::decimal("123.4"));
    TEST_ASSERT(s_dec5 == "pad: 123.40");

    std::string s_dec6 = compat::format("zero: {:.3f}", compat::decimal("0"));
    TEST_ASSERT(s_dec6 == "zero: 0.000");

    std::string s_dec7 = compat::format("default: {}", compat::decimal("42.5"));
    TEST_ASSERT(s_dec7 == "default: 42.5");

    // 12. std::hash<compat::bigint> 與 std::unordered_set<compat::bigint>
    {
        std::unordered_set<compat::bigint> bi_set;
        bi_set.insert(compat::bigint(123456789));
        bi_set.insert(compat::bigint(-42));
        bi_set.insert(compat::bigint(0));
        bi_set.insert(compat::bigint("123456789012345678901234567890"));

        TEST_ASSERT(bi_set.count(compat::bigint(123456789)) == 1);
        TEST_ASSERT(bi_set.count(compat::bigint(-42)) == 1);
        TEST_ASSERT(bi_set.count(compat::bigint(0)) == 1);
        TEST_ASSERT(bi_set.count(compat::bigint("123456789012345678901234567890")) == 1);
        TEST_ASSERT(bi_set.count(compat::bigint(999)) == 0);

        // 重複插入等值鍵不應增加大小
        size_t orig_size = bi_set.size();
        bi_set.insert(compat::bigint(123456789));
        TEST_ASSERT(bi_set.size() == orig_size);
    }

    // 13. std::hash<compat::decimal> 與 std::unordered_set<compat::decimal>
    {
        std::unordered_set<compat::decimal> dec_set;
        dec_set.insert(compat::decimal("1.0"));
        dec_set.insert(compat::decimal("3.14159"));
        dec_set.insert(compat::decimal("0"));
        dec_set.insert(compat::decimal("-42.5"));

        // 驗證規格化後等值鍵（如 1.0 與 1.00、0 與 0.000）正確命中
        TEST_ASSERT(dec_set.count(compat::decimal("1.00")) == 1);
        TEST_ASSERT(dec_set.count(compat::decimal("1")) == 1);
        TEST_ASSERT(dec_set.count(compat::decimal("0.000")) == 1);
        TEST_ASSERT(dec_set.count(compat::decimal("-42.500")) == 1);
        TEST_ASSERT(dec_set.count(compat::decimal("99.9")) == 0);

        // 等值鍵插入驗證去重
        size_t orig_dec_size = dec_set.size();
        dec_set.insert(compat::decimal("1.0000"));
        TEST_ASSERT(dec_set.size() == orig_dec_size);
    }

    std::cout << "[PASS] test_format passed." << std::endl;
}
