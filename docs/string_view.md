# 字串檢視 (compat::string_view)

定義於標頭檔 [`<compat/StringView.hpp>`](file:///D:/program/C++/CPP-Compat/include/compat/StringView.hpp)。  
所屬命名空間：`compat`。

`compat::string_view` 是一個非擁有型（Non-owning）的唯讀字串引用物件，完全對齊 ISO C++17 `std::string_view` 標準規格。  
在支援 C++17 的編譯器下直接別名為 `std::string_view`；在 C++11/14 環境或強制 Fallback 模式下，無縫啟用具備完整成員函式、運算子與 `std::hash` 特化的純自研輕量實作。

---

## 類別樣板宣告 (Declaration)

```cpp
namespace compat {
#if COMPAT_HAS_STD_STRING_VIEW
    using string_view = std::string_view;
#else
    namespace detail {
        class string_view;
    }
    using string_view = detail::string_view;
#endif
}
```

---

## 成員型別 (Member Types)

| 成員型別 | 定義 |
| :--- | :--- |
| `traits_type` | `std::char_traits<char>` |
| `value_type` | `char` |
| `pointer` | `const char*` |
| `const_pointer` | `const char*` |
| `reference` | `const char&` |
| `const_reference` | `const char&` |
| `const_iterator` | 指向唯讀字元的隨機存取迭代器 (`const char*`) |
| `iterator` | `const_iterator` |
| `size_type` | `std::size_t` |
| `difference_type` | `std::ptrdiff_t` |

---

## 成員常數 (Member Constants)

| 成員常數 | 數值 / 意義 |
| :--- | :--- |
| `npos` | `static constexpr size_type npos = size_type(-1);`<br>表示未找到或到達末尾的哨兵常數 |

---

## 成員函式 (Member Functions)

### 建構與賦值 (Construction & Assignment)

```cpp
constexpr string_view() noexcept;
constexpr string_view(const char* s, size_type count) noexcept;
constexpr string_view(const char* s) noexcept;
constexpr string_view(const string_view& other) noexcept = default;
string_view& operator=(const string_view& view) noexcept = default;
```

### 元素存取 (Element Access)

| 函式 | 簽章 | 說明與例外保證 |
| :--- | :--- | :--- |
| `operator[]` | `constexpr const_reference operator[](size_type pos) const noexcept;` | 存取索引 `pos` 處的字元。不進行邊界檢查。 |
| `at` | `constexpr const_reference at(size_type pos) const;` | 存取索引 `pos` 處的字元。越界時透過 `COMPAT_THROW_OR_ABORT` 拋出 `std::out_of_range`。 |
| `front` | `constexpr const_reference front() const noexcept;` | 存取首字元。字串為空時行為未定義。 |
| `back` | `constexpr const_reference back() const noexcept;` | 存取末字元。字串為空時行為未定義。 |
| `data` | `constexpr const_pointer data() const noexcept;` | 回傳底層字元陣列首指針。 |

### 容量與大小 (Capacity)

| 函式 | 簽章 | 說明 |
| :--- | :--- | :--- |
| `size`, `length` | `constexpr size_type size() const noexcept;`<br>`constexpr size_type length() const noexcept;` | 回傳字串長度（字元數量）。 |
| `max_size` | `constexpr size_type max_size() const noexcept;` | 回傳支援的最大字元數量。 |
| `empty` | `constexpr bool empty() const noexcept;` | 檢查長度是否為零。 |

### 修改作業 (Modifiers)

| 函式 | 簽章 | 說明 |
| :--- | :--- | :--- |
| `remove_prefix` | `constexpr void remove_prefix(size_type n) noexcept;` | 縮短前綴，將檢視範圍向右移動 `n` 個字元。 |
| `remove_suffix` | `constexpr void remove_suffix(size_type n) noexcept;` | 縮短後綴，將檢視範圍長度減少 `n` 個字元。 |
| `swap` | `constexpr void swap(string_view& s) noexcept;` | 交換兩個 `string_view` 物件。 |

### 字串搜尋作業 (Search Operations)

| 函式 | 說明與複雜度 |
| :--- | :--- |
| `starts_with(string_view x)` / `starts_with(char c)` | 檢查前綴是否相符 ($O(M)$)。 |
| `ends_with(string_view x)` / `ends_with(char c)` | 檢查後綴是否相符 ($O(M)$)。 |
| `find(string_view v, size_type pos = 0)` | 由前向後尋找子字串或字元，找不到回傳 `npos` ($O(N \times M)$)。 |
| `rfind(string_view v, size_type pos = npos)` | 由後向前尋找子字串或字元，找不到回傳 `npos`。 |
| `substr(size_type pos = 0, size_type count = npos)` | 回傳子字串檢視。`pos > size()` 時拋出 `std::out_of_range`。 |
| `compare(string_view v)` | 字典序比較，回傳 `< 0`, `0`, `> 0`。 |

---

## 非成員函式與特化 (Non-Member Functions & Specializations)

- **比較運算子**：完整支援 `==`, `!=`, `<`, `<=`, `>`, `>=`，可與 `const char*` 及 `std::string` 雙向混用。
- **串流輸出運算子**：`std::ostream& operator<<(std::ostream& os, string_view sv)`。
- **雜湊特化**：`std::hash<compat::string_view>`（基於 FNV-1a 高效常數散列演算法實作）。

---

## 範例程式碼 (Example)

```cpp
#include <compat/StringView.hpp>
#include <iostream>

int main() {
    compat::string_view sv = "Hello, Modern C++ World!";

    // 1. 基本屬性
    std::cout << "Size: " << sv.size() << "\n";
    std::cout << "Starts with 'Hello': " << std::boolalpha << sv.starts_with("Hello") << "\n";

    // 2. 切片與縮短前綴 (Zero Allocation)
    sv.remove_prefix(7);
    std::cout << "After remove_prefix: " << sv << "\n"; // "Modern C++ World!"

    // 3. 子字串擷取
    auto sub = sv.substr(0, 10);
    std::cout << "Substr: " << sub << "\n"; // "Modern C++"

    // 4. std::hash 特化
    std::hash<compat::string_view> hasher;
    std::cout << "Hash: " << hasher(sub) << "\n";

    return 0;
}
```

---

## 參閱 (See Also)
- [API 參考首頁](README.md)
- [格式化輸出 (Format)](format.md)
- [強型別解析 (Parse)](parse.md)
