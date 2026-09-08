#include "test_helpers.hpp"
#include <compat/Format.hpp>
#include <string>
#include <cstdint>

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

    std::cout << "[PASS] test_format passed." << std::endl;
}
