# 視圖適配器 (compat::views)

定義於標頭檔 [`<compat/View.hpp>`](file:///D:/program/C++/CPP-Compat/include/compat/View.hpp)（或 [`<compat/Ranges.hpp>`](file:///D:/program/C++/CPP-Compat/include/compat/Ranges.hpp)）。  
所屬命名空間：`compat::views`。

視圖（View）是輕量級、具備常數時間複製與移動複雜度（$O(1)$）且具備惰性求值（Lazy Evaluation）特性的 Range 物件。  
`compat::views` 不僅提供完整 ISO C++20 核心視圖，更領先業界實作了 **ISO C++23 `as_const`** 與 **ISO C++26 `concat` (P2542R8)、`cache_latest` (P3138R5)**，並完整支援 UNIX 管道運算子語法（`operator|`）。

---

## 管道管線機制 (Pipeline Architecture: `operator|`)

所有的視圖適配器均繼承自 `range_adaptor_closure`。您可以將多個操作透過管道運算子流暢串聯：

```cpp
auto pipeline = numbers 
    | compat::views::filter([](int n) { return n % 2 != 0; })
    | compat::views::transform([](int n) { return n * n; })
    | compat::views::take(5);
```

---

## 核心視圖目錄 (Views Inventory)

### 1. 產生與基礎視圖 (Generators & Factories)

| 視圖物件 | 類別樣板型別 | 說明與標準版本 |
| :--- | :--- | :--- |
| `compat::views::empty<T>` | `empty_view<T>` | 產生元素個數永遠為 0 的唯讀空視圖 (C++20)。 |
| `compat::views::single(x)` | `single_view<T>` | 產生僅包含單一元素 `x` 的視圖 (C++20)。僅當 `T` 滿足借用概念時特化為 `borrowed_range`，杜絕臨時物件迭代器懸空。 |
| `compat::views::iota(first, [last])` | `iota_view<W, Bound>` | 產生從 `first` 遞增的整數或迭代器序列，可為有界或無窮序列 (C++20)。 |
| `compat::views::all(r)` | `all_t<R>` | 將可檢視容器轉化為視圖（左值轉為 `subrange`，右值轉為 `owning_view`）(C++20)。 |

### 2. 轉換與過濾視圖 (Transformers & Filters)

| 視圖物件 | 類別樣板型別 | 說明與標準版本 |
| :--- | :--- | :--- |
| `compat::views::filter(pred)` | `filter_view<V, Pred>` | 依據謂詞 `pred` 惰性過濾元素。快取首個相符元素 (C++20)。 |
| `compat::views::transform(func)` | `transform_view<V, F>` | 依據轉換函式 `func` 惰性對每個元素進行投影轉換 (C++20)。 |
| `compat::views::take(n)` | `take_view<V>` | 取出前 `n` 個元素；若不足 `n` 個則取至結尾 (C++20)。 |
| `compat::views::take_while(pred)` | `take_while_view<V, Pred>` | 依據謂詞 `pred` 條件滿足時截取元素，直至首個不滿足處停止 (C++20)。 |
| `compat::views::drop(n)` | `drop_view<V>` | 略過前 `n` 個元素，從第 `n+1` 個元素開始遍歷 (C++20)。 |
| `compat::views::drop_while(pred)` | `drop_while_view<V, Pred>` | 依據謂詞 `pred` 條件滿足時跳過元素，自首個不滿足處開始遍歷 (C++20)。 |
| `compat::views::reverse` | `reverse_view<V>` | 將雙向 Range 反向逆序遍歷 (C++20)。 |

### 3. 現代前沿視圖 (C++23 & C++26 Views)

| 視圖物件 | 類別樣板型別 | 規格草案與核心架構 |
| :--- | :--- | :--- |
| `compat::views::as_const` | `as_const_view<V>` | **ISO C++23 (P2278R4)**：將底層區間視為唯讀，解參考回傳 `const` 參考。 |
| `compat::views::cache_latest` | `cache_latest_view<V>` | **ISO C++26 (P3138R5)**：快取最後解參考之數值。具備嚴格的 `non-propagating-cache` 語意（複製與移動時重置快取槽，杜絕懸空）。 |
| `compat::views::concat(v1, v2, ...)` | `concat_view<Views...>` | **ISO C++26 (P2542R8 / N4984 / Plan 36)**：零堆積多區間串接視圖。<br>- **前綴長度儲存**：扁平化陣列快取前綴大小。<br>- **雙向狀態機**：自動跳過中間空區間，精準支援反向遍歷。<br>- **隨機存取跳轉階梯**：以 $O(1)$ 分支預測切換存取各子視圖下標。 |

---

## 範例程式碼 (Examples)

### 1. 現代管線串接 (Iota + Filter + Transform + Take)

```cpp
#include <compat/Ranges.hpp>
#include <compat/Print.hpp>

int main() {
    using namespace compat::views;

    // 產生 1 到 100，篩選偶數，平方後取前 5 個
    auto results = iota(1, 100)
        | filter([](int n) { return n % 2 == 0; })
        | transform([](int n) { return n * n; })
        | take(5);

    compat::print("Pipeline output: ");
    for (int v : results) {
        compat::print("{} ", v);
    }
    compat::println("");
    // 輸出: Pipeline output: 4 16 36 64 100

    return 0;
}
```

### 2. ISO C++26 `views::concat` 串接異質長度容器

```cpp
#include <compat/Ranges.hpp>
#include <compat/Print.hpp>
#include <vector>
#include <array>
#include <string>

int main() {
    std::vector<int> a = {1, 2};
    std::vector<int> empty_b; // 空區間測試
    std::array<int, 3> c = {3, 4, 5};

    // 串接三個區間
    auto concatenated = compat::views::concat(a, empty_b, c);

    compat::println("Total concatenated size: {}", compat::ranges::size(concatenated)); // 5
    compat::println("Random access element at [3]: {}", concatenated[3]);             // 4

    compat::print("Reverse iteration: ");
    for (auto it = compat::ranges::rbegin(concatenated); it != compat::ranges::rend(concatenated); ++it) {
        compat::print("{} ", *it);
    }
    compat::println("");
    // 輸出: Reverse iteration: 5 4 3 2 1

    return 0;
}
```

---

## 參閱 (See Also)
- [API 參考首頁](README.md)
- [範圍基礎與概念 (Ranges)](ranges.md)
- [受約束範圍演算法 (Algorithms)](algorithms.md)
