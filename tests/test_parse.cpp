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

    // 7. compat::from_chars 底層零配置解析引擎測試
    // 十進位有號與無號整數解析 (含指針與錯誤碼驗證)
    int32_t val_dec = 0;
    const char* dec_str = "12345extra";
    compat::from_chars_result r_dec = compat::from_chars(dec_str, dec_str + 10, val_dec);
    TEST_ASSERT(r_dec);
    TEST_ASSERT(val_dec == 12345);
    TEST_ASSERT(r_dec.ptr == dec_str + 5);
    TEST_ASSERT(r_dec.ec == std::errc{});

    int32_t val_neg = 0;
    const char* neg_str = "-9876";
    compat::from_chars_result r_neg = compat::from_chars(neg_str, neg_str + 5, val_neg);
    TEST_ASSERT(r_neg);
    TEST_ASSERT(val_neg == -9876);
    TEST_ASSERT(r_neg.ptr == neg_str + 5);

    // 進位制測試 (16 進位與 2 進位)
    uint32_t val_hex = 0;
    const char* hex_str = "1a2f";
    compat::from_chars_result r_hex = compat::from_chars(hex_str, hex_str + 4, val_hex, 16);
    TEST_ASSERT(r_hex);
    TEST_ASSERT(val_hex == 0x1a2f);
    TEST_ASSERT(r_hex.ptr == hex_str + 4);

    uint8_t val_bin = 0;
    const char* bin_str = "10110011";
    compat::from_chars_result r_bin = compat::from_chars(bin_str, bin_str + 8, val_bin, 2);
    TEST_ASSERT(r_bin);
    TEST_ASSERT(val_bin == 0b10110011);
    TEST_ASSERT(r_bin.ptr == bin_str + 8);

    // 整數溢位測試 (std::errc::result_out_of_range)
    uint8_t val_ovf = 0;
    const char* ovf_str = "256";
    compat::from_chars_result r_ovf = compat::from_chars(ovf_str, ovf_str + 3, val_ovf);
    TEST_ASSERT(!r_ovf);
    TEST_ASSERT(r_ovf.ec == std::errc::result_out_of_range);

    int8_t val_ovf_s = 0;
    const char* ovf_s_str = "-129";
    compat::from_chars_result r_ovf_s = compat::from_chars(ovf_s_str, ovf_s_str + 4, val_ovf_s);
    TEST_ASSERT(!r_ovf_s);
    TEST_ASSERT(r_ovf_s.ec == std::errc::result_out_of_range);

    // 浮點數與科學記號解析
    double val_flt = 0.0;
    const char* flt_str = "123.456trailing";
    compat::from_chars_result r_flt = compat::from_chars(flt_str, flt_str + 15, val_flt);
    TEST_ASSERT(r_flt);
    TEST_ASSERT(std::abs(val_flt - 123.456) < 1e-6);
    TEST_ASSERT(r_flt.ptr == flt_str + 7);

    float val_sci = 0.0f;
    const char* sci_str = "-1.25e-3";
    compat::from_chars_result r_sci = compat::from_chars(sci_str, sci_str + 8, val_sci);
    TEST_ASSERT(r_sci);
    TEST_ASSERT(std::abs(val_sci - (-0.00125f)) < 1e-6f);
    TEST_ASSERT(r_sci.ptr == sci_str + 8);

    // 特殊浮點數 (NaN 與 Inf)
    double val_nan = 0.0;
    const char* nan_str = "NaN";
    compat::from_chars_result r_nan_fc = compat::from_chars(nan_str, nan_str + 3, val_nan);
    TEST_ASSERT(r_nan_fc);
    TEST_ASSERT(std::isnan(val_nan));

    double val_inf1 = 0.0;
    const char* inf1_str = "infinity";
    compat::from_chars_result r_inf1 = compat::from_chars(inf1_str, inf1_str + 8, val_inf1);
    TEST_ASSERT(r_inf1);
    TEST_ASSERT(std::isinf(val_inf1) && val_inf1 > 0);

    double val_inf2 = 0.0;
    const char* inf2_str = "-inf";
    compat::from_chars_result r_inf2 = compat::from_chars(inf2_str, inf2_str + 4, val_inf2);
    TEST_ASSERT(r_inf2);
    TEST_ASSERT(std::isinf(val_inf2) && val_inf2 < 0);

    // 浮點數溢位測試 (std::errc::result_out_of_range)
    double val_flt_ovf = 0.0;
    const char* flt_ovf_str = "1e309";
    compat::from_chars_result r_flt_ovf = compat::from_chars(flt_ovf_str, flt_ovf_str + 5, val_flt_ovf);
    TEST_ASSERT(!r_flt_ovf);
    TEST_ASSERT(r_flt_ovf.ec == std::errc::result_out_of_range);

    float val_flt32_ovf = 0.0f;
    const char* flt32_ovf_str = "1e39";
    compat::from_chars_result r_flt32_ovf = compat::from_chars(flt32_ovf_str, flt32_ovf_str + 4, val_flt32_ovf);
    TEST_ASSERT(!r_flt32_ovf);
    TEST_ASSERT(r_flt32_ovf.ec == std::errc::result_out_of_range);

    std::cout << "[PASS] test_parse passed." << std::endl;
}
