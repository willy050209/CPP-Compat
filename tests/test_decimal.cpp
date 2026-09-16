#include "test_helpers.hpp"
#include <compat/Decimal.hpp>
#include <compat/BigInt.hpp>
#include <sstream>
#include <string>
#include <cstdint>
#include <cmath>

/// <summary>
/// 執行 compat::decimal 完整單元測試套件。
/// </summary>
void run_test_decimal() {
    using compat::decimal;
    using compat::bigint;

    std::cout << "--- Running Decimal Tests ---" << std::endl;

    // 1. 數字初始化與轉型測試（各整數型態、浮點數、bigint、布林值）
    {
        decimal d_bool_t(true);
        decimal d_bool_f(false);
        TEST_ASSERT(d_bool_t == 1);
        TEST_ASSERT(d_bool_f == 0);

        int8_t i8 = -42;
        int16_t i16 = -1234;
        int32_t i32 = -987654;
        int64_t i64 = -9223372036854775807LL;
        decimal di8(i8), di16(i16), di32(i32), di64(i64);
        TEST_ASSERT(di8 == -42);
        TEST_ASSERT(di16 == -1234);
        TEST_ASSERT(di32 == -987654);
        TEST_ASSERT(di64 == -9223372036854775807LL);

        uint8_t u8 = 255;
        uint16_t u16 = 65535;
        uint32_t u32 = 4294967295U;
        uint64_t u64 = 18446744073709551615ULL;
        decimal du8(u8), du16(u16), du32(u32), du64(u64);
        TEST_ASSERT(du8 == 255);
        TEST_ASSERT(du16 == 65535);
        TEST_ASSERT(du32 == 4294967295ULL);
        TEST_ASSERT(du64 == u64);

        // 浮點數初始化
        decimal df(3.14f);
        decimal dd(3.14);
        decimal dld(static_cast<long double>(3.14));
        TEST_ASSERT(dd == decimal("3.14"));

        // compat::bigint 初始化
        bigint bi("12345678901234567890");
        decimal dbi(bi);
        TEST_ASSERT(dbi == bi);
        TEST_ASSERT(dbi.to_string() == "12345678901234567890");

        // unscaled 與 scale 初始化
        decimal d_unscaled(bigint(12345), 2);
        TEST_ASSERT(d_unscaled == decimal("123.45"));
        TEST_ASSERT(d_unscaled.to_string() == "123.45");

        // 預設建構子為 0
        decimal d_def;
        TEST_ASSERT(d_def == 0);
        TEST_ASSERT(d_def.is_zero());
        TEST_ASSERT(d_def.to_string() == "0");
    }

    // 2. 儲存架構與 128-bit SBO 驗證（<= 38 位十進位達成 0 堆積配置）
    {
        // 34 位有效十進位數字（IEEE 754-2008 decimal128 標準）
        decimal d34("1234567890123456789012345678901234");
        TEST_ASSERT(d34.is_sbo());
        TEST_ASSERT(d34.is_small());
        TEST_ASSERT(d34.to_string() == "1234567890123456789012345678901234");

        // 38 位十進位數字（仍落在 128-bit SBO 內）
        decimal d38("99999999999999999999999999999999999999");
        TEST_ASSERT(d38.is_sbo());

        // 超出 128-bit（> 38 位十進位，動態配置）
        decimal d45("123456789012345678901234567890123456789012345");
        TEST_ASSERT(!d45.is_sbo());
        TEST_ASSERT(d45.to_string() == "123456789012345678901234567890123456789012345");
    }

    // 3. 靜態常數與工廠（常數代理模式）
    {
        TEST_ASSERT(decimal::zero == 0);
        TEST_ASSERT(decimal::zero() == 0);
        TEST_ASSERT(decimal::one == 1);
        TEST_ASSERT(decimal::one() == 1);

        decimal z = decimal::zero;
        decimal z_fn = decimal::zero();
        TEST_ASSERT(z == z_fn);

        decimal o = decimal::one;
        decimal o_fn = decimal::one();
        TEST_ASSERT(o == o_fn);

        decimal inf = decimal::infinity;
        decimal inf_fn = decimal::infinity();
        TEST_ASSERT(inf.is_infinite());
        TEST_ASSERT(inf_fn.is_infinite());
        TEST_ASSERT(inf > 0);
        TEST_ASSERT(inf == inf_fn);

        decimal neg_inf = -decimal::infinity;
        TEST_ASSERT(neg_inf.is_infinite());
        TEST_ASSERT(neg_inf < 0);
        TEST_ASSERT(neg_inf < inf);

        decimal nan_val = decimal::nan;
        TEST_ASSERT(nan_val.is_nan());
        TEST_ASSERT(decimal::nan().is_nan());
        TEST_ASSERT(decimal::NaN().is_nan());
    }

    // 4. 算數與 In-place 運算子 (+, -, *, /, %, +=, -=, *=, /=, %=, ++, --)
    {
        decimal a("12.34");
        decimal b("5.6");

        // + 與 +=
        decimal c = a + b;
        TEST_ASSERT(c == decimal("17.94"));
        TEST_ASSERT(c.to_string() == "17.94");

        decimal a_plus = a;
        a_plus += b;
        TEST_ASSERT(a_plus == c);

        // - 與 -=
        decimal d = a - b;
        TEST_ASSERT(d == decimal("6.74"));
        TEST_ASSERT(d.to_string() == "6.74");

        decimal a_minus = a;
        a_minus -= b;
        TEST_ASSERT(a_minus == d);

        // * 與 *=
        decimal e = a * b; // 12.34 * 5.6 = 69.104
        TEST_ASSERT(e == decimal("69.104"));
        TEST_ASSERT(e.to_string() == "69.104");

        decimal a_mul = a;
        a_mul *= b;
        TEST_ASSERT(a_mul == e);

        // % 與 %=
        decimal m1("7.3");
        decimal m2("2.1");
        decimal mod_res = m1 % m2; // 7.3 - 3 * 2.1 = 7.3 - 6.3 = 1.0 -> 1
        TEST_ASSERT(mod_res == decimal("1.0"));
        TEST_ASSERT(mod_res.to_string() == "1");

        decimal m_inplace = m1;
        m_inplace %= m2;
        TEST_ASSERT(m_inplace == mod_res);

        // 一元正負號
        decimal pos = +a;
        decimal neg = -a;
        TEST_ASSERT(pos == a);
        TEST_ASSERT(neg == decimal("-12.34"));
        TEST_ASSERT(-neg == a);

        // 前置與後置遞增遞減
        decimal count("10.5");
        TEST_ASSERT(++count == decimal("11.5"));
        TEST_ASSERT(count == decimal("11.5"));
        TEST_ASSERT(count++ == decimal("11.5"));
        TEST_ASSERT(count == decimal("12.5"));

        TEST_ASSERT(--count == decimal("11.5"));
        TEST_ASSERT(count == decimal("11.5"));
        TEST_ASSERT(count-- == decimal("11.5"));
        TEST_ASSERT(count == decimal("10.5"));
    }

    // 5. 除法精度與銀行家捨入法 (Banker's Rounding / Half-Even)
    {
        // 有限整除
        decimal half = decimal(1) / decimal(2);
        TEST_ASSERT(half == decimal("0.5"));
        TEST_ASSERT(half.to_string() == "0.5");

        decimal quarter = decimal(1) / decimal(4);
        TEST_ASSERT(quarter == decimal("0.25"));
        TEST_ASSERT(quarter.to_string() == "0.25");

        decimal eighth = decimal(1) / decimal(8);
        TEST_ASSERT(eighth == decimal("0.125"));
        TEST_ASSERT(eighth.to_string() == "0.125");

        // 預設 34 位精度除法
        decimal third = decimal(1) / decimal(3);
        TEST_ASSERT(third.to_string() == "0.3333333333333333333333333333333333");

        decimal two_thirds = decimal(2) / decimal(3);
        TEST_ASSERT(two_thirds.to_string() == "0.6666666666666666666666666666666667");

        // 自訂精度除法 divide(other, precision)
        decimal d_p4 = decimal(1).divide(decimal(3), 4);
        TEST_ASSERT(d_p4.to_string() == "0.3333");

        decimal d_p4_roundup = decimal(2).divide(decimal(3), 4);
        TEST_ASSERT(d_p4_roundup.to_string() == "0.6667");

        // 經典銀行家捨入測試：當捨棄部分剛好為 0.5 時，捨入至最接近的偶數位
        // 1/8 = 0.125 -> 精度 2 有效位數：下一個位數為 5 且無餘數。前一位數為 2 (偶數) -> 捨入為 0.12
        decimal r1 = decimal(1).divide(decimal(8), 2);
        TEST_ASSERT(r1.to_string() == "0.12");

        // 3/8 = 0.375 -> 精度 2 有效位數：下一個位數為 5 且無餘數。前一位數為 7 (奇數) -> 進位為 0.38
        decimal r2 = decimal(3).divide(decimal(8), 2);
        TEST_ASSERT(r2.to_string() == "0.38");

        // 5/8 = 0.625 -> 精度 2 有效位數：前一位數為 2 (偶數) -> 捨入為 0.62
        decimal r3 = decimal(5).divide(decimal(8), 2);
        TEST_ASSERT(r3.to_string() == "0.62");

        // 7/8 = 0.875 -> 精度 2 有效位數：前一位數為 7 (奇數) -> 進位為 0.88
        decimal r4 = decimal(7).divide(decimal(8), 2);
        TEST_ASSERT(r4.to_string() == "0.88");

        // round(decimal_places) 輔助函式測試
        TEST_ASSERT(decimal("2.5").round(0).to_string() == "2");
        TEST_ASSERT(decimal("3.5").round(0).to_string() == "4");
        TEST_ASSERT(decimal("-2.5").round(0).to_string() == "-2");
        TEST_ASSERT(decimal("-3.5").round(0).to_string() == "-4");
        TEST_ASSERT(decimal("0.125").round(2).to_string() == "0.12");
        TEST_ASSERT(decimal("0.135").round(2).to_string() == "0.14");
    }

    // 6. 特殊值處理 (Infinity, -Infinity, NaN)
    {
        decimal zero(0);
        decimal pos(10);
        decimal neg(-10);

        // 除以 0
        decimal pos_inf = pos / zero;
        TEST_ASSERT(pos_inf.is_infinite());
        TEST_ASSERT(pos_inf > 0);
        TEST_ASSERT(pos_inf.to_string() == "inf");

        decimal neg_inf = neg / zero;
        TEST_ASSERT(neg_inf.is_infinite());
        TEST_ASSERT(neg_inf < 0);
        TEST_ASSERT(neg_inf.to_string() == "-inf");

        decimal nan_0_div_0 = zero / zero;
        TEST_ASSERT(nan_0_div_0.is_nan());
        TEST_ASSERT(nan_0_div_0.to_string() == "nan");

        // 無窮大相減：inf - inf = NaN
        decimal inf_minus_inf = pos_inf - pos_inf;
        TEST_ASSERT(inf_minus_inf.is_nan());

        // 無窮大乘 0：inf * 0 = NaN
        decimal inf_times_zero = pos_inf * zero;
        TEST_ASSERT(inf_times_zero.is_nan());

        // 無窮大除以無窮大：inf / inf = NaN
        decimal inf_div_inf = pos_inf / pos_inf;
        TEST_ASSERT(inf_div_inf.is_nan());

        // 無窮大算術
        TEST_ASSERT((pos_inf + 5).is_infinite());
        TEST_ASSERT((pos_inf + 5) > 0);
        TEST_ASSERT((pos_inf * 2).is_infinite());
        TEST_ASSERT((pos_inf * -2).is_infinite());
        TEST_ASSERT((pos_inf * -2) < 0);
        TEST_ASSERT((5 / pos_inf) == 0);

        // 取模特殊值
        TEST_ASSERT((pos % zero).is_nan());
        TEST_ASSERT((pos_inf % pos).is_nan());
        TEST_ASSERT((pos % pos_inf) == pos);
    }

    // 7. 與各整數型別、浮點型別及 compat::bigint 雙向隱式轉型運算
    {
        decimal d("10.5");
        bigint b(20);

        // 與整數混合
        TEST_ASSERT(d + 10 == decimal("20.5"));
        TEST_ASSERT(10 + d == decimal("20.5"));
        TEST_ASSERT(d - 5 == decimal("5.5"));
        TEST_ASSERT(20 - d == decimal("9.5"));
        TEST_ASSERT(d * 2 == decimal("21"));
        TEST_ASSERT(2 * d == decimal("21"));

        // 與浮點數混合
        TEST_ASSERT(d + 0.5 == decimal("11"));
        TEST_ASSERT(0.5 + d == decimal("11"));

        // 與 compat::bigint 混合
        TEST_ASSERT(d + b == decimal("30.5"));
        TEST_ASSERT(b + d == decimal("30.5"));
        TEST_ASSERT(b - d == decimal("9.5"));

        // 明確轉型運算子
        TEST_ASSERT(static_cast<int64_t>(decimal("123.45")) == 123);
        TEST_ASSERT(static_cast<int32_t>(decimal("-456.78")) == -456);
        TEST_ASSERT(static_cast<double>(decimal("12.5")) == 12.5);
        TEST_ASSERT(static_cast<bigint>(decimal("987654321.123")) == bigint("987654321"));
    }

    // 8. 邏輯運算子與比較運算子
    {
        decimal d_zero(0);
        decimal d_pos(5);
        decimal d_neg(-5);
        decimal d_nan = decimal::nan;
        decimal d_inf = decimal::infinity;

        // operator bool()
        TEST_ASSERT(!d_zero);
        TEST_ASSERT(!d_nan);
        TEST_ASSERT(bool(d_pos));
        TEST_ASSERT(bool(d_neg));
        TEST_ASSERT(bool(d_inf));

        // operator!()
        TEST_ASSERT(!d_zero == true);
        TEST_ASSERT(!d_nan == true);
        TEST_ASSERT(!d_pos == false);

        // && 與 ||
        TEST_ASSERT((d_pos && d_neg) == true);
        TEST_ASSERT((d_pos && d_zero) == false);
        TEST_ASSERT((d_zero || d_pos) == true);
        TEST_ASSERT((d_zero || d_nan) == false);

        // 比較運算子 (==, !=, <, <=, >, >=)
        decimal d1("1.50");
        decimal d2("1.5");
        TEST_ASSERT(d1 == d2);
        TEST_ASSERT(d1 <= d2);
        TEST_ASSERT(d1 >= d2);
        TEST_ASSERT(!(d1 < d2));
        TEST_ASSERT(!(d1 > d2));
        TEST_ASSERT(!(d1 != d2));

        decimal d3("2.0");
        TEST_ASSERT(d1 < d3);
        TEST_ASSERT(d1 <= d3);
        TEST_ASSERT(d3 > d1);
        TEST_ASSERT(d3 >= d1);
        TEST_ASSERT(d1 != d3);

        // NaN 比較規範：NaN 與任何數（包含自身）比較 == 為 false，!= 為 true
        TEST_ASSERT(!(d_nan == d_nan));
        TEST_ASSERT(d_nan != d_nan);
        TEST_ASSERT(!(d_nan == d_pos));
        TEST_ASSERT(d_nan != d_pos);
        TEST_ASSERT(!(d_nan < d_pos));
        TEST_ASSERT(!(d_nan > d_pos));
        TEST_ASSERT(!(d_nan <= d_pos));
        TEST_ASSERT(!(d_nan >= d_pos));

        // 無窮大比較
        TEST_ASSERT(d_inf > d_pos);
        TEST_ASSERT(-d_inf < d_neg);
        TEST_ASSERT(d_inf == decimal::infinity);
        TEST_ASSERT(d_inf != -d_inf);
    }

    // 9. 字串解析 (from_string) 與輸出格式化 (to_string, operator<<)
    {
        // 整數與小數
        decimal s1 = decimal::from_string("123");
        TEST_ASSERT(s1 == 123);

        decimal s2 = decimal::from_string("-456.78");
        TEST_ASSERT(s2.to_string() == "-456.78");

        decimal s3 = decimal::from_string(".5");
        TEST_ASSERT(s3 == decimal("0.5"));

        decimal s4 = decimal::from_string("5.");
        TEST_ASSERT(s4 == 5);

        // 科學記號
        decimal exp1 = decimal::from_string("1.23e-4");
        TEST_ASSERT(exp1 == decimal("0.000123"));
        TEST_ASSERT(exp1.to_string() == "0.000123");

        decimal exp2 = decimal::from_string("2.5E+3");
        TEST_ASSERT(exp2 == 2500);
        TEST_ASSERT(exp2.to_string() == "2500");

        // 特殊值字串
        decimal sinf1 = decimal::from_string("inf");
        decimal sinf2 = decimal::from_string("-inf");
        decimal sinf3 = decimal::from_string("infinity");
        decimal sinf4 = decimal::from_string("-infinity");
        TEST_ASSERT(sinf1.is_infinite() && sinf1 > 0);
        TEST_ASSERT(sinf2.is_infinite() && sinf2 < 0);
        TEST_ASSERT(sinf3.is_infinite() && sinf3 > 0);
        TEST_ASSERT(sinf4.is_infinite() && sinf4 < 0);

        decimal snan = decimal::from_string("nan");
        TEST_ASSERT(snan.is_nan());
        decimal snan_caps = decimal::from_string("NAN");
        TEST_ASSERT(snan_caps.is_nan());

        // ostream 輸出串流
        std::stringstream ss;
        ss << decimal("123.456");
        TEST_ASSERT(ss.str() == "123.456");

        // 異常字串測試
        TEST_ASSERT_THROWS(decimal::from_string(""), std::invalid_argument);
        TEST_ASSERT_THROWS(decimal::from_string("abc"), std::invalid_argument);
        TEST_ASSERT_THROWS(decimal::from_string("1.2.3"), std::invalid_argument);
        TEST_ASSERT_THROWS(decimal::from_string("1e"), std::invalid_argument);
    }

    std::cout << "All Decimal tests passed successfully!" << std::endl;
}
