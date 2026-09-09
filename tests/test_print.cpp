#include "test_helpers.hpp"
#include <compat/Print.hpp>
#include <sstream>
#include <string>
#include <cstdint>

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
    auto format(const Color& c, FormatContext& ctx) const {
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

    std::cout << "[PASS] test_print passed." << std::endl;
}
