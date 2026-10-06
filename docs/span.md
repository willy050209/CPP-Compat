# 連續緩衝區切片視圖 (compat::span)

定義於標頭檔 [`<compat/Span.hpp>`](file:///D:/P/CPP/CPP-Compat/include/compat/Span.hpp)。  
所屬命名空間：`compat`。

`compat::span<T, Extent>` 是一個非擁有型（Non-owning）的連續記憶體緩衝區視圖（Contiguous Sequence View），對齊 ISO C++20 `std::span` 標準規範。  
它提供了零記憶體複製、型別安全且向下相容至 **C++11** 的陣列與連續容器封裝，並透過進階的邊界安全箝位防禦機制，杜絕緩衝區溢位（Buffer Overrun）與無號數下溢問題。

---

## 核心亮點與設計特色

1. **跨標準與全編譯器向下相容**：
   * 支援 C++11, C++14, C++17, C++20, C++23, C++26。
   * 在 C++14+ 環境下所有成員方法與子區間切片皆支援 `constexpr` 編譯期常數計算。
   * 在 C++20 環境下原生支援與 `std::span` 的隱含雙向互轉（Implicit Two-Way Interoperability）。
2. **強健的子視圖邊界計算與自動安全箝位 (Robust Bounds Clamping)**：
   * 標準 ISO `std::span::subspan(offset, count)` 規範在 `count > size() - offset` 時屬於未定義行為（UB）或觸發終止斷言。
   * `compat::span::subspan` 具備雙重安全防護：
     - 精確支援 `offset == size()` 合法末端空區間（回傳指向末端的 0 長度 span，而非空指標）。
     - 若 `offset > size()`，安全回退為末端 0 長度切片，徹底避免野指標計算。
     - 若 `count == dynamic_extent` 或傳入的 `count` 超出可用剩餘元素（`count > size() - offset`），自動安全夾取至最大可用長度 `max_count`，從根本消除緩衝區溢位隱患。
   * `first(count)` 與 `last(count)` 同樣具備自動上限夾取防護，傳入過大長度時自動截取至 `size()`。
3. **完整支援位元組視圖轉換 (`as_bytes` / `as_writable_bytes`)**：
   * 提供抽象型別 `compat::byte`（C++17+ 對齊 `std::byte`，C++11/14 提供型別安全的 `enum class byte : unsigned char`）。
   * 提供 `compat::as_bytes` 與 `compat::as_writable_bytes` 輔助函式，支援將任意型別之連續緩衝區安全檢視為唯讀或可寫位元組序列。

---

## 類別樣板宣告 (Declaration)

```cpp
namespace compat {

    COMPAT_INLINE_VAR constexpr std::size_t dynamic_extent = static_cast<std::size_t>(-1);

    template <typename T, std::size_t Extent = dynamic_extent>
    class span;

} // namespace compat
```

---

## 成員型別 (Member Types)

| 成員型別 | 定義 |
| :--- | :--- |
| `element_type` | `T` |
| `value_type` | `std::remove_cv_t<T>` |
| `size_type` | `std::size_t` |
| `difference_type` | `std::ptrdiff_t` |
| `pointer` | `T*` |
| `const_pointer` | `const T*` |
| `reference` | `T&` |
| `const_reference` | `const T&` |
| `iterator` | `T*` (隨機存取連續迭代器) |
| `const_iterator` | `const T*` |
| `reverse_iterator` | `std::reverse_iterator<iterator>` |
| `const_reverse_iterator` | `std::reverse_iterator<const_iterator>` |

---

## 成員常數 (Member Constants)

| 成員常數 | 定義 |
| :--- | :--- |
| `extent` | `static constexpr std::size_t extent = Extent;` |

---

## 成員函式 (Member Functions)

### 建構與賦值 (Constructors & Assignment)

```cpp
// 1. 預設建構子 (建立空視圖)
COMPAT_CONSTEXPR_14 span() noexcept;

// 2. 起始指標與元素計數
COMPAT_CONSTEXPR_14 span(pointer ptr, size_type count) noexcept;

// 3. 起始指標與末端指標區間 [first, last)
COMPAT_CONSTEXPR_14 span(pointer first, pointer last) noexcept;

// 4. 原生 C 靜態陣列引用
template <std::size_t N>
COMPAT_CONSTEXPR_14 span(element_type (&arr)[N]) noexcept;

// 5. std::array (非 const 與 const)
template <std::size_t N>
COMPAT_CONSTEXPR_14 span(std::array<value_type, N>& arr) noexcept;
template <std::size_t N>
COMPAT_CONSTEXPR_14 span(const std::array<value_type, N>& arr) noexcept;

// 6. 連續記憶體容器 (如 std::vector, std::string)
template <typename Container>
COMPAT_CONSTEXPR_14 span(Container& cont) noexcept;
template <typename Container>
COMPAT_CONSTEXPR_14 span(const Container& cont) noexcept;

// 7. 型別轉換建構子 (允許 non-const 轉換為 const 視圖)
template <typename OtherType, std::size_t OtherExtent>
COMPAT_CONSTEXPR_14 span(const span<OtherType, OtherExtent>& other) noexcept;

// 8. C++20 std::span 雙向互轉 (當編譯器支援 std::span 時)
constexpr span(std::span<element_type, Extent> s) noexcept;
constexpr operator std::span<element_type, Extent>() const noexcept;
```

---

### 元素存取 (Element Access)

```cpp
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 reference operator[](size_type idx) const noexcept;
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 reference front() const noexcept;
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 reference back() const noexcept;
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 pointer data() const noexcept;
```

---

### 觀察器 (Observers)

```cpp
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 size_type size() const noexcept;
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 size_type size_bytes() const noexcept;
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool empty() const noexcept;
```

---

### 迭代器 (Iterators)

```cpp
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 iterator begin() const noexcept;
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 iterator end() const noexcept;
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 const_iterator cbegin() const noexcept;
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 const_iterator cend() const noexcept;
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 reverse_iterator rbegin() const noexcept;
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 reverse_iterator rend() const noexcept;
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 const_reverse_iterator crbegin() const noexcept;
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 const_reverse_iterator crend() const noexcept;
```

---

### 子視圖切片 (Subviews)

```cpp
// 執行期動態切片 (含自動邊界防禦與上限夾取)
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 span<element_type, dynamic_extent>
subspan(size_type offset, size_type count = dynamic_extent) const noexcept;

// 前綴與後綴切片
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 span<element_type, dynamic_extent>
first(size_type count) const noexcept;

COMPAT_NODISCARD COMPAT_CONSTEXPR_14 span<element_type, dynamic_extent>
last(size_type count) const noexcept;

// 編譯期靜態樣板切片
template <std::size_t Offset, std::size_t Count = dynamic_extent>
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 auto subspan() const noexcept;

template <std::size_t Count>
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 span<element_type, Count> first() const noexcept;

template <std::size_t Count>
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 span<element_type, Count> last() const noexcept;
```

---

## 非成員函式 (Non-member Functions)

```cpp
// 轉換為唯讀位元組視圖
template <typename T, std::size_t Extent>
COMPAT_NODISCARD COMPAT_CONSTEXPR_14
span<const byte, ...> as_bytes(span<T, Extent> s) noexcept;

// 轉換為可寫位元組視圖
template <typename T, std::size_t Extent>
COMPAT_NODISCARD COMPAT_CONSTEXPR_14
span<byte, ...> as_writable_bytes(span<T, Extent> s) noexcept;
```

---

## 使用範例 (Examples)

### 1. 基礎用法與函式參數借用

```cpp
#include <compat/Span.hpp>
#include <vector>
#include <iostream>

void print_buffer(compat::span<const int> buf) {
    for (int v : buf) {
        std::cout << v << " ";
    }
    std::cout << "\n";
}

int main() {
    int arr[] = {1, 2, 3, 4, 5};
    std::vector<int> vec = {10, 20, 30};

    print_buffer(arr); // 原生陣列自動推導長度
    print_buffer(vec); // std::vector 自動借用連續緩衝區
}
```

### 2. 安全子區間切片與上限夾取 (Subspan Clamping)

```cpp
#include <compat/Span.hpp>
#include <cassert>

void test_clamping() {
    int data[] = {10, 20, 30, 40, 50};
    compat::span<int> sp(data);

    // 1. 標準切片: [1] 開始取 3 個元素 -> {20, 30, 40}
    auto sub1 = sp.subspan(1, 3);
    assert(sub1.size() == 3);

    // 2. 末端合法空切片: offset == size() -> 長度為 0 的空 span
    auto sub_end = sp.subspan(5);
    assert(sub_end.empty());
    assert(sub_end.data() == sp.data() + 5);

    // 3. 安全夾取: 剩餘元素僅 2 個 ({40, 50})，即使傳入 count = 100 亦安全夾取至 2
    auto clamped = sp.subspan(3, 100);
    assert(clamped.size() == 2);
    assert(clamped[0] == 40 && clamped[1] == 50);

    // 4. first 與 last 夾取:
    auto f = sp.first(999);
    assert(f.size() == 5);
}
```
