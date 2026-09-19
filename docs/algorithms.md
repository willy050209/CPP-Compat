# 受約束範圍演算法 (compat::ranges algorithms)

定義於標頭檔 [`<compat/Algorithm.hpp>`](file:///D:/program/C++/CPP-Compat/include/compat/Algorithm.hpp)（或 [`<compat/Ranges.hpp>`](file:///D:/program/C++/CPP-Compat/include/compat/Ranges.hpp)）。  
所屬命名空間：`compat::ranges`。

`compat::ranges` 演算法家族完全對齊 **ISO C++20 `std::ranges` 演算法**標準規格。  
相較於傳統 C++ `<algorithm>`，範圍演算法具備以下現代特性：
1. **容器直接傳入**：支援直接傳入整個容器 `ranges::sort(vec)`，亦支援傳統迭代器-哨兵配對 `ranges::sort(first, last)`。
2. **Niebloid 呼叫防護**：演算法以常數函式物件形式存在，嚴格隔離引數相依查詢（ADL），杜絕不受控的舊式全域多載劫持。
3. **原生投影支援 (Projections)**：所有演算法均原生支援投影可呼叫物件，可直接傳入成員指標（如 `&Person::age`）或自訂轉換 Lambda。
4. **標準標籤結果型別 (Tagged Results)**：傳回具備描述性成員名稱的結果結構體（如 `in_out_result` 具備 `.in` 與 `.out`），徹底淘汰模稜兩可的 `std::pair`。

---

## 投影與調用架構 (Projections & Invocations)

- `compat::identity`：標準透明恆等式投影函式物件。
- `compat::detail::invoke`：在 C++11 下無縫模擬 C++17 `std::invoke`，支援：
  - 普通函式指標與仿函式（Functors / Lambdas）。
  - 成員函式指標（Pointer to Member Functions, PMF）。
  - 成員變數指標（Pointer to Member Data, PMD）。

```cpp
// 投影範例 1：依照結構體中的 age 欄位進行預設升冪排序
compat::ranges::sort(people, {}, &Person::age);

// 投影範例 2：直接傳入自訂 Comparator Lambda 與投影（完全無歧義匹配）
compat::ranges::sort(users, [](int a, int b) { return a > b; }, &User::score);

// 投影範例 3：find_if 搭配條件 Lambda 與投影
auto it = compat::ranges::find_if(users, [](int s) { return s >= 90; }, &User::score);
```

> [!TIP]
> **多載重載解析保證 (Overload Resolution Guarantee)**  
> 所有支援迭代器對與 Range 重載的演算法（如 `sort`、`find_if` 等），其迭代器版本均嚴格約束 `sentinel_for<S, I> && !range<decay<I>>`。當傳入 `(container, lambda, projection)` 呼叫時，由於 lambda/述詞無法與容器迭代器比對，迭代器重載將精準被 SFINAE 剔除，保證 100% 直達 Range 重載，徹底消除 MSVC/GCC/Clang 上的函式重載歧義（Overload Ambiguity / C2668 / E0308）。

---

## 標籤結果型別 (Tagged Result Types)

| 結果型別 | 結構成員 | 標準對齊別名 |
| :--- | :--- | :--- |
| `in_fun_result<I, F>` | `I in; F fun;` | `for_each_result` |
| `in_out_result<I, O>` | `I in; O out;` | `copy_result`, `copy_n_result`, `copy_backward_result`, `move_result`, `move_backward_result`, `unary_transform_result`, `reverse_copy_result`, `rotate_copy_result`, `unique_copy_result`, `replace_copy_result`, `replace_copy_if_result`, `remove_copy_result`, `remove_copy_if_result`, `partial_sort_copy_result` |
| `in_in_result<I1, I2>` | `I1 in1; I2 in2;` | `mismatch_result`, `swap_ranges_result` |
| `in_in_out_result<I1, I2, O>`| `I1 in1; I2 in2; O out;` | `binary_transform_result`, `merge_result`, `set_union_result`, `set_intersection_result`, `set_symmetric_difference_result` |
| `in_out_out_result<I, O1, O2>`| `I in; O1 out1; O2 out2;` | `partition_copy_result` |
| `min_max_result<T>` | `T min; T max;` | `minmax_result`, `minmax_element_result` |
| `in_found_result<I>` | `I in; bool found;` | `next_permutation_result`, `prev_permutation_result` |
| `in_value_result<I, T>` | `I in; T value;` | `fold_left_with_iter_result`, `fold_left_first_with_iter_result` (C++23) |
| `out_value_result<O, T>` | `O out; T value;` | `iota_result` (C++23) |

---

## 演算法完整清單 (Algorithm Inventory)

### 1. 非修改型序列操作 (Non-modifying Sequence Operations)

| 演算法 | 說明與標準版本 |
| :--- | :--- |
| `all_of(r, pred, [proj])` | 檢查範圍內所有元素是否皆滿足謂詞 (C++20)。 |
| `any_of(r, pred, [proj])` | 檢查範圍內是否至少存在一個元素滿足謂詞 (C++20)。 |
| `none_of(r, pred, [proj])` | 檢查範圍內是否完全沒有元素滿足謂詞 (C++20)。 |
| `for_each(r, f, [proj])` | 對範圍內每一元素依序呼叫可呼叫物件 `f` (C++20)。 |
| `for_each_n(first, n, f, [proj])` | 對從 `first` 開始的前 `n` 個元素呼叫 `f` (C++20)。 |
| `count(r, value, [proj])` | 計算等於指定值 `value` 的元素數量 (C++20)。 |
| `count_if(r, pred, [proj])` | 計算滿足謂詞 `pred` 的元素數量 (C++20)。 |
| `mismatch(r1, r2, [pred], [proj1], [proj2])` | 找出兩個範圍首個不相符的位置，回傳 `mismatch_result` (C++20)。 |
| `equal(r1, r2, [pred], [proj1], [proj2])` | 判斷兩個範圍是否完全相等 (C++20)。 |
| `lexicographical_compare(r1, r2, [comp], ...)` | 依據字典順序比較兩個範圍 (C++20)。 |
| `find(r, value, [proj])` | 尋找第一個等於指定值的元素迭代器 (C++20)。 |
| `find_if(r, pred, [proj])` | 尋找第一個滿足謂詞的元素迭代器 (C++20)。 |
| `find_if_not(r, pred, [proj])` | 尋找第一個不滿足謂詞的元素迭代器 (C++20)。 |
| `find_last(r, value, [proj])` | 尋找最後一個等於指定值的子範圍 `subrange` (C++23)。 |
| `find_last_if(r, pred, [proj])` | 尋找最後一個滿足謂詞的子範圍 `subrange` (C++23)。 |
| `find_last_if_not(r, pred, [proj])` | 尋找最後一個不滿足謂詞的子範圍 `subrange` (C++23)。 |
| `find_end(r1, r2, [pred], ...)` | 尋找最後一次出現子序列 `r2` 的位置 (C++20)。 |
| `find_first_of(r1, r2, [pred], ...)` | 尋找 `r1` 中第一個出現在 `r2` 集合內的元素 (C++20)。 |
| `adjacent_find(r, [pred], [proj])` | 尋找首對相鄰且相等的元素 (C++20)。 |
| `search(r1, r2, [pred], [proj1], [proj2])` | 於範圍 `r1` 中搜尋子範圍 `r2` 首度出現的位置，回傳 `subrange` (C++20)。 |
| `search_n(r, count, value, [pred], [proj])` | 搜尋連續出現 `count` 次指定值的子範圍 (C++20)。 |
| `contains(r, value, [proj])` | 檢查範圍是否包含指定元素 (C++23)。 |
| `contains_subrange(r1, r2, [pred], ...)` | 檢查範圍是否包含完整子序列 `r2` (C++23)。 |
| `starts_with(r1, r2, [pred], ...)` | 檢查範圍 `r1` 是否以 `r2` 作為開頭 (C++23)。 |
| `ends_with(r1, r2, [pred], ...)` | 檢查範圍 `r1` 是否以 `r2` 作為結尾 (C++23)。 |

### 2. 摺疊歸約操作 (Fold Operations: C++23)

| 演算法 | 說明與標準版本 |
| :--- | :--- |
| `fold_left(r, init, f)` | 從左側開始以初始值進行摺疊歸約計算 (C++23)。 |
| `fold_left_first(r, f)` | 從左側首元素開始摺疊歸約，回傳 `compat::optional<T>` (C++23)。 |
| `fold_right(r, init, f)` | 從右側開始摺疊歸約計算 (C++23)。 |
| `fold_right_last(r, f)` | 從右側末元素開始摺疊歸約，回傳 `compat::optional<T>` (C++23)。 |
| `fold_left_with_iter(r, init, f)` | 摺疊歸約並同時回傳結束迭代器，回傳 `fold_left_with_iter_result` (C++23)。 |
| `fold_left_first_with_iter(r, f)` | 首元素摺疊並同時回傳結束迭代器 (C++23)。 |

### 3. 修改型序列操作 (Modifying Sequence Operations)

| 演算法 | 說明與標準版本 |
| :--- | :--- |
| `copy(r, out)` / `copy_if` / `copy_n` / `copy_backward` | 複製範圍元素至輸出迭代器 (C++20)。 |
| `move(r, out)` / `move_backward` | 移動範圍元素至輸出迭代器 (C++20)。 |
| `fill(r, value)` / `fill_n(first, n, value)` | 以指定數值填滿範圍 (C++20)。 |
| `transform(r, out, f, [proj])`<br>`transform(r1, r2, out, f, [proj1], [proj2])` | 一元或二元元素轉換並輸出至目標迭代器 (C++20)。 |
| `generate(r, gen)` / `generate_n(first, n, gen)` | 透過生成器反覆賦值 (C++20)。 |
| `remove(r, value, [proj])` / `remove_if` | 邏輯移除元素，回傳指向新結尾的 `subrange` (C++20)。 |
| `remove_copy(r, out, value, [proj])` / `remove_copy_if` | 複製時排除符合條件的元素至目標輸出 (C++20)。 |
| `replace(r, old_val, new_val, [proj])` / `replace_if` | 將符合條件的元素替換為新值 (C++20)。 |
| `replace_copy(r, out, old_val, new_val, [proj])` / `replace_copy_if` | 複製時將符合條件的元素替換至目標輸出 (C++20)。 |
| `swap_ranges(r1, r2)` | 交換兩個範圍對應位置的元素 (C++20)。 |
| `reverse(r)` / `reverse_copy(r, out)` | 原地反轉範圍元素，或反轉複製 (C++20)。 |
| `rotate(r, middle)` / `rotate_copy(r, middle, out)` | 迴轉範圍使 `middle` 成為新首元素 (C++20)。 |
| `unique(r, [comp], [proj])` / `unique_copy(r, out, ...)` | 原地消除相鄰重複元素，或複製時去重 (C++20)。 |
| `shift_left(r, n)` / `shift_right(r, n)` | 平移元素位置，回傳剩餘有效區間之 `subrange` (C++23)。 |
| `shuffle(r, g)` | 使用均勻隨機亂數引擎對範圍進行洗牌重排 (C++20)。 |
| `sample(r, out, n, g)` | 從母體範圍中隨機抽樣 `n` 個元素 (C++20)。 |

### 4. 分割與排序操作 (Partitioning & Sorting Operations)

| 演算法 | 說明與標準版本 |
| :--- | :--- |
| `is_partitioned(r, pred, [proj])` | 檢查範圍是否已按謂詞完成二元分割 (C++20)。 |
| `partition(r, pred, [proj])` | 原地重排使符合謂詞之元素置於前半部 (C++20)。 |
| `partition_copy(r, out_true, out_false, pred, [proj])` | 將二元分割結果分別複製至兩目標輸出 (C++20)。 |
| `stable_partition(r, pred, [proj])` | 穩定分割，保持元素之原始相對順序 (C++20)。 |
| `partition_point(r, pred, [proj])` | 於已分割範圍中以二分搜尋定位分界點 (C++20)。 |
| `is_sorted(r, [comp], [proj])` | 檢查範圍是否已排序 (C++20)。 |
| `is_sorted_until(r, [comp], [proj])` | 找出保持排序的最大前綴結尾迭代器 (C++20)。 |
| `sort(r, [comp], [proj])` | 對隨機存取範圍進行極速排序（預設遞增）(C++20)。 |
| `stable_sort(r, [comp], [proj])` | 穩定排序，保持相等元素的相對順序 (C++20)。 |
| `partial_sort(r, middle, [comp], [proj])` | 部分排序，使前 `middle - first` 個元素就定位 (C++20)。 |
| `partial_sort_copy(r_in, r_out, [comp], ...)` | 部分排序並複製結果至目標範圍 (C++20)。 |
| `nth_element(r, nth, [comp], [proj])` | 重排範圍使第 `nth` 個元素就定位，其餘元素依大小分布於兩側 (C++20)。 |

### 5. 二分搜尋操作 (Binary Search Operations: $O(\log N)$)

| 演算法 | 說明與標準版本 |
| :--- | :--- |
| `lower_bound(r, value, [comp], [proj])` | 尋找首個不小於指定值的元素迭代器 (C++20)。 |
| `upper_bound(r, value, [comp], [proj])` | 尋找首個大於指定值的元素迭代器 (C++20)。 |
| `equal_range(r, value, [comp], [proj])` | 同時找出相等於指定值的子範圍 `subrange` (C++20)。 |
| `binary_search(r, value, [comp], [proj])` | 快速判斷指定值是否存在於已排序範圍中 (C++20)。 |

### 6. 二項堆積操作 (Heap Operations)

| 演算法 | 說明與標準版本 |
| :--- | :--- |
| `is_heap(r, [comp], [proj])` | 檢查範圍是否為最大堆積 (C++20)。 |
| `is_heap_until(r, [comp], [proj])` | 尋找維持堆積性質的最大前綴結尾迭代器 (C++20)。 |
| `make_heap(r, [comp], [proj])` | 將隨機存取範圍重組成堆積結構 (C++20)。 |
| `push_heap(r, [comp], [proj])` | 將末端新加入的元素插入堆積結構中 (C++20)。 |
| `pop_heap(r, [comp], [proj])` | 將堆積頂端最大元素移至末端 (C++20)。 |
| `sort_heap(r, [comp], [proj])` | 將堆積轉換為升冪已排序範圍 (C++20)。 |

### 7. 集合與合併操作 (Sets & Merges)

| 演算法 | 說明與標準版本 |
| :--- | :--- |
| `includes(r1, r2, [comp], ...)` | 判斷已排序範圍 `r1` 是否完全包含已排序範圍 `r2` (C++20)。 |
| `set_union(r1, r2, out, [comp], ...)` | 計算兩已排序集合之聯集 (C++20)。 |
| `set_intersection(r1, r2, out, [comp], ...)` | 計算兩已排序集合之交集 (C++20)。 |
| `set_difference(r1, r2, out, [comp], ...)` | 計算兩已排序集合之差集 ($r_1 \setminus r_2$) (C++20)。 |
| `set_symmetric_difference(r1, r2, out, ...)` | 計算兩已排序集合之對稱差集 ($r_1 \triangle r_2$) (C++20)。 |
| `merge(r1, r2, out, [comp], ...)` | 合併兩已排序範圍至輸出目標 (C++20)。 |
| `inplace_merge(r, middle, [comp], ...)` | 原地合併兩連續相鄰的已排序子範圍 (C++20)。 |

### 8. 排列操作 (Permutations)

| 演算法 | 說明與標準版本 |
| :--- | :--- |
| `is_permutation(r1, r2, [pred], ...)` | 判斷範圍 `r2` 是否為 `r1` 之排列重組 (C++20)。 |
| `next_permutation(r, [comp], [proj])` | 變換為字典順序之下一個排列，回傳 `next_permutation_result` (C++20)。 |
| `prev_permutation(r, [comp], [proj])` | 變換為字典順序之上一個排列，回傳 `prev_permutation_result` (C++20)。 |

### 9. 數值與隨機生成 (Numeric & Random)

| 演算法 | 說明與標準版本 |
| :--- | :--- |
| `iota(r, value)` | 依序以遞增數值填滿範圍，回傳 `out_value_result` (C++23)。 |
| `generate_random(r, gen)` | 以均勻隨機亂數引擎依序填滿範圍元素 (C++26)。 |

### 10. 未初始化記憶體演算法 (Specialized Uninitialized Memory: `<compat/Memory.hpp>`)

| 函式 | 說明與標準版本 |
| :--- | :--- |
| `construct_at(p, args...)` | 在指標 `p` 處就地呼叫建構子配置物件 (C++20)。 |
| `destroy_at(p)` | 呼叫指標 `p` 處物件之解構子 (C++20)。 |
| `destroy(r)` / `destroy_n(first, n)` | 解構範圍內的所有物件 (C++20)。 |
| `uninitialized_copy(r_in, r_out)` / `uninitialized_copy_n` | 複製範圍元素至未初始化記憶體區間（含例外 Rollback 安全防護）(C++20)。 |
| `uninitialized_fill(r, value)` / `uninitialized_fill_n` | 以值填滿未初始化記憶體區間 (C++20)。 |
| `uninitialized_move(r_in, r_out)` / `uninitialized_move_n` | 移動元素至未初始化記憶體區間 (C++20)。 |
| `uninitialized_default_construct(r)` / `_n` | 以預設建構填滿未初始化記憶體 (C++20)。 |
| `uninitialized_value_construct(r)` / `_n` | 以值建構填滿未初始化記憶體 (C++20)。 |

### 11. 極值與截斷操作 (Minimum / Maximum & Clamp)

| 演算法 | 說明與標準版本 |
| :--- | :--- |
| `min(r, [comp], [proj])` / `min(a, b, [comp], [proj])` | 取得範圍或二者間之最小值 (C++20)。 |
| `max(r, [comp], [proj])` / `max(a, b, [comp], [proj])` | 取得範圍或二者間之最大值 (C++20)。 |
| `minmax(r, [comp], [proj])` | 同時取得最小值與最大值，回傳 `minmax_result` (C++20)。 |
| `min_element(r, [comp], [proj])` | 尋找指向最小值的元素迭代器 (C++20)。 |
| `max_element(r, [comp], [proj])` | 尋找指向最大值的元素迭代器 (C++20)。 |
| `minmax_element(r, [comp], [proj])` | 同時尋找最小與最大值迭代器，回傳 `minmax_element_result` (C++20)。 |
| `clamp(val, lo, hi, [comp], [proj])` | 將數值截斷限制於 `[lo, hi]` 區間內 (C++20)。 |

---

## 範例程式碼 (Example)

```cpp
#include <compat/Algorithm.hpp>
#include <compat/Print.hpp>
#include <vector>
#include <string>

struct User {
    std::string name;
    int score;
};

int main() {
    std::vector<User> users = {
        {"Alice", 85},
        {"Bob", 92},
        {"Charlie", 78},
        {"Diana", 95}
    };

    // 1. 使用成員指標投影依成績降序排序
    compat::ranges::sort(users, [](int a, int b) { return a > b; }, &User::score);

    compat::println("Top Scorer: {} ({})", users.front().name, users.front().score);

    // 2. 尋找特定分數門檻的玩家
    auto it = compat::ranges::find_if(users, [](int s) { return s < 80; }, &User::score);
    if (it != users.end()) {
        compat::println("User below 80 points: {}", it->name);
    }

    // 3. 極值與摺疊 (C++23)
    std::vector<int> numbers = {10, 25, 3, 44, 18};
    auto mm = compat::ranges::minmax(numbers);
    compat::println("Min: {}, Max: {}", mm.min, mm.max);

    int sum = compat::ranges::fold_left(numbers, 0, [](int acc, int x) { return acc + x; });
    compat::println("Total Sum: {}", sum);

    return 0;
}
```

---

## 參閱 (See Also)
- [API 參考首頁](README.md)
- [範圍基礎與概念 (Ranges)](ranges.md)
- [視圖適配器 (Views)](views.md)
