#include "test_helpers.hpp"
#include <compat/Parse.hpp>
#include <cstdint>
#include <cmath>
#include <string>

/// <summary>
/// 測試 compat::parse<T> 之各種固定寬度整數、浮點數、布林值、非法字串與邊界溢位狀況。
/// </summary>
void run_test_parse() {
    std::cout << "[TEST] Running test_parse..." << std::endl;

    // 1. 有號固定寬度整數解析
    auto r_i8 = compat::parse<int8_t>("127");
    TEST_ASSERT(r_i8.has_value() && r_i8.value() == 127);

    auto r_i8_neg = compat::parse<int8_t>("-128");
    TEST_ASSERT(r_i8_neg.has_value() && r_i8_neg.value() == -128);

    auto r_i16 = compat::parse<int16_t>("32767");
    TEST_ASSERT(r_i16.has_value() && r_i16.value() == 32767);

    auto r_i16_neg = compat::parse<int16_t>("-32768");
    TEST_ASSERT(r_i16_neg.has_value() && r_i16_neg.value() == -32768);

    auto r_i32 = compat::parse<int32_t>("2147483647");
    TEST_ASSERT(r_i32.has_value() && r_i32.value() == 2147483647);

    auto r_i32_neg = compat::parse<int32_t>("-2147483648");
    TEST_ASSERT(r_i32_neg.has_value() && r_i32_neg.value() == -2147483647 - 1);

    auto r_i32_plus = compat::parse<int32_t>("+42");
    TEST_ASSERT(r_i32_plus.has_value() && r_i32_plus.value() == 42);

    auto r_i64 = compat::parse<int64_t>("9223372036854775807");
    TEST_ASSERT(r_i64.has_value() && r_i64.value() == INT64_C(9223372036854775807));

    auto r_i64_neg = compat::parse<int64_t>("-9223372036854775808");
    TEST_ASSERT(r_i64_neg.has_value());

    // 2. 無號固定寬度整數解析
    auto r_u8 = compat::parse<uint8_t>("255");
    TEST_ASSERT(r_u8.has_value() && r_u8.value() == 255);

    auto r_u16 = compat::parse<uint16_t>("65535");
    TEST_ASSERT(r_u16.has_value() && r_u16.value() == 65535);

    auto r_u32 = compat::parse<uint32_t>("4294967295");
    TEST_ASSERT(r_u32.has_value() && r_u32.value() == UINT32_C(4294967295));

    auto r_u64 = compat::parse<uint64_t>("18446744073709551615");
    TEST_ASSERT(r_u64.has_value() && r_u64.value() == UINT64_C(18446744073709551615));

    // 無號整數帶負號應失敗
    auto r_u32_neg = compat::parse<uint32_t>("-10");
    TEST_ASSERT(!r_u32_neg.has_value());

    // 3. 浮點數解析
    auto r_double = compat::parse<double>("3.14159");
    TEST_ASSERT(r_double.has_value() && std::abs(r_double.value() - 3.14159) < 1e-5);

    auto r_double_neg = compat::parse<double>("-2.718");
    TEST_ASSERT(r_double_neg.has_value() && std::abs(r_double_neg.value() - (-2.718)) < 1e-5);

    auto r_double_exp = compat::parse<double>("1.5e2");
    TEST_ASSERT(r_double_exp.has_value() && std::abs(r_double_exp.value() - 150.0) < 1e-5);

    auto r_float = compat::parse<float>("0.125");
    TEST_ASSERT(r_float.has_value() && std::abs(r_float.value() - 0.125f) < 1e-5f);

    auto r_nan = compat::parse<double>("nan");
    TEST_ASSERT(r_nan.has_value() && std::isnan(r_nan.value()));

    auto r_inf = compat::parse<double>("inf");
    TEST_ASSERT(r_inf.has_value() && std::isinf(r_inf.value()));

    // 4. 布林值解析
    auto r_b1 = compat::parse<bool>("true");
    TEST_ASSERT(r_b1.has_value() && r_b1.value() == true);
    auto r_b2 = compat::parse<bool>("True");
    TEST_ASSERT(r_b2.has_value() && r_b2.value() == true);
    auto r_b3 = compat::parse<bool>("1");
    TEST_ASSERT(r_b3.has_value() && r_b3.value() == true);

    auto r_b4 = compat::parse<bool>("false");
    TEST_ASSERT(r_b4.has_value() && r_b4.value() == false);
    auto r_b5 = compat::parse<bool>("False");
    TEST_ASSERT(r_b5.has_value() && r_b5.value() == false);
    auto r_b6 = compat::parse<bool>("0");
    TEST_ASSERT(r_b6.has_value() && r_b6.value() == false);

    auto r_b_err = compat::parse<bool>("yes");
    TEST_ASSERT(!r_b_err.has_value());

    // 5. 字串與 string_view 解析
    auto r_str = compat::parse<std::string>("hello world");
    TEST_ASSERT(r_str.has_value() && r_str.value() == "hello world");

    auto r_sv = compat::parse<compat::string_view>("compat_view");
    TEST_ASSERT(r_sv.has_value() && r_sv.value() == "compat_view");

    // 6. Fail-fast 非法字串與溢位狀況
    // 空字串
    TEST_ASSERT(!compat::parse<int32_t>("").has_value());
    TEST_ASSERT(!compat::parse<uint32_t>("").has_value());
    TEST_ASSERT(!compat::parse<double>("").has_value());
    TEST_ASSERT(!compat::parse<bool>("").has_value());

    // 僅有正負號無數字
    TEST_ASSERT(!compat::parse<int32_t>("+").has_value());
    TEST_ASSERT(!compat::parse<int32_t>("-").has_value());
    TEST_ASSERT(!compat::parse<double>("+").has_value());

    // 包含非法非數字字元
    TEST_ASSERT(!compat::parse<int32_t>("123abc").has_value());
    TEST_ASSERT(!compat::parse<uint32_t>("456 78").has_value());
    TEST_ASSERT(!compat::parse<double>("12.34.56").has_value());

    // 整數溢位測試
    TEST_ASSERT(!compat::parse<int8_t>("128").has_value());
    TEST_ASSERT(!compat::parse<int8_t>("-129").has_value());
    TEST_ASSERT(!compat::parse<uint8_t>("256").has_value());

    TEST_ASSERT(!compat::parse<int16_t>("32768").has_value());
    TEST_ASSERT(!compat::parse<int16_t>("-32769").has_value());
    TEST_ASSERT(!compat::parse<uint16_t>("65536").has_value());

    TEST_ASSERT(!compat::parse<int32_t>("2147483648").has_value());
    TEST_ASSERT(!compat::parse<int32_t>("-2147483649").has_value());
    TEST_ASSERT(!compat::parse<uint32_t>("4294967296").has_value());

    TEST_ASSERT(!compat::parse<int64_t>("9223372036854775808").has_value());
    TEST_ASSERT(!compat::parse<int64_t>("-9223372036854775809").has_value());
    TEST_ASSERT(!compat::parse<uint64_t>("18446744073709551616").has_value());

    std::cout << "[PASS] test_parse passed." << std::endl;
}
