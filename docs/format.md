# 格式化輸出 (compat::format)

定義於標頭檔 [`<compat/Format.hpp>`](file:///D:/program/C++/CPP-Compat/include/compat/Format.hpp)。  
所屬命名空間：`compat`。

`compat::format` 提供型別安全、高效且符合 Python/Rust 風格的文字格式化能力，完全對齊 **ISO C++20 `std::format`** 語意。  
在 C++20 及以上支援環境直接轉發至原生 `std::format`；在 C++11/14/17 環境下無縫切換為純自研格式化引擎，支援佔位符替換、逸出字元與 `compat::formatter<T>` 自訂型別特化擴充。

---

## 函式樣板宣告 (Declaration)

```cpp
namespace compat {

    /// <summary>
    /// 依據格式化字串 fmt 與可變引數列表生成 std::string。
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

## 格式化字串語法 (Format String Syntax)

`CPP-Compat` 支援標準的大括號佔位符語法：

1. **自動位置佔位符 `{}`**：  
   依照傳入引數的順序自動依序替換。例如 `compat::format("{} + {} = {}", 1, 2, 3)` 會產生 `"1 + 2 = 3"`。
2. **大括號逸出 `{{` 與 `}}`**：  
   連續兩對大括號會被轉義為單一的字面值大括號字元 `{` 或 `}`，不被視為引數佔位符。
3. **引數過多或不足檢查**：  
   當引數數量少於佔位符時，自研引擎會拋出例外或安全中止（遵循 Fail-fast 原則）；未被使用的額外引數將被忽略。

---

## 原生支援型別 (Built-in Supported Types)

無需額外設定，自研引擎直接支援以下型別：
- **整數型別**：`int8_t`, `uint8_t`, `int16_t`, `uint16_t`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`, `char`, `bool` (`"true"` / `"false"`)
- **浮點數型別**：`float`, `double`, `long double`
- **字串與檢視**：`const char*`, `char*`, `std::string`, `compat::string_view`
- **指標型別**：`void*`, `const void*`, `std::nullptr_t`（輸出為十六進位位址或 `"nullptr"`）

---

## 自訂型別擴充介面 (`compat::formatter<T>`)

若要使您自訂的結構或類別能夠直接作為 `compat::format` 與 `compat::println` 的引數，只需特化 `compat::formatter<T>`：

### 特化規範

```cpp
template <>
struct compat::formatter<MyType> {
    static std::string format(const MyType& val) {
        // 返回格式化後的 std::string
    }
};
```

> [!TIP]
> 當切換至原生 C++20 `std::format` 模式時，此特化介面會自動與原生 `std::formatter` 橋接相容。

---

## 範例程式碼 (Example)

### 1. 基本型別格式化

```cpp
#include <compat/Format.hpp>
#include <iostream>

int main() {
    int32_t count = 42;
    double ratio = 3.14159;
    const char* tag = "ALPHA";

    std::string text = compat::format("Item count: {}, Ratio: {}, Tag: [{}], Escaped: {{Braces}}",
                                      count, ratio, tag);

    std::cout << text << "\n";
    // 輸出: Item count: 42, Ratio: 3.14159, Tag: [ALPHA], Escaped: {Braces}

    return 0;
}
```

### 2. 為自訂類別實現 `formatter`

```cpp
#include <compat/Format.hpp>
#include <iostream>

struct Vector2D {
    float x;
    float y;
};

// 特化 compat::formatter
template <>
struct compat::formatter<Vector2D> {
    static std::string format(const Vector2D& vec) {
        return compat::format("Vec2({:.2f}, {:.2f})", vec.x, vec.y);
    }
};

int main() {
    Vector2D pos{12.5f, -8.3f};
    std::string msg = compat::format("Current Player Position: {}", pos);
    std::cout << msg << "\n";
    // 輸出: Current Player Position: Vec2(12.50, -8.30)
    return 0;
}
```

---

## 參閱 (See Also)
- [API 參考首頁](README.md)
- [終端列印 (Print)](print.md)
- [字串檢視 (String View)](string_view.md)
