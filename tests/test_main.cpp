#include "test_helpers.hpp"
#include <iostream>

void run_test_expected();
void run_test_format();
void run_test_parse();
void run_test_print();
void run_test_string_view();
void run_test_bigint();
void run_test_decimal();
void run_test_cmath();
void run_test_bitset();

/// <summary>
/// 單元測試套件整合進入點。
/// </summary>
/// <returns>測試結果狀態碼 (0 代表全部通過)</returns>
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  Starting CPP-Compat Full Test Suite   " << std::endl;
    std::cout << "========================================" << std::endl;

    run_test_string_view();
    run_test_expected();
    run_test_format();
    run_test_print();
    run_test_parse();
    run_test_bigint();
    run_test_decimal();
    run_test_cmath();
    run_test_bitset();

    TestStats& stats = GetGlobalTestStats();
    std::cout << "========================================" << std::endl;
    std::cout << "Test Summary:" << std::endl;
    std::cout << "  Total Assertions: " << stats.total << std::endl;
    std::cout << "  Passed:           " << stats.passed << std::endl;
    std::cout << "  Failed:           " << stats.failed << std::endl;
    std::cout << "========================================" << std::endl;

    if (stats.failed == 0) {
        std::cout << "ALL TESTS PASSED SUCCESSFULLY!" << std::endl;
        return 0;
    } else {
        std::cerr << "TEST SUITE FAILED WITH " << stats.failed << " ERRORS." << std::endl;
        return 1;
    }
}
