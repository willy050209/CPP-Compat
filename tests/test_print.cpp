#define _CRT_SECURE_NO_WARNINGS
#include "test_helpers.hpp"
#include <compat/Print.hpp>
#include <sstream>
#include <string>
#include <cstdint>
#include <cstdio>

#if COMPAT_HAS_STD_FORMAT
#  include <format>
#endif

/// <summary>
/// A Color struct for testing custom formatter support in print/println.
/// </summary>
struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

#if COMPAT_HAS_STD_FORMAT
template <>
struct std::formatter<Color> : std::formatter<std::string> {
    auto format(const Color& c, std::format_context& ctx) const {
        return std::formatter<std::string>::format(
            "rgb(" + std::to_string(c.r) + ", " + std::to_string(c.g) + ", " + std::to_string(c.b) + ")", ctx);
    }
};
#else
template <>
struct compat::formatter<Color, char> {
    template <typename FormatContext>
    auto format(const Color& c, FormatContext& ctx) const -> decltype(ctx.out()) {
        std::string s = "rgb(" + std::to_string(c.r) + ", " + std::to_string(c.g) + ", " + std::to_string(c.b) + ")";
        auto it = ctx.out();
        for (char ch : s) {
            *it++ = ch;
        }
        ctx.advance_to(it);
        return it;
    }
};
#endif

/// <summary>
/// 測試 compat::print 與 compat::println 之串流與標準輸出行為。
/// </summary>
void run_test_print() {
    std::cout << "[TEST] Running test_print..." << std::endl;

    // 1. 測試 compat::print 輸出至 std::ostream
    std::ostringstream oss1;
    compat::print(oss1, "Key: {}, Value: {}", "Port", 8080);
    TEST_ASSERT(oss1.str() == "Key: Port, Value: 8080");

    // 2. 測試 compat::println 輸出至 std::ostream (包含換行)
    std::ostringstream oss2;
    compat::println(oss2, "Status: {}", "SUCCESS");
    TEST_ASSERT(oss2.str() == "Status: SUCCESS\n");

    // 3. 測試無引數 println 換行
    std::ostringstream oss3;
    compat::println(oss3);
    TEST_ASSERT(oss3.str() == "\n");

    // 4. 測試標準輸出 (確保不崩潰且能正常執行)
    compat::print("   [Console Output Test] compat::print: value = {}\n", 999);
    compat::println("   [Console Output Test] compat::println: message = {}", "Hello from test_print");
    compat::println();

    // 5. 測試 UTF-8 特殊字元輸出 (包含繁體中文、日文、Emoji 等)
    compat::println("   [Console Output UTF-8] 繁體中文測試: 現代 C++ 格式化輸出 🚀");
    compat::println("   [Console Output UTF-8] こんにちは世界 (UTF-8 Japanese Test)");

    // 6. 測試自訂型別 Color 輸出至串流與標準輸出
    std::ostringstream oss_color;
    Color cyan{0, 255, 255};
    compat::println(oss_color, "Theme color: {}", cyan);
    TEST_ASSERT(oss_color.str() == "Theme color: rgb(0, 255, 255)\n");

    // 7. 測試 compat::print 輸出至 std::FILE* (使用 std::tmpfile)
    std::FILE* tf1 = std::tmpfile();
    TEST_ASSERT(tf1 != nullptr);
    compat::print(tf1, "Key: {}, Value: {}", "Port", 8080);
    std::rewind(tf1);
    char buf1[64] = {0};
    std::size_t n1 = std::fread(buf1, 1, sizeof(buf1) - 1, tf1);
    buf1[n1] = '\0';
    std::fclose(tf1);
    TEST_ASSERT(std::string(buf1) == "Key: Port, Value: 8080");

    // 8. 測試 compat::println 輸出至 std::FILE* (包含換行)
    std::FILE* tf2 = std::tmpfile();
    TEST_ASSERT(tf2 != nullptr);
    compat::println(tf2, "Status: {}", "SUCCESS");
    std::rewind(tf2);
    char buf2[64] = {0};
    std::size_t n2 = std::fread(buf2, 1, sizeof(buf2) - 1, tf2);
    buf2[n2] = '\0';
    std::fclose(tf2);
    TEST_ASSERT(std::string(buf2) == "Status: SUCCESS\n");

    // 9. 測試 compat::println(std::FILE*) 單獨輸出換行
    std::FILE* tf3 = std::tmpfile();
    TEST_ASSERT(tf3 != nullptr);
    compat::println(tf3);
    std::rewind(tf3);
    char buf3[16] = {0};
    std::size_t n3 = std::fread(buf3, 1, sizeof(buf3) - 1, tf3);
    buf3[n3] = '\0';
    std::fclose(tf3);
    TEST_ASSERT(std::string(buf3) == "\n");

    // 10. 測試自訂型別 Color 輸出至 std::FILE*
    std::FILE* tf4 = std::tmpfile();
    TEST_ASSERT(tf4 != nullptr);
    Color magenta{255, 0, 255};
    compat::println(tf4, "Color: {}", magenta);
    std::rewind(tf4);
    char buf4[64] = {0};
    std::size_t n4 = std::fread(buf4, 1, sizeof(buf4) - 1, tf4);
    buf4[n4] = '\0';
    std::fclose(tf4);
    TEST_ASSERT(std::string(buf4) == "Color: rgb(255, 0, 255)\n");

    // 11. 測試標準串流 stdout 與 stderr (確保不崩潰且能正常執行)
    compat::print(stdout, "   [FILE* stdout Test] code = {}\n", 200);
    compat::println(stdout, "   [FILE* stdout Test] msg = {}", "OK");
    compat::println(stdout);
    compat::print(stderr, "   [FILE* stderr Test] notice = {}\n", "debug info");

    // 12. 測試 nullptr 防護 (快速失敗)
#if COMPAT_HAS_EXCEPTIONS
    TEST_ASSERT_THROWS(compat::print(static_cast<std::FILE*>(nullptr), "test"), std::invalid_argument);
    TEST_ASSERT_THROWS(compat::println(static_cast<std::FILE*>(nullptr), "test"), std::invalid_argument);
    TEST_ASSERT_THROWS(compat::println(static_cast<std::FILE*>(nullptr)), std::invalid_argument);
#endif

    std::cout << "[PASS] test_print passed." << std::endl;
}
