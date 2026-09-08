#include "test_helpers.hpp"
#include <compat/Expected.hpp>
#include <string>
#include <cstdint>
#include <stdexcept>

/// <summary>
/// 測試 compat::expected 與 compat::unexpected 之功能完整性。
/// 包含 has_value(), value(), error(), 例外拋出 (std::logic_error) 與 operator bool()。
/// </summary>
void run_test_expected() {
    std::cout << "[TEST] Running test_expected..." << std::endl;

    // 1. 測試正常值狀態 (Value State)
    compat::expected<int32_t, std::string> val_exp(12345);
    TEST_ASSERT(val_exp.has_value());
    TEST_ASSERT(static_cast<bool>(val_exp));
    TEST_ASSERT(val_exp.value() == 12345);
    TEST_ASSERT(*val_exp == 12345);

    // 2. 測試錯誤狀態 (Error State)
    compat::expected<int32_t, std::string> err_exp(compat::unexpected<std::string>("NetworkTimeout"));
    TEST_ASSERT(!err_exp.has_value());
    TEST_ASSERT(!static_cast<bool>(err_exp));
    TEST_ASSERT(err_exp.error() == "NetworkTimeout");

    // 例外拋出測試：存取錯誤狀態之 value() 應拋出例外
    // 自研 Fallback 拋出 std::logic_error，C++23 原生拋出 std::bad_expected_access (繼承自 std::exception)
#if !COMPAT_HAS_STD_EXPECTED
    TEST_ASSERT_THROWS(val_exp.error(), std::logic_error);
    TEST_ASSERT_THROWS(err_exp.value(), std::logic_error);
#else
    TEST_ASSERT_THROWS(err_exp.value(), std::exception);
#endif

    // 3. 測試字串與指標存取運算子 operator->()
    compat::expected<std::string, int32_t> str_exp("Modern C++ Compatibility");
    TEST_ASSERT(str_exp.has_value());
    TEST_ASSERT(str_exp->length() == 24);
    TEST_ASSERT(str_exp.value() == "Modern C++ Compatibility");

    // 4. 測試拷貝與賦值
    compat::expected<int32_t, std::string> copied = val_exp;
    TEST_ASSERT(copied.has_value());
    TEST_ASSERT(copied.value() == 12345);

    compat::expected<int32_t, std::string> assigned(0);
    assigned = err_exp;
    TEST_ASSERT(!assigned.has_value());
    TEST_ASSERT(assigned.error() == "NetworkTimeout");

    std::cout << "[PASS] test_expected passed." << std::endl;
}
