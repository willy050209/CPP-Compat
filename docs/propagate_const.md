# 常數傳播指標 (compat::propagate_const)

定義於標頭檔 [`<compat/PropagateConst.hpp>`](../include/compat/PropagateConst.hpp)。  
亦可透過總括標頭檔 [`<compat/Compat.hpp>`](../include/compat/Compat.hpp) 引入。  
所屬命名空間：`compat`。

`compat::propagate_const<T>` 是一個常數傳播指標包裝器（Const-Propagating Pointer Wrapper），嚴格對齊 ISO/IEC TS 19568:2017 (C++ Library Fundamentals TS v2 / v3) 中的 `std::experimental::propagate_const` 規範。  
它解決了 C++ 原生指標與智慧指標長久以來的「淺常數性（Shallow Constness）」缺陷，將常數性嚴格傳播到底層指標所指向的物件（深常數性，Deep Constness），並向下相容至 **C++11**。

---

## 核心痛點與設計動機

### 原生指標的「淺常數性」陷阱

在標準 C++ 中，類別內部包含指標或智慧指標（如 `T*`、`std::unique_ptr<T>`）時，指標的常數性與指向目標的常數性是分離的：
- 在 `const` 成員函式中，`this` 指標本身被視為指向 const 物件，因此指標成員變數為 `const`（即指標本身不可重新指向他人）。
- **然而，指標所指向的記憶體並未被 const 保護**！成員函式仍可自由修改目標物件的狀態。

```cpp
struct Widget {
    std::unique_ptr<Impl> pimpl;

    void inspect() const {
        pimpl->modify_state(); // 編譯成功！但 inspect() 語意上為 const，卻意外修改了底層狀態！
    }
};
```

這種淺常數性在實作 **Pimpl 慣用法（Pointer to Implementation）** 時尤為危險，破壞了類別對外承諾的常數正確性（Const-Correctness）。

### `compat::propagate_const` 的解決方案

`compat::propagate_const<T>` 將指標封裝為具有「深常數性傳播」特性的智慧包裝器：
- 當包裝器本身處於 **非常數（non-const）** 存取路徑時，回傳指向非常數目標的指標或引用 (`element_type*` / `element_type&`)。
- 當包裝器本身處於 **常數（const）** 存取路徑時，強制回傳指向常數目標的指標或引用 (`const element_type*` / `const element_type&`)。
- 若在 `const` 上下文中企圖呼叫目標物件的變更方法，編譯器將在**編譯期直接拒絕**並報錯。

---

## 核心亮點與設計特色

1. **跨標準與全編譯器向下相容**：
   * 支援 C++11, C++14, C++17, C++20, C++23, C++26。
   * 在 C++14+ 環境下所有成員方法、觀察器與輔助函式皆支援 `constexpr` 編譯期常數計算。
   * C++20+ 自動支援 `<=>` 三向比較運算子（Three-Way Comparison / Spaceship Operator）。
2. **零開銷抽象 (Zero Overhead Abstraction)**：
   * 記憶體佈局與原始底層指標（`T*` 或 `std::unique_ptr<T>`）完全相同，無任何虛擬函式表（vtable）或額外成員開銷。
   * 所有存取操作均為可內聯（Inline）的直通呼叫，編譯器最佳化後產生與裸指標完全相同的機器碼。
3. **符合 ISO TS 規範的條件式轉換與生命週期安全**：
   * 依據 ISO TS 規範，顯式刪除複製建構子與複製賦值運算子，僅允許移動語意（Move Semantics），防止意外淺複製造成的所有權混亂。
   * 支援條件式隱含轉換至裸指標：僅當底層型別 `T` 本身可隱含轉換為裸指標（如 `U*`）時才提供 `operator element_type*()`，避免非預期脫逸（如 `std::unique_ptr` 不會被隱含轉為裸指標）。
4. **雙軌透傳架構 (Dual-Track)**：
   * 若編譯器標準庫已原生支援 `<experimental/propagate_const>`（如 GCC libstdc++ 在 C++14+），預設自動透明別名至 `std::experimental::propagate_const`，達到零重複與極致標準相容。
   * 在 MSVC、Clang、C++11 或指定 `COMPAT_FORCE_SELF_IMPLEMENTATION` 時，啟用 100% 規格對齊之自研 Fallback 實作。
5. **標準庫相容性**：
   * 提供 `compat::get_underlying` 非成員輔助函式。
   * 於 `namespace std` 中完整特化 `std::hash` 及全套比較函式物件（`equal_to`, `not_equal_to`, `less`, `greater`, `less_equal`, `greater_equal`），可直接作為 `std::unordered_map` 或 `std::map` 的鍵值。

---

## 類別樣板宣告 (Declaration)

```cpp
namespace compat {

    template <typename T>
    class propagate_const;

} // namespace compat
```

### 樣板參數限制
- `T` 必須為**物件指標**（`U*`，其中 `U` 為非陣列之物件型別）或**類別指標型別**（Class Type，且支援解參考運算子 `*` 與 `->`，例如 `std::unique_ptr<U>`、`std::shared_ptr<U>`）。
- 不支援陣列型別或指向函式的指標。

---

## 成員型別 (Member Types)

| 成員型別 | 定義 | 說明 |
| :--- | :--- | :--- |
| `element_type` | `std::remove_reference_t<decltype(*std::declval<T&>())>` | 被包裝指標所指向之目標物件型別 |

---

## 成員函式 (Member Functions)

### 建構與賦值 (Constructors & Assignment)

```cpp
// 1. 預設建構子 (當 T 支援預設建構時可用)
COMPAT_CONSTEXPR_14 propagate_const();

// 2. 移動建構子
constexpr propagate_const(propagate_const&&) = default;

// 3. 複製建構子 (顯式刪除，符合 ISO TS 規範)
propagate_const(const propagate_const&) = delete;

// 4. 相容 propagate_const<U> 移動建構子 (支援隱含或顯式轉換)
template <typename U>
COMPAT_CONSTEXPR_14 propagate_const(propagate_const<U>&& pu);

// 5. 轉發建構子 (由相容之指標或智慧指標物件就地建構)
template <typename U>
COMPAT_CONSTEXPR_14 propagate_const(U&& u);

// 6. 移動賦值運算子
COMPAT_CONSTEXPR_14 propagate_const& operator=(propagate_const&&) = default;

// 7. 複製賦值運算子 (顯式刪除)
propagate_const& operator=(const propagate_const&) = delete;

// 8. 相容 propagate_const<U> 移動賦值運算子
template <typename U>
COMPAT_CONSTEXPR_14 propagate_const& operator=(propagate_const<U>&& pu);

// 9. 轉發賦值運算子
template <typename U>
COMPAT_CONSTEXPR_14 propagate_const& operator=(U&& u);
```

### 觀察器 (Observers)

```cpp
// 檢查被包裝之指標是否非空
COMPAT_NODISCARD explicit COMPAT_CONSTEXPR_14 operator bool() const;

// 取得目標物件之成員存取指標 (常數傳播)
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 const element_type* operator->() const;
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 element_type* operator->();

// 解參考取得目標物件之引用 (常數傳播)
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 const element_type& operator*() const;
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 element_type& operator*();

// 取得目標物件之裸指標 (常數傳播)
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 const element_type* get() const;
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 element_type* get();
```

### 隱含型別轉換 (Implicit Conversions)

僅當 `T` 本身可隱含轉換為裸指標（例如 `T` 為 `U*`）時啟用：

```cpp
// 隱含轉換為常數裸指標 (僅 const 存取路徑)
COMPAT_CONSTEXPR_14 operator const element_type*() const;

// 隱含轉換為非常數裸指標 (僅 non-const 存取路徑)
COMPAT_CONSTEXPR_14 operator element_type*();
```

### 修改器 (Modifiers)

```cpp
// 與另一個 propagate_const 交換底層指標物件
COMPAT_CONSTEXPR_14 void swap(propagate_const& pt) noexcept;
```

---

## 非成員函式 (Non-member Functions)

### 底層指標存取 (get_underlying)

```cpp
// 存取被包裝之常數指標物件
template <typename T>
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 const T&
get_underlying(const propagate_const<T>& pt) noexcept;

// 存取被包裝之非常數指標物件
template <typename T>
COMPAT_NODISCARD COMPAT_CONSTEXPR_14 T&
get_underlying(propagate_const<T>& pt) noexcept;
```

### 交換 (swap)

```cpp
template <typename T>
COMPAT_CONSTEXPR_14 void swap(propagate_const<T>& p1, propagate_const<T>& p2) noexcept;
```

### 關聯性比較運算子 (Relational Operators)

支援與 `nullptr_t`、其他 `propagate_const<U>` 以及底層相容指標型別進行全套比較：

```cpp
// 與 nullptr 比較
template <typename T> bool operator==(const propagate_const<T>& pt, std::nullptr_t);
template <typename T> bool operator==(std::nullptr_t, const propagate_const<T>& pt);
template <typename T> bool operator!=(const propagate_const<T>& pt, std::nullptr_t);
template <typename T> bool operator!=(std::nullptr_t, const propagate_const<T>& pt);

// propagate_const 互比
template <typename T, typename U> bool operator==(const propagate_const<T>& pt, const propagate_const<U>& pu);
template <typename T, typename U> bool operator!=(const propagate_const<T>& pt, const propagate_const<U>& pu);
template <typename T, typename U> bool operator<(const propagate_const<T>& pt, const propagate_const<U>& pu);
template <typename T, typename U> bool operator>(const propagate_const<T>& pt, const propagate_const<U>& pu);
template <typename T, typename U> bool operator<=(const propagate_const<T>& pt, const propagate_const<U>& pu);
template <typename T, typename U> bool operator>=(const propagate_const<T>& pt, const propagate_const<U>& pu);

// 與相容指標型別互比
template <typename T, typename U> bool operator==(const propagate_const<T>& pt, const U& u);
template <typename T, typename U> bool operator!=(const propagate_const<T>& pt, const U& u);
template <typename T, typename U> bool operator<(const propagate_const<T>& pt, const U& u);
template <typename T, typename U> bool operator>(const propagate_const<T>& pt, const U& u);
template <typename T, typename U> bool operator<=(const propagate_const<T>& pt, const U& u);
template <typename T, typename U> bool operator>=(const propagate_const<T>& pt, const U& u);

template <typename T, typename U> bool operator==(const T& t, const propagate_const<U>& pu);
template <typename T, typename U> bool operator!=(const T& t, const propagate_const<U>& pu);
template <typename T, typename U> bool operator<(const T& t, const propagate_const<U>& pu);
template <typename T, typename U> bool operator>(const T& t, const propagate_const<U>& pu);
template <typename T, typename U> bool operator<=(const T& t, const propagate_const<U>& pu);
template <typename T, typename U> bool operator>=(const T& t, const propagate_const<U>& pu);

// C++20 三向比較運算子 (Spaceship Operator)
#if defined(__cpp_impl_three_way_comparison)
template <typename T, typename U> constexpr auto operator<=>(const propagate_const<T>& pt, const propagate_const<U>& pu);
template <typename T, typename U> constexpr auto operator<=>(const propagate_const<T>& pt, const U& u);
#endif
```

---

## 標準庫輔助特化 (Standard Library Specializations)

為確保可無縫整合至 STL 關聯容器，在 `namespace std` 中提供下列特化：

```cpp
namespace std {

    // 雜湊支援 (std::hash)
    template <typename T>
    struct hash<compat::propagate_const<T>>;

    // 函式比較物件
    template <typename T> struct equal_to<compat::propagate_const<T>>;
    template <typename T> struct not_equal_to<compat::propagate_const<T>>;
    template <typename T> struct less<compat::propagate_const<T>>;
    template <typename T> struct greater<compat::propagate_const<T>>;
    template <typename T> struct less_equal<compat::propagate_const<T>>;
    template <typename T> struct greater_equal<compat::propagate_const<T>>;

} // namespace std
```

---

## 範例程式碼 (Examples)

### 範例 1：Pimpl 慣用法中的深常數性防護 (Const-Correct Pimpl)

```cpp
#include <compat/PropagateConst.hpp>
#include <memory>
#include <iostream>

class HeavyEngine {
    struct Impl {
        int throttle = 0;
        void set_throttle(int val) { throttle = val; }
        int get_throttle() const { return throttle; }
    };

    // 使用 compat::propagate_const 包裝 unique_ptr
    compat::propagate_const<std::unique_ptr<Impl>> pimpl;

public:
    HeavyEngine() : pimpl(new Impl()) {}

    // 非常數成員函式：可變更 Impl 狀態
    void accelerate(int step) {
        pimpl->set_throttle(pimpl->get_throttle() + step);
    }

    // 常數成員函式：僅允許唯讀存取
    int get_status() const {
        // pimpl->set_throttle(10); // 編譯錯誤！常數路徑下 pimpl 傳播 const，禁止呼叫 non-const 方法！
        return pimpl->get_throttle(); // 呼叫 const 方法，編譯成功
    }
};

int main() {
    HeavyEngine engine;
    engine.accelerate(50);

    const HeavyEngine& read_only_engine = engine;
    std::cout << "Engine status: " << read_only_engine.get_status() << "\n";
    return 0;
}
```

### 範例 2：裸指標包裝與底層存取 (Raw Pointer & get_underlying)

```cpp
#include <compat/PropagateConst.hpp>
#include <cassert>

void DemonstrateRawPointer() {
    int value = 42;
    compat::propagate_const<int*> ptr(&value);

    // 非常數解參考
    *ptr = 100;
    assert(value == 100);

    // 常數引用檢視
    const compat::propagate_const<int*>& cptr = ptr;
    assert(*cptr == 100);
    // *cptr = 200; // 編譯錯誤！不可對 const int& 賦值

    // 透過 compat::get_underlying 存取底層原始指標
    int* raw = compat::get_underlying(ptr);
    assert(raw == &value);
}
```

---

## 參閱 (See Also)

- [`<compat/Config.hpp>`](config.md)：特性檢測開關與巨集配置手冊。
- [`<compat/Span.hpp>`](span.md)：連續記憶體緩衝區視圖。
- [ISO/IEC TS 19568:2017](https://en.cppreference.com/w/cpp/experimental/propagate_const)：C++ Extensions for Library Fundamentals `std::experimental::propagate_const`。
