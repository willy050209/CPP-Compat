#include "test_helpers.hpp"
#include <compat/Format.hpp>
#include <string>
#include <cstdint>
#include <stdexcept>


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

    // 10. 格式化規格測試：寬度、對齊、填充、零填充、進位制、正負號、精度、截斷
    TEST_ASSERT(compat::format("{:8}", 1) == "       1");
    TEST_ASSERT(compat::format("{:<8}", 1) == "1       ");
    TEST_ASSERT(compat::format("{:>8}", 1) == "       1");
    TEST_ASSERT(compat::format("{:^8}", 1) == "   1    ");
    TEST_ASSERT(compat::format("{:*^8}", 1) == "***1****");
    TEST_ASSERT(compat::format("{:+}", 1) == "+1");
    TEST_ASSERT(compat::format("{:+}", -1) == "-1");
    TEST_ASSERT(compat::format("{: }", 1) == " 1");
    TEST_ASSERT(compat::format("{: }", -1) == "-1");
    TEST_ASSERT(compat::format("{:-}", 1) == "1");
    TEST_ASSERT(compat::format("{:08}", 1) == "00000001");
    TEST_ASSERT(compat::format("{:08d}", 42) == "00000042");
    TEST_ASSERT(compat::format("{:+08d}", 42) == "+0000042");
    TEST_ASSERT(compat::format("{:08d}", -42) == "-0000042");
    TEST_ASSERT(compat::format("{:x}", 255) == "ff");
    TEST_ASSERT(compat::format("{:X}", 255) == "FF");
    TEST_ASSERT(compat::format("{:#x}", 255) == "0xff");
    TEST_ASSERT(compat::format("{:#X}", 255) == "0XFF");
    TEST_ASSERT(compat::format("{:#b}", 5) == "0b101");
    TEST_ASSERT(compat::format("{:#B}", 5) == "0B101");
    TEST_ASSERT(compat::format("{:#o}", 64) == "0100");
    TEST_ASSERT(compat::format("{:.2f}", 3.14159) == "3.14");
    TEST_ASSERT(compat::format("{:8.2f}", 3.14159) == "    3.14");
    TEST_ASSERT(compat::format("{:<8.2f}", 3.14159) == "3.14    ");
    TEST_ASSERT(compat::format("{:+8.2f}", 3.14159) == "   +3.14");
    TEST_ASSERT(compat::format("{:8}", "hello") == "hello   ");
    TEST_ASSERT(compat::format("{:>8}", "hello") == "   hello");
    TEST_ASSERT(compat::format("{:.3s}", "hello") == "hel");
    TEST_ASSERT(compat::format("{:8.3s}", "hello") == "hel     ");
    TEST_ASSERT(compat::format("{:6}", true) == "true  ");
    TEST_ASSERT(compat::format("{:d}", true) == "1");
    TEST_ASSERT(compat::format("{:d}", false) == "0");
    TEST_ASSERT(compat::format("{:4}", 'a') == "a   ");
    TEST_ASSERT(compat::format("{:>4}", 'a') == "   a");
    TEST_ASSERT(compat::format("[{:8}] [{:<6}] [{:#06x}]", 1, "test", 42) == "[       1] [test  ] [0x002a]");

    // CJK & Unicode East Asian Width (UAX #11 / P1868R2)
    // Note: ISO C++ [format.string.std]/11 permits standard library implementations
    // that do not support UAX #11 to fall back to code units (e.g. libstdc++ in GCC 13).
    const bool supports_uax11 = (compat::format("{:4}", "一") == "一  ");
    if (supports_uax11) {
        TEST_ASSERT(compat::format("{:8}", "一號") == "一號    ");
        TEST_ASSERT(compat::format("{:<8}", "一號") == "一號    ");
        TEST_ASSERT(compat::format("{:>8}", "一號") == "    一號");
        TEST_ASSERT(compat::format("{:^8}", "一號") == "  一號  ");
        TEST_ASSERT(compat::format("{:*^8}", "一號") == "**一號**");
        TEST_ASSERT(compat::format("{:.2}", "一號二號") == "一");
        TEST_ASSERT(compat::format("{:.3}", "一號二號") == "一");
        TEST_ASSERT(compat::format("{:.4}", "一號二號") == "一號");
        TEST_ASSERT(compat::format("{:8.2}", "一號二號") == "一      ");
        TEST_ASSERT(compat::format("{:8}{:8}{:8}{:8}", "一號", "二號", "三號", "四號") == "一號    二號    三號    四號    ");
    } else {
        TEST_ASSERT(compat::format("{:8}", "一號") == "一號  ");
        TEST_ASSERT(compat::format("{:<8}", "一號") == "一號  ");
        TEST_ASSERT(compat::format("{:>8}", "一號") == "  一號");
    }

    // 11. Phase 3 & Contract Baseline Tests: TEST-FMT-001 through TEST-FMT-005
    // TEST-FMT-001: large precision overflow check (Memory safety / buffer overflow resistance)
    {
        std::string s_prec1 = compat::format("{:.6f}", 1.0);
        TEST_ASSERT(s_prec1 == "1.000000");

        std::string s_prec2 = compat::format("{:.127f}", 1.0);
        TEST_ASSERT(s_prec2.length() >= 128);

        std::string s_prec3 = compat::format("{:.128f}", 1.0);
        TEST_ASSERT(s_prec3.length() >= 129);

        std::string s_prec4 = compat::format("{:.200f}", 1.0);
        TEST_ASSERT(s_prec4.length() >= 201);
    }

    // TEST-FMT-002: manual argument indexing and arbitrary ordering / reuse
    {
        TEST_ASSERT(compat::format("{0} {1}", "Hello", "World") == "Hello World");
        TEST_ASSERT(compat::format("{1} {0}", "World", "Hello") == "Hello World");
        TEST_ASSERT(compat::format("{0} {0} {0}", "Echo") == "Echo Echo Echo");
        TEST_ASSERT(compat::format("{2} + {0} = {1}", 1, 3, 2) == "2 + 1 = 3");
        TEST_ASSERT(compat::format("{1:8} {0:<6}", "a", "b") == "b        a     ");
        TEST_ASSERT(compat::format("{1:>8} {0:<6}", "a", "b") == "       b a     ");
    }

    // TEST-FMT-003: index validation & error handling
#if COMPAT_HAS_EXCEPTIONS
    {
        // Out of bounds manual index
        TEST_ASSERT_THROWS(compat::format("{2}", 1, 2), std::invalid_argument);

        // Mixed automatic and manual indexing
        TEST_ASSERT_THROWS(compat::format("{0} {}", 1, 2), std::invalid_argument);
        TEST_ASSERT_THROWS(compat::format("{} {0}", 1, 2), std::invalid_argument);
    }
#endif

    // TEST-FMT-004: default floating precision preserves significant figures without 6-digit %g truncation
    {
        std::string flt_str = compat::format("{}", 1.23456789);
        // Under default %g (6 digits) it would be truncated to 1.23457
        // With round-trip precision it preserves at least 8 significant figures
        TEST_ASSERT(flt_str.find("1.2345678") != std::string::npos);
    }

    // TEST-FMT-005: Locale independence (floating point decimal is always dot)
    {
        std::string pi_str = compat::format("{}", 3.14159);
        TEST_ASSERT(pi_str.find('.') != std::string::npos);
        TEST_ASSERT(pi_str.find(',') == std::string::npos);

        std::string pi_prec = compat::format("{:.2f}", 3.14159);
        TEST_ASSERT(pi_prec == "3.14");
    }

    std::cout << "[PASS] test_format passed." << std::endl;
}

