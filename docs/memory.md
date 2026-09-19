# 未初始化記憶體演算法 (compat::ranges memory)

定義於標頭檔 [`<compat/Memory.hpp>`](file:///D:/program/C++/CPP-Compat/include/compat/Memory.hpp)。  
所屬命名空間：`compat::ranges`。

`compat::ranges` 記憶體演算法家族完全對齊 **ISO C++20 `std::ranges` Specialized Memory Algorithms** 標準規格。  
本模組提供針對尚未構造物件的未初始化原始記憶體（Uninitialized Raw Memory）進行就地構造、解構、複製、移動與填滿的高效能泛型演算法。

---

## 核心特性與保證 (Core Features & Guarantees)

1. **強例外安全回滾保證 (Strong Exception Rollback Safety)**：  
   標準規定：在構造過程中若任何物件的建構子拋出例外，必須**依相反順序完全解構所有已成功構造的暫存物件**，釋放資源後再重新向外傳遞例外。  
   在 C++11/14 Fallback 下，本庫實作了專屬的 RAII `rollback_guard` 交易式防護機制，百分之百達成強例外安全合約。
2. **Niebloid 呼叫防護**：  
   所有函式皆以常數函式物件（Customization Point Objects, CPO）形式定義，完全隔離引數相依查詢（ADL），杜絕不受控的全域多載劫持。
3. **Range 與迭代器對雙重介面**：  
   支援直接傳入整個容器/區間（如 `ranges::destroy(vec)`），亦支援傳統迭代器-哨兵配對（如 `ranges::destroy(first, last)`）。
4. **雙軌透傳架構**：  
   在 C++20 環境下透明透傳至原生 `std::ranges`；在 C++11/14/17 或 `-DCOMPAT_FORCE_SELF_IMPLEMENTATION=ON` 下啟用零第三方相依的純自研實作。

---

## 標籤結果型別 (Tagged Result Types)

| 結果型別 | 結構成員 | 標準對齊別名 | 說明 |
| :--- | :--- | :--- | :--- |
| `in_out_result<I, O>` | `I in; O out;` | `uninitialized_copy_result`, `uninitialized_move_result` | 同時回傳輸入來源讀取結束位置與目標寫入結束位置 |
| `in_out_result<I, O>` | `I in; O out;` | `uninitialized_copy_n_result`, `uninitialized_move_n_result` | 以數量 `n` 限制時的輸出結果 |

---

## 演算法完整清單 (Function Inventory)

### 1. 物件構造與解構 (Object Lifecycle: construct_at & destroy_at)

```cpp
template <typename T, typename... Args>
COMPAT_CONSTEXPR_20 T* construct_at(T* p, Args&&... args);

template <typename T>
COMPAT_CONSTEXPR_20 void destroy_at(T* p) noexcept;
```

- **`construct_at(p, args...)`**：  
  在指標 `p` 所指向的未初始化記憶體位址處，以就地 placement new（`::new (static_cast<void*>(p)) T(std::forward<Args>(args)...)`）直接建構型別 `T` 物件，並回傳指向該物件的指標 `p`。
- **`destroy_at(p)`**：  
  若 `p` 不為空指標，直接呼叫物件之解構子（`p->~T()`）。

---

### 2. 範圍物件解構 (Range Destruction: destroy & destroy_n)

```cpp
template <typename I, typename S>
COMPAT_CONSTEXPR_20 I destroy(I first, S last) noexcept;

template <typename R>
COMPAT_CONSTEXPR_20 borrowed_iterator_t<R> destroy(R&& r) noexcept;

template <typename I>
COMPAT_CONSTEXPR_20 I destroy_n(I first, std::size_t n) noexcept;
```

- **語意**：依序對區間 `[first, last)` 或前 `n` 個元素呼叫 `destroy_at(std::addressof(*it))`。
- **複雜度**：$O(N)$ 次解構子呼叫。
- **例外保證**：`noexcept`，解構子保證不拋出例外。

---

### 3. 未初始化複製 (Uninitialized Copy: uninitialized_copy & uninitialized_copy_n)

```cpp
template <typename I1, typename S1, typename O, typename S2>
uninitialized_copy_result<I1, O> 
uninitialized_copy(I1 first, S1 last, O result, S2 result_last);

template <typename R1, typename R2>
uninitialized_copy_result<borrowed_iterator_t<R1>, borrowed_iterator_t<R2>> 
uninitialized_copy(R1&& in_range, R2&& out_range);

template <typename I, typename O, typename S2>
uninitialized_copy_n_result<I, O> 
uninitialized_copy_n(I first, std::size_t n, O result, S2 result_last);
```

- **語意**：將來源區間中的元素逐一拷貝建構至 `result` 指向的未初始化記憶體空間。
- **回滾機制**：若拷貝建構過程中拋出例外，自動依序解構已在目標空間構造好的物件。

---

### 4. 未初始化填滿 (Uninitialized Fill: uninitialized_fill & uninitialized_fill_n)

```cpp
template <typename I, typename S, typename T>
I uninitialized_fill(I first, S last, const T& value);

template <typename R, typename T>
borrowed_iterator_t<R> uninitialized_fill(R&& r, const T& value);

template <typename I, typename T>
I uninitialized_fill_n(I first, std::size_t n, const T& value);
```

- **語意**：在未初始化區間以常數參考 `value` 呼叫複製建構子填滿整個區間。
- **回滾機制**：具備強例外安全保證。

---

### 5. 未初始化移動 (Uninitialized Move: uninitialized_move & uninitialized_move_n)

```cpp
template <typename I1, typename S1, typename O, typename S2>
uninitialized_move_result<I1, O> 
uninitialized_move(I1 first, S1 last, O result, S2 result_last);

template <typename R1, typename R2>
uninitialized_move_result<borrowed_iterator_t<R1>, borrowed_iterator_t<R2>> 
uninitialized_move(R1&& in_range, R2&& out_range);

template <typename I, typename O, typename S2>
uninitialized_move_n_result<I, O> 
uninitialized_move_n(I first, std::size_t n, O result, S2 result_last);
```

- **語意**：將來源區間元素透過 `std::move(*first)` 移動建構至目標未初始化記憶體。
- **回滾機制**：若移動建構子拋出例外，解構目標端所有已建構物件。

---

### 6. 未初始化預設/值建構 (Default & Value Construction)

```cpp
// 預設建構 (Default Construction: ::new (p) T)
template <typename I, typename S>
I uninitialized_default_construct(I first, S last);
template <typename R>
borrowed_iterator_t<R> uninitialized_default_construct(R&& r);
template <typename I>
I uninitialized_default_construct_n(I first, std::size_t n);

// 值建構 (Value Construction: ::new (p) T())
template <typename I, typename S>
I uninitialized_value_construct(I first, S last);
template <typename R>
borrowed_iterator_t<R> uninitialized_value_construct(R&& r);
template <typename I>
I uninitialized_value_construct_n(I first, std::size_t n);
```

- **差別**：
  - `default_construct`：對於純純量型別（如 `int`, `float`）不進行零初始化（保留記憶體垃圾值），適用極致效能。
  - `value_construct`：對基本型別執行零初始化（Zero-initialization, `int() == 0`）。

---

## 範例程式碼 (Example)

```cpp
#include <compat/Memory.hpp>
#include <compat/Print.hpp>
#include <vector>
#include <string>

int main() {
    // 1. 分配未初始化原始緩衝區
    alignas(alignof(std::string)) char buffer[sizeof(std::string) * 3];
    std::string* pool = reinterpret_cast<std::string*>(buffer);

    // 2. 使用 construct_at 原地建構單一物件
    compat::ranges::construct_at(&pool[0], "First Object");
    compat::println("Created: {}", pool[0]);

    // 3. 單一物件解構
    compat::ranges::destroy_at(&pool[0]);

    // 4. 批量拷貝建構至原始記憶體 (具備交易式回滾保護)
    std::vector<std::string> source = {"Alpha", "Beta", "Gamma"};
    auto res = compat::ranges::uninitialized_copy(source.begin(), source.end(), pool, pool + 3);

    for (std::size_t i = 0; i < 3; ++i) {
        compat::println("Pool[{}]: {}", i, pool[i]);
    }

    // 5. 批量解構整個緩衝區
    compat::ranges::destroy_n(pool, 3);
    compat::println("All pool objects successfully destroyed.");

    return 0;
}
```

---

## 參閱 (See Also)
- [API 參考首頁](README.md)
- [受約束範圍演算法 (Algorithms)](algorithms.md)
- [範圍基礎與概念 (Ranges)](ranges.md)
