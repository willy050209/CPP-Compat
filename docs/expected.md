# 期望值與錯誤處理 (compat::expected)

定義於標頭檔 [`<compat/Expected.hpp>`](file:///D:/program/C++/CPP-Compat/include/compat/Expected.hpp)。  
所屬命名空間：`compat`。

`compat::expected<T, E>` 是一個詞彙型別（Vocabulary Type），封裝了運算成功產生的預期數值（型別 `T`）或失敗時攜帶的非預期錯誤值（型別 `E`）。它完全遵循 **ISO C++23 `std::expected`** 標準合約，支援 Monadic Operations，旨在取代 C 風格錯誤碼與 C++ 例外處理，落實純函數、型別安全且零額外記憶體配置的錯誤傳遞架構。

---

## 類別樣板宣告 (Declaration)

```cpp
namespace compat {
    // 成功值標籤與錯誤值容器
    template <typename E>
    class unexpected;

    // 異常存取例外物件
    template <typename E>
    class bad_expected_access;

    // 基礎 expected 類別
    template <typename T, typename E>
    class expected;

    // void 特化版（無成功回傳值）
    template <typename E>
    class expected<void, E>;
}
```

---

## 底層儲存架構與相容性分流 (Storage Architectures)

```mermaid
flowchart TD
    Mode{編譯環境與模式}
    Mode -- "C++23 且標準庫支援" --> Native["std::expected&lt;T, E&gt;"]
    Mode -- "C++17 (有 &lt;variant&gt;)" --> VarImpl["compat::detail::variant_impl::expected&lt;T, E&gt;<br>(基於 std::variant&lt;T, unexpected&lt;E&gt;&gt;)"]
    Mode -- "C++11/14 或無 variant" --> UnionImpl["compat::detail::union_impl::expected&lt;T, E&gt;<br>(基於 Tagged Unrestricted Union)"]
```

1. **Native C++23 模式**：直接別名 `std::expected`、`std::unexpected`、`std::bad_expected_access`。
2. **C++17 Variant Fallback**：透過 `std::variant` 實作，自動享有現代 STL 的例外保證與移動語意。
3. **C++11/14 Tagged Union Fallback**：  
   純自研**無限制聯合體（Unrestricted Union）**。當 `T` 與 `E` 均為 Trivial 類型時，自研 `expected` 亦自動成為 Trivial 類型，並透過精確的顯式解構式呼叫管理非平凡（Non-trivial）型別的生命週期。

---

## 成員型別 (Member Types)

| 成員型別 | 定義 |
| :--- | :--- |
| `value_type` | `T`（在 `expected<void, E>` 中為 `void`） |
| `error_type` | `E` |
| `unexpected_type` | `compat::unexpected<E>` |
| `template <typename U> rebind<U>` | `compat::expected<U, E>` |

---

## 成員函式 (Member Functions)

### 觀察者 (Observers)

| 函式 | 簽章 | 說明與例外保證 |
| :--- | :--- | :--- |
| `has_value` | `constexpr bool has_value() const noexcept;` | 檢查物件當前是否包含預期成功值。 |
| `operator bool` | `constexpr explicit operator bool() const noexcept;` | 等價於 `has_value()`。 |
| `value()` | `constexpr T& value() &;`<br>`constexpr const T& value() const &;`<br>`constexpr T&& value() &&;`<br>`constexpr const T&& value() const &&;` | 存取成功值。若為錯誤狀態，拋出 `bad_expected_access<E>(error())`（或呼叫 `COMPAT_THROW_OR_ABORT`）。 |
| `operator*()` | `constexpr T& operator*() & noexcept;`<br>`constexpr const T& operator*() const & noexcept;` | 解參考取得成功值。若為錯誤狀態，行為未定義（零檢查開銷）。 |
| `operator->()` | `constexpr T* operator->() noexcept;`<br>`constexpr const T* operator->() const noexcept;` | 箭頭運算子存取成功值指標。 |
| `error()` | `constexpr E& error() & noexcept;`<br>`constexpr const E& error() const & noexcept;`<br>`constexpr E&& error() && noexcept;` | 存取錯誤值。呼叫前必須確保 `!has_value()`。 |
| `value_or(U&& default_value)` | `constexpr T value_or(U&& default_value) const&;` | 若有值回傳該值，否則回傳預設值。 |
| `error_or(G&& default_error)` | `constexpr E error_or(G&& default_error) const&;` | 若為錯誤回傳錯誤值，否則回傳預設錯誤。 |

---

## 鏈式單子操作 (Monadic Operations)

符合現代函式語言風格，避免層層巢狀 `if (!res)` 檢查：

### 1. `and_then`
```cpp
template <typename F>
constexpr auto and_then(F&& f) & -> /* 返回 f(*this) 的 expected */;
```
- **語意**：當前有值時，將數值傳入函式 `f`（`f` 必須回傳另一個 `expected`）；若當前為錯誤狀態，直接轉發當前錯誤，短路執行。

### 2. `transform`
```cpp
template <typename F>
constexpr auto transform(F&& f) & -> /* 返回包含 f(*this) 的 expected */;
```
- **語意**：當前有值時，將數值經由 `f` 轉換並重新包裝入 `expected`；若為錯誤直接轉發。

### 3. `or_else`
```cpp
template <typename F>
constexpr auto or_else(F&& f) & -> /* 錯誤修復回傳 */;
```
- **語意**：當前為錯誤狀態時，將錯誤傳入 `f` 進行補救或重新包裝；若當前有值則直接轉發數值。

### 4. `transform_error`
```cpp
template <typename F>
constexpr auto transform_error(F&& f) & -> /* 轉換錯誤型別 */;
```
- **語意**：當前為錯誤狀態時，將錯誤值傳入 `f` 轉換為新型別的錯誤並包裝；若當前有值則直接轉發數值。

---

## `expected<void, E>` 特化規格

當函式僅代表成功執行完畢，不需要回傳數值時，使用 `expected<void, E>`：
- 建構式支援預設建構 `expected<void, E>()` 表示成功。
- `value()` 回傳 `void`。若為錯誤狀態則拋出異常或終止。
- `operator*()` 僅作為語法斷言，回傳 `void`。

---

## 範例程式碼 (Example)

```cpp
#include <compat/Expected.hpp>
#include <iostream>
#include <string>

// 1. 回傳 expected<T, E> 的安全函式
compat::expected<int, std::string> ParsePositive(int val) {
    if (val <= 0) {
        return compat::unexpected<std::string>("Value must be strictly positive");
    }
    return val;
}

// 2. 回傳 expected<void, E> 的驗證函式
compat::expected<void, std::string> ValidateMax(int val) {
    if (val > 100) {
        return compat::unexpected<std::string>("Value exceeds maximum limit of 100");
    }
    return {};
}

int main() {
    // 成功單子鏈
    auto result = ParsePositive(42)
        .and_then([](int v) -> compat::expected<int, std::string> {
            return v * 2;
        })
        .transform([](int v) {
            return v + 10;
        });

    if (result) {
        std::cout << "Success Pipeline: " << *result << "\n"; // 94
    }

    // 失敗鏈與 or_else 救援
    auto fallback = ParsePositive(-5)
        .or_else([](const std::string& err) -> compat::expected<int, std::string> {
            std::cout << "Caught error: [" << err << "], applying fallback...\n";
            return 1; // 補救值
        });

    std::cout << "Recovered Value: " << fallback.value() << "\n"; // 1

    return 0;
}
```

---

## 參閱 (See Also)
- [API 參考首頁](README.md)
- [強型別解析 (Parse)](parse.md)
- [特性檢測與配置 (Config)](config.md)
