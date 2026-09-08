#include "test_helpers.hpp"
#include <compat/Print.hpp>
#include <sstream>
#include <string>
#include <cstdint>

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

    std::cout << "[PASS] test_print passed." << std::endl;
}
