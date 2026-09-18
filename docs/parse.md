# 強型別解析 (compat::parse / from_chars)

定義於標頭檔 [`<compat/Parse.hpp>`](file:///D:/program/C++/CPP-Compat/include/compat/Parse.hpp)。  
所屬命名空間：`compat`。

`compat::parse<T>` 與 `compat::from_chars` 提供強型別、零記憶體配置（Zero-heap）、零 Locale 相依性、執行緒安全且無例外的文字轉數值解析引擎，完全遵循 **ISO C++17 `std::from_chars`** 規格。

---

## 函式與結構宣告 (Declaration)

```cpp
namespace compat {

    /// <summary>
    /// from_chars 專用解析結果結構體（對齊 std::from_chars_result）。
    /// </summary>
    struct from_chars_result {
        const char* ptr; // 解析成功終止的位置（或失敗時解析停頓處）
        std::errc ec;    // 錯誤碼（成功時為 std::errc{}）

        // 隱式轉換為布林值，方便 if (result) 判定
        constexpr explicit operator bool() const noexcept {
            return ec == std::errc{};
        }
    };

    // =========================================================================
    // 1. 低階解析：from_chars (整數，基數 2~36)
    // =========================================================================
    template <typename IntT>
    from_chars_result from_chars(const char* first, const char* last, IntT& value, int base = 10) noexcept;

    // =========================================================================
    // 2. 低階解析：from_chars (浮點數：float, double, long double)
    // =========================================================================
    template <typename FloatT>
    from_chars_result from_chars(const char* first, const char* last, FloatT& value) noexcept;

    // =========================================================================
    // 3. 高階友善解析：parse<T> (回傳 expected<T, std::errc>)
    // =========================================================================
    template <typename T>
    expected<T, std::errc> parse(compat::string_view str, int base = 10) noexcept;

} // namespace compat
```

---

## 核心演算法與特性保證 (Guarantees & Characteristics)

1. **完全零動態配置 (Zero Heap Allocation)**：  
   不呼叫 `malloc` / `new`，純粹以暫存器與有限棧空間進行狀態機解析，完全適合嵌入式微控制器與高頻交易系統。
2. **完全獨立於 Locale (Locale-Independent)**：  
   不受 `std::setlocale` 影響，小數點永遠強制為 `.`（杜絕歐洲語系小數點 `,` 引發的解析錯誤）。
3. **整數進位支援 (Radix 2 ~ 36)**：  
   支援二進位（`base = 2`）、八進位（`base = 8`）、十進位（`base = 10`）、十六進位（`base = 16`，大小寫均可）乃至高達 36 進位的任意字元映射。
4. **極限溢位保護 (Overflow & Underflow Protection)**：  
   在乘加前預先進行邊界偵測（`value > (max - digit) / base`），精確回報 `std::errc::result_out_of_range`，絕不觸發未定義行為（UB）。
5. **完整 IEEE 754 浮點支援**：  
   - 支援常規小數與科學記號（`1.234e-5`, `+4.5E+2`）。
   - 支援特殊符號：`NaN`, `nan`, `inf`, `+inf`, `-infinity`。

---

## 錯誤碼映射 (Error Codes)

| 錯誤碼 (`std::errc`) | 發生條件 |
| :--- | :--- |
| `std::errc{}` (0) | 解析成功。若使用 `parse<T>`，字串必須整串解析完畢（不可殘留未解析字元）。 |
| `std::errc::invalid_argument` | 缺少有效數值前綴、無法識別任何合法數字、或字串為空。 |
| `std::errc::result_out_of_range` | 數值超過目標型別的最小/最大表示範圍（溢位）。 |

---

## 範例程式碼 (Example)

### 1. 高階 `parse<T>` 範例 (搭配 Monadic `expected`)

```cpp
#include <compat/Parse.hpp>
#include <compat/Print.hpp>

void TestHighLevelParse() {
    // 解析十進位整數
    auto int_res = compat::parse<int32_t>("12345");
    if (int_res) {
        compat::println("Parsed integer: {}", *int_res);
    }

    // 解析十六進位整數
    auto hex_res = compat::parse<uint32_t>("FF00AA", 16);
    if (hex_res) {
        compat::println("Parsed hex: {:#X}", *hex_res);
    }

    // 解析浮點數科學記號
    auto float_res = compat::parse<double>("-2.5e4");
    if (float_res) {
        compat::println("Parsed scientific double: {}", *float_res); // -25000.0
    }

    // 溢位檢測
    auto ovf_res = compat::parse<uint8_t>("256"); // uint8_t 最大為 255
    if (!ovf_res && ovf_res.error() == std::errc::result_out_of_range) {
        compat::println("Correctly detected uint8_t overflow!");
    }
}
```

### 2. 低階 `from_chars` 串流解析範例

```cpp
#include <compat/Parse.hpp>
#include <compat/Print.hpp>

void TestLowLevelFromChars() {
    const char* str = "100,200,300";
    const char* ptr = str;
    const char* end = str + 11;

    while (ptr < end) {
        int val = 0;
        auto res = compat::from_chars(ptr, end, val);
        if (res) {
            compat::println("Extracted item: {}", val);
            ptr = res.ptr;
            if (ptr < end && *ptr == ',') {
                ++ptr; // 跳過分隔符
            }
        } else {
            break;
        }
    }
}
```

---

## 參閱 (See Also)
- [API 參考首頁](README.md)
- [期望值與錯誤處理 (Expected)](expected.md)
- [字串檢視 (String View)](string_view.md)
