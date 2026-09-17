
#include "test_helpers.hpp"
#include <compat/BigInt.hpp>
#include <compat/Decimal.hpp>
#include <compat/CMath.hpp>
#include <iostream>

#if (COMPAT_CPLUSPLUS >= COMPAT_CXX_20)

// ----------------------------------------------------------------------------
// 編譯期驗證：BigInt
// ----------------------------------------------------------------------------
namespace test_ce_bigint {
    // 1. 常數建構與常數代理
    constexpr compat::bigint b0;
    static_assert(b0 == 0, "bigint default ctor must be 0");
    static_assert(b0.is_zero(), "bigint b0 must be zero");
    static_assert(b0.is_sbo(), "bigint b0 must be SBO");

    constexpr compat::bigint b_proxy_zero = compat::bigint::zero;
    constexpr compat::bigint b_proxy_one = compat::bigint::one();
    static_assert(b_proxy_zero == 0, "bigint::zero proxy check");
    static_assert(b_proxy_one == 1, "bigint::one proxy check");

    constexpr compat::bigint b42(42);
    constexpr compat::bigint b_neg(-100);
    static_assert(b42 == 42, "bigint(42) == 42");
    static_assert(b_neg == -100, "bigint(-100) == -100");
    static_assert(b42.sign() == 1, "b42 sign must be 1");
    static_assert(b_neg.sign() == -1, "b_neg sign must be -1");

    // 2. 算術運算
    constexpr compat::bigint sum = b42 + 8;
    static_assert(sum == 50, "42 + 8 == 50");

    constexpr compat::bigint diff = b42 - 50;
    static_assert(diff == -8, "42 - 50 == -8");

    constexpr compat::bigint prod = b42 * 2;
    static_assert(prod == 84, "42 * 2 == 84");

    constexpr compat::bigint quot = b42 / 10;
    static_assert(quot == 4, "42 / 10 == 4");

    constexpr compat::bigint rem = b42 % 10;
    static_assert(rem == 2, "42 % 10 == 2");

    // 3. 位元運算與位移
    constexpr compat::bigint b_and = compat::bigint(0b1100) & compat::bigint(0b1010);
    static_assert(b_and == 0b1000, "0b1100 & 0b1010 == 0b1000");

    constexpr compat::bigint b_or = compat::bigint(0b1100) | compat::bigint(0b1010);
    static_assert(b_or == 0b1110, "0b1100 | 0b1010 == 0b1110");

    constexpr compat::bigint b_xor = compat::bigint(0b1100) ^ compat::bigint(0b1010);
    static_assert(b_xor == 0b0110, "0b1100 ^ 0b1010 == 0b0110");

    constexpr compat::bigint b_shl = compat::bigint(1) << 10;
    static_assert(b_shl == 1024, "1 << 10 == 1024");

    constexpr compat::bigint b_shr = compat::bigint(1024) >> 5;
    static_assert(b_shr == 32, "1024 >> 5 == 32");

    // 4. 比較運算與邏輯運算
    static_assert(b42 > 10, "42 > 10");
    static_assert(b42 >= 42, "42 >= 42");
    static_assert(b42 < 100, "42 < 100");
    static_assert(b42 <= 42, "42 <= 42");
    static_assert(b42 != 0, "42 != 0");
    static_assert(static_cast<bool>(b42), "b42 is truthy");
    static_assert(!compat::bigint(0), "bigint(0) is falsy");
    static_assert((b42 && true) == true, "b42 && true");
    static_assert((compat::bigint(0) || false) == false, "0 || false");

    // 5. 常數表達式函式求值
    constexpr compat::bigint calc_factorial(int n) {
        compat::bigint r = 1;
        for (int i = 2; i <= n; ++i) {
            r *= i;
        }
        return r;
    }
    constexpr compat::bigint fact10 = calc_factorial(10);
    static_assert(fact10 == 3628800, "10! == 3628800");

    // 6. 字串解析
    constexpr compat::bigint b_from_str("1234567890123456789");
    static_assert(b_from_str > 0, "parsed bigint > 0");
    static_assert(b_from_str % 10 == 9, "parsed bigint % 10 == 9");
}

// ----------------------------------------------------------------------------
// 編譯期驗證：Decimal
// ----------------------------------------------------------------------------
namespace test_ce_decimal {
    // 1. 常數建構與代理
    constexpr compat::decimal d0;
    static_assert(d0.is_zero(), "decimal default ctor is zero");
    static_assert(d0.is_sbo(), "decimal d0 must be SBO");

    constexpr compat::decimal d_proxy_zero = compat::decimal::zero;
    constexpr compat::decimal d_proxy_one = compat::decimal::one();
    static_assert(d_proxy_zero == 0, "decimal::zero check");
    static_assert(d_proxy_one == 1, "decimal::one check");

    constexpr compat::decimal d_from_int(12345);
    static_assert(d_from_int == 12345, "d_from_int == 12345");

    constexpr compat::decimal d_from_unscaled(compat::bigint(12345), 2);
    static_assert(d_from_unscaled.scale() == 2, "scale == 2");
    static_assert(d_from_unscaled.unscaled() == 12345, "unscaled == 12345");

    // 2. 字串解析
    constexpr compat::decimal d_pi("3.1415");
    static_assert(d_pi.scale() == 4, "pi scale == 4");
    static_assert(d_pi.unscaled() == 31415, "pi unscaled == 31415");

    // 3. 算術運算
    constexpr compat::decimal a("1.25");
    constexpr compat::decimal b("2.50");
    constexpr compat::decimal c_add = a + b;
    static_assert(c_add == compat::decimal("3.75"), "1.25 + 2.50 == 3.75");

    constexpr compat::decimal c_sub = b - a;
    static_assert(c_sub == compat::decimal("1.25"), "2.50 - 1.25 == 1.25");

    constexpr compat::decimal c_mul = a * b;
    static_assert(c_mul == compat::decimal("3.125"), "1.25 * 2.50 == 3.125");

    constexpr compat::decimal c_div = compat::decimal("10.0").divide(compat::decimal("4.0"), 2);
    static_assert(c_div == compat::decimal("2.5"), "10.0 / 4.0 == 2.5");

    constexpr compat::decimal c_mod = compat::decimal("5.5") % compat::decimal("2.0");
    static_assert(c_mod == compat::decimal("1.5"), "5.5 % 2.0 == 1.5");

    // 4. 捨入 (Banker's Rounding / Half-Even)
    constexpr compat::decimal d_round1 = compat::decimal("2.5").round(0);
    static_assert(d_round1 == 2, "2.5 round half-even == 2");

    constexpr compat::decimal d_round2 = compat::decimal("3.5").round(0);
    static_assert(d_round2 == 4, "3.5 round half-even == 4");

    // 5. 比較與邏輯
    static_assert(a < b, "1.25 < 2.50");
    static_assert(b > a, "2.50 > 1.25");
    static_assert(a <= a, "1.25 <= 1.25");
    static_assert(a >= a, "1.25 >= 1.25");
    static_assert(a != b, "1.25 != 2.50");
    static_assert(static_cast<bool>(a), "a is truthy");
    static_assert(!d0, "d0 is falsy");
    static_assert((a && true) == true, "a && true");
    static_assert((d0 || false) == false, "d0 || false");

    // 6. 特殊值
    constexpr compat::decimal d_inf = compat::decimal::infinity;
    constexpr compat::decimal d_nan = compat::decimal::nan;
    static_assert(d_inf.is_infinite(), "d_inf is infinite");
    static_assert(d_nan.is_nan(), "d_nan is nan");
    static_assert(d_inf > a, "infinity > 1.25");
    static_assert((d_nan == d_nan) == false, "nan == nan is false");
    static_assert((d_nan != d_nan) == true, "nan != nan is true");
}

// ----------------------------------------------------------------------------
// 編譯期驗證：CMath
// ----------------------------------------------------------------------------
namespace test_ce_cmath {
    // 1. 常數函式
    constexpr compat::decimal pi = compat::detail::cmath_pi();
    static_assert(pi.is_sbo(), "cmath_pi must be in SBO buffer");
    static_assert(pi > 3, "cmath_pi > 3");
    static_assert(pi < 4, "cmath_pi < 4");

    constexpr compat::decimal two_pi = compat::detail::cmath_two_pi();
    static_assert(two_pi > 6, "two_pi > 6");

    constexpr compat::decimal ln2 = compat::detail::cmath_ln2();
    static_assert(ln2 > 0 && ln2 < 1, "0 < ln2 < 1");

    constexpr compat::decimal ln10 = compat::detail::cmath_ln10();
    static_assert(ln10 > 2 && ln10 < 3, "2 < ln10 < 3");

    // 2. 數值分類與符號
    static_assert(compat::isnan(compat::decimal::nan), "isnan check");
    static_assert(!compat::isnan(pi), "!isnan(pi) check");
    static_assert(compat::isinf(compat::decimal::infinity), "isinf check");
    static_assert(compat::isfinite(pi), "isfinite check");
    static_assert(compat::signbit(compat::decimal("-1.5")), "signbit check");
    static_assert(!compat::signbit(pi), "!signbit(pi) check");

    constexpr compat::decimal cs = compat::copysign(compat::decimal("2.5"), compat::decimal("-1.0"));
    static_assert(cs == compat::decimal("-2.5"), "copysign check");

    // 3. 捨入函式
    constexpr compat::decimal fl = compat::floor(compat::decimal("3.7"));
    static_assert(fl == 3, "floor(3.7) == 3");

    constexpr compat::decimal ce = compat::ceil(compat::decimal("3.2"));
    static_assert(ce == 4, "ceil(3.2) == 4");

    constexpr compat::decimal tr = compat::trunc(compat::decimal("-3.7"));
    static_assert(tr == -3, "trunc(-3.7) == -3");

    constexpr compat::decimal rd = compat::round(compat::decimal("3.5"));
    static_assert(rd == 4, "round(3.5) == 4");

    // 4. abs
    constexpr compat::bigint bi_abs = compat::abs(compat::bigint(-42));
    static_assert(bi_abs == 42, "abs(bigint(-42)) == 42");

    constexpr compat::decimal dec_abs = compat::abs(compat::decimal("-12.34"));
    static_assert(dec_abs == compat::decimal("12.34"), "abs(decimal(-12.34)) == 12.34");
}

#endif // COMPAT_CPLUSPLUS >= COMPAT_CXX_20

/// <summary>
/// 執行編譯期常數求值之執行期相容性驗證測試。
/// </summary>
void run_test_constexpr() {
    std::cout << "[Testing Constexpr Evaluation (C++20+)]" << std::endl;

#if (COMPAT_CPLUSPLUS >= COMPAT_CXX_20)
    // 執行期再次核對編譯期常數結果
    TEST_ASSERT(test_ce_bigint::fact10 == 3628800);
    TEST_ASSERT(test_ce_bigint::b_from_str > 0);
    TEST_ASSERT(test_ce_decimal::c_add == compat::decimal("3.75"));
    TEST_ASSERT(test_ce_decimal::c_mul == compat::decimal("3.125"));
    TEST_ASSERT(test_ce_decimal::d_round1 == 2);
    TEST_ASSERT(test_ce_decimal::d_round2 == 4);
    TEST_ASSERT(test_ce_cmath::pi > 3 && test_ce_cmath::pi < 4);
    TEST_ASSERT(test_ce_cmath::fl == 3);
    TEST_ASSERT(test_ce_cmath::ce == 4);
    TEST_ASSERT(test_ce_cmath::tr == -3);
    TEST_ASSERT(test_ce_cmath::rd == 4);
    TEST_ASSERT(test_ce_cmath::bi_abs == 42);
    TEST_ASSERT(test_ce_cmath::dec_abs == compat::decimal("12.34"));
    std::cout << "  -> Constexpr static_assert and runtime verification passed." << std::endl;
#else
    std::cout << "  -> Skipped on C++ < 20 (standard does not support non-literal constexpr types)." << std::endl;
#endif
}
