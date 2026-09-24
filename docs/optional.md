# 選擇性值 (compat::optional)

定義於標頭檔 [`<compat/Optional.hpp>`](file:///D:/program/C++/CPP-Compat/include/compat/Optional.hpp)。  
所屬命名空間：`compat`。

`compat::optional<T>` 表示一個可能存在或為空的型別安全包裝器，完全對齊 **ISO C++17 `std::optional`** 標準規格。  
在支援 C++17 的編譯器下直接透明別名（Type Alias）至 `std::optional`；在舊版 C++11/14 環境下啟用基於未受限聯合體（Unrestricted Union）之純自研 Fallback 實作，零堆積配置、保證最佳對齊與正確的生命週期管理。

---

## 核心型別與常數 (Types & Constants)

```cpp
namespace compat {
    template <typename T>
    class optional;

    struct nullopt_t { /* ... */ };
    constexpr nullopt_t nullopt{/* ... */};

    class bad_optional_access : public std::exception { /* ... */ };
}
```

| 型別 / 實體 | 說明 |
| :--- | :--- |
| `compat::optional<T>` | 管理可能存在的值之可選型別樣板。 |
| `compat::nullopt_t` | 空狀態指示型別。 |
| `compat::nullopt` | 用於初始化或賦值空狀態的常數單例物件。 |
| `compat::bad_optional_access` | 當呼叫 `.value()` 存取空狀態時拋出之標準例外。 |

---

## 成員函式 (Member Functions)

### 1. 建構與賦值 (Construction & Assignment)

- **預設建構**：`optional()` 或 `optional(nullopt)` 構造為空狀態（`has_value() == false`）。
- **值建構**：`optional(const T&)` / `optional(T&&)` 原地複製或移動建構值。
- **賦值操作**：支援 `nullopt` 賦值（銷毀內部值並重設為空）、值賦值與同型別 `optional` 賦值。
- **就地構造 (Emplace)**：
  ```cpp
  template <typename... Args>
  T& emplace(Args&&... args);
  ```
  若當前已包含值，先銷毀既有值；接著在內部未受限聯合儲存區中，以 `std::forward<Args>(args)...` 就地直接建構型別 `T`，並回傳新建構物件的參考。保證建構完成後處於合法有效狀態。
- **重設狀態 (Reset)**：
  ```cpp
  void reset() noexcept;
  ```
  若當前包含值，顯式呼叫內部物件之解構式並銷毀之；將狀態標誌重設為未包含值（`has_value() == false`）。保證 `noexcept`。

### 2. 特徵傳遞與 C++11 特殊成員支援 (Trait Propagation Invariant)

`compat::optional<T>` 嚴格遵循 ISO C++ 標準的特殊成員特徵傳遞合約。即使在缺乏 C++20 `requires` 的 **C++11/14** 環境下，亦透過 SFINAE 與基底類別選擇機制完整實作：
- **複製建構與移動建構**：
  - 若 `T` 不具備可複製建構性（`is_copy_constructible<T>::value == false`，如 `std::unique_ptr`），則 `optional<T>` 的複製建構式將在編譯期被禁用（`delete`），使 `std::is_copy_constructible<optional<T>>::value` 精確為 `false`。
  - 移動建構性同理依 `T` 傳遞。
- **複製賦值與移動賦值**：
  - 複製與移動賦值運算子同理依 `T` 的賦值能力精確啟用或禁用。
- **平凡性傳遞 (Triviality)**：
  - 若 `T` 為平凡型別（Trivial Type），`optional<T>` 亦保證可成為平凡複製/解構型別。
- **條件式 `noexcept`**：
  - 移動建構與交換操作嚴格依 `std::is_nothrow_move_constructible<T>` 與 `std::is_nothrow_move_assignable<T>` 條件化推導 `noexcept`，絕不引發非預期 `std::terminate`。

### 3. 狀態檢查與觀察 (Observers)

- **狀態判斷**：
  - `has_value()`：回傳內部是否包含實體值。
  - `operator bool()`：等同於 `has_value()`，方便於 `if (opt)` 條件判斷。
- **值存取**：
  - `operator*()` / `operator->()`：直接存取內部物件的參考或指標（**前置條件**：`has_value() == true`；不進行邊界檢查以追求零開銷）。
  - `value()`：安全存取內部值。若 `has_value() == false` 則拋出 `compat::bad_optional_access` 例外（在 `-fno-exceptions` 下觸發 `COMPAT_THROW_OR_ABORT` 執行 fail-fast）。
  - `value_or(default_value)`：若包含值則回傳該值，否則回傳傳入之預設回退值。

---

## 非成員輔助函式 (Non-member Functions)

### `compat::make_optional`

```cpp
template <typename T, typename... Args>
compat::optional<T> make_optional(Args&&... args);
```

- **說明**：根據引數直接原地建構並回傳含有值的 `optional<T>`，語意對齊 `std::make_optional`。

---

## 演算法協同運作 (Interoperability: C++23 Folds)

在 ISO C++23 的受約束範圍演算法中，非空摺疊函式（如 `ranges::fold_left_first` 與 `ranges::fold_right_last`）要求在傳入空範圍時回傳空的可選值，在非空範圍時回傳計算結果。  
`CPP-Compat` 藉由 `<compat/Optional.hpp>`，讓這些 C++23 前沿演算法在純 C++11 環境下亦能享有標準的 `optional` 回傳語意：

```cpp
std::vector<int> empty_vec;
auto res = compat::ranges::fold_left_first(empty_vec, [](int a, int b) { return a + b; });
if (!res.has_value()) {
    // 成功處理空範圍，無需魔術數或哨兵值！
}
```

---

## 範例程式碼 (Example)

```cpp
#include <compat/Optional.hpp>
#include <compat/Print.hpp>
#include <string>

compat::optional<int> ParsePort(const std::string& str) {
    if (str.empty()) return compat::nullopt;
    int port = std::stoi(str);
    if (port < 1 || port > 65535) {
        return compat::nullopt; // 超出範圍
    }
    return port;
}

int main() {
    auto valid_port = ParsePort("8080");
    if (valid_port) {
        compat::println("Server listening on port: {}", *valid_port);
    }

    auto invalid_port = ParsePort("99999");
    compat::println("Port: {}", invalid_port.value_or(80)); // 回退至預設 80

    // 使用 make_optional
    auto opt_str = compat::make_optional<std::string>(5, 'A');
    compat::println("Created: {}", opt_str.value()); // AAAAA

    return 0;
}
```

---

## 參閱 (See Also)
- [API 參考首頁](README.md)
- [期望值與錯誤處理 (Expected)](expected.md)
- [受約束範圍演算法 (Algorithms)](algorithms.md)
