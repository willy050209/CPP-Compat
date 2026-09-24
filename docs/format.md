# 格式化輸出 (compat::format)

定義於標頭檔 [`<compat/Format.hpp>`](file:///D:/program/C++/CPP-Compat/include/compat/Format.hpp)。  
所屬命名空間：`compat`。

`compat::format` 提供型別安全、高效且符合現代 C++ 標準的字串格式化能力，完全對齊 **ISO C++20 `std::format`** 語意。  
在 C++20 及以上支援環境直接轉發至原生 `std::format`（Zero Overhead）；在 C++11/14/17 環境下無縫切換為純自研格式化引擎，完整支援 **標準格式化規格語法**、**Unicode 東亞寬度 (UAX #11)**、佔位符替換、轉義字元與 `compat::formatter<T>` 自訂型別擴充。

> [!NOTE]
> 在 GCC 13 (`_GLIBCXX_RELEASE < 14`) 環境下，因 libstdc++ 初始版本的 `std::format` 尚未支援 P1868R2 (東亞寬度計算) 且在精度截斷時會直接切斷多位元組 UTF-8 字節，`CPP-Compat` 會自動將 `COMPAT_HAS_STD_FORMAT` 設為 `0` 並切換至自研 Fallback 引擎，確保所有平台（Windows / Linux / macOS）字串排版與安全性表現完全一致。

---

## 函式樣板宣告 (Declaration)

```cpp
namespace compat {

    /// <summary>
    /// 依據格式化字串 fmt 與可變引數列表生成格式化後的 std::string。
    /// </summary>
    template <typename... Args>
    std::string format(const char* fmt, Args&&... args);

    template <typename... Args>
    std::string format(compat::string_view fmt, Args&&... args);

    /// <summary>
    /// 自訂型別格式化擴充特化介面（對齊 std::formatter<T>）。
    /// </summary>
    template <typename T, typename CharT = char>
    struct formatter;

} // namespace compat
```

---

## 格式化規格語法 (Format Specifier Syntax)

`compat::format` 遵循 ISO C++20 標準格式化規格語法：

```
{[arg_id]:[[fill]align][sign][#][0][width][.precision][type]}
```

### 1. 語法元素說明

| 元素 | 語法 / 符號 | 說明 | 範例 |
| :--- | :--- | :--- | :--- |
| **arg_id** | 空白（自動索引）或非負整數（手動索引） | 指定引數來源序號。支援 `{}` 自動依序遞增（`0, 1, 2...`）與 `{0}`, `{1}` 明確手動下標。**嚴禁混用**：在同一次格式化字串中混用自動與手動索引將拋出 `std::runtime_error`（或觸發 fail-fast）。 | `"{0} + {1} = {0}"`<br>`"{}, {}"` |
| **fill** | 任意單一字元（預設為空白 `' '`） | 用於補齊寬度的填充字元。若指定填充字元，後面必須緊跟對齊指示符 `align`。 | `"{:*<8}"`（以 `*` 填充） |
| **align** | `<`（靠左）<br>`>`（靠右）<br>`^`（置中） | 指定文字對齊方向。<br>- 非數值（字串、布林）預設為靠左 `<`<br>- 數值（整數、浮點數、指標）預設為靠右 `>` | `"{:<8}"` → `"1       "`<br>`"{:>8}"` → `"       1"`<br>`"{:^8}"` → `"   1    "` |
| **sign** | `+`（正負號均顯示）<br>`-`（僅負號顯示，預設）<br>` `（非負數開頭補空白） | 控制數值的正負號符號顯示方式。 | `"{:+d}"` → `"+42"`<br>`"{: d}"` → `" 42"` |
| **#** | `#`（替代形式 / 前綴旗標） | 為進位制數值加上基底前綴：<br>- 十六進位：`0x` 或 `0X`<br>- 二進位：`0b` 或 `0B`<br>- 八進位：`0` | `"{:#x}"` → `"0x2a"`<br>`"{:#010x}"` → `"0x0000002a"`<br>`"{:#b}"` → `"0b101010"` |
| **0** | `0`（前導零補齊） | 以 `'0'` 補齊數值至指定寬度。零會填補於正負號或進位前綴之後、數字主體之前。 | `"{:05d}"` → `"00042"`<br>`"{:+06d}"` → `"+00042"` |
| **width** | 正整數 | 欄位的**最小欄位寬度**（對字串而言為**顯示欄位寬度**，非位元組數）。若內容長度小於寬度則以 `fill` 填充。 | `"{:8}"`（至少佔 8 欄位） |
| **.precision** | `.` 加上非負整數 | 控制精度：<br>- 浮點數：小數點後的位數（若未指定，`double` 預設採 `%.17g`，`long double` 採 `%.21Lg` 完整保留所有有效位數）<br>- 字串：**最大允許的顯示欄位寬度**（自動安全截斷） | `"{:.2f}"` → `"3.14"`<br>`"{:.2}"` 對 `"一號"` → `"一"` |
| **type** | 型別標記字元 | 指定引數的輸出呈現方式（詳見下表）。 | `"{:x}"`, `"{:b}"`, `"{:f}"` |

### 2. 引數索引與格式化安全不變量 (Formatting Invariants)

- **型別擦除參數索引表 (Type-Erased Table)**：  
  自研格式化引擎拋棄了傳統可變引數遞迴剝離（Recursive Peeling）作法，在格式化開始前建立扁平化型別擦除參數跳轉表（`FormatArgRef`），以 $O(1)$ 時間複雜度隨機檢索任意序號的引數，天然支援任意順序、重複下標（如 `"{1} {0} {1}"`）。
- **嚴格索引模式檢查**：  
  格式解析器嚴格追蹤當前索引模式（`Manual` 或 `Automatic`）。若在手動模式下遭遇 `{}`，或在自動模式下遭遇 `{0}`，將精準通報語法錯誤，防止非預期參數對齊錯位。
- **浮點數預設精度保證**：  
  當格式字串未給予 `.precision` 時（如 `compat::format("{}", 1.23456789012345)`），引擎不再採用 CRT 傳統的 `%.6g`（此舉會遺失 9 位有效數字），而是依 IEEE 754 型別極限自動啟用最完整精度（`float` 採 `%.9g`、`double` 採 `%.17g`、`long double` 採 `%.21Lg`），忠實保留浮點數原始精度。
- **任意大精度緩衝區防溢位保證**：  
  在面對極端精度格式化（如 `{:.128f}`, `{:.1000f}`）時，引擎配備兩階段緩衝管理：優先使用 128 位元組 stack 緩衝；若所需長度超過 stack 容量，自動安全擴展至足量動態緩衝區並重新格式化，徹底消除 ASan `stack-buffer-overflow` 越界讀寫風險。

### 3. 呈現型別 (Presentation Types)

| 型別字元 | 適用資料型別 | 輸出格式說明 | 範例 |
| :---: | :--- | :--- | :--- |
| `d` | 整數、布林值 | 十進位整數 | `compat::format("{:d}", 42)` → `"42"` |
| `x` | 整數、布林值 | 小寫十六進位整數 | `compat::format("{:x}", 255)` → `"ff"` |
| `X` | 整數、布林值 | 大寫十六進位整數 | `compat::format("{:X}", 255)` → `"FF"` |
| `b` | 整數、布林值 | 小寫二進位整數 | `compat::format("{:b}", 10)` → `"1010"` |
| `B` | 整數、布林值 | 大寫二進位整數 | `compat::format("{:B}", 10)` → `"1010"` |
| `o` | 整數、布林值 | 八進位整數 | `compat::format("{:o}", 64)` → `"100"` |
| `f`, `F` | 浮點數 | 固定小數點表示法 | `compat::format("{:.2f}", 3.14159)` → `"3.14"` |
| `e`, `E` | 浮點數 | 科學記號表示法 | `compat::format("{:.2e}", 1234.5)` → `"1.23e+03"` |
| `g`, `G` | 浮點數 | 一般精簡表示法（自動選擇固定小數點或科學記號） | `compat::format("{:g}", 0.00012)` → `"0.00012"` |
| `s` | 字串、檢視、布林 | 字串輸出（布林輸出 `"true"` / `"false"`） | `compat::format("{:s}", "hello")` → `"hello"` |
| `c` | 整數、字元 | 字元輸出（轉為對應 ASCII/Unicode 字元） | `compat::format("{:c}", 65)` → `"A"` |
| `p` | 指標型別 | 指標記憶體位址（十六進位，通常附帶 `0x` 前綴） | `compat::format("{:p}", ptr)` → `"0x7ffd1234"` |

---

## Unicode 東亞寬度 (East Asian Width, UAX #11 / P1868R2)

在等寬字型終端機中，漢字、日文假名、韓文字元與部分 Emoji 的寬度為一般西文字元的兩倍（全形，2 個欄位）。  
若按位元組（UTF-8 為 3~4 位元組）計算寬度，將導致終端排版錯位。

`compat::format` 的自研引擎完全實作了 **Unicode Standard Annex #11 (UAX #11)** 與 **ISO C++20 P1868R2** 規範：
- **顯示寬度精確計算**：
  - 西文字母、數字、半形符號：佔 **1 欄位**。
  - CJK 漢字、平假名、片假名、諺文、全形 ASCII 變體、多數 Emoji：佔 **2 欄位**。
  - 控制字元、結合音標符號（Combining Diacritical Marks）、零寬連字元（ZWJ/ZWNJ）：佔 **0 欄位**。
- **多欄位對齊保證**：
  ```cpp
  // 「一號」包含 2 個中文字 = 4 欄位寬度
  // 設定寬度 8 時，自動補 4 個空白，使各欄在終端精確佔用 8 個欄位：
  compat::format("{:8}{:8}{:8}{:8}", "一號", "二號", "三號", "四號");
  // 輸出: "一號    二號    三號    四號    "
  ```
- **精度安全截斷 (`.precision`)**：
  - 依照**顯示欄位寬度**安全截斷，不切斷多位元組 UTF-8 字元。
  - 例如 `compat::format("{:.2}", "一號二號")` 輸出 `"一"`（佔 2 欄位）；而 `compat::format("{:.3}", "一號二號")` 亦輸出 `"一"`（因包含下一字元需 4 欄位，超過寬度 3）。

---

## 寬字元與寬字串原生支援 (Wide String Support)

`compat::format` 具備原生將寬字元編碼即時轉譯至 UTF-8 的能力，支援以下型別無縫混用：
- `wchar_t`, `const wchar_t*`, `wchar_t[N]`, `std::wstring`
- `char16_t`, `const char16_t*`, `std::u16string`
- `char32_t`, `const char32_t*`, `std::u32string`
- `char8_t`, `const char8_t*`, `std::u8string`（C++20+）

```cpp
std::wstring ws = L"寬字串測試 🌟";
std::string result = compat::format("寬字元: {}, 寬字串: {:16}", L'中', ws);
```

---

## 原生支援型別 (Built-in Supported Types)

| 分類 | 支援型別 |
| :--- | :--- |
| **整數型別** | `int8_t`, `uint8_t`, `int16_t`, `uint16_t`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`, `char`, `signed char`, `unsigned char`, `bool` |
| **浮點數型別** | `float`, `double`, `long double` |
| **字串與檢視** | `const char*`, `char*`, `char[N]`, `std::string`, `compat::string_view` |
| **寬字元與字串** | `wchar_t`, `const wchar_t*`, `std::wstring`, `char16_t`, `std::u16string`, `char32_t`, `std::u32string` |
| **指標型別** | `void*`, `const void*`, `std::nullptr_t` |

---

## 自訂型別擴充介面 (`compat::formatter<T>`)

若要讓自訂類別支援 `compat::format`，只需特化 `compat::formatter<T>`：

```cpp
template <typename CharT>
struct compat::formatter<MyType, CharT> {
    // 1. 可選：解析格式字串規格
    template <typename ParseContext>
    auto parse(ParseContext& ctx) -> decltype(ctx.begin()) {
        // 解析規格，返回結束迭代器
        return ctx.begin();
    }

    // 2. 格式化邏輯
    template <typename FormatContext>
    auto format(const MyType& val, FormatContext& ctx) const -> decltype(ctx.out()) {
        return compat::format_to(ctx.out(), "MyType({})", val.value);
    }
};
```

---

## 範例程式碼 (Example)

### 1. 寬度、對齊與填充

```cpp
#include <compat/Format.hpp>
#include <iostream>

int main() {
    // 預設靠右對齊
    std::cout << "[" << compat::format("{:8}", 1) << "]\n";
    // 輸出: [       1]

    // 靠左對齊
    std::cout << "[" << compat::format("{:<8}", 1) << "]\n";
    // 輸出: [1       ]

    // 置中對齊與自訂填充字元
    std::cout << "[" << compat::format("{:*^10}", "title") << "]\n";
    // 輸出: [**title***]

    // 前導零補齊與正負號
    std::cout << "[" << compat::format("{:+08d}", 42) << "]\n";
    // 輸出: [+0000042]

    return 0;
}
```

### 2. 進位制與浮點數格式化

```cpp
#include <compat/Format.hpp>
#include <iostream>

int main() {
    // 十六進位與二進位
    std::cout << compat::format("Hex: {:#x}, Upper: {:#X}\n", 255, 255);
    // 輸出: Hex: 0xff, Upper: 0xFF

    std::cout << compat::format("Binary: {:#b}, Hex Pad: {:#010x}\n", 42, 42);
    // 輸出: Binary: 0b101010, Hex Pad: 0x0000002a

    // 浮點數精度
    std::cout << compat::format("Pi: {:.2f}, Exp: {:.3e}\n", 3.14159265, 12345.678);
    // 輸出: Pi: 3.14, Exp: 1.235e+04

    return 0;
}
```

### 3. CJK 東亞寬度多欄表格對齊

```cpp
#include <compat/Format.hpp>
#include <iostream>

int main() {
    // 每個欄位均為 8 欄位寬度，中文字（4 欄寬）自動補 4 空白，完美對齊
    std::cout << compat::format("{:8}{:8}{:8}{:8}\n", "一號", "二號", "三號", "四號");
    std::cout << compat::format("{:8}{:8}{:8}{:8}\n", "Item A", "Item B", "Item C", "Item D");
    // 輸出:
    // 一號    二號    三號    四號    
    // Item A  Item B  Item C  Item D  

    // 精度安全截斷（依顯示寬度截斷，不切斷 UTF-8 字元）
    std::cout << compat::format("{:.2}\n", "一號二號"); // 輸出: 一
    std::cout << compat::format("{:.4}\n", "一號二號"); // 輸出: 一號

    return 0;
}
```

---

## 參閱 (See Also)

- [API 參考首頁](README.md)
- [終端列印 (Print)](print.md)
- [字串檢視 (String View)](string_view.md)
- [強型別解析 (Parse)](parse.md)
