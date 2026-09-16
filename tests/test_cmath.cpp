#include "test_helpers.hpp"
#include <compat/CMath.hpp>
#include <compat/BigInt.hpp>
#include <compat/Decimal.hpp>
#include <string>
#include <cstdint>
#include <cmath>

/// <summary>
/// 執行 compat 數學函式庫 (CMath) 完整單元測試套件。
/// </summary>
void run_test_cmath() {
    using compat::bigint;
    using compat::decimal;

    std::cout << "--- Running CMath Tests ---" << std::endl;

    // ==========================================
    // 1. BigInt 數學函式測試
    // ==========================================
    {
        // abs
        TEST_ASSERT(compat::abs(bigint(0)) == 0);
        TEST_ASSERT(compat::abs(bigint(42)) == 42);
        TEST_ASSERT(compat::abs(bigint(-42)) == 42);
        bigint large_neg("-123456789012345678901234567890");
        bigint large_pos("123456789012345678901234567890");
        TEST_ASSERT(compat::abs(large_neg) == large_pos);

        // sqrt / isqrt
        TEST_ASSERT(compat::isqrt(bigint(0)) == 0);
        TEST_ASSERT(compat::isqrt(bigint(1)) == 1);
        TEST_ASSERT(compat::isqrt(bigint(2)) == 1);
        TEST_ASSERT(compat::isqrt(bigint(3)) == 1);
        TEST_ASSERT(compat::isqrt(bigint(4)) == 2);
        TEST_ASSERT(compat::isqrt(bigint(8)) == 2);
        TEST_ASSERT(compat::isqrt(bigint(9)) == 3);
        TEST_ASSERT(compat::isqrt(bigint(10)) == 3);
        TEST_ASSERT(compat::isqrt(bigint(99)) == 9);
        TEST_ASSERT(compat::isqrt(bigint(100)) == 10);
        TEST_ASSERT(compat::isqrt(bigint(144)) == 12);
        TEST_ASSERT(compat::sqrt(bigint(144)) == 12);

        // 大數平方根測試
        bigint b_sq("10000000000000000000000000000000000000000"); // 10^40
        bigint b_root("100000000000000000000"); // 10^20
        TEST_ASSERT(compat::isqrt(b_sq) == b_root);
        TEST_ASSERT(compat::isqrt(b_sq + 1) == b_root);
        TEST_ASSERT(compat::isqrt(b_sq - 1) == b_root - 1);

        // 負數平方根拋出 std::invalid_argument
        TEST_ASSERT_THROWS(compat::isqrt(bigint(-1)), std::invalid_argument);
        TEST_ASSERT_THROWS(compat::isqrt(bigint(-99)), std::invalid_argument);
        TEST_ASSERT_THROWS(compat::sqrt(bigint(-42)), std::invalid_argument);

        // cbrt / icbrt
        TEST_ASSERT(compat::icbrt(bigint(0)) == 0);
        TEST_ASSERT(compat::icbrt(bigint(1)) == 1);
        TEST_ASSERT(compat::icbrt(bigint(2)) == 1);
        TEST_ASSERT(compat::icbrt(bigint(7)) == 1);
        TEST_ASSERT(compat::icbrt(bigint(8)) == 2);
        TEST_ASSERT(compat::icbrt(bigint(26)) == 2);
        TEST_ASSERT(compat::icbrt(bigint(27)) == 3);
        TEST_ASSERT(compat::icbrt(bigint(-1)) == -1);
        TEST_ASSERT(compat::icbrt(bigint(-8)) == -2);
        TEST_ASSERT(compat::icbrt(bigint(-27)) == -3);
        TEST_ASSERT(compat::cbrt(bigint(-64)) == -4);
        TEST_ASSERT(compat::cbrt(bigint(125)) == 5);

        // 大數立方根測試
        bigint b_cube("1000000000000000000000000000000"); // 10^30
        bigint b_croot("10000000000"); // 10^10
        TEST_ASSERT(compat::icbrt(b_cube) == b_croot);
        TEST_ASSERT(compat::icbrt(b_cube + 1) == b_croot);
        TEST_ASSERT(compat::icbrt(b_cube - 1) == b_croot - 1);
        TEST_ASSERT(compat::icbrt(-b_cube) == -b_croot);

        // pow(bigint, unsigned int)
        TEST_ASSERT(compat::pow(bigint(2), 0u) == 1);
        TEST_ASSERT(compat::pow(bigint(2), 1u) == 2);
        TEST_ASSERT(compat::pow(bigint(2), 10u) == 1024);
        TEST_ASSERT(compat::pow(bigint(-2), 3u) == -8);
        TEST_ASSERT(compat::pow(bigint(-2), 4u) == 16);
        TEST_ASSERT(compat::pow(bigint(0), 0u) == 1);
        TEST_ASSERT(compat::pow(bigint(0), 5u) == 0);
        TEST_ASSERT(compat::pow(bigint(10), 15u) == bigint("1000000000000000"));

        // pow(bigint, bigint)
        TEST_ASSERT(compat::pow(bigint(3), bigint(4)) == 81);
        TEST_ASSERT(compat::pow(bigint(10), bigint(0)) == 1);
        TEST_ASSERT(compat::pow(bigint(0), bigint(0)) == 1);
        TEST_ASSERT(compat::pow(bigint(1), bigint(-5)) == 1);
        TEST_ASSERT(compat::pow(bigint(-1), bigint(-5)) == -1);
        TEST_ASSERT(compat::pow(bigint(-1), bigint(-6)) == 1);
        TEST_ASSERT_THROWS(compat::pow(bigint(2), bigint(-1)), std::invalid_argument);
        TEST_ASSERT_THROWS(compat::pow(bigint(0), bigint(-1)), std::invalid_argument);
    }

    // ==========================================
    // 2. Decimal 分類與基本數學函式測試
    // ==========================================
    {
        // 數值分類
        TEST_ASSERT(compat::isnan(decimal::nan) == true);
        TEST_ASSERT(compat::isnan(decimal(123)) == false);
        TEST_ASSERT(compat::isinf(decimal::infinity) == true);
        TEST_ASSERT(compat::isinf(-decimal::infinity) == true);
        TEST_ASSERT(compat::isinf(decimal(123)) == false);
        TEST_ASSERT(compat::isfinite(decimal(42)) == true);
        TEST_ASSERT(compat::isfinite(decimal::infinity) == false);
        TEST_ASSERT(compat::isfinite(decimal::nan) == false);
        TEST_ASSERT(compat::signbit(decimal("-5.5")) == true);
        TEST_ASSERT(compat::signbit(decimal("5.5")) == false);

        // copysign
        TEST_ASSERT(compat::copysign(decimal("3.5"), decimal("-1.0")) == decimal("-3.5"));
        TEST_ASSERT(compat::copysign(decimal("-3.5"), decimal("1.0")) == decimal("3.5"));
        TEST_ASSERT(compat::copysign(decimal("-3.5"), decimal("-2.0")) == decimal("-3.5"));
        TEST_ASSERT(compat::copysign(decimal("3.5"), decimal("2.0")) == decimal("3.5"));

        // abs
        TEST_ASSERT(compat::abs(decimal(0)) == 0);
        TEST_ASSERT(compat::abs(decimal("123.456")) == decimal("123.456"));
        TEST_ASSERT(compat::abs(decimal("-123.456")) == decimal("123.456"));
        TEST_ASSERT(compat::abs(decimal::infinity) == decimal::infinity);
        TEST_ASSERT(compat::abs(-decimal::infinity) == decimal::infinity);
        TEST_ASSERT(compat::isnan(compat::abs(decimal::nan)));
    }

    // ==========================================
    // 3. Decimal 捨入、取模與餘數函式測試
    // ==========================================
    {
        // floor
        TEST_ASSERT(compat::floor(decimal("2.7")) == 2);
        TEST_ASSERT(compat::floor(decimal("2.0")) == 2);
        TEST_ASSERT(compat::floor(decimal("-2.3")) == -3);
        TEST_ASSERT(compat::floor(decimal("-2.0")) == -2);
        TEST_ASSERT(compat::floor(decimal(0)) == 0);

        // ceil
        TEST_ASSERT(compat::ceil(decimal("2.3")) == 3);
        TEST_ASSERT(compat::ceil(decimal("2.0")) == 2);
        TEST_ASSERT(compat::ceil(decimal("-2.7")) == -2);
        TEST_ASSERT(compat::ceil(decimal("-2.0")) == -2);
        TEST_ASSERT(compat::ceil(decimal(0)) == 0);

        // trunc
        TEST_ASSERT(compat::trunc(decimal("2.7")) == 2);
        TEST_ASSERT(compat::trunc(decimal("2.0")) == 2);
        TEST_ASSERT(compat::trunc(decimal("-2.7")) == -2);
        TEST_ASSERT(compat::trunc(decimal("-2.0")) == -2);
        TEST_ASSERT(compat::trunc(decimal(0)) == 0);

        // round (Half away from zero)
        TEST_ASSERT(compat::round(decimal("2.3")) == 2);
        TEST_ASSERT(compat::round(decimal("2.5")) == 3);
        TEST_ASSERT(compat::round(decimal("2.7")) == 3);
        TEST_ASSERT(compat::round(decimal("-2.3")) == -2);
        TEST_ASSERT(compat::round(decimal("-2.5")) == -3);
        TEST_ASSERT(compat::round(decimal("-2.7")) == -3);
        TEST_ASSERT(compat::round(decimal("3.5")) == 4);
        TEST_ASSERT(compat::round(decimal("-3.5")) == -4);

        // fmod: x - trunc(x/y) * y
        TEST_ASSERT(compat::fmod(decimal("5.3"), decimal("2.0")) == decimal("1.3"));
        TEST_ASSERT(compat::fmod(decimal("-5.3"), decimal("2.0")) == decimal("-1.3"));
        TEST_ASSERT(compat::fmod(decimal("5.3"), decimal("-2.0")) == decimal("1.3"));
        TEST_ASSERT(compat::fmod(decimal("5.0"), decimal("2.5")) == 0);

        // remainder: x - round(x/y) * y
        TEST_ASSERT(compat::remainder(decimal("5.0"), decimal("2.0")) == decimal("1.0"));
        TEST_ASSERT(compat::remainder(decimal("7.0"), decimal("2.0")) == decimal("-1.0"));
        TEST_ASSERT(compat::remainder(decimal("7.0"), decimal("3.0")) == decimal("1.0"));
    }

    // ==========================================
    // 4. Decimal 平方根、立方根與 hypot 測試
    // ==========================================
    {
        // sqrt
        TEST_ASSERT(compat::sqrt(decimal(0)) == 0);
        TEST_ASSERT(compat::sqrt(decimal(1)) == 1);
        TEST_ASSERT(compat::sqrt(decimal(4)) == 2);
        TEST_ASSERT(compat::sqrt(decimal(9)) == 3);
        TEST_ASSERT(compat::sqrt(decimal(100)) == 10);
        TEST_ASSERT(compat::sqrt(decimal("0.25")) == decimal("0.5"));
        TEST_ASSERT(compat::sqrt(decimal::infinity) == decimal::infinity);
        TEST_ASSERT(compat::isnan(compat::sqrt(decimal(-1))));

        // 34 位有效位數精度驗證
        decimal sqrt2 = compat::sqrt(decimal(2));
        decimal diff2 = compat::abs(sqrt2 * sqrt2 - decimal(2));
        TEST_ASSERT(diff2 < decimal("1e-30"));

        decimal big_val("123456789012345678901234567890.1234");
        decimal r_big = compat::sqrt(big_val);
        decimal diff_big = compat::abs(r_big * r_big - big_val);
        TEST_ASSERT(diff_big / big_val < decimal("1e-30"));

        // cbrt
        TEST_ASSERT(compat::cbrt(decimal(0)) == 0);
        TEST_ASSERT(compat::cbrt(decimal(1)) == 1);
        TEST_ASSERT(compat::cbrt(decimal(8)) == 2);
        TEST_ASSERT(compat::cbrt(decimal(-8)) == -2);
        TEST_ASSERT(compat::cbrt(decimal(27)) == 3);
        TEST_ASSERT(compat::cbrt(decimal(-27)) == -3);
        TEST_ASSERT(compat::cbrt(decimal::infinity) == decimal::infinity);
        TEST_ASSERT(compat::cbrt(-decimal::infinity) == -decimal::infinity);

        decimal cbrt2 = compat::cbrt(decimal(2));
        decimal diff_cb = compat::abs(cbrt2 * cbrt2 * cbrt2 - decimal(2));
        TEST_ASSERT(diff_cb < decimal("1e-30"));

        // hypot
        TEST_ASSERT(compat::hypot(decimal(3), decimal(4)) == 5);
        TEST_ASSERT(compat::hypot(decimal(5), decimal(12)) == 13);
        TEST_ASSERT(compat::hypot(decimal(0), decimal(7)) == 7);
        TEST_ASSERT(compat::hypot(decimal(-3), decimal(4)) == 5);
        TEST_ASSERT(compat::hypot(decimal::infinity, decimal(100)) == decimal::infinity);
    }

    // ==========================================
    // 5. Decimal 指數與對數函式測試
    // ==========================================
    {
        // exp
        TEST_ASSERT(compat::exp(decimal(0)) == 1);
        TEST_ASSERT(compat::exp(decimal::infinity) == decimal::infinity);
        TEST_ASSERT(compat::exp(-decimal::infinity) == 0);
        TEST_ASSERT(compat::isnan(compat::exp(decimal::nan)));

        decimal e1 = compat::exp(decimal(1));
        decimal exp_expected("2.718281828459045235360287471352662");
        TEST_ASSERT(compat::abs(e1 - exp_expected) < decimal("1e-30"));

        decimal e_neg1 = compat::exp(decimal(-1));
        TEST_ASSERT(compat::abs(e1 * e_neg1 - decimal(1)) < decimal("1e-30"));

        // log
        TEST_ASSERT(compat::log(decimal(1)) == 0);
        TEST_ASSERT(compat::isnan(compat::log(decimal(-1))));
        TEST_ASSERT(compat::log(decimal(0)) == -decimal::infinity);
        TEST_ASSERT(compat::log(decimal::infinity) == decimal::infinity);

        decimal ln_e = compat::log(e1);
        TEST_ASSERT(compat::abs(ln_e - decimal(1)) < decimal("1e-30"));

        decimal ln2 = compat::log(decimal(2));
        decimal ln2_expected("0.6931471805599453094172321214581765");
        TEST_ASSERT(compat::abs(ln2 - ln2_expected) < decimal("1e-30"));

        // 互逆性驗證 log(exp(x)) == x
        decimal x_val("3.14159");
        TEST_ASSERT(compat::abs(compat::log(compat::exp(x_val)) - x_val) < decimal("1e-28"));

        // log10
        TEST_ASSERT(compat::log10(decimal(1)) == 0);
        TEST_ASSERT(compat::log10(decimal(10)) == 1);
        TEST_ASSERT(compat::log10(decimal(100)) == 2);
        TEST_ASSERT(compat::log10(decimal(1000)) == 3);
        TEST_ASSERT(compat::log10(decimal("0.1")) == -1);
        TEST_ASSERT(compat::log10(decimal("0.01")) == -2);

        // log2
        TEST_ASSERT(compat::log2(decimal(1)) == 0);
        TEST_ASSERT(compat::log2(decimal(2)) == 1);
        TEST_ASSERT(compat::log2(decimal(8)) == 3);
        TEST_ASSERT(compat::log2(decimal(1024)) == 10);
        TEST_ASSERT(compat::log2(decimal("0.5")) == -1);
        TEST_ASSERT(compat::log2(decimal("0.25")) == -2);
    }

    // ==========================================
    // 6. Decimal 冪次 pow 函式測試
    // ==========================================
    {
        // pow(decimal, int)
        TEST_ASSERT(compat::pow(decimal(2), 0) == 1);
        TEST_ASSERT(compat::pow(decimal(2), 3) == 8);
        TEST_ASSERT(compat::pow(decimal(2), -2) == decimal("0.25"));
        TEST_ASSERT(compat::pow(decimal("2.5"), 2) == decimal("6.25"));
        TEST_ASSERT(compat::pow(decimal("-2"), 3) == -8);
        TEST_ASSERT(compat::pow(decimal("-2"), 4) == 16);

        // pow(decimal, decimal)
        TEST_ASSERT(compat::pow(decimal(5), decimal(0)) == 1);
        TEST_ASSERT(compat::pow(decimal(0), decimal(0)) == 1);
        TEST_ASSERT(compat::pow(decimal(1), decimal("3.14")) == 1);
        TEST_ASSERT(compat::pow(decimal(4), decimal("0.5")) == 2);
        TEST_ASSERT(compat::abs(compat::pow(decimal(27), decimal(1) / decimal(3)) - decimal(3)) < decimal("1e-28"));
        TEST_ASSERT(compat::isnan(compat::pow(decimal(-2), decimal("0.5"))));
    }

    // ==========================================
    // 7. Decimal 三角函式測試
    // ==========================================
    {
        // sin, cos, tan
        TEST_ASSERT(compat::sin(decimal(0)) == 0);
        TEST_ASSERT(compat::cos(decimal(0)) == 1);
        TEST_ASSERT(compat::tan(decimal(0)) == 0);
        TEST_ASSERT(compat::isnan(compat::sin(decimal::infinity)));
        TEST_ASSERT(compat::isnan(compat::cos(decimal::infinity)));

        decimal pi("3.141592653589793238462643383279502884");
        decimal pi_over_2 = pi / decimal(2);
        decimal pi_over_4 = pi / decimal(4);
        decimal pi_over_6 = pi / decimal(6);
        decimal pi_over_3 = pi / decimal(3);

        // sin(pi) 接近 0, cos(pi) 接近 -1
        TEST_ASSERT(compat::abs(compat::sin(pi)) < decimal("1e-30"));
        TEST_ASSERT(compat::abs(compat::cos(pi) - decimal(-1)) < decimal("1e-30"));

        // sin(pi/2) 接近 1, cos(pi/2) 接近 0
        TEST_ASSERT(compat::abs(compat::sin(pi_over_2) - decimal(1)) < decimal("1e-30"));
        TEST_ASSERT(compat::abs(compat::cos(pi_over_2)) < decimal("1e-30"));

        // sin(pi/6) 接近 0.5, cos(pi/3) 接近 0.5
        TEST_ASSERT(compat::abs(compat::sin(pi_over_6) - decimal("0.5")) < decimal("1e-30"));
        TEST_ASSERT(compat::abs(compat::cos(pi_over_3) - decimal("0.5")) < decimal("1e-30"));

        // tan(pi/4) 接近 1
        TEST_ASSERT(compat::abs(compat::tan(pi_over_4) - decimal(1)) < decimal("1e-25"));

        // 恆等式 sin^2(x) + cos^2(x) == 1
        decimal test_ang("1.23456");
        decimal s = compat::sin(test_ang);
        decimal c = compat::cos(test_ang);
        TEST_ASSERT(compat::abs((s * s + c * c) - decimal(1)) < decimal("1e-30"));
    }

    // ==========================================
    // 8. 命名空間與 ADL 機制驗證
    // ==========================================
    {
        // ADL 測試（無前綴自動查找 compat::sqrt）
        bigint b(100);
        decimal d(100);
        TEST_ASSERT(sqrt(b) == 10);
        TEST_ASSERT(sqrt(d) == 10);

        // std:: 命名空間直接呼叫驗證
        TEST_ASSERT(std::abs(bigint(-50)) == 50);
        TEST_ASSERT(std::abs(decimal("-50.5")) == decimal("50.5"));
        TEST_ASSERT(std::sqrt(bigint(64)) == 8);
        TEST_ASSERT(std::sqrt(decimal(64)) == 8);
        TEST_ASSERT(std::cbrt(decimal(64)) == 4);
        TEST_ASSERT(std::floor(decimal("3.7")) == 3);
        TEST_ASSERT(std::ceil(decimal("3.2")) == 4);
        TEST_ASSERT(std::round(decimal("3.5")) == 4);
        TEST_ASSERT(std::pow(decimal(2), 5) == 32);
    }

    std::cout << "All CMath tests passed successfully!" << std::endl;
}
