#include "test_helpers.hpp"
#include <compat/BigInt.hpp>
#include <sstream>
#include <string>
#include <cstdint>

/// <summary>
/// 執行 compat::bigint 完整單元測試套件。
/// </summary>
void run_test_bigint() {
    using compat::bigint;

    std::cout << "--- Running BigInt Tests ---" << std::endl;

    // 1. 數字初始化測試（各整數型態 int8_t~int64_t、uint8_t~uint64_t、bool）
    {
        bigint b_bool_t(true);
        bigint b_bool_f(false);
        TEST_ASSERT(b_bool_t == 1);
        TEST_ASSERT(b_bool_f == 0);

        int8_t i8 = -42;
        int16_t i16 = -1234;
        int32_t i32 = -987654;
        int64_t i64 = -9223372036854775807LL;
        bigint bi8(i8), bi16(i16), bi32(i32), bi64(i64);
        TEST_ASSERT(bi8 == -42);
        TEST_ASSERT(bi16 == -1234);
        TEST_ASSERT(bi32 == -987654);
        TEST_ASSERT(bi64 == -9223372036854775807LL);

        uint8_t u8 = 255;
        uint16_t u16 = 65535;
        uint32_t u32 = 4294967295U;
        uint64_t u64 = 18446744073709551615ULL;
        bigint bu8(u8), bu16(u16), bu32(u32), bu64(u64);
        TEST_ASSERT(bu8 == 255);
        TEST_ASSERT(bu16 == 65535);
        TEST_ASSERT(bu32 == 4294967295ULL);
        TEST_ASSERT(bu64 == u64);

        bigint b_zero(0);
        TEST_ASSERT(b_zero == 0);
        TEST_ASSERT(b_zero.is_sbo());
    }

    // 2. SBO 驗證（<= 128 位元數值正常運作且不拋錯，大於 128 位元數值正確動態配置運作）
    {
        // 128-bit unsigned max = 2^128 - 1 = 340282366920938463463374607431768211455
        bigint sbo_max("340282366920938463463374607431768211455");
        TEST_ASSERT(sbo_max.is_sbo());
        TEST_ASSERT(sbo_max.to_string() == "340282366920938463463374607431768211455");

        // 負的 128-bit 數值也落在 2 limbs 以內
        bigint sbo_neg("-340282366920938463463374607431768211455");
        TEST_ASSERT(sbo_neg.is_sbo());

        // 2^128 = 340282366920938463463374607431768211456 (3 limbs, 超出 128 位元需 heap 動態配置)
        bigint heap_num("340282366920938463463374607431768211456");
        TEST_ASSERT(!heap_num.is_sbo());
        TEST_ASSERT(heap_num.to_string() == "340282366920938463463374607431768211456");

        // SBO 與 Heap 數值運算
        bigint sum = sbo_max + 1;
        TEST_ASSERT(!sum.is_sbo());
        TEST_ASSERT(sum == heap_num);

        bigint diff = sum - 1;
        TEST_ASSERT(diff.is_sbo());
        TEST_ASSERT(diff == sbo_max);
    }

    // 3. 靜態常數與靜態方法
    {
        TEST_ASSERT(bigint::zero == 0);
        TEST_ASSERT(bigint::zero() == 0);
        TEST_ASSERT(bigint::one == 1);
        TEST_ASSERT(bigint::one() == 1);

        bigint z = bigint::zero;
        bigint z_fn = bigint::zero();
        TEST_ASSERT(z == z_fn);

        bigint o = bigint::one;
        bigint o_fn = bigint::one();
        TEST_ASSERT(o == o_fn);
    }

    // 4. 算數運算子 (+, -, *, /, %) 與 In-place (+=, -=, *=, /=, %=)
    {
        bigint a(100);
        bigint b(30);

        TEST_ASSERT(a + b == 130);
        TEST_ASSERT(a - b == 70);
        TEST_ASSERT(a * b == 3000);
        TEST_ASSERT(a / b == 3);
        TEST_ASSERT(a % b == 10);

        // 正負號運算
        bigint neg_a(-100);
        bigint neg_b(-30);

        TEST_ASSERT(neg_a + b == -70);
        TEST_ASSERT(a + neg_b == 70);
        TEST_ASSERT(neg_a - b == -130);
        TEST_ASSERT(a - neg_b == 130);
        TEST_ASSERT(neg_a * b == -3000);
        TEST_ASSERT(neg_a * neg_b == 3000);
        TEST_ASSERT(neg_a / b == -3);
        TEST_ASSERT(a / neg_b == -3);
        TEST_ASSERT(neg_a / neg_b == 3);

        // C++ 取模符號與被除數一致
        TEST_ASSERT(neg_a % b == -10);
        TEST_ASSERT(a % neg_b == 10);
        TEST_ASSERT(neg_a % neg_b == -10);

        // In-place 運算
        bigint x = a;
        x += b; TEST_ASSERT(x == 130);
        x -= b; TEST_ASSERT(x == 100);
        x *= b; TEST_ASSERT(x == 3000);
        x /= b; TEST_ASSERT(x == 100);
        x %= b; TEST_ASSERT(x == 10);

        // 正負一元運算子
        bigint pos(42);
        TEST_ASSERT(+pos == 42);
        TEST_ASSERT(-pos == -42);
        TEST_ASSERT(-(-pos) == 42);

        // 除以零異常檢測
        TEST_ASSERT_THROWS(a / bigint::zero, std::invalid_argument);
        TEST_ASSERT_THROWS(a % bigint::zero, std::invalid_argument);
        TEST_ASSERT_THROWS(x /= 0, std::invalid_argument);
        TEST_ASSERT_THROWS(x %= 0, std::invalid_argument);
    }

    // 5. 混合型別自動轉型（例如 b + 10, 10 + b, b * -5, -5 * b 等雙向運算）
    {
        bigint b(50);
        TEST_ASSERT(b + 10 == 60);
        TEST_ASSERT(10 + b == 60);
        TEST_ASSERT(b - 10 == 40);
        TEST_ASSERT(10 - b == -40);
        TEST_ASSERT(b * -5 == -250);
        TEST_ASSERT(-5 * b == -250);
        TEST_ASSERT(b / 5 == 10);
        TEST_ASSERT(100 / b == 2);
        TEST_ASSERT(b % 7 == 1);
        TEST_ASSERT(103 % b == 3);

        TEST_ASSERT(b == 50);
        TEST_ASSERT(50 == b);
        TEST_ASSERT(b != 40);
        TEST_ASSERT(40 != b);
        TEST_ASSERT(b > 40);
        TEST_ASSERT(40 < b);
        TEST_ASSERT(b >= 50);
        TEST_ASSERT(50 <= b);
    }

    // 6. 遞增遞減 (++, -- 前置與後置）
    {
        bigint v(10);
        TEST_ASSERT(++v == 11);
        TEST_ASSERT(v == 11);

        TEST_ASSERT(v++ == 11);
        TEST_ASSERT(v == 12);

        TEST_ASSERT(--v == 11);
        TEST_ASSERT(v == 11);

        TEST_ASSERT(v-- == 11);
        TEST_ASSERT(v == 10);

        // 跨越 0 遞增遞減
        bigint zero_val(0);
        --zero_val;
        TEST_ASSERT(zero_val == -1);
        ++zero_val;
        TEST_ASSERT(zero_val == 0);
    }

    // 7. 位元運算子 (&, |, ^, ~, <<, >> 與 &=, |=, ^=, <<=, >>=)
    {
        bigint a(0b1100);
        bigint b(0b1010);

        TEST_ASSERT((a & b) == 0b1000);
        TEST_ASSERT((a | b) == 0b1110);
        TEST_ASSERT((a ^ b) == 0b0110);

        bigint x = a;
        x &= b; TEST_ASSERT(x == 0b1000);
        x = a;
        x |= b; TEST_ASSERT(x == 0b1110);
        x = a;
        x ^= b; TEST_ASSERT(x == 0b0110);

        // 位元非 (two's complement ~n == -n - 1)
        bigint n(5);
        TEST_ASSERT(~n == -6);
        bigint zero_b(0);
        TEST_ASSERT(~zero_b == -1);
        bigint neg1(-1);
        TEST_ASSERT(~neg1 == 0);

        // 位移運算子
        bigint s(1);
        TEST_ASSERT((s << 0) == 1);
        TEST_ASSERT((s << 4) == 16);
        TEST_ASSERT((s << 64) == bigint("18446744073709551616"));
        TEST_ASSERT((s << 130) > s);

        bigint s_shifted = s << 130;
        TEST_ASSERT((s_shifted >> 130) == 1);
        TEST_ASSERT((s_shifted >> 131) == 0);

        // In-place 位移
        bigint mut_s(2);
        mut_s <<= 5;
        TEST_ASSERT(mut_s == 64);
        mut_s >>= 3;
        TEST_ASSERT(mut_s == 8);

        // 負數位移 (算術右移語意 -5 >> 1 == -3)
        bigint neg_shift(-5);
        TEST_ASSERT((neg_shift >> 1) == -3);
    }

    // 8. 邏輯運算子
    {
        bigint b_zero(0);
        bigint b_pos(123);
        bigint b_neg(-456);

        // explicit operator bool()
        TEST_ASSERT(!static_cast<bool>(b_zero));
        TEST_ASSERT(static_cast<bool>(b_pos));
        TEST_ASSERT(static_cast<bool>(b_neg));

        // operator!
        TEST_ASSERT(!b_zero);
        TEST_ASSERT(!(!b_pos));
        TEST_ASSERT(!(!b_neg));

        // operator&& 與 operator||
        TEST_ASSERT(b_pos && b_neg);
        TEST_ASSERT(!(b_pos && b_zero));
        TEST_ASSERT(!(b_zero && b_neg));
        TEST_ASSERT(b_pos || b_zero);
        TEST_ASSERT(b_zero || b_neg);
        TEST_ASSERT(!(b_zero || bigint(0)));

        // 混合型別邏輯運算
        TEST_ASSERT(b_pos && true);
        TEST_ASSERT(b_pos && 1);
        TEST_ASSERT(!(b_pos && false));
        TEST_ASSERT(!(b_pos && 0));
        TEST_ASSERT(b_zero || 5);
        TEST_ASSERT(!(b_zero || 0));
    }

    // 9. 比較運算子
    {
        bigint x(100);
        bigint y(200);
        bigint z(100);

        TEST_ASSERT(x == z);
        TEST_ASSERT(x != y);
        TEST_ASSERT(x < y);
        TEST_ASSERT(x <= y);
        TEST_ASSERT(x <= z);
        TEST_ASSERT(y > x);
        TEST_ASSERT(y >= x);
        TEST_ASSERT(z >= x);

        bigint neg(-50);
        TEST_ASSERT(neg < x);
        TEST_ASSERT(x > neg);
        TEST_ASSERT(neg < 0);
    }

    // 9.1 靜態常數與代理 (bigint::zero, bigint::one, bigint::zero(), bigint::one())
    {
        TEST_ASSERT(bigint::zero == 0);
        TEST_ASSERT(bigint::zero() == 0);
        TEST_ASSERT(bigint::one == 1);
        TEST_ASSERT(bigint::one() == 1);

        bigint b_z = bigint::zero;
        bigint b_z_fn = bigint::zero();
        TEST_ASSERT(b_z == b_z_fn);
        TEST_ASSERT(b_z == 0);

        bigint b_o = bigint::one;
        bigint b_o_fn = bigint::one();
        TEST_ASSERT(b_o == b_o_fn);
        TEST_ASSERT(b_o == 1);
    }

    // 10. 字串解析與例外拋出
    {
        bigint from_s1 = bigint::from_string("123456789012345678901234567890");
        bigint ctor_s1("123456789012345678901234567890");
        TEST_ASSERT(from_s1 == ctor_s1);
        TEST_ASSERT(from_s1.to_string() == "123456789012345678901234567890");

        bigint from_neg = bigint::from_string("-987654321098765432109876543210");
        TEST_ASSERT(from_neg.to_string() == "-987654321098765432109876543210");

        bigint from_pos = bigint::from_string("+12345");
        TEST_ASSERT(from_pos == 12345);

        // 異常字串
        TEST_ASSERT_THROWS(bigint::from_string(""), std::invalid_argument);
        TEST_ASSERT_THROWS(bigint::from_string("+"), std::invalid_argument);
        TEST_ASSERT_THROWS(bigint::from_string("-"), std::invalid_argument);
        TEST_ASSERT_THROWS(bigint::from_string("123a45"), std::invalid_argument);
        TEST_ASSERT_THROWS(bigint::from_string("abc"), std::invalid_argument);
        TEST_ASSERT_THROWS(bigint("invalid"), std::invalid_argument);
    }

    // 11. 輸出 (to_string 與 operator<<)
    {
        bigint num("-12345678901234567890");
        TEST_ASSERT(num.to_string() == "-12345678901234567890");

        std::ostringstream oss;
        oss << num;
        TEST_ASSERT(oss.str() == "-12345678901234567890");
    }

    // 12. Rule of Five 驗證 (Copy, Move, Self-assignment)
    {
        bigint original("9999999999999999999999999999999999999999");
        TEST_ASSERT(!original.is_sbo());

        // Copy Ctor
        bigint copied(original);
        TEST_ASSERT(copied == original);
        TEST_ASSERT(!copied.is_sbo());

        // Copy Assign
        bigint assigned(10);
        assigned = original;
        TEST_ASSERT(assigned == original);
        TEST_ASSERT(!assigned.is_sbo());

        // Move Ctor
        bigint move_source("8888888888888888888888888888888888888888");
        bigint moved(std::move(move_source));
        TEST_ASSERT(moved.to_string() == "8888888888888888888888888888888888888888");

        // Move Assign
        bigint move_assigned(5);
        move_assigned = std::move(moved);
        TEST_ASSERT(move_assigned.to_string() == "8888888888888888888888888888888888888888");

        // Self assignment
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#pragma clang diagnostic ignored "-Wself-assign"
#endif
        assigned = assigned;
        TEST_ASSERT(assigned == original);
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
    }

    // 13. 長位數乘法 (Karatsuba 與長乘法驗證)
    {
        // 50-digit numbers
        bigint big_a("12345678901234567890123456789012345678901234567890");
        bigint big_b("98765432109876543210987654321098765432109876543210");
        bigint prod = big_a * big_b;
        TEST_ASSERT(prod.to_string() == "1219326311370217952261850327338667885945115073915611949397448712086533622923332237463801111263526900");

        // 驗證乘除互逆: (A * B) / B == A
        TEST_ASSERT(prod / big_b == big_a);
        TEST_ASSERT(prod % big_b == 0);
    }

    std::cout << "--- BigInt Tests Completed Successfully ---" << std::endl;
}
