#pragma once

// CMath.hpp
// High-precision mathematical functions for BigInt and Decimal in CPP-Compat.
// Zero external dependencies, downward compatible from C++23 to C++11.

#include "Config.hpp"
#include "BigInt.hpp"
#include "Decimal.hpp"
#include <cstdint>
#include <cmath>
#include <stdexcept>
#include <utility>
#include <string>

namespace compat {

namespace detail {

    /// <summary>
    /// 取得高精度圓周率 Pi 常數（50 位有效數字）。
    /// </summary>
    /// <returns>高精度 Pi 之 decimal 參考</returns>
    inline const decimal& cmath_pi() {
        static const decimal val("3.1415926535897932384626433832795028841971693993751");
        return val;
    }

    /// <summary>
    /// 取得高精度 2*Pi 常數（50 位有效數字）。
    /// </summary>
    /// <returns>高精度 2*Pi 之 decimal 參考</returns>
    inline const decimal& cmath_two_pi() {
        static const decimal val("6.2831853071795864769252867665590057683943387987502");
        return val;
    }

    /// <summary>
    /// 取得高精度 Pi/2 常數（50 位有效數字）。
    /// </summary>
    /// <returns>高精度 Pi/2 之 decimal 參考</returns>
    inline const decimal& cmath_pi_over_2() {
        static const decimal val("1.5707963267948966192313216916397514420985846996875");
        return val;
    }

    /// <summary>
    /// 取得高精度 ln(2) 常數（50 位有效數字）。
    /// </summary>
    /// <returns>高精度 ln(2) 之 decimal 參考</returns>
    inline const decimal& cmath_ln2() {
        static const decimal val("0.693147180559945309417232121458176568075500134360255");
        return val;
    }

    /// <summary>
    /// 取得高精度 ln(10) 常數（50 位有效數字）。
    /// </summary>
    /// <returns>高精度 ln(10) 之 decimal 參考</returns>
    inline const decimal& cmath_ln10() {
        static const decimal val("2.30258509299404568401799145468436420760110148862877");
        return val;
    }

} // namespace detail

// ============================================================================
// 1. BigInt 數學函式
// ============================================================================

/// <summary>
/// 計算 bigint 之絕對值。
/// </summary>
/// <param name="x">輸入整數</param>
/// <returns>絕對值結果</returns>
COMPAT_NODISCARD inline bigint abs(const bigint& x) noexcept {
    return (x.sign() < 0) ? -x : x;
}

/// <summary>
/// 計算 bigint 之整數平方根，回傳最大滿足 r^2 &lt;= x 之整數。
/// </summary>
/// <param name="x">非負輸入整數</param>
/// <returns>整數平方根</returns>
/// <exception cref="std::invalid_argument">若 x 為負數時拋出</exception>
COMPAT_NODISCARD inline bigint isqrt(const bigint& x) {
    if (x.sign() < 0) {
        COMPAT_THROW_OR_ABORT(std::invalid_argument("isqrt: square root of negative bigint"));
    }
    if (x == 0) {
        return bigint(0);
    }
    if (x == 1) {
        return bigint(1);
    }

    // 計算二進位長度以決定牛頓迭代之初值估計
    size_t count = x.limb_count();
    const uint64_t* limbs = x.limbs();
    uint64_t high_limb = limbs[count - 1];
    int high_bits = 0;
    while (high_limb > 0) {
        high_bits++;
        high_limb >>= 1;
    }
    size_t bit_len = (count - 1) * 64 + static_cast<size_t>(high_bits);
    size_t shift = (bit_len + 1) / 2;

    // 牛頓-拉弗森整數迭代演算法
    bigint x0 = bigint(1) << shift;
    bigint x1 = (x0 + x / x0) >> 1;
    while (x1 < x0) {
        x0 = std::move(x1);
        x1 = (x0 + x / x0) >> 1;
    }
    while (x0 * x0 > x) {
        --x0;
    }
    return x0;
}

/// <summary>
/// 計算 bigint 之整數平方根（isqrt 之別名）。
/// </summary>
/// <param name="x">非負輸入整數</param>
/// <returns>整數平方根</returns>
/// <exception cref="std::invalid_argument">若 x 為負數時拋出</exception>
COMPAT_NODISCARD inline bigint sqrt(const bigint& x) {
    return isqrt(x);
}

/// <summary>
/// 計算 bigint 之整數立方根，回傳整數立方根（奇函數，支援負數）。
/// </summary>
/// <param name="x">輸入整數</param>
/// <returns>整數立方根</returns>
COMPAT_NODISCARD inline bigint icbrt(const bigint& x) noexcept {
    if (x == 0) {
        return bigint(0);
    }
    if (x.sign() < 0) {
        return -icbrt(-x);
    }
    if (x == 1) {
        return bigint(1);
    }

    // 計算二進位長度以決定立方根初值位移
    size_t count = x.limb_count();
    const uint64_t* limbs = x.limbs();
    uint64_t high_limb = limbs[count - 1];
    int high_bits = 0;
    while (high_limb > 0) {
        high_bits++;
        high_limb >>= 1;
    }
    size_t bit_len = (count - 1) * 64 + static_cast<size_t>(high_bits);
    size_t shift = (bit_len + 2) / 3;

    // 牛頓-拉弗森整數立方根迭代
    bigint x0 = bigint(1) << shift;
    bigint x1 = (x0 * 2 + x / (x0 * x0)) / 3;
    while (x1 < x0) {
        x0 = std::move(x1);
        x1 = (x0 * 2 + x / (x0 * x0)) / 3;
    }
    while (x0 * x0 * x0 > x) {
        --x0;
    }
    while ((x0 + 1) * (x0 + 1) * (x0 + 1) <= x) {
        ++x0;
    }
    return x0;
}

/// <summary>
/// 計算 bigint 之整數立方根（icbrt 之別名）。
/// </summary>
/// <param name="x">輸入整數</param>
/// <returns>整數立方根</returns>
COMPAT_NODISCARD inline bigint cbrt(const bigint& x) noexcept {
    return icbrt(x);
}

/// <summary>
/// 計算 bigint 之整數非負次方冪（快速冪演算法）。
/// </summary>
/// <param name="base">底數</param>
/// <param name="exp">無符號指數</param>
/// <returns>冪次結果</returns>
COMPAT_NODISCARD inline bigint pow(const bigint& base, unsigned int exp) {
    if (exp == 0) {
        return bigint(1);
    }
    if (base == 0) {
        return bigint(0);
    }
    if (base == 1) {
        return bigint(1);
    }
    if (base == -1) {
        return (exp & 1u) ? bigint(-1) : bigint(1);
    }

    // 二元快速冪
    bigint res(1);
    bigint b = base;
    while (exp > 0) {
        if (exp & 1u) {
            res *= b;
        }
        exp >>= 1;
        if (exp > 0) {
            b *= b;
        }
    }
    return res;
}

/// <summary>
/// 計算 bigint 之整數冪（支援 bigint 指數）。
/// </summary>
/// <param name="base">底數</param>
/// <param name="exp">指數</param>
/// <returns>冪次結果</returns>
/// <exception cref="std::invalid_argument">若指數為負且底數非 1 或 -1 時拋出</exception>
COMPAT_NODISCARD inline bigint pow(const bigint& base, const bigint& exp) {
    if (exp.sign() < 0) {
        if (base == 1) {
            return bigint(1);
        }
        if (base == -1) {
            return (exp % 2 != 0) ? bigint(-1) : bigint(1);
        }
        COMPAT_THROW_OR_ABORT(std::invalid_argument("pow: negative exponent in bigint pow"));
    }
    if (exp == 0) {
        return bigint(1);
    }
    if (base == 0) {
        return bigint(0);
    }
    if (base == 1) {
        return bigint(1);
    }
    if (base == -1) {
        return (exp % 2 != 0) ? bigint(-1) : bigint(1);
    }

    // 二元快速冪
    bigint res(1);
    bigint b = base;
    bigint e = exp;
    while (e > 0) {
        if (e % 2 != 0) {
            res *= b;
        }
        e >>= 1;
        if (e > 0) {
            b *= b;
        }
    }
    return res;
}

// ============================================================================
// 2. Decimal 數值分類與符號函式
// ============================================================================

/// <summary>
/// 檢查 decimal 是否為非數值 (NaN)。
/// </summary>
/// <param name="x">輸入數值</param>
/// <returns>若為 NaN 回傳 true，否則回傳 false</returns>
COMPAT_NODISCARD inline bool isnan(const decimal& x) noexcept {
    return x.is_nan();
}

/// <summary>
/// 檢查 decimal 是否為無窮大 (+/- Infinity)。
/// </summary>
/// <param name="x">輸入數值</param>
/// <returns>若為無窮大回傳 true，否則回傳 false</returns>
COMPAT_NODISCARD inline bool isinf(const decimal& x) noexcept {
    return x.is_infinite();
}

/// <summary>
/// 檢查 decimal 是否為有限數值。
/// </summary>
/// <param name="x">輸入數值</param>
/// <returns>若非 NaN 且非無窮大回傳 true，否則回傳 false</returns>
COMPAT_NODISCARD inline bool isfinite(const decimal& x) noexcept {
    return x.is_finite();
}

/// <summary>
/// 檢查 decimal 之符號位元是否為負。
/// </summary>
/// <param name="x">輸入數值</param>
/// <returns>若為負數回傳 true，否則回傳 false</returns>
COMPAT_NODISCARD inline bool signbit(const decimal& x) noexcept {
    return x.sign() < 0 || x.unscaled().sign() < 0;
}

/// <summary>
/// 結合 mag 之絕對值大小與 sgn 之正負號。
/// </summary>
/// <param name="mag">數值大小</param>
/// <param name="sgn">符號來源</param>
/// <returns>調整符號後之數值</returns>
COMPAT_NODISCARD inline decimal copysign(const decimal& mag, const decimal& sgn) noexcept {
    if (mag.is_nan()) {
        return mag;
    }
    bool sgn_neg = (sgn.sign() < 0 || sgn.unscaled().sign() < 0);
    bool mag_neg = (mag.sign() < 0 || mag.unscaled().sign() < 0);
    if (sgn_neg != mag_neg) {
        return -mag;
    }
    return mag;
}

/// <summary>
/// 計算 decimal 之絕對值。
/// </summary>
/// <param name="x">輸入數值</param>
/// <returns>絕對值結果</returns>
COMPAT_NODISCARD inline decimal abs(const decimal& x) noexcept {
    if (x.is_nan()) {
        return x;
    }
    if (x.is_infinite()) {
        return decimal::infinity;
    }
    if (x.sign() < 0) {
        return -x;
    }
    return x;
}

// ============================================================================
// 3. Decimal 捨入、取模與餘數函式
// ============================================================================

/// <summary>
/// 計算不大於 x 的最大整數（向下取整）。
/// </summary>
/// <param name="x">輸入數值</param>
/// <returns>向下取整後之整數</returns>
COMPAT_NODISCARD inline decimal floor(const decimal& x) {
    if (x.is_nan() || x.is_infinite()) {
        return x;
    }
    int64_t s = x.scale();
    if (s <= 0) {
        return x;
    }
    const bigint& u = x.unscaled();
    bigint p = detail::DecimalCore::power_of_10(static_cast<size_t>(s));
    bigint q = u / p;
    bigint r = u % p;
    if (r == 0) {
        return x;
    }
    if (u.sign() < 0) {
        return decimal(q - 1, 0);
    }
    return decimal(q, 0);
}

/// <summary>
/// 計算不小於 x 的最小整數（向上取整）。
/// </summary>
/// <param name="x">輸入數值</param>
/// <returns>向上取整後之整數</returns>
COMPAT_NODISCARD inline decimal ceil(const decimal& x) {
    if (x.is_nan() || x.is_infinite()) {
        return x;
    }
    int64_t s = x.scale();
    if (s <= 0) {
        return x;
    }
    const bigint& u = x.unscaled();
    bigint p = detail::DecimalCore::power_of_10(static_cast<size_t>(s));
    bigint q = u / p;
    bigint r = u % p;
    if (r == 0) {
        return x;
    }
    if (u.sign() > 0) {
        return decimal(q + 1, 0);
    }
    return decimal(q, 0);
}

/// <summary>
/// 向零捨入整數部分（截斷小數）。
/// </summary>
/// <param name="x">輸入數值</param>
/// <returns>截斷後之整數</returns>
COMPAT_NODISCARD inline decimal trunc(const decimal& x) {
    if (x.is_nan() || x.is_infinite()) {
        return x;
    }
    int64_t s = x.scale();
    if (s <= 0) {
        return x;
    }
    const bigint& u = x.unscaled();
    bigint p = detail::DecimalCore::power_of_10(static_cast<size_t>(s));
    bigint q = u / p;
    return decimal(q, 0);
}

/// <summary>
/// 四捨五入至最接近整數（Half away from zero，符合標準 cmath 語意）。
/// </summary>
/// <param name="x">輸入數值</param>
/// <returns>四捨五入後之整數</returns>
COMPAT_NODISCARD inline decimal round(const decimal& x) {
    if (x.is_nan() || x.is_infinite()) {
        return x;
    }
    int64_t s = x.scale();
    if (s <= 0) {
        return x;
    }
    const bigint& u = x.unscaled();
    bigint p = detail::DecimalCore::power_of_10(static_cast<size_t>(s));
    bigint q = u / p;
    bigint r = u % p;
    if (r == 0) {
        return x;
    }
    if (u.sign() > 0) {
        if (r * 2 >= p) {
            return decimal(q + 1, 0);
        }
        return decimal(q, 0);
    } else {
        if ((-r) * 2 >= p) {
            return decimal(q - 1, 0);
        }
        return decimal(q, 0);
    }
}

/// <summary>
/// 計算浮點除法取模：x - trunc(x/y) * y。
/// </summary>
/// <param name="x">被除數</param>
/// <param name="y">除數</param>
/// <returns>取模餘數</returns>
COMPAT_NODISCARD inline decimal fmod(const decimal& x, const decimal& y) {
    if (x.is_nan() || y.is_nan()) {
        return decimal::nan;
    }
    if (x.is_infinite() || y.is_zero()) {
        return decimal::nan;
    }
    if (y.is_infinite()) {
        return x;
    }
    if (x.is_zero()) {
        return x;
    }
    decimal q = trunc(x / y);
    return x - q * y;
}

/// <summary>
/// 計算 IEEE-754 浮點餘數：x - round(x/y) * y（Round to nearest, ties to even）。
/// </summary>
/// <param name="x">被除數</param>
/// <param name="y">除數</param>
/// <returns>IEEE 餘數</returns>
COMPAT_NODISCARD inline decimal remainder(const decimal& x, const decimal& y) {
    if (x.is_nan() || y.is_nan()) {
        return decimal::nan;
    }
    if (x.is_infinite() || y.is_zero()) {
        return decimal::nan;
    }
    if (y.is_infinite()) {
        return x;
    }
    if (x.is_zero()) {
        return x;
    }
    decimal div_val = x / y;
    decimal n = div_val.round(0);
    return x - n * y;
}

// ============================================================================
// 4. Decimal 根號與次方函式
// ============================================================================

/// <summary>
/// 計算 decimal 之平方根（牛頓-拉弗森迭代法，收斂至 34 位有效位數）。
/// </summary>
/// <param name="x">非負輸入數值</param>
/// <returns>平方根結果</returns>
COMPAT_NODISCARD inline decimal sqrt(const decimal& x) {
    if (x.is_nan()) {
        return decimal::nan;
    }
    if (x.sign() < 0) {
        return decimal::nan;
    }
    if (x.is_zero()) {
        return decimal(0);
    }
    if (x.is_infinite()) {
        return decimal::infinity;
    }

    // 初值估算
    double d_val = static_cast<double>(x);
    decimal y0;
    if (std::isfinite(d_val) && d_val > 0.0) {
        y0 = decimal(std::sqrt(d_val));
    } else {
        std::string s_str = x.unscaled().to_string();
        int64_t total_exp = static_cast<int64_t>(s_str.size()) - 1 - x.scale();
        size_t take = (s_str.size() > 16) ? 16 : s_str.size();
        double mantissa = std::stod(s_str.substr(0, take));
        int64_t adj_exp = total_exp - (static_cast<int64_t>(s_str.size()) - 1) + (static_cast<int64_t>(take) - 1);
        double d_approx = mantissa * std::pow(10.0, static_cast<double>(adj_exp));
        if (std::isfinite(d_approx) && d_approx > 0.0) {
            y0 = decimal(std::sqrt(d_approx));
        } else {
            y0 = decimal(1);
        }
    }

    // 牛頓迭代：y_{n+1} = (y_n + x / y_n) / 2
    decimal y = y0;
    for (int i = 0; i < 4; ++i) {
        y = (y + x.divide(y, 40)).divide(decimal(2), 40);
    }
    return y.divide(decimal(1), 34);
}

/// <summary>
/// 計算 decimal 之立方根（牛頓法，奇函數）。
/// </summary>
/// <param name="x">輸入數值</param>
/// <returns>立方根結果</returns>
COMPAT_NODISCARD inline decimal cbrt(const decimal& x) {
    if (x.is_nan()) {
        return decimal::nan;
    }
    if (x.is_zero()) {
        return decimal(0);
    }
    if (x.is_infinite()) {
        return x;
    }
    if (x.sign() < 0) {
        return -cbrt(-x);
    }

    double d_val = static_cast<double>(x);
    decimal y0;
    if (std::isfinite(d_val) && d_val > 0.0) {
        y0 = decimal(std::cbrt(d_val));
    } else {
        y0 = decimal(1);
    }

    // 牛頓迭代：y_{n+1} = (2 * y_n + x / y_n^2) / 3
    decimal y = y0;
    for (int i = 0; i < 4; ++i) {
        decimal y2 = y * y;
        decimal term = x.divide(y2, 40);
        y = (y * 2 + term).divide(decimal(3), 40);
    }
    return y.divide(decimal(1), 34);
}

/// <summary>
/// 計算直角三角形斜邊長 sqrt(x^2 + y^2)。
/// </summary>
/// <param name="x">第一股長度</param>
/// <param name="y">第二股長度</param>
/// <returns>斜邊長度</returns>
COMPAT_NODISCARD inline decimal hypot(const decimal& x, const decimal& y) {
    if (x.is_infinite() || y.is_infinite()) {
        return decimal::infinity;
    }
    if (x.is_nan() || y.is_nan()) {
        return decimal::nan;
    }
    if (x.is_zero()) {
        return abs(y);
    }
    if (y.is_zero()) {
        return abs(x);
    }
    decimal ax = abs(x);
    decimal ay = abs(y);
    decimal m_max = (ax > ay) ? ax : ay;
    decimal m_min = (ax > ay) ? ay : ax;
    decimal t = m_min.divide(m_max, 40);
    return (m_max * sqrt(decimal(1) + t * t)).divide(decimal(1), 34);
}

/// <summary>
/// 計算 decimal 之整數次方（快速冪演算法）。
/// </summary>
/// <param name="base">底數</param>
/// <param name="exp">整數指數</param>
/// <returns>冪次結果</returns>
COMPAT_NODISCARD inline decimal pow(const decimal& base, int exp) {
    if (exp == 0) {
        return decimal(1);
    }
    if (base.is_nan()) {
        return decimal::nan;
    }
    if (base.is_zero()) {
        if (exp > 0) {
            return decimal(0);
        }
        return decimal::infinity;
    }
    if (base == decimal(1)) {
        return decimal(1);
    }

    int64_t e = exp;
    bool neg = false;
    if (e < 0) {
        neg = true;
        e = -e;
    }
    decimal res = decimal(1);
    decimal b = base;
    while (e > 0) {
        if (e & 1) {
            res = (res * b).divide(decimal(1), 38);
        }
        e >>= 1;
        if (e > 0) {
            b = (b * b).divide(decimal(1), 38);
        }
    }
    if (neg) {
        return decimal(1).divide(res, 34);
    }
    return res.divide(decimal(1), 34);
}

// ============================================================================
// 5. Decimal 指數與對數函式
// ============================================================================

/// <summary>
/// 計算自然指數函式 exp(x)（Range Reduction 模 ln 2 後泰勒級數展開）。
/// </summary>
/// <param name="x">輸入數值</param>
/// <returns>exp(x) 結果</returns>
COMPAT_NODISCARD inline decimal exp(const decimal& x) {
    if (x.is_nan()) {
        return decimal::nan;
    }
    if (x.is_zero()) {
        return decimal(1);
    }
    if (x.is_infinite()) {
        return (x.sign() > 0) ? decimal::infinity : decimal(0);
    }
    if (x > decimal(10000)) {
        return decimal::infinity;
    }
    if (x < decimal(-10000)) {
        return decimal(0);
    }

    // Range Reduction: x = k * ln(2) + r
    const decimal& ln2 = detail::cmath_ln2();
    decimal k_dec = round(x / ln2);
    decimal r = x - k_dec * ln2;

    // Scaling and squaring: r' = r / 128 (2^7)
    decimal r_prime = r.divide(decimal(128), 42);
    decimal term = r_prime;
    decimal sum = decimal(1) + r_prime;
    for (int n = 2; n <= 18; ++n) {
        term = (term * r_prime).divide(decimal(n), 42);
        sum += term;
        if (abs(term) < decimal("1e-40")) {
            break;
        }
    }

    // 平方 7 次還原
    for (int i = 0; i < 7; ++i) {
        sum = (sum * sum).divide(decimal(1), 40);
    }

    // 乘上 2^k
    int64_t k = static_cast<int64_t>(k_dec);
    if (k > 0) {
        sum = (sum * pow(decimal(2), static_cast<int>(k))).divide(decimal(1), 34);
    } else if (k < 0) {
        sum = sum.divide(pow(decimal(2), static_cast<int>(-k)), 34);
    } else {
        sum = sum.divide(decimal(1), 34);
    }
    return sum;
}

/// <summary>
/// 計算自然對數 ln(x)（x &gt; 0）。
/// </summary>
/// <param name="x">正數輸入數值</param>
/// <returns>ln(x) 結果</returns>
COMPAT_NODISCARD inline decimal log(const decimal& x) {
    if (x.is_nan()) {
        return decimal::nan;
    }
    if (x.sign() < 0) {
        return decimal::nan;
    }
    if (x.is_zero()) {
        return -decimal::infinity;
    }
    if (x.is_infinite()) {
        return decimal::infinity;
    }
    if (x == decimal(1)) {
        return decimal(0);
    }
    const decimal& ln2 = detail::cmath_ln2();
    const decimal& ln10 = detail::cmath_ln10();

    // 依 10 的次冪進行初步範圍縮減
    std::string s_str = x.unscaled().to_string();
    int64_t p10 = static_cast<int64_t>(s_str.size()) - 1 - x.scale();
    decimal m;
    if (p10 > 0) {
        m = x.divide(detail::DecimalCore::power_of_10(static_cast<size_t>(p10)), 42);
    } else if (p10 < 0) {
        m = (x * detail::DecimalCore::power_of_10(static_cast<size_t>(-p10))).divide(decimal(1), 42);
    } else {
        m = x;
    }

    // 依 2 的次冪縮減至 [1/sqrt(2), sqrt(2)]
    int64_t p2 = 0;
    decimal sqrt2("1.414213562373095048801688724209698");
    decimal inv_sqrt2("0.707106781186547524400844362104849");

    while (m > sqrt2) {
        m = m.divide(decimal(2), 42);
        p2++;
    }
    while (m < inv_sqrt2) {
        m = (m * decimal(2)).divide(decimal(1), 42);
        p2--;
    }

    // 快速收斂級數：ln(m) = 2 * u * sum(u^(2j) / (2j+1)), u = (m-1)/(m+1)
    decimal u = (m - decimal(1)).divide(m + decimal(1), 42);
    decimal u2 = (u * u).divide(decimal(1), 42);
    decimal u_pow = u;
    decimal series_sum = u;
    for (int j = 1; j <= 30; ++j) {
        u_pow = (u_pow * u2).divide(decimal(1), 42);
        decimal term = u_pow.divide(decimal(2 * j + 1), 42);
        series_sum += term;
        if (abs(term) < decimal("1e-40")) {
            break;
        }
    }
    decimal ln_m = (series_sum * decimal(2)).divide(decimal(1), 40);

    // 重組：ln(x) = p10 * ln(10) + p2 * ln(2) + ln(m)
    decimal res = ln_m;
    if (p10 != 0) {
        res += decimal(p10) * ln10;
    }
    if (p2 != 0) {
        res += decimal(p2) * ln2;
    }
    return res.divide(decimal(1), 34);
}

/// <summary>
/// 計算常用對數 log10(x)（以 10 為底之對數）。
/// </summary>
/// <param name="x">正數輸入數值</param>
/// <returns>log10(x) 結果</returns>
COMPAT_NODISCARD inline decimal log10(const decimal& x) {
    if (x.is_nan()) {
        return decimal::nan;
    }
    if (x.sign() < 0) {
        return decimal::nan;
    }
    if (x.is_zero()) {
        return -decimal::infinity;
    }
    if (x.is_infinite()) {
        return decimal::infinity;
    }
    if (x == decimal(1)) {
        return decimal(0);
    }
    if (x.scale() == 0 && x.unscaled() > 0) {
        bigint u = x.unscaled();
        int64_t p = 0;
        while (u > 1 && (u % 10) == 0) {
            u /= 10;
            ++p;
        }
        if (u == 1) {
            return decimal(p);
        }
    }
    if (x < decimal(1) && x > decimal(0)) {
        decimal inv = decimal(1) / x;
        if (inv.scale() == 0 && inv.unscaled() > 0) {
            bigint u = inv.unscaled();
            int64_t p = 0;
            while (u > 1 && (u % 10) == 0) {
                u /= 10;
                ++p;
            }
            if (u == 1) {
                return decimal(-p);
            }
        }
    }
    const decimal& ln10 = detail::cmath_ln10();
    return log(x).divide(ln10, 34);
}

/// <summary>
/// 計算以 2 為底之對數 log2(x)。
/// </summary>
/// <param name="x">正數輸入數值</param>
/// <returns>log2(x) 結果</returns>
COMPAT_NODISCARD inline decimal log2(const decimal& x) {
    if (x.is_nan()) {
        return decimal::nan;
    }
    if (x.sign() < 0) {
        return decimal::nan;
    }
    if (x.is_zero()) {
        return -decimal::infinity;
    }
    if (x.is_infinite()) {
        return decimal::infinity;
    }
    if (x == decimal(1)) {
        return decimal(0);
    }
    if (x.scale() == 0 && x.unscaled() > 0) {
        bigint u = x.unscaled();
        int64_t p = 0;
        while (u > 1 && (u % 2) == 0) {
            u /= 2;
            ++p;
        }
        if (u == 1) {
            return decimal(p);
        }
    }
    if (x < decimal(1) && x > decimal(0)) {
        decimal inv = decimal(1) / x;
        if (inv.scale() == 0 && inv.unscaled() > 0) {
            bigint u = inv.unscaled();
            int64_t p = 0;
            while (u > 1 && (u % 2) == 0) {
                u /= 2;
                ++p;
            }
            if (u == 1) {
                return decimal(-p);
            }
        }
    }
    const decimal& ln2 = detail::cmath_ln2();
    return log(x).divide(ln2, 34);
}

/// <summary>
/// 計算實數冪次 base^exp（支援 decimal 底數與指數）。
/// </summary>
/// <param name="base">底數</param>
/// <param name="exp">指數</param>
/// <returns>冪次結果</returns>
COMPAT_NODISCARD inline decimal pow(const decimal& base, const decimal& exp) {
    if (exp.is_zero()) {
        return decimal(1);
    }
    if (base == decimal(1)) {
        return decimal(1);
    }
    if (base.is_nan() || exp.is_nan()) {
        return decimal::nan;
    }
    if (base.is_zero()) {
        if (exp.sign() > 0) {
            return decimal(0);
        }
        return decimal::infinity;
    }

    // 若指數為整數，優先走整數快速冪路徑以保證精確性
    bool exp_is_int = (exp.scale() <= 0 || exp == trunc(exp));
    if (exp_is_int) {
        try {
            int64_t int_exp = static_cast<int64_t>(exp);
            if (int_exp >= -2147483647 && int_exp <= 2147483647) {
                return pow(base, static_cast<int>(int_exp));
            }
        } catch (...) {
            // 指數超出範圍時進入通則路徑
        }
    }

    // 負底數之非整數冪非實數，回傳 NaN
    if (base.sign() < 0) {
        return decimal::nan;
    }

    // 通則路徑：x^y = exp(y * ln(x))
    decimal ln_base = log(base);
    decimal y_ln_x = exp * ln_base;
    return compat::exp(y_ln_x);
}

// ============================================================================
// 6. Decimal 三角函式
// ============================================================================

/// <summary>
/// 計算正弦函式 sin(x)（模 2*Pi 後高階泰勒展開）。
/// </summary>
/// <param name="x">弧度角</param>
/// <returns>sin(x) 結果</returns>
COMPAT_NODISCARD inline decimal sin(const decimal& x) {
    if (x.is_nan() || x.is_infinite()) {
        return decimal::nan;
    }
    if (x.is_zero()) {
        return decimal(0);
    }
    const decimal& two_pi = detail::cmath_two_pi();
    const decimal& pi_over_2 = detail::cmath_pi_over_2();

    // 模 2*Pi 範圍縮減
    decimal k = round(x / two_pi);
    decimal r = x - k * two_pi;

    // 象限縮減至 [-Pi/4, Pi/4]
    decimal quad_dec = round(r / pi_over_2);
    decimal t = r - quad_dec * pi_over_2;
    int quad = static_cast<int>(static_cast<int64_t>(quad_dec)) % 4;
    if (quad < 0) {
        quad += 4;
    }

    decimal t2 = (t * t).divide(decimal(1), 42);

    // 泰勒級數 sin(t)
    decimal sin_t = t;
    decimal term_s = t;
    for (int n = 3; n <= 25; n += 2) {
        term_s = -(term_s * t2).divide(decimal(n * (n - 1)), 42);
        sin_t += term_s;
        if (abs(term_s) < decimal("1e-40")) {
            break;
        }
    }

    // 泰勒級數 cos(t)
    decimal cos_t = decimal(1);
    decimal term_c = decimal(1);
    for (int n = 2; n <= 24; n += 2) {
        term_c = -(term_c * t2).divide(decimal(n * (n - 1)), 42);
        cos_t += term_c;
        if (abs(term_c) < decimal("1e-40")) {
            break;
        }
    }

    decimal res;
    switch (quad) {
        case 0: res = sin_t; break;
        case 1: res = cos_t; break;
        case 2: res = -sin_t; break;
        case 3: res = -cos_t; break;
        default: res = sin_t; break;
    }
    return res.divide(decimal(1), 34);
}

/// <summary>
/// 計算餘弦函式 cos(x)（模 2*Pi 後高階泰勒展開）。
/// </summary>
/// <param name="x">弧度角</param>
/// <returns>cos(x) 結果</returns>
COMPAT_NODISCARD inline decimal cos(const decimal& x) {
    if (x.is_nan() || x.is_infinite()) {
        return decimal::nan;
    }
    if (x.is_zero()) {
        return decimal(1);
    }
    const decimal& two_pi = detail::cmath_two_pi();
    const decimal& pi_over_2 = detail::cmath_pi_over_2();

    // 模 2*Pi 範圍縮減
    decimal k = round(x / two_pi);
    decimal r = x - k * two_pi;

    // 象限縮減至 [-Pi/4, Pi/4]
    decimal quad_dec = round(r / pi_over_2);
    decimal t = r - quad_dec * pi_over_2;
    int quad = static_cast<int>(static_cast<int64_t>(quad_dec)) % 4;
    if (quad < 0) {
        quad += 4;
    }

    decimal t2 = (t * t).divide(decimal(1), 42);

    // 泰勒級數 sin(t)
    decimal sin_t = t;
    decimal term_s = t;
    for (int n = 3; n <= 25; n += 2) {
        term_s = -(term_s * t2).divide(decimal(n * (n - 1)), 42);
        sin_t += term_s;
        if (abs(term_s) < decimal("1e-40")) {
            break;
        }
    }

    // 泰勒級數 cos(t)
    decimal cos_t = decimal(1);
    decimal term_c = decimal(1);
    for (int n = 2; n <= 24; n += 2) {
        term_c = -(term_c * t2).divide(decimal(n * (n - 1)), 42);
        cos_t += term_c;
        if (abs(term_c) < decimal("1e-40")) {
            break;
        }
    }

    decimal res;
    switch (quad) {
        case 0: res = cos_t; break;
        case 1: res = -sin_t; break;
        case 2: res = -cos_t; break;
        case 3: res = sin_t; break;
        default: res = cos_t; break;
    }
    return res.divide(decimal(1), 34);
}

/// <summary>
/// 計算正切函式 tan(x) = sin(x) / cos(x)。
/// </summary>
/// <param name="x">弧度角</param>
/// <returns>tan(x) 結果</returns>
COMPAT_NODISCARD inline decimal tan(const decimal& x) {
    if (x.is_nan() || x.is_infinite()) {
        return decimal::nan;
    }
    if (x.is_zero()) {
        return decimal(0);
    }
    decimal s = sin(x);
    decimal c = cos(x);
    if (c.is_zero()) {
        return (s.sign() >= 0) ? decimal::infinity : -decimal::infinity;
    }
    return s.divide(c, 34);
}

} // namespace compat

// ============================================================================
// std 命名空間重載函式
// ============================================================================
namespace std {

// --- BigInt 重載 ---

/// <summary>
/// std::abs 重載：compat::bigint 絕對值。
/// </summary>
/// <param name="x">輸入整數</param>
/// <returns>絕對值結果</returns>
inline compat::bigint abs(const compat::bigint& x) noexcept {
    return compat::abs(x);
}

/// <summary>
/// std::sqrt 重載：compat::bigint 整數平方根。
/// </summary>
/// <param name="x">輸入整數</param>
/// <returns>整數平方根</returns>
inline compat::bigint sqrt(const compat::bigint& x) {
    return compat::sqrt(x);
}

/// <summary>
/// std::isqrt 重載：compat::bigint 整數平方根。
/// </summary>
/// <param name="x">輸入整數</param>
/// <returns>整數平方根</returns>
inline compat::bigint isqrt(const compat::bigint& x) {
    return compat::isqrt(x);
}

/// <summary>
/// std::cbrt 重載：compat::bigint 整數立方根。
/// </summary>
/// <param name="x">輸入整數</param>
/// <returns>整數立方根</returns>
inline compat::bigint cbrt(const compat::bigint& x) noexcept {
    return compat::cbrt(x);
}

/// <summary>
/// std::icbrt 重載：compat::bigint 整數立方根。
/// </summary>
/// <param name="x">輸入整數</param>
/// <returns>整數立方根</returns>
inline compat::bigint icbrt(const compat::bigint& x) noexcept {
    return compat::icbrt(x);
}

/// <summary>
/// std::pow 重載：compat::bigint 非負整數冪。
/// </summary>
/// <param name="base">底數</param>
/// <param name="exp">無符號指數</param>
/// <returns>冪次結果</returns>
inline compat::bigint pow(const compat::bigint& base, unsigned int exp) {
    return compat::pow(base, exp);
}

/// <summary>
/// std::pow 重載：compat::bigint 整數冪。
/// </summary>
/// <param name="base">底數</param>
/// <param name="exp">指數</param>
/// <returns>冪次結果</returns>
inline compat::bigint pow(const compat::bigint& base, const compat::bigint& exp) {
    return compat::pow(base, exp);
}

// --- Decimal 重載 ---

/// <summary>
/// std::isnan 重載：判斷 compat::decimal 是否為 NaN。
/// </summary>
/// <param name="x">輸入數值</param>
/// <returns>若為 NaN 回傳 true</returns>
inline bool isnan(const compat::decimal& x) noexcept {
    return compat::isnan(x);
}

/// <summary>
/// std::isinf 重載：判斷 compat::decimal 是否為無窮大。
/// </summary>
/// <param name="x">輸入數值</param>
/// <returns>若為無窮大回傳 true</returns>
inline bool isinf(const compat::decimal& x) noexcept {
    return compat::isinf(x);
}

/// <summary>
/// std::isfinite 重載：判斷 compat::decimal 是否為有限值。
/// </summary>
/// <param name="x">輸入數值</param>
/// <returns>若為有限值回傳 true</returns>
inline bool isfinite(const compat::decimal& x) noexcept {
    return compat::isfinite(x);
}

/// <summary>
/// std::signbit 重載：判斷 compat::decimal 符號位元。
/// </summary>
/// <param name="x">輸入數值</param>
/// <returns>若為負回傳 true</returns>
inline bool signbit(const compat::decimal& x) noexcept {
    return compat::signbit(x);
}

/// <summary>
/// std::copysign 重載：compat::decimal 符號複製。
/// </summary>
/// <param name="mag">大小</param>
/// <param name="sgn">符號</param>
/// <returns>調整後之數值</returns>
inline compat::decimal copysign(const compat::decimal& mag, const compat::decimal& sgn) noexcept {
    return compat::copysign(mag, sgn);
}

/// <summary>
/// std::abs 重載：compat::decimal 絕對值。
/// </summary>
/// <param name="x">輸入數值</param>
/// <returns>絕對值結果</returns>
inline compat::decimal abs(const compat::decimal& x) noexcept {
    return compat::abs(x);
}

/// <summary>
/// std::floor 重載：compat::decimal 向下取整。
/// </summary>
/// <param name="x">輸入數值</param>
/// <returns>向下取整結果</returns>
inline compat::decimal floor(const compat::decimal& x) {
    return compat::floor(x);
}

/// <summary>
/// std::ceil 重載：compat::decimal 向上取整。
/// </summary>
/// <param name="x">輸入數值</param>
/// <returns>向上取整結果</returns>
inline compat::decimal ceil(const compat::decimal& x) {
    return compat::ceil(x);
}

/// <summary>
/// std::trunc 重載：compat::decimal 朝零捨入。
/// </summary>
/// <param name="x">輸入數值</param>
/// <returns>朝零捨入結果</returns>
inline compat::decimal trunc(const compat::decimal& x) {
    return compat::trunc(x);
}

/// <summary>
/// std::round 重載：compat::decimal 四捨五入（Half away from zero）。
/// </summary>
/// <param name="x">輸入數值</param>
/// <returns>四捨五入結果</returns>
inline compat::decimal round(const compat::decimal& x) {
    return compat::round(x);
}

/// <summary>
/// std::fmod 重載：compat::decimal 取模。
/// </summary>
/// <param name="x">被除數</param>
/// <param name="y">除數</param>
/// <returns>取模餘數</returns>
inline compat::decimal fmod(const compat::decimal& x, const compat::decimal& y) {
    return compat::fmod(x, y);
}

/// <summary>
/// std::remainder 重載：compat::decimal IEEE 餘數。
/// </summary>
/// <param name="x">被除數</param>
/// <param name="y">除數</param>
/// <returns>IEEE 餘數</returns>
inline compat::decimal remainder(const compat::decimal& x, const compat::decimal& y) {
    return compat::remainder(x, y);
}

/// <summary>
/// std::sqrt 重載：compat::decimal 平方根。
/// </summary>
/// <param name="x">輸入數值</param>
/// <returns>平方根結果</returns>
inline compat::decimal sqrt(const compat::decimal& x) {
    return compat::sqrt(x);
}

/// <summary>
/// std::cbrt 重載：compat::decimal 立方根。
/// </summary>
/// <param name="x">輸入數值</param>
/// <returns>立方根結果</returns>
inline compat::decimal cbrt(const compat::decimal& x) {
    return compat::cbrt(x);
}

/// <summary>
/// std::hypot 重載：compat::decimal 斜邊長。
/// </summary>
/// <param name="x">股 1</param>
/// <param name="y">股 2</param>
/// <returns>斜邊長</returns>
inline compat::decimal hypot(const compat::decimal& x, const compat::decimal& y) {
    return compat::hypot(x, y);
}

/// <summary>
/// std::exp 重載：compat::decimal 自然指數。
/// </summary>
/// <param name="x">輸入數值</param>
/// <returns>指數結果</returns>
inline compat::decimal exp(const compat::decimal& x) {
    return compat::exp(x);
}

/// <summary>
/// std::log 重載：compat::decimal 自然對數。
/// </summary>
/// <param name="x">輸入數值</param>
/// <returns>對數結果</returns>
inline compat::decimal log(const compat::decimal& x) {
    return compat::log(x);
}

/// <summary>
/// std::log10 重載：compat::decimal 常用對數。
/// </summary>
/// <param name="x">輸入數值</param>
/// <returns>常用對數結果</returns>
inline compat::decimal log10(const compat::decimal& x) {
    return compat::log10(x);
}

/// <summary>
/// std::log2 重載：compat::decimal 以 2 為底對數。
/// </summary>
/// <param name="x">輸入數值</param>
/// <returns>對數結果</returns>
inline compat::decimal log2(const compat::decimal& x) {
    return compat::log2(x);
}

/// <summary>
/// std::pow 重載：compat::decimal 整數冪。
/// </summary>
/// <param name="base">底數</param>
/// <param name="exp">整數指數</param>
/// <returns>冪次結果</returns>
inline compat::decimal pow(const compat::decimal& base, int exp) {
    return compat::pow(base, exp);
}

/// <summary>
/// std::pow 重載：compat::decimal 實數冪。
/// </summary>
/// <param name="base">底數</param>
/// <param name="exp">指數</param>
/// <returns>冪次結果</returns>
inline compat::decimal pow(const compat::decimal& base, const compat::decimal& exp) {
    return compat::pow(base, exp);
}

/// <summary>
/// std::sin 重載：compat::decimal 正弦。
/// </summary>
/// <param name="x">弧度角</param>
/// <returns>正弦結果</returns>
inline compat::decimal sin(const compat::decimal& x) {
    return compat::sin(x);
}

/// <summary>
/// std::cos 重載：compat::decimal 餘弦。
/// </summary>
/// <param name="x">弧度角</param>
/// <returns>餘弦結果</returns>
inline compat::decimal cos(const compat::decimal& x) {
    return compat::cos(x);
}

/// <summary>
/// std::tan 重載：compat::decimal 正切。
/// </summary>
/// <param name="x">弧度角</param>
/// <returns>正切結果</returns>
inline compat::decimal tan(const compat::decimal& x) {
    return compat::tan(x);
}

} // namespace std
