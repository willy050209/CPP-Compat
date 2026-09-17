#pragma once

// Decimal.hpp
// High-precision decimal floating-point facade class for CPP-Compat.
// Aligned with IEEE 754-2008 decimal128 standard (34 significant decimal digits).
// Zero external dependencies, downward compatible from C++23 to C++11.

#include "Config.hpp"
#include "StringView.hpp"
#include "Format.hpp"
#include "BigInt.hpp"
#include "detail/DecimalCore.hpp"
#include <cstdint>
#include <string>
#include <iostream>
#include <functional>
#include <type_traits>
#include <stdexcept>
#include <cmath>
#include <limits>
#include <cstdlib>

#if defined(__cpp_impl_three_way_comparison) && (__cpp_impl_three_way_comparison >= 201907L)
#include <compare>
#endif

namespace compat {

class decimal;

namespace detail {

/// <summary>
/// 常數代理種類列舉。
/// </summary>
enum class DecimalConstantKind {
    Zero,
    One,
    Infinity,
    NaN
};

/// <summary>
/// 十進位常數代理結構，支援常數屬性與函式呼叫兩種存取語法（如 decimal::zero 與 decimal::zero()）。
/// </summary>
struct DecimalConstantProxy {
    DecimalConstantKind kind;

    /// <summary>
    /// 建構常數代理物件。
    /// </summary>
    /// <param name="k">常數種類</param>
    constexpr explicit DecimalConstantProxy(DecimalConstantKind k) noexcept : kind(k) {}

    /// <summary>
    /// 隱式轉換至 decimal 實例。
    /// </summary>
    /// <returns>對應之 decimal 物件</returns>
    COMPAT_CONSTEXPR_20 operator decimal() const;

    /// <summary>
    /// 函式呼叫運算子，回傳對應之 decimal 實例。
    /// </summary>
    /// <returns>對應之 decimal 物件</returns>
    COMPAT_CONSTEXPR_20 decimal operator()() const;

    /// <summary>
    /// 一元負號運算子（支援 -decimal::infinity）。
    /// </summary>
    /// <returns>反轉正負號後之 decimal 物件</returns>
    COMPAT_CONSTEXPR_20 decimal operator-() const;

    /// <summary>
    /// 常數代理相等比較運算子。
    /// </summary>
    /// <typeparam name="T">比較目標型別</typeparam>
    /// <param name="p">常數代理物件</param>
    /// <param name="other">比較目標</param>
    /// <returns>若相等回傳 true</returns>
    template <typename T>
    friend COMPAT_CONSTEXPR_20 bool operator==(DecimalConstantProxy p, const T& other);

    /// <summary>
    /// 常數代理相等比較運算子。
    /// </summary>
    /// <typeparam name="T">比較目標型別</typeparam>
    /// <param name="other">比較目標</param>
    /// <param name="p">常數代理物件</param>
    /// <returns>若相等回傳 true</returns>
    template <typename T>
    friend COMPAT_CONSTEXPR_20 bool operator==(const T& other, DecimalConstantProxy p);

    /// <summary>
    /// 常數代理不相等比較運算子。
    /// </summary>
    /// <typeparam name="T">比較目標型別</typeparam>
    /// <param name="p">常數代理物件</param>
    /// <param name="other">比較目標</param>
    /// <returns>若不相等回傳 true</returns>
    template <typename T>
    friend COMPAT_CONSTEXPR_20 bool operator!=(DecimalConstantProxy p, const T& other);

    /// <summary>
    /// 常數代理不相等比較運算子。
    /// </summary>
    /// <typeparam name="T">比較目標型別</typeparam>
    /// <param name="other">比較目標</param>
    /// <param name="p">常數代理物件</param>
    /// <returns>若不相等回傳 true</returns>
    template <typename T>
    friend COMPAT_CONSTEXPR_20 bool operator!=(const T& other, DecimalConstantProxy p);
};

/// <summary>
/// 十進位常數模板基底結構，確保在 C++11/C++14 header-only 環境下具備弱符號鏈結與外部定義。
/// </summary>
template <typename T = void>
struct DecimalConstants {
    static constexpr DecimalConstantProxy zero{DecimalConstantKind::Zero};
    static constexpr DecimalConstantProxy one{DecimalConstantKind::One};
    static constexpr DecimalConstantProxy infinity{DecimalConstantKind::Infinity};
    static constexpr DecimalConstantProxy nan{DecimalConstantKind::NaN};
    static constexpr DecimalConstantProxy NaN{DecimalConstantKind::NaN};
};

#if (COMPAT_CPLUSPLUS < COMPAT_CXX_17)
template <typename T>
constexpr DecimalConstantProxy DecimalConstants<T>::zero;
template <typename T>
constexpr DecimalConstantProxy DecimalConstants<T>::one;
template <typename T>
constexpr DecimalConstantProxy DecimalConstants<T>::infinity;
template <typename T>
constexpr DecimalConstantProxy DecimalConstants<T>::nan;
template <typename T>
constexpr DecimalConstantProxy DecimalConstants<T>::NaN;
#endif

} // namespace detail

/// <summary>
/// 高精度任意精度十進位浮點數門面類別，基於 bigint 實現 128-bit SBO 特性。
/// 支援 IEEE 754-2008 decimal128 標準（預設 34 位有效十進位數字）與銀行家捨入法 (Half-Even)。
/// </summary>
class decimal : public detail::DecimalConstants<> {
private:
    bigint m_unscaled{0};
    int64_t m_scale{0};
    bool m_is_infinity{false};
    bool m_is_nan{false};

    friend struct detail::DecimalConstantProxy;

public:


    /// <summary>
    /// 預設建構子：初始化為數值 0。
    /// </summary>
    COMPAT_CONSTEXPR_20 decimal() noexcept = default;

    /// <summary>
    /// 複製建構子。
    /// </summary>
    /// <param name="other">來源 decimal</param>
    COMPAT_CONSTEXPR_20 decimal(const decimal& other) = default;

    /// <summary>
    /// 移動建構子。
    /// </summary>
    /// <param name="other">來源 decimal（右值）</param>
    COMPAT_CONSTEXPR_20 decimal(decimal&& other) noexcept = default;

    /// <summary>
    /// 複製賦值運算子。
    /// </summary>
    /// <param name="other">來源 decimal</param>
    /// <returns>自身參考</returns>
    COMPAT_CONSTEXPR_20 decimal& operator=(const decimal& other) = default;

    /// <summary>
    /// 移動賦值運算子。
    /// </summary>
    /// <param name="other">來源 decimal（右值）</param>
    /// <returns>自身參考</returns>
    COMPAT_CONSTEXPR_20 decimal& operator=(decimal&& other) noexcept = default;

    /// <summary>
    /// 解構子。
    /// </summary>
    COMPAT_CONSTEXPR_20 ~decimal() = default;

    /// <summary>
    /// 自未縮放整數與縮放位數建構十進位數。
    /// </summary>
    /// <param name="unscaled">未縮放整數</param>
    /// <param name="scale">小數縮放位數（數值為 unscaled * 10^-scale）</param>
    COMPAT_CONSTEXPR_20 decimal(bigint unscaled, int64_t scale)
        : m_unscaled(std::move(unscaled)), m_scale(scale), m_is_infinity(false), m_is_nan(false) {
        detail::DecimalCore::normalize(m_unscaled, m_scale, false, false);
    }

    /// <summary>
    /// 自 compat::bigint 隱式建構（scale 為 0）。
    /// </summary>
    /// <param name="val">來源 bigint 整數</param>
    COMPAT_CONSTEXPR_20 decimal(const bigint& val)
        : m_unscaled(val), m_scale(0), m_is_infinity(false), m_is_nan(false) {}

    /// <summary>
    /// 自 compat::bigint 右值隱式建構（scale 為 0）。
    /// </summary>
    /// <param name="val">來源 bigint 右值</param>
    COMPAT_CONSTEXPR_20 decimal(bigint&& val) noexcept
        : m_unscaled(std::move(val)), m_scale(0), m_is_infinity(false), m_is_nan(false) {}

    /// <summary>
    /// 自布林值建構：true 為 1，false 為 0。
    /// </summary>
    /// <param name="b">布林值</param>
    COMPAT_CONSTEXPR_20 decimal(bool b) noexcept
        : m_unscaled(b ? 1 : 0), m_scale(0), m_is_infinity(false), m_is_nan(false) {}

    /// <summary>
    /// 自 8 位元有符號整數隱式建構。
    /// </summary>
    /// <param name="v">8 位元整數</param>
    COMPAT_CONSTEXPR_20 decimal(int8_t v) noexcept
        : m_unscaled(v), m_scale(0), m_is_infinity(false), m_is_nan(false) {}

    /// <summary>
    /// 自 16 位元有符號整數隱式建構。
    /// </summary>
    /// <param name="v">16 位元整數</param>
    COMPAT_CONSTEXPR_20 decimal(int16_t v) noexcept
        : m_unscaled(v), m_scale(0), m_is_infinity(false), m_is_nan(false) {}

    /// <summary>
    /// 自 32 位元有符號整數隱式建構。
    /// </summary>
    /// <param name="v">32 位元整數</param>
    COMPAT_CONSTEXPR_20 decimal(int32_t v) noexcept
        : m_unscaled(v), m_scale(0), m_is_infinity(false), m_is_nan(false) {}

    /// <summary>
    /// 自 64 位元有符號整數隱式建構。
    /// </summary>
    /// <param name="v">64 位元整數</param>
    COMPAT_CONSTEXPR_20 decimal(int64_t v) noexcept
        : m_unscaled(v), m_scale(0), m_is_infinity(false), m_is_nan(false) {}

    /// <summary>
    /// 自 8 位元無符號整數隱式建構。
    /// </summary>
    /// <param name="v">8 位元無符號整數</param>
    COMPAT_CONSTEXPR_20 decimal(uint8_t v) noexcept
        : m_unscaled(v), m_scale(0), m_is_infinity(false), m_is_nan(false) {}

    /// <summary>
    /// 自 16 位元無符號整數隱式建構。
    /// </summary>
    /// <param name="v">16 位元無符號整數</param>
    COMPAT_CONSTEXPR_20 decimal(uint16_t v) noexcept
        : m_unscaled(v), m_scale(0), m_is_infinity(false), m_is_nan(false) {}

    /// <summary>
    /// 自 32 位元無符號整數隱式建構。
    /// </summary>
    /// <param name="v">32 位元無符號整數</param>
    COMPAT_CONSTEXPR_20 decimal(uint32_t v) noexcept
        : m_unscaled(v), m_scale(0), m_is_infinity(false), m_is_nan(false) {}

    /// <summary>
    /// 自 64 位元無符號整數隱式建構。
    /// </summary>
    /// <param name="v">64 位元無符號整數</param>
    COMPAT_CONSTEXPR_20 decimal(uint64_t v) noexcept
        : m_unscaled(v), m_scale(0), m_is_infinity(false), m_is_nan(false) {}

    /// <summary>
    /// 自其他原生整數型別建構之泛型模板。
    /// </summary>
    /// <typeparam name="T">整數型別</typeparam>
    /// <param name="v">整數值</param>
    template <typename T, typename std::enable_if<
        std::is_integral<T>::value &&
        !std::is_same<T, bool>::value &&
        !std::is_same<T, int8_t>::value &&
        !std::is_same<T, int16_t>::value &&
        !std::is_same<T, int32_t>::value &&
        !std::is_same<T, int64_t>::value &&
        !std::is_same<T, uint8_t>::value &&
        !std::is_same<T, uint16_t>::value &&
        !std::is_same<T, uint32_t>::value &&
        !std::is_same<T, uint64_t>::value, int>::type = 0>
    COMPAT_CONSTEXPR_20 decimal(T v) noexcept
        : m_unscaled(v), m_scale(0), m_is_infinity(false), m_is_nan(false) {}

    /// <summary>
    /// 自單精度浮點數隱式建構。
    /// </summary>
    /// <param name="v">float 數值</param>
    decimal(float v) {
        detail::DecimalCore::from_float(m_unscaled, m_scale, m_is_infinity, m_is_nan, v);
    }

    /// <summary>
    /// 自倍精度浮點數隱式建構。
    /// </summary>
    /// <param name="v">double 數值</param>
    decimal(double v) {
        detail::DecimalCore::from_float(m_unscaled, m_scale, m_is_infinity, m_is_nan, v);
    }

    /// <summary>
    /// 自延伸倍精度浮點數隱式建構。
    /// </summary>
    /// <param name="v">long double 數值</param>
    decimal(long double v) {
        detail::DecimalCore::from_float(m_unscaled, m_scale, m_is_infinity, m_is_nan, v);
    }

    /// <summary>
    /// 自字串視圖明確建構 decimal。
    /// </summary>
    /// <param name="sv">十進位字串視圖</param>
    /// <exception cref="std::invalid_argument">字串無效時拋出</exception>
    explicit COMPAT_CONSTEXPR_20 decimal(compat::string_view sv) {
        detail::DecimalCore::from_string(m_unscaled, m_scale, m_is_infinity, m_is_nan, sv);
    }

    /// <summary>
    /// 自 C-style 字串明確建構 decimal。
    /// </summary>
    /// <param name="s">字串指標</param>
    /// <exception cref="std::invalid_argument">指標為 null 或格式不合法時拋出</exception>
    explicit COMPAT_CONSTEXPR_20 decimal(const char* s) {
        if (s == nullptr) {
            COMPAT_THROW_OR_ABORT(std::invalid_argument("null string pointer"));
        }
        detail::DecimalCore::from_string(m_unscaled, m_scale, m_is_infinity, m_is_nan, compat::string_view(s));
    }

    /// <summary>
    /// 自 std::string 明確建構 decimal。
    /// </summary>
    /// <param name="s">字串物件</param>
    /// <exception cref="std::invalid_argument">字串格式不合法時拋出</exception>
    explicit decimal(const std::string& s) {
        detail::DecimalCore::from_string(m_unscaled, m_scale, m_is_infinity, m_is_nan, compat::string_view(s.data(), s.size()));
    }

    /// <summary>
    /// 靜態輔助工廠方法：自字串解析 decimal。
    /// </summary>
    /// <param name="sv">十進位字串視圖</param>
    /// <returns>解析完成之 decimal 物件</returns>
    /// <exception cref="std::invalid_argument">字串為空或包含無效字元時拋出</exception>
    COMPAT_CONSTEXPR_20 static decimal from_string(compat::string_view sv) {
        decimal res;
        detail::DecimalCore::from_string(res.m_unscaled, res.m_scale, res.m_is_infinity, res.m_is_nan, sv);
        return res;
    }

    /// <summary>
    /// 查詢是否正在使用 128-bit SBO 緩衝區（未配置堆積記憶體）。
    /// </summary>
    /// <returns>若符合 SBO 回傳 true，否則回傳 false</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_20 bool is_sbo() const noexcept {
        return m_unscaled.is_sbo();
    }

    /// <summary>
    /// 查詢是否為小數值（SBO 模式之別名）。
    /// </summary>
    /// <returns>若為 SBO 儲存回傳 true</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_20 bool is_small() const noexcept {
        return m_unscaled.is_sbo();
    }

    /// <summary>
    /// 查詢是否為 NaN。
    /// </summary>
    /// <returns>若為 NaN 回傳 true</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_20 bool is_nan() const noexcept {
        return m_is_nan;
    }

    /// <summary>
    /// 查詢是否為無窮大（包含正負無窮大）。
    /// </summary>
    /// <returns>若為無窮大回傳 true</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_20 bool is_infinite() const noexcept {
        return m_is_infinity;
    }

    /// <summary>
    /// 查詢是否為無窮大（別名）。
    /// </summary>
    /// <returns>若為無窮大回傳 true</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_20 bool is_infinity() const noexcept {
        return m_is_infinity;
    }

    /// <summary>
    /// 查詢是否為有限數值。
    /// </summary>
    /// <returns>非 NaN 且非無窮大時回傳 true</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_20 bool is_finite() const noexcept {
        return !m_is_nan && !m_is_infinity;
    }

    /// <summary>
    /// 查詢數值是否為 0。
    /// </summary>
    /// <returns>若為零回傳 true</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_20 bool is_zero() const noexcept {
        return !m_is_nan && !m_is_infinity && m_unscaled == 0;
    }

    /// <summary>
    /// 取得數值符號。
    /// </summary>
    /// <returns>正數或正無窮回傳 1，負數或負無窮回傳 -1，零或 NaN 回傳 0</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_20 int8_t sign() const noexcept {
        if (m_is_nan) {
            return 0;
        }
        if (m_is_infinity) {
            return (m_unscaled < 0) ? -1 : 1;
        }
        return m_unscaled.sign();
    }

    /// <summary>
    /// 取得未縮放整數之 const 參考。
    /// </summary>
    /// <returns>bigint 參考</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_20 const bigint& unscaled() const noexcept {
        return m_unscaled;
    }

    /// <summary>
    /// 取得小數縮放位數。
    /// </summary>
    /// <returns>scale 數值</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_20 int64_t scale() const noexcept {
        return m_scale;
    }

    /// <summary>
    /// 明確轉型為布林值（非 0 且非 NaN 為 true，0 或 NaN 為 false）。
    /// </summary>
    /// <returns>布林值</returns>
    explicit COMPAT_CONSTEXPR_20 operator bool() const noexcept {
        if (m_is_nan) {
            return false;
        }
        if (m_is_infinity) {
            return true;
        }
        return m_unscaled != 0;
    }

    /// <summary>
    /// 邏輯非運算子。
    /// </summary>
    /// <returns>若數值為 0 或 NaN 回傳 true，否則回傳 false</returns>
    COMPAT_CONSTEXPR_20 bool operator!() const noexcept {
        return !static_cast<bool>(*this);
    }

    /// <summary>
    /// 明確轉型為 compat::bigint（截斷小數部分）。
    /// </summary>
    /// <returns>bigint 整數</returns>
    /// <exception cref="std::domain_error">數值為 NaN 或無窮大時拋出</exception>
    explicit COMPAT_CONSTEXPR_20 operator bigint() const {
        if (m_is_nan || m_is_infinity) {
            COMPAT_THROW_OR_ABORT(std::domain_error("cannot convert special value to bigint"));
        }
        if (m_scale <= 0) {
            if (m_scale == 0) {
                return m_unscaled;
            }
            return m_unscaled * detail::DecimalCore::power_of_10(static_cast<size_t>(-m_scale));
        }
        return m_unscaled / detail::DecimalCore::power_of_10(static_cast<size_t>(m_scale));
    }

    /// <summary>
    /// 明確轉型為 64 位元有符號整數（可能截斷）。
    /// </summary>
    /// <returns>int64_t 數值</returns>
    explicit COMPAT_CONSTEXPR_20 operator int64_t() const {
        return static_cast<int64_t>(static_cast<bigint>(*this));
    }

    /// <summary>
    /// 明確轉型為 64 位元無符號整數（可能截斷）。
    /// </summary>
    /// <returns>uint64_t 數值</returns>
    explicit COMPAT_CONSTEXPR_20 operator uint64_t() const {
        return static_cast<uint64_t>(static_cast<bigint>(*this));
    }

    /// <summary>
    /// 明確轉型為 32 位元有符號整數（可能截斷）。
    /// </summary>
    /// <returns>int32_t 數值</returns>
    explicit COMPAT_CONSTEXPR_20 operator int32_t() const {
        return static_cast<int32_t>(static_cast<bigint>(*this));
    }

    /// <summary>
    /// 明確轉型為 32 位元無符號整數（可能截斷）。
    /// </summary>
    /// <returns>uint32_t 數值</returns>
    explicit COMPAT_CONSTEXPR_20 operator uint32_t() const {
        return static_cast<uint32_t>(static_cast<bigint>(*this));
    }

    /// <summary>
    /// 明確轉型為 16 位元有符號整數（可能截斷）。
    /// </summary>
    /// <returns>int16_t 數值</returns>
    explicit COMPAT_CONSTEXPR_20 operator int16_t() const {
        return static_cast<int16_t>(static_cast<int32_t>(static_cast<bigint>(*this)));
    }

    /// <summary>
    /// 明確轉型為 16 位元無符號整數（可能截斷）。
    /// </summary>
    /// <returns>uint16_t 數值</returns>
    explicit COMPAT_CONSTEXPR_20 operator uint16_t() const {
        return static_cast<uint16_t>(static_cast<uint32_t>(static_cast<bigint>(*this)));
    }

    /// <summary>
    /// 明確轉型為 8 位元有符號整數（可能截斷）。
    /// </summary>
    /// <returns>int8_t 數值</returns>
    explicit COMPAT_CONSTEXPR_20 operator int8_t() const {
        return static_cast<int8_t>(static_cast<int32_t>(static_cast<bigint>(*this)));
    }

    /// <summary>
    /// 明確轉型為 8 位元無符號整數（可能截斷）。
    /// </summary>
    /// <returns>uint8_t 數值</returns>
    explicit COMPAT_CONSTEXPR_20 operator uint8_t() const {
        return static_cast<uint8_t>(static_cast<uint32_t>(static_cast<bigint>(*this)));
    }

    /// <summary>
    /// 明確轉型為 double 倍精度浮點數。
    /// </summary>
    /// <returns>近似 double 數值</returns>
    explicit operator double() const noexcept {
        if (m_is_nan) {
            return std::numeric_limits<double>::quiet_NaN();
        }
        if (m_is_infinity) {
            return (m_unscaled < 0) ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity();
        }
        std::string s = to_string();
        return std::strtod(s.c_str(), nullptr);
    }

    /// <summary>
    /// 明確轉型為 float 單精度浮點數。
    /// </summary>
    /// <returns>近似 float 數值</returns>
    explicit operator float() const noexcept {
        return static_cast<float>(static_cast<double>(*this));
    }

    /// <summary>
    /// 明確轉型為 long double 延伸倍精度浮點數。
    /// </summary>
    /// <returns>近似 long double 數值</returns>
    explicit operator long double() const noexcept {
        if (m_is_nan) {
            return std::numeric_limits<long double>::quiet_NaN();
        }
        if (m_is_infinity) {
            return (m_unscaled < 0) ? -std::numeric_limits<long double>::infinity() : std::numeric_limits<long double>::infinity();
        }
        std::string s = to_string();
        return std::strtold(s.c_str(), nullptr);
    }

    /// <summary>
    /// 轉換為標準十進位小數表示法字串。
    /// </summary>
    /// <returns>標準十進位字串</returns>
    COMPAT_NODISCARD std::string to_string() const {
        return detail::DecimalCore::to_string(m_unscaled, m_scale, m_is_infinity, m_is_nan);
    }

    /// <summary>
    /// 依指定精度執行除法運算。
    /// </summary>
    /// <param name="other">除數</param>
    /// <param name="precision">有效十進位數字精度（預設為 34 位）</param>
    /// <returns>除法結果</returns>
    /// <summary>
    /// 依指定精度執行除法運算。
    /// </summary>
    /// <param name="other">除數</param>
    /// <param name="precision">有效十進位數字精度（預設為 34 位）</param>
    /// <returns>除法結果</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_20 decimal divide(const decimal& other, int32_t precision = 34) const {
        decimal res;
        detail::DecimalCore::div(res.m_unscaled, res.m_scale, res.m_is_infinity, res.m_is_nan,
                                 m_unscaled, m_scale, m_is_infinity, m_is_nan,
                                 other.m_unscaled, other.m_scale, other.m_is_infinity, other.m_is_nan,
                                 precision);
        return res;
    }

    /// <summary>
    /// 依指定小數位數執行銀行家捨入 (Banker's Rounding / Half-Even)。
    /// </summary>
    /// <param name="decimal_places">目標小數位數（預設為 0，捨入至整數）</param>
    /// <returns>捨入後之 decimal 物件</returns>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_20 decimal round(int64_t decimal_places = 0) const {
        if (m_is_nan || m_is_infinity) {
            return *this;
        }
        decimal res = *this;
        detail::DecimalCore::round_half_even(res.m_unscaled, res.m_scale, decimal_places);
        return res;
    }

    /// <summary>
    /// 一元正號運算子。
    /// </summary>
    /// <returns>自身副本</returns>
    COMPAT_CONSTEXPR_20 decimal operator+() const {
        return *this;
    }

    /// <summary>
    /// 一元負號運算子。
    /// </summary>
    /// <returns>正負號反轉後之結果</returns>
    COMPAT_CONSTEXPR_20 decimal operator-() const {
        if (m_is_nan) {
            return *this;
        }
        decimal res = *this;
        res.m_unscaled = -res.m_unscaled;
        return res;
    }

    /// <summary>
    /// 前置遞增運算子：++d。
    /// </summary>
    /// <returns>遞增後之自身參考</returns>
    COMPAT_CONSTEXPR_20 decimal& operator++() {
        *this += 1;
        return *this;
    }

    /// <summary>
    /// 後置遞增運算子：d++。
    /// </summary>
    /// <returns>遞增前之舊值</returns>
    COMPAT_CONSTEXPR_20 decimal operator++(int) {
        decimal tmp = *this;
        *this += 1;
        return tmp;
    }

    /// <summary>
    /// 前置遞減運算子：--d。
    /// </summary>
    /// <returns>遞減後之自身參考</returns>
    COMPAT_CONSTEXPR_20 decimal& operator--() {
        *this -= 1;
        return *this;
    }

    /// <summary>
    /// 後置遞減運算子：d--。
    /// </summary>
    /// <returns>遞減前之舊值</returns>
    COMPAT_CONSTEXPR_20 decimal operator--(int) {
        decimal tmp = *this;
        *this -= 1;
        return tmp;
    }

    /// <summary>
    /// 加法複合賦值運算子。
    /// </summary>
    /// <param name="rhs">加數</param>
    /// <returns>自身參考</returns>
    COMPAT_CONSTEXPR_20 decimal& operator+=(const decimal& rhs) {
        decimal res;
        detail::DecimalCore::add(res.m_unscaled, res.m_scale, res.m_is_infinity, res.m_is_nan,
                                 m_unscaled, m_scale, m_is_infinity, m_is_nan,
                                 rhs.m_unscaled, rhs.m_scale, rhs.m_is_infinity, rhs.m_is_nan);
        *this = std::move(res);
        return *this;
    }

    /// <summary>
    /// 減法複合賦值運算子。
    /// </summary>
    /// <param name="rhs">減數</param>
    /// <returns>自身參考</returns>
    COMPAT_CONSTEXPR_20 decimal& operator-=(const decimal& rhs) {
        decimal res;
        detail::DecimalCore::sub(res.m_unscaled, res.m_scale, res.m_is_infinity, res.m_is_nan,
                                 m_unscaled, m_scale, m_is_infinity, m_is_nan,
                                 rhs.m_unscaled, rhs.m_scale, rhs.m_is_infinity, rhs.m_is_nan);
        *this = std::move(res);
        return *this;
    }

    /// <summary>
    /// 乘法複合賦值運算子。
    /// </summary>
    /// <param name="rhs">乘數</param>
    /// <returns>自身參考</returns>
    COMPAT_CONSTEXPR_20 decimal& operator*=(const decimal& rhs) {
        decimal res;
        detail::DecimalCore::mul(res.m_unscaled, res.m_scale, res.m_is_infinity, res.m_is_nan,
                                 m_unscaled, m_scale, m_is_infinity, m_is_nan,
                                 rhs.m_unscaled, rhs.m_scale, rhs.m_is_infinity, rhs.m_is_nan);
        *this = std::move(res);
        return *this;
    }

    /// <summary>
    /// 除法複合賦值運算子（採用預設 34 位精度與銀行家捨入法）。
    /// </summary>
    /// <param name="rhs">除數</param>
    /// <returns>自身參考</returns>
    COMPAT_CONSTEXPR_20 decimal& operator/=(const decimal& rhs) {
        *this = divide(rhs, 34);
        return *this;
    }

    /// <summary>
    /// 取模複合賦值運算子。
    /// </summary>
    /// <param name="rhs">除數</param>
    /// <returns>自身參考</returns>
    COMPAT_CONSTEXPR_20 decimal& operator%=(const decimal& rhs) {
        decimal res;
        detail::DecimalCore::mod(res.m_unscaled, res.m_scale, res.m_is_infinity, res.m_is_nan,
                                 m_unscaled, m_scale, m_is_infinity, m_is_nan,
                                 rhs.m_unscaled, rhs.m_scale, rhs.m_is_infinity, rhs.m_is_nan);
        *this = std::move(res);
        return *this;
    }

    /// <summary>
    /// 雙目加法運算子。
    /// </summary>
    /// <param name="lhs">左運算元</param>
    /// <param name="rhs">右運算元</param>
    /// <returns>加法結果</returns>
    friend COMPAT_CONSTEXPR_20 decimal operator+(decimal lhs, const decimal& rhs) {
        lhs += rhs;
        return lhs;
    }

    /// <summary>
    /// 雙目減法運算子。
    /// </summary>
    /// <param name="lhs">被減數</param>
    /// <param name="rhs">減數</param>
    /// <returns>減法結果</returns>
    friend COMPAT_CONSTEXPR_20 decimal operator-(decimal lhs, const decimal& rhs) {
        lhs -= rhs;
        return lhs;
    }

    /// <summary>
    /// 雙目乘法運算子。
    /// </summary>
    /// <param name="lhs">乘數</param>
    /// <param name="rhs">乘數</param>
    /// <returns>乘法結果</returns>
    friend COMPAT_CONSTEXPR_20 decimal operator*(decimal lhs, const decimal& rhs) {
        lhs *= rhs;
        return lhs;
    }

    /// <summary>
    /// 雙目除法運算子（採用預設 34 位精度與銀行家捨入法）。
    /// </summary>
    /// <param name="lhs">被除數</param>
    /// <param name="rhs">除數</param>
    /// <returns>商</returns>
    friend COMPAT_CONSTEXPR_20 decimal operator/(const decimal& lhs, const decimal& rhs) {
        return lhs.divide(rhs, 34);
    }

    /// <summary>
    /// 雙目取模運算子。
    /// </summary>
    /// <param name="lhs">被除數</param>
    /// <param name="rhs">除數</param>
    /// <returns>餘數</returns>
    friend COMPAT_CONSTEXPR_20 decimal operator%(decimal lhs, const decimal& rhs) {
        lhs %= rhs;
        return lhs;
    }

    /// <summary>
    /// 相等比較運算子（NaN 比較均為 false）。
    /// </summary>
    /// <param name="lhs">左運算元</param>
    /// <param name="rhs">右運算元</param>
    /// <returns>若相等回傳 true</returns>
    friend COMPAT_CONSTEXPR_20 bool operator==(const decimal& lhs, const decimal& rhs) noexcept {
        return detail::DecimalCore::compare(lhs.m_unscaled, lhs.m_scale, lhs.m_is_infinity, lhs.m_is_nan,
                                            rhs.m_unscaled, rhs.m_scale, rhs.m_is_infinity, rhs.m_is_nan) == 0;
    }

    /// <summary>
    /// 不相等比較運算子（NaN 比較均為 true）。
    /// </summary>
    /// <param name="lhs">左運算元</param>
    /// <param name="rhs">右運算元</param>
    /// <returns>若不相等回傳 true</returns>
    friend COMPAT_CONSTEXPR_20 bool operator!=(const decimal& lhs, const decimal& rhs) noexcept {
        if (lhs.m_is_nan || rhs.m_is_nan) {
            return true;
        }
        return !(lhs == rhs);
    }

    /// <summary>
    /// 小於比較運算子。
    /// </summary>
    /// <param name="lhs">左運算元</param>
    /// <param name="rhs">右運算元</param>
    /// <returns>若 lhs &lt; rhs 回傳 true</returns>
    friend COMPAT_CONSTEXPR_20 bool operator<(const decimal& lhs, const decimal& rhs) noexcept {
        return detail::DecimalCore::compare(lhs.m_unscaled, lhs.m_scale, lhs.m_is_infinity, lhs.m_is_nan,
                                            rhs.m_unscaled, rhs.m_scale, rhs.m_is_infinity, rhs.m_is_nan) == -1;
    }

    /// <summary>
    /// 小於等於比較運算子。
    /// </summary>
    /// <param name="lhs">左運算元</param>
    /// <param name="rhs">右運算元</param>
    /// <returns>若 lhs &lt;= rhs 回傳 true</returns>
    friend COMPAT_CONSTEXPR_20 bool operator<=(const decimal& lhs, const decimal& rhs) noexcept {
        if (lhs.m_is_nan || rhs.m_is_nan) {
            return false;
        }
        int cmp = detail::DecimalCore::compare(lhs.m_unscaled, lhs.m_scale, lhs.m_is_infinity, lhs.m_is_nan,
                                              rhs.m_unscaled, rhs.m_scale, rhs.m_is_infinity, rhs.m_is_nan);
        return cmp == -1 || cmp == 0;
    }

    /// <summary>
    /// 大於比較運算子。
    /// </summary>
    /// <param name="lhs">左運算元</param>
    /// <param name="rhs">右運算元</param>
    /// <returns>若 lhs &gt; rhs 回傳 true</returns>
    friend COMPAT_CONSTEXPR_20 bool operator>(const decimal& lhs, const decimal& rhs) noexcept {
        return rhs < lhs;
    }

    /// <summary>
    /// 大於等於比較運算子。
    /// </summary>
    /// <param name="lhs">左運算元</param>
    /// <param name="rhs">右運算元</param>
    /// <returns>若 lhs &gt;= rhs 回傳 true</returns>
    friend COMPAT_CONSTEXPR_20 bool operator>=(const decimal& lhs, const decimal& rhs) noexcept {
        return rhs <= lhs;
    }

#if defined(__cpp_impl_three_way_comparison) && (__cpp_impl_three_way_comparison >= 201907L)
    /// <summary>
    /// C++20 三向比較運算子（Spaceship Operator）。
    /// </summary>
    /// <param name="lhs">左運算元</param>
    /// <param name="rhs">右運算元</param>
    /// <returns>偏序比較結果 std::partial_ordering</returns>
    friend COMPAT_CONSTEXPR_20 std::partial_ordering operator<=>(const decimal& lhs, const decimal& rhs) noexcept {
        if (lhs.m_is_nan || rhs.m_is_nan) {
            return std::partial_ordering::unordered;
        }
        int cmp = detail::DecimalCore::compare(lhs.m_unscaled, lhs.m_scale, lhs.m_is_infinity, lhs.m_is_nan,
                                              rhs.m_unscaled, rhs.m_scale, rhs.m_is_infinity, rhs.m_is_nan);
        if (cmp < 0) return std::partial_ordering::less;
        if (cmp > 0) return std::partial_ordering::greater;
        return std::partial_ordering::equivalent;
    }
#endif

    /// <summary>
    /// 雙目邏輯及運算子。
    /// </summary>
    /// <param name="lhs">左運算元</param>
    /// <param name="rhs">右運算元</param>
    /// <returns>邏輯及結果</returns>
    friend COMPAT_CONSTEXPR_20 bool operator&&(const decimal& lhs, const decimal& rhs) noexcept {
        return static_cast<bool>(lhs) && static_cast<bool>(rhs);
    }

    /// <summary>
    /// 雙目邏輯或運算子。
    /// </summary>
    /// <param name="lhs">左運算元</param>
    /// <param name="rhs">右運算元</param>
    /// <returns>邏輯或結果</returns>
    friend COMPAT_CONSTEXPR_20 bool operator||(const decimal& lhs, const decimal& rhs) noexcept {
        return static_cast<bool>(lhs) || static_cast<bool>(rhs);
    }

    /// <summary>
    /// 輸出串流運算子。
    /// </summary>
    /// <param name="os">目標輸出串流</param>
    /// <param name="val">待輸出之 decimal</param>
    /// <returns>串流參考</returns>
    friend std::ostream& operator<<(std::ostream& os, const decimal& val) {
        os << val.to_string();
        return os;
    }
};

namespace detail {

/// <summary>
/// 依指定精度將 decimal 格式化為字串（支援指定小數位數與補零）。
/// </summary>
/// <param name="d">待格式化之 decimal</param>
/// <param name="precision">小數位數（負數代表預設輸出）</param>
/// <returns>格式化後之字串</returns>
inline std::string FormatDecimalToString(const decimal& d, int precision) {
    if (d.is_nan()) {
        return "nan";
    }
    if (d.is_infinite()) {
        return (d.sign() < 0) ? "-inf" : "inf";
    }
    if (precision < 0) {
        return d.to_string();
    }
    decimal rounded = d.round(precision);
    bigint u = rounded.unscaled();
    int64_t s = rounded.scale();
    bool negative = (u < 0);
    if (negative) {
        u = -u;
    }
    std::string u_str = u.to_string();
    std::string int_part;
    std::string frac_part;
    if (s <= 0) {
        int_part = u_str;
        if (s < 0) {
            int_part.append(static_cast<size_t>(-s), '0');
        }
        frac_part = "";
    } else {
        size_t s_len = u_str.size();
        if (s_len > static_cast<size_t>(s)) {
            size_t int_len = s_len - static_cast<size_t>(s);
            int_part = u_str.substr(0, int_len);
            frac_part = u_str.substr(int_len);
        } else {
            int_part = "0";
            size_t leading_zeros = static_cast<size_t>(s) - s_len;
            frac_part = std::string(leading_zeros, '0') + u_str;
        }
    }
    if (precision > 0) {
        if (frac_part.size() < static_cast<size_t>(precision)) {
            frac_part.append(static_cast<size_t>(precision) - frac_part.size(), '0');
        } else if (frac_part.size() > static_cast<size_t>(precision)) {
            frac_part.resize(static_cast<size_t>(precision));
        }
    }
    std::string res;
    if (negative) {
        res.push_back('-');
    }
    res += int_part;
    if (precision > 0) {
        res.push_back('.');
        res += frac_part;
    }
    return res;
}

    COMPAT_CONSTEXPR_20 inline DecimalConstantProxy::operator decimal() const {
        switch (kind) {
            case DecimalConstantKind::Zero:
                return decimal(0);
            case DecimalConstantKind::One:
                return decimal(1);
            case DecimalConstantKind::Infinity: {
                decimal d;
                d.m_is_infinity = true;
                d.m_unscaled = 1;
                return d;
            }
            case DecimalConstantKind::NaN: {
                decimal d;
                d.m_is_nan = true;
                return d;
            }
            default:
                return decimal(0);
        }
    }

    COMPAT_CONSTEXPR_20 inline decimal DecimalConstantProxy::operator()() const {
        return static_cast<decimal>(*this);
    }

    COMPAT_CONSTEXPR_20 inline decimal DecimalConstantProxy::operator-() const {
        return -static_cast<decimal>(*this);
    }

    template <typename T>
    COMPAT_CONSTEXPR_20 inline bool operator==(DecimalConstantProxy p, const T& other) {
        return static_cast<decimal>(p) == other;
    }

    template <typename T>
    COMPAT_CONSTEXPR_20 inline bool operator==(const T& other, DecimalConstantProxy p) {
        return other == static_cast<decimal>(p);
    }

    template <typename T>
    COMPAT_CONSTEXPR_20 inline bool operator!=(DecimalConstantProxy p, const T& other) {
        return static_cast<decimal>(p) != other;
    }

    template <typename T>
    COMPAT_CONSTEXPR_20 inline bool operator!=(const T& other, DecimalConstantProxy p) {
        return other != static_cast<decimal>(p);
    }

} // namespace detail

/// <summary>
/// decimal 與原生整數/布林型別之混合邏輯及運算子（左 decimal 右原生）。
/// </summary>
/// <typeparam name="T">整數或布林型別</typeparam>
/// <param name="lhs">左 decimal</param>
/// <param name="rhs">右原生數值</param>
/// <returns>邏輯及結果</returns>
template <typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
COMPAT_CONSTEXPR_20 inline bool operator&&(const decimal& lhs, T rhs) noexcept {
    return static_cast<bool>(lhs) && (rhs != 0);
}

/// <summary>
/// decimal 與原生整數/布林型別之混合邏輯及運算子（左原生右 decimal）。
/// </summary>
/// <typeparam name="T">整數或布林型別</typeparam>
/// <param name="lhs">左原生數值</param>
/// <param name="rhs">右 decimal</param>
/// <returns>邏輯及結果</returns>
template <typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
COMPAT_CONSTEXPR_20 inline bool operator&&(T lhs, const decimal& rhs) noexcept {
    return (lhs != 0) && static_cast<bool>(rhs);
}

/// <summary>
/// decimal 與原生整數/布林型別之混合邏輯或運算子（左 decimal 右原生）。
/// </summary>
/// <typeparam name="T">整數或布林型別</typeparam>
/// <param name="lhs">左 decimal</param>
/// <param name="rhs">右原生數值</param>
/// <returns>邏輯或結果</returns>
template <typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
COMPAT_CONSTEXPR_20 inline bool operator||(const decimal& lhs, T rhs) noexcept {
    return static_cast<bool>(lhs) || (rhs != 0);
}

/// <summary>
/// decimal 與原生整數/布林型別之混合邏輯或運算子（左原生右 decimal）。
/// </summary>
/// <typeparam name="T">整數或布林型別</typeparam>
/// <param name="lhs">左原生數值</param>
/// <param name="rhs">右 decimal</param>
/// <returns>邏輯或結果</returns>
template <typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
COMPAT_CONSTEXPR_20 inline bool operator||(T lhs, const decimal& rhs) noexcept {
    return (lhs != 0) || static_cast<bool>(rhs);
}

#if !COMPAT_HAS_STD_FORMAT

/// <summary>
/// Formatter specialization for compat::decimal in fallback mode.
/// </summary>
/// <typeparam name="CharT">Character type, defaults to char.</typeparam>
template <typename CharT>
struct formatter<compat::decimal, CharT> {
    int precision = -1;

    /// <summary>
    /// Parses format specifications for decimal, supporting {:.Nf} and {:.N}.
    /// </summary>
    /// <typeparam name="ParseContext">Format parse context type.</typeparam>
    /// <param name="ctx">Parse context reference.</param>
    /// <returns>Iterator pointing to the end of format specification.</returns>
    template <typename ParseContext>
    COMPAT_CONSTEXPR_14 auto parse(ParseContext& ctx) -> decltype(ctx.begin()) {
        auto it = ctx.begin();
        auto end = ctx.end();
        if (it != end && *it == ':') {
            ++it;
        }
        if (it != end && *it == '.') {
            ++it;
            int p = 0;
            while (it != end && *it >= '0' && *it <= '9') {
                p = p * 10 + (*it - '0');
                ++it;
            }
            precision = p;
        }
        if (it != end && (*it == 'f' || *it == 'F')) {
            ++it;
        }
        return it;
    }

    /// <summary>
    /// Formats compat::decimal into decimal string according to parsed precision.
    /// </summary>
    /// <typeparam name="FormatContext">Format context type.</typeparam>
    /// <param name="val">The decimal value to format.</param>
    /// <param name="ctx">Format context reference.</param>
    /// <returns>Updated output iterator.</returns>
    template <typename FormatContext>
    auto format(const compat::decimal& val, FormatContext& ctx) const -> decltype(ctx.out()) {
        std::string s = detail::FormatDecimalToString(val, precision);
        auto it = ctx.out();
        for (char c : s) {
            *it++ = static_cast<CharT>(c);
        }
        ctx.advance_to(it);
        return it;
    }
};

#endif // !COMPAT_HAS_STD_FORMAT

} // namespace compat

#if COMPAT_HAS_STD_FORMAT
namespace std {

/// <summary>
/// std::formatter specialization for compat::decimal.
/// </summary>
/// <typeparam name="CharT">Character type.</typeparam>
template <typename CharT>
struct formatter<compat::decimal, CharT> {
    int precision = -1;

    /// <summary>
    /// Parses format specifications for decimal, supporting {:.Nf} and {:.N}.
    /// </summary>
    /// <typeparam name="ParseContext">Format parse context type.</typeparam>
    /// <param name="ctx">Parse context reference.</param>
    /// <returns>Iterator pointing to the end of format specification.</returns>
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin()) {
        auto it = ctx.begin();
        auto end = ctx.end();
        if (it != end && *it == ':') {
            ++it;
        }
        if (it != end && *it == '.') {
            ++it;
            int p = 0;
            while (it != end && *it >= '0' && *it <= '9') {
                p = p * 10 + (*it - '0');
                ++it;
            }
            precision = p;
        }
        if (it != end && (*it == 'f' || *it == 'F')) {
            ++it;
        }
        if (it != end && *it != '}') {
            throw std::format_error("invalid format specifier for decimal");
        }
        return it;
    }

    /// <summary>
    /// Formats compat::decimal into decimal string according to parsed precision.
    /// </summary>
    /// <typeparam name="FormatContext">Format context type.</typeparam>
    /// <param name="val">The decimal value to format.</param>
    /// <param name="ctx">Format context reference.</param>
    /// <returns>Updated output iterator.</returns>
    template <typename FormatContext>
    auto format(const compat::decimal& val, FormatContext& ctx) const -> decltype(ctx.out()) {
        std::string s = compat::detail::FormatDecimalToString(val, precision);
        auto it = ctx.out();
        for (char c : s) {
            *it++ = static_cast<CharT>(c);
        }
        return it;
    }
};

} // namespace std
#endif // COMPAT_HAS_STD_FORMAT

namespace std {

/// <summary>
/// std::hash specialization for compat::decimal.
/// </summary>
template <>
struct hash<compat::decimal> {
    /// <summary>
    /// Computes hash value for compat::decimal combining normalized unscaled integer and scale.
    /// </summary>
    /// <param name="d">The decimal value to hash.</param>
    /// <returns>Hash value.</returns>
    size_t operator()(const compat::decimal& d) const noexcept {
        if (d.is_nan()) {
            return static_cast<size_t>(0x7fc00000UL);
        }
        if (d.is_infinite()) {
            return (d.sign() < 0) ? static_cast<size_t>(0xff800000UL) : static_cast<size_t>(0x7f800000UL);
        }
        compat::bigint u = d.unscaled();
        int64_t s = d.scale();
        compat::detail::DecimalCore::normalize(u, s, false, false);
        size_t h1 = std::hash<compat::bigint>{}(u);
        size_t h2 = std::hash<int64_t>{}(s);
        return h1 ^ (h2 + static_cast<size_t>(0x9e3779b97f4a7c15ULL) + (h1 << 6) + (h1 >> 2));
    }
};

} // namespace std
