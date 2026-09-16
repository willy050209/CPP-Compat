#pragma once

// DecimalCore.hpp
// Core arbitrary-precision decimal algorithms and storage utilities for CPP-Compat.
// Zero external dependencies, downward compatible from C++23 to C++11.

#include "../Config.hpp"
#include "../StringView.hpp"
#include "../BigInt.hpp"
#include <cstdint>
#include <cstddef>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace compat {
namespace detail {

/// <summary>
/// 核心高精度十進位浮點數演算法與輔助函式集合。
/// </summary>
class DecimalCore {
public:
    /// <summary>
    /// 計算 10 的非負整數次方之 bigint。
    /// </summary>
    /// <param name="exp">次方數</param>
    /// <returns>10^exp 之 bigint 數值</returns>
    static bigint power_of_10(size_t exp) {
        static const uint64_t POW10_TABLE[20] = {
            1ULL,
            10ULL,
            100ULL,
            1000ULL,
            10000ULL,
            100000ULL,
            1000000ULL,
            10000000ULL,
            100000000ULL,
            1000000000ULL,
            10000000000ULL,
            100000000000ULL,
            1000000000000ULL,
            10000000000000ULL,
            100000000000000ULL,
            1000000000000000ULL,
            10000000000000000ULL,
            100000000000000000ULL,
            1000000000000000000ULL,
            10000000000000000000ULL
        };

        if (exp < 20) {
            return bigint(POW10_TABLE[exp]);
        }

        // 二元快速冪演算法
        bigint base(10);
        bigint res(1);
        size_t e = exp;
        while (e > 0) {
            if (e & 1) {
                res *= base;
            }
            if (e > 1) {
                base *= base;
            }
            e >>= 1;
        }
        return res;
    }

    /// <summary>
    /// 標準化未縮放整數與小數縮放位數，消除尾端多餘零並將負縮放轉為整數。
    /// </summary>
    /// <param name="unscaled">未縮放整數參考</param>
    /// <param name="scale">小數縮放位數參考</param>
    /// <param name="is_nan">是否為 NaN</param>
    /// <param name="is_infinity">是否為無窮大</param>
    static void normalize(bigint& unscaled, int64_t& scale, bool is_nan, bool is_infinity) {
        if (is_nan || is_infinity) {
            return;
        }
        if (unscaled == 0) {
            scale = 0;
            return;
        }

        // 若 scale < 0，代表整數倍率（如 1e2），將 10^(-scale) 乘入 unscaled 轉為標準 scale = 0
        if (scale < 0) {
            unscaled *= power_of_10(static_cast<size_t>(-scale));
            scale = 0;
        }

        // 當 scale > 0 時，去除小數尾端多餘的 0
        while (scale >= 8 && (unscaled % 100000000ULL) == 0) {
            unscaled /= 100000000ULL;
            scale -= 8;
        }
        while (scale > 0 && (unscaled % 10) == 0) {
            unscaled /= 10;
            scale--;
        }
    }

    /// <summary>
    /// 自字串視圖解析十進位浮點數值。
    /// </summary>
    /// <param name="unscaled">輸出之未縮放整數</param>
    /// <param name="scale">輸出之小數縮放位數</param>
    /// <param name="is_infinity">輸出之無窮大標記</param>
    /// <param name="is_nan">輸出之 NaN 標記</param>
    /// <param name="sv">輸入字串視圖</param>
    /// <exception cref="std::invalid_argument">字串為空或格式不合法時拋出</exception>
    static void from_string(bigint& unscaled, int64_t& scale, bool& is_infinity, bool& is_nan, compat::string_view sv) {
        // 去除前後空白
        size_t start = 0;
        size_t end = sv.size();
        while (start < end && (sv[start] == ' ' || sv[start] == '\t' || sv[start] == '\r' || sv[start] == '\n')) {
            start++;
        }
        while (end > start && (sv[end - 1] == ' ' || sv[end - 1] == '\t' || sv[end - 1] == '\r' || sv[end - 1] == '\n')) {
            end--;
        }

        if (start >= end) {
            COMPAT_THROW_OR_ABORT(std::invalid_argument("empty decimal string"));
        }

        compat::string_view trimmed(sv.data() + start, end - start);

        // 轉換為全小寫副本以比對特殊字串
        std::string lower_str;
        lower_str.reserve(trimmed.size());
        for (size_t i = 0; i < trimmed.size(); ++i) {
            char c = trimmed[i];
            if (c >= 'A' && c <= 'Z') {
                c = static_cast<char>(c + ('a' - 'A'));
            }
            lower_str.push_back(c);
        }

        // 特殊常數比對
        if (lower_str == "nan" || lower_str == "+nan" || lower_str == "-nan") {
            is_nan = true;
            is_infinity = false;
            unscaled = 0;
            scale = 0;
            return;
        }
        if (lower_str == "inf" || lower_str == "+inf" || lower_str == "infinity" || lower_str == "+infinity") {
            is_nan = false;
            is_infinity = true;
            unscaled = 1;
            scale = 0;
            return;
        }
        if (lower_str == "-inf" || lower_str == "-infinity") {
            is_nan = false;
            is_infinity = true;
            unscaled = -1;
            scale = 0;
            return;
        }

        // 解析常規數值
        size_t idx = 0;
        size_t len = trimmed.size();
        int8_t sign = 1;

        if (trimmed[idx] == '+') {
            idx++;
        } else if (trimmed[idx] == '-') {
            sign = -1;
            idx++;
        }

        if (idx >= len) {
            COMPAT_THROW_OR_ABORT(std::invalid_argument("expected digits after sign"));
        }

        std::string digits;
        digits.reserve(len);
        bool has_dot = false;
        int64_t frac_digits = 0;
        bool has_any_digit = false;

        while (idx < len && trimmed[idx] != 'e' && trimmed[idx] != 'E') {
            char c = trimmed[idx];
            if (c >= '0' && c <= '9') {
                digits.push_back(c);
                has_any_digit = true;
                if (has_dot) {
                    frac_digits++;
                }
            } else if (c == '.') {
                if (has_dot) {
                    COMPAT_THROW_OR_ABORT(std::invalid_argument("multiple decimal points in decimal string"));
                }
                has_dot = true;
            } else {
                COMPAT_THROW_OR_ABORT(std::invalid_argument("invalid character in decimal string"));
            }
            idx++;
        }

        if (!has_any_digit) {
            COMPAT_THROW_OR_ABORT(std::invalid_argument("no digits found in mantissa"));
        }

        int64_t exp_val = 0;
        if (idx < len && (trimmed[idx] == 'e' || trimmed[idx] == 'E')) {
            idx++;
            if (idx >= len) {
                COMPAT_THROW_OR_ABORT(std::invalid_argument("empty exponent in decimal string"));
            }

            int8_t exp_sign = 1;
            if (trimmed[idx] == '+') {
                idx++;
            } else if (trimmed[idx] == '-') {
                exp_sign = -1;
                idx++;
            }

            if (idx >= len) {
                COMPAT_THROW_OR_ABORT(std::invalid_argument("expected digits after exponent sign"));
            }

            bool has_exp_digit = false;
            while (idx < len) {
                char c = trimmed[idx];
                if (c >= '0' && c <= '9') {
                    // 防止 exponent 過度溢位
                    if (exp_val < 1000000000LL) {
                        exp_val = exp_val * 10 + (c - '0');
                    }
                    has_exp_digit = true;
                } else {
                    COMPAT_THROW_OR_ABORT(std::invalid_argument("invalid character in exponent"));
                }
                idx++;
            }

            if (!has_exp_digit) {
                COMPAT_THROW_OR_ABORT(std::invalid_argument("empty exponent digits"));
            }
            exp_val *= exp_sign;
        }

        if (idx != len) {
            COMPAT_THROW_OR_ABORT(std::invalid_argument("unexpected trailing characters in decimal string"));
        }

        scale = frac_digits - exp_val;
        is_nan = false;
        is_infinity = false;
        unscaled = bigint(digits);
        if (sign < 0) {
            unscaled = -unscaled;
        }

        normalize(unscaled, scale, false, false);
    }

    /// <summary>
    /// 自原生浮點數型別轉換為十進位浮點數組件。
    /// </summary>
    /// <typeparam name="FloatT">浮點型別</typeparam>
    /// <param name="unscaled">輸出之未縮放整數</param>
    /// <param name="scale">輸出之小數縮放位數</param>
    /// <param name="is_infinity">輸出之無窮大標記</param>
    /// <param name="is_nan">輸出之 NaN 標記</param>
    /// <param name="val">原生浮點數值</param>
    template <typename FloatT>
    static void from_float(bigint& unscaled, int64_t& scale, bool& is_infinity, bool& is_nan, FloatT val) {
        if (std::isnan(static_cast<double>(val))) {
            is_nan = true;
            is_infinity = false;
            unscaled = 0;
            scale = 0;
            return;
        }
        if (std::isinf(static_cast<double>(val))) {
            is_infinity = true;
            is_nan = false;
            unscaled = (val < 0) ? -1 : 1;
            scale = 0;
            return;
        }
        if (val == 0) {
            is_nan = false;
            is_infinity = false;
            unscaled = 0;
            scale = 0;
            return;
        }

        char buf[128];
        if (sizeof(FloatT) <= sizeof(float)) {
            std::snprintf(buf, sizeof(buf), "%.7g", static_cast<double>(val));
            float parsed = std::strtof(buf, nullptr);
            if (parsed != static_cast<float>(val)) {
                std::snprintf(buf, sizeof(buf), "%.9g", static_cast<double>(val));
            }
        } else {
            std::snprintf(buf, sizeof(buf), "%.15g", static_cast<double>(val));
            double parsed = std::strtod(buf, nullptr);
            if (parsed != static_cast<double>(val)) {
                std::snprintf(buf, sizeof(buf), "%.17g", static_cast<double>(val));
            }
        }

        from_string(unscaled, scale, is_infinity, is_nan, compat::string_view(buf));
    }

    /// <summary>
    /// 將十進位浮點數格式化為標準字串表示法。
    /// </summary>
    /// <param name="unscaled">未縮放整數</param>
    /// <param name="scale">小數縮放位數</param>
    /// <param name="is_infinity">是否為無窮大</param>
    /// <param name="is_nan">是否為 NaN</param>
    /// <returns>格式化後之字串</returns>
    static std::string to_string(const bigint& unscaled, int64_t scale, bool is_infinity, bool is_nan) {
        if (is_nan) {
            return "nan";
        }
        if (is_infinity) {
            return (unscaled < 0) ? "-inf" : "inf";
        }
        if (unscaled == 0) {
            return "0";
        }

        std::string s = (unscaled < 0 ? (-unscaled) : unscaled).to_string();
        std::string res;
        res.reserve(s.size() + (scale > 0 ? static_cast<size_t>(scale) + 2 : 2));

        if (unscaled < 0) {
            res.push_back('-');
        }

        if (scale <= 0) {
            res += s;
            if (scale < 0) {
                res.append(static_cast<size_t>(-scale), '0');
            }
        } else {
            size_t s_len = s.size();
            if (s_len > static_cast<size_t>(scale)) {
                size_t int_len = s_len - static_cast<size_t>(scale);
                res.append(s, 0, int_len);
                res.push_back('.');
                res.append(s, int_len, static_cast<size_t>(scale));
            } else {
                res += "0.";
                size_t zeros = static_cast<size_t>(scale) - s_len;
                if (zeros > 0) {
                    res.append(zeros, '0');
                }
                res += s;
            }
        }
        return res;
    }

    /// <summary>
    /// 執行兩十進位數之加法運算。
    /// </summary>
    /// <param name="r_u">結果未縮放整數</param>
    /// <param name="r_s">結果小數縮放位數</param>
    /// <param name="r_inf">結果無窮大標記</param>
    /// <param name="r_nan">結果 NaN 標記</param>
    /// <param name="a_u">運算元 A 未縮放整數</param>
    /// <param name="a_s">運算元 A 小數縮放位數</param>
    /// <param name="a_inf">運算元 A 無窮大標記</param>
    /// <param name="a_nan">運算元 A NaN 標記</param>
    /// <param name="b_u">運算元 B 未縮放整數</param>
    /// <param name="b_s">運算元 B 小數縮放位數</param>
    /// <param name="b_inf">運算元 B 無窮大標記</param>
    /// <param name="b_nan">運算元 B NaN 標記</param>
    static void add(bigint& r_u, int64_t& r_s, bool& r_inf, bool& r_nan,
                    const bigint& a_u, int64_t a_s, bool a_inf, bool a_nan,
                    const bigint& b_u, int64_t b_s, bool b_inf, bool b_nan) {
        if (a_nan || b_nan) {
            r_nan = true;
            r_inf = false;
            r_u = 0;
            r_s = 0;
            return;
        }
        if (a_inf && b_inf) {
            int8_t sa = (a_u < 0) ? -1 : 1;
            int8_t sb = (b_u < 0) ? -1 : 1;
            if (sa != sb) {
                // inf - inf = NaN
                r_nan = true;
                r_inf = false;
                r_u = 0;
                r_s = 0;
                return;
            }
            r_inf = true;
            r_nan = false;
            r_u = sa;
            r_s = 0;
            return;
        }
        if (a_inf) {
            r_inf = true;
            r_nan = false;
            r_u = (a_u < 0) ? -1 : 1;
            r_s = 0;
            return;
        }
        if (b_inf) {
            r_inf = true;
            r_nan = false;
            r_u = (b_u < 0) ? -1 : 1;
            r_s = 0;
            return;
        }

        r_nan = false;
        r_inf = false;
        int64_t max_s = (a_s > b_s) ? a_s : b_s;
        bigint ua = a_u;
        bigint ub = b_u;

        if (max_s > a_s) {
            ua *= power_of_10(static_cast<size_t>(max_s - a_s));
        }
        if (max_s > b_s) {
            ub *= power_of_10(static_cast<size_t>(max_s - b_s));
        }

        r_u = ua + ub;
        r_s = max_s;
        normalize(r_u, r_s, false, false);
    }

    /// <summary>
    /// 執行兩十進位數之減法運算。
    /// </summary>
    /// <param name="r_u">結果未縮放整數</param>
    /// <param name="r_s">結果小數縮放位數</param>
    /// <param name="r_inf">結果無窮大標記</param>
    /// <param name="r_nan">結果 NaN 標記</param>
    /// <param name="a_u">運算元 A 未縮放整數</param>
    /// <param name="a_s">運算元 A 小數縮放位數</param>
    /// <param name="a_inf">運算元 A 無窮大標記</param>
    /// <param name="a_nan">運算元 A NaN 標記</param>
    /// <param name="b_u">運算元 B 未縮放整數</param>
    /// <param name="b_s">運算元 B 小數縮放位數</param>
    /// <param name="b_inf">運算元 B 無窮大標記</param>
    /// <param name="b_nan">運算元 B NaN 標記</param>
    static void sub(bigint& r_u, int64_t& r_s, bool& r_inf, bool& r_nan,
                    const bigint& a_u, int64_t a_s, bool a_inf, bool a_nan,
                    const bigint& b_u, int64_t b_s, bool b_inf, bool b_nan) {
        add(r_u, r_s, r_inf, r_nan, a_u, a_s, a_inf, a_nan, -b_u, b_s, b_inf, b_nan);
    }

    /// <summary>
    /// 執行兩十進位數之乘法運算。
    /// </summary>
    /// <param name="r_u">結果未縮放整數</param>
    /// <param name="r_s">結果小數縮放位數</param>
    /// <param name="r_inf">結果無窮大標記</param>
    /// <param name="r_nan">結果 NaN 標記</param>
    /// <param name="a_u">運算元 A 未縮放整數</param>
    /// <param name="a_s">運算元 A 小數縮放位數</param>
    /// <param name="a_inf">運算元 A 無窮大標記</param>
    /// <param name="a_nan">運算元 A NaN 標記</param>
    /// <param name="b_u">運算元 B 未縮放整數</param>
    /// <param name="b_s">運算元 B 小數縮放位數</param>
    /// <param name="b_inf">運算元 B 無窮大標記</param>
    /// <param name="b_nan">運算元 B NaN 標記</param>
    static void mul(bigint& r_u, int64_t& r_s, bool& r_inf, bool& r_nan,
                    const bigint& a_u, int64_t a_s, bool a_inf, bool a_nan,
                    const bigint& b_u, int64_t b_s, bool b_inf, bool b_nan) {
        if (a_nan || b_nan) {
            r_nan = true;
            r_inf = false;
            r_u = 0;
            r_s = 0;
            return;
        }
        if (a_inf || b_inf) {
            if (a_u == 0 || b_u == 0) {
                // inf * 0 = NaN
                r_nan = true;
                r_inf = false;
                r_u = 0;
                r_s = 0;
                return;
            }
            int8_t sa = (a_u < 0) ? -1 : 1;
            int8_t sb = (b_u < 0) ? -1 : 1;
            r_inf = true;
            r_nan = false;
            r_u = sa * sb;
            r_s = 0;
            return;
        }

        r_nan = false;
        r_inf = false;
        r_u = a_u * b_u;
        r_s = a_s + b_s;
        normalize(r_u, r_s, false, false);
    }

    /// <summary>
    /// 執行除法運算，支援指定精度與銀行家捨入法 (Banker's Rounding / Half-Even)。
    /// </summary>
    /// <param name="r_u">結果未縮放整數</param>
    /// <param name="r_s">結果小數縮放位數</param>
    /// <param name="r_inf">結果無窮大標記</param>
    /// <param name="r_nan">結果 NaN 標記</param>
    /// <param name="a_u">被除數未縮放整數</param>
    /// <param name="a_s">被除數小數縮放位數</param>
    /// <param name="a_inf">被除數無窮大標記</param>
    /// <param name="a_nan">被除數 NaN 標記</param>
    /// <param name="b_u">除數未縮放整數</param>
    /// <param name="b_s">除數小數縮放位數</param>
    /// <param name="b_inf">除數無窮大標記</param>
    /// <param name="b_nan">除數 NaN 標記</param>
    /// <param name="precision">有效十進位數字精度（預設為 34 位）</param>
    static void div(bigint& r_u, int64_t& r_s, bool& r_inf, bool& r_nan,
                    const bigint& a_u, int64_t a_s, bool a_inf, bool a_nan,
                    const bigint& b_u, int64_t b_s, bool b_inf, bool b_nan,
                    int32_t precision = 34) {
        if (a_nan || b_nan) {
            r_nan = true;
            r_inf = false;
            r_u = 0;
            r_s = 0;
            return;
        }
        if (b_inf) {
            if (a_inf) {
                // inf / inf = NaN
                r_nan = true;
                r_inf = false;
                r_u = 0;
                r_s = 0;
                return;
            }
            // x / inf = 0
            r_nan = false;
            r_inf = false;
            r_u = 0;
            r_s = 0;
            return;
        }
        if (b_u == 0) {
            if (a_u == 0) {
                // 0 / 0 = NaN
                r_nan = true;
                r_inf = false;
                r_u = 0;
                r_s = 0;
                return;
            }
            // 非零 / 0 = ±infinity
            int8_t sa = (a_u < 0) ? -1 : 1;
            r_inf = true;
            r_nan = false;
            r_u = sa;
            r_s = 0;
            return;
        }
        if (a_inf) {
            int8_t sa = (a_u < 0) ? -1 : 1;
            int8_t sb = (b_u < 0) ? -1 : 1;
            r_inf = true;
            r_nan = false;
            r_u = sa * sb;
            r_s = 0;
            return;
        }
        if (a_u == 0) {
            r_nan = false;
            r_inf = false;
            r_u = 0;
            r_s = 0;
            return;
        }

        if (precision <= 0) {
            precision = 34;
        }

        int8_t res_sign = 1;
        bigint ua = a_u;
        bigint ub = b_u;
        if (ua < 0) {
            res_sign = -res_sign;
            ua = -ua;
        }
        if (ub < 0) {
            res_sign = -res_sign;
            ub = -ub;
        }

        size_t da = ua.to_string().size();
        size_t db = ub.to_string().size();

        // 擴增被除數位數以滿足目標精度及捨入判斷
        int64_t K = static_cast<int64_t>(precision) - (static_cast<int64_t>(da) - static_cast<int64_t>(db)) + 5;
        if (K < 0) {
            K = 0;
        }

        bigint num = ua;
        if (K > 0) {
            num *= power_of_10(static_cast<size_t>(K));
        }

        bigint Q = num / ub;
        bigint R = num % ub;

        std::string q_str = Q.to_string();
        size_t dq = q_str.size();
        int64_t current_scale = a_s - b_s + K;

        // 若商位數大於期望有效數字 precision，執行銀行家捨入
        if (dq > static_cast<size_t>(precision)) {
            size_t L = dq - static_cast<size_t>(precision);
            bigint divisor = power_of_10(L);
            bigint half = power_of_10(L - 1) * 5;
            bigint q_drop = Q / divisor;
            bigint rem = Q % divisor;

            bool round_up = false;
            if (rem > half) {
                round_up = true;
            } else if (rem < half) {
                round_up = false;
            } else {
                // rem == half: 若尚有不可整除餘數 R，代表大於一半
                if (R != 0) {
                    round_up = true;
                } else {
                    // 恰好為一半：捨入至最接近的偶數位 (Half-Even)
                    round_up = (q_drop % 2 != 0);
                }
            }

            if (round_up) {
                q_drop += 1;
                // 若進位造成位數增加（如 999 -> 1000），調整位移
                if (q_drop.to_string().size() > static_cast<size_t>(precision)) {
                    q_drop /= 10;
                    L++;
                }
            }

            r_u = (res_sign < 0) ? -q_drop : q_drop;
            r_s = current_scale - static_cast<int64_t>(L);
        } else {
            r_u = (res_sign < 0) ? -Q : Q;
            r_s = current_scale;
        }

        normalize(r_u, r_s, false, false);
    }

    /// <summary>
    /// 執行兩十進位數之取模運算。
    /// </summary>
    /// <param name="r_u">結果未縮放整數</param>
    /// <param name="r_s">結果小數縮放位數</param>
    /// <param name="r_inf">結果無窮大標記</param>
    /// <param name="r_nan">結果 NaN 標記</param>
    /// <param name="a_u">被除數未縮放整數</param>
    /// <param name="a_s">被除數小數縮放位數</param>
    /// <param name="a_inf">被除數無窮大標記</param>
    /// <param name="a_nan">被除數 NaN 標記</param>
    /// <param name="b_u">除數未縮放整數</param>
    /// <param name="b_s">除數小數縮放位數</param>
    /// <param name="b_inf">除數無窮大標記</param>
    /// <param name="b_nan">除數 NaN 標記</param>
    static void mod(bigint& r_u, int64_t& r_s, bool& r_inf, bool& r_nan,
                    const bigint& a_u, int64_t a_s, bool a_inf, bool a_nan,
                    const bigint& b_u, int64_t b_s, bool b_inf, bool b_nan) {
        if (a_nan || b_nan || a_inf || b_u == 0) {
            r_nan = true;
            r_inf = false;
            r_u = 0;
            r_s = 0;
            return;
        }
        if (b_inf) {
            r_nan = false;
            r_inf = false;
            r_u = a_u;
            r_s = a_s;
            return;
        }

        r_nan = false;
        r_inf = false;
        int64_t max_s = (a_s > b_s) ? a_s : b_s;
        bigint ua = a_u;
        bigint ub = b_u;

        if (max_s > a_s) {
            ua *= power_of_10(static_cast<size_t>(max_s - a_s));
        }
        if (max_s > b_s) {
            ub *= power_of_10(static_cast<size_t>(max_s - b_s));
        }

        r_u = ua % ub;
        r_s = max_s;
        normalize(r_u, r_s, false, false);
    }

    /// <summary>
    /// 依指定小數位數執行銀行家捨入 (Banker's Rounding / Half-Even)。
    /// </summary>
    /// <param name="unscaled">未縮放整數參考</param>
    /// <param name="scale">小數縮放位數參考</param>
    /// <param name="decimal_places">目標小數位數</param>
    static void round_half_even(bigint& unscaled, int64_t& scale, int64_t decimal_places) {
        if (scale <= decimal_places) {
            return;
        }

        int64_t L = scale - decimal_places;
        bigint divisor = power_of_10(static_cast<size_t>(L));
        bigint half = power_of_10(static_cast<size_t>(L - 1)) * 5;

        int8_t sign = (unscaled < 0) ? -1 : 1;
        bigint u_abs = (unscaled < 0) ? -unscaled : unscaled;
        bigint q = u_abs / divisor;
        bigint rem = u_abs % divisor;

        if (rem > half) {
            q += 1;
        } else if (rem == half) {
            if (q % 2 != 0) {
                q += 1;
            }
        }

        unscaled = (sign < 0) ? -q : q;
        scale = decimal_places;
        normalize(unscaled, scale, false, false);
    }

    /// <summary>
    /// 比較兩十進位數之大小關係。
    /// </summary>
    /// <param name="a_u">運算元 A 未縮放整數</param>
    /// <param name="a_s">運算元 A 小數縮放位數</param>
    /// <param name="a_inf">運算元 A 無窮大標記</param>
    /// <param name="a_nan">運算元 A NaN 標記</param>
    /// <param name="b_u">運算元 B 未縮放整數</param>
    /// <param name="b_s">運算元 B 小數縮放位數</param>
    /// <param name="b_inf">運算元 B 無窮大標記</param>
    /// <param name="b_nan">運算元 B NaN 標記</param>
    /// <returns>小於傳回 -1，等於傳回 0，大於傳回 1，若含 NaN 傳回 -2（無序）</returns>
    static int compare(const bigint& a_u, int64_t a_s, bool a_inf, bool a_nan,
                       const bigint& b_u, int64_t b_s, bool b_inf, bool b_nan) {
        if (a_nan || b_nan) {
            return -2; // 無序 (unordered)
        }
        if (a_inf && b_inf) {
            int8_t sa = (a_u < 0) ? -1 : 1;
            int8_t sb = (b_u < 0) ? -1 : 1;
            if (sa < sb) return -1;
            if (sa > sb) return 1;
            return 0;
        }
        if (a_inf) {
            return (a_u < 0) ? -1 : 1;
        }
        if (b_inf) {
            return (b_u < 0) ? 1 : -1;
        }

        if (a_s == b_s) {
            if (a_u < b_u) return -1;
            if (a_u > b_u) return 1;
            return 0;
        }

        int64_t max_s = (a_s > b_s) ? a_s : b_s;
        bigint ua = a_u;
        bigint ub = b_u;

        if (max_s > a_s) {
            ua *= power_of_10(static_cast<size_t>(max_s - a_s));
        }
        if (max_s > b_s) {
            ub *= power_of_10(static_cast<size_t>(max_s - b_s));
        }

        if (ua < ub) return -1;
        if (ua > ub) return 1;
        return 0;
    }
};

} // namespace detail
} // namespace compat
