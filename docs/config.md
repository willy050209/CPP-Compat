# 特性檢測與配置 (Config API Reference)

定義於標頭檔 [`<compat/Config.hpp>`](file:///D:/program/C++/CPP-Compat/include/compat/Config.hpp)。  
所屬命名空間：`compat`, `compat::detail`。

`Config.hpp` 是整個 `CPP-Compat` 函式庫的核心基礎架構標頭，負責編譯器標準探測、標準庫特徵檢測巨集（Feature Test Macros）、雙軌執行模式切換開關、例外安全封裝以及未定義行為（UB）防護終止函式。

---

## 語言標準層級常數 (Standard Dialect Constants)

| 常數巨集 | 數值 | 說明 |
| :--- | :--- | :--- |
| `COMPAT_CXX_11` | `201103L` | C++11 標準基準值 |
| `COMPAT_CXX_14` | `201402L` | C++14 標準基準值 |
| `COMPAT_CXX_17` | `201703L` | C++17 標準基準值 |
| `COMPAT_CXX_20` | `202002L` | C++20 標準基準值 |
| `COMPAT_CXX_23` | `202302L` | C++23 標準基準值 |
| `COMPAT_CXX_26` | `202602L` | C++26 工作草案基準值 |
| `COMPAT_CPLUSPLUS` | 編譯器宏 | 統整 MSVC `_MSVC_LANG` 與 GCC/Clang `__cplusplus`，精確反映真實啟用的語言標準 |

---

## 雙軌編譯模式切換巨集 (Mode Control Switches)

使用者可在專案的 CMake 配置或編譯參數中指定以下巨集，精確控制要使用原生標準庫或自研 Fallback：

| 編譯選項 / 巨集 | 預設值 | 描述與行為 |
| :--- | :--- | :--- |
| `COMPAT_FORCE_SELF_IMPLEMENTATION`<br>（別名：`COMPAT_FORCE_FALLBACK`） | 未定義 | **強制啟用自研 Fallback 模式**。<br>即便當前編譯器支援現代標準（如 C++20/23），亦強制停用原生 STL 映射，全面啟用純自研降階實作。適合用於跨編譯器一致性測試或除錯。 |
| `COMPAT_FORCE_STD_IMPLEMENTATION` | 未定義 | **強制啟用原生 std 模式**。<br>若編譯器或標準庫版本不足以支援該功能，編譯器將觸發 `#error` 中斷編譯，防止意外降階。 |
| *兩者同時定義* | - | 觸發編譯期 `#error`（互斥設定）。 |

---

## 特性檢測巨集 (Feature Detection Macros)

在預設（自動探測）模式下，`CPP-Compat` 會自動探測標準庫 `<version>` 及 `__cpp_lib_*` 巨集，並設置以下狀態旗標（`1` 表示支援並使用原生標準庫，`0` 表示啟用自研 Fallback）：

| 特性巨集 | 檢測依據與標準版本 | 說明 |
| :--- | :--- | :--- |
| `COMPAT_HAS_STD_OPTIONAL` | `__has_include(<optional>)` (C++17) | 是否支援原生 `std::optional` / `std::nullopt` |
| `COMPAT_HAS_STD_STRING_VIEW` | `__cpp_lib_string_view >= 201606L` (C++17) | 是否支援原生 `std::string_view` |
| `COMPAT_HAS_STD_VARIANT` | `__cpp_lib_variant >= 201606L` (C++17) | 是否支援原生 `std::variant` |
| `COMPAT_HAS_STD_FORMAT` | `__cpp_lib_format >= 201907L` (C++20)<br>（libstdc++ 需 $\ge 14$） | 是否支援原生 `std::format`（GCC 13 因未實作 P1868R2 東亞寬度且會截斷 UTF-8 字節，自動降階使用高品質自研實作） |
| `COMPAT_HAS_STD_EXPECTED` | `__cpp_lib_expected >= 202202L` (C++23) | 是否支援原生 `std::expected` / `std::unexpected` |
| `COMPAT_HAS_STD_PRINT` | `__cpp_lib_print >= 202207L` (C++23) | 是否支援原生 `std::print` / `std::println` |
| `COMPAT_HAS_STD_RANGES` | `__cpp_lib_ranges >= 201911L` (C++20) | 是否支援原生 `std::ranges` 核心基礎設施 |
| `COMPAT_HAS_STD_VIEWS_AS_CONST` | `__cpp_lib_ranges_as_const >= 202207L` (C++23) | 是否支援原生 `std::views::as_const` (P2278R4) |
| `COMPAT_HAS_STD_CONSTANT_RANGE` | `__cpp_lib_ranges_constant_range >= 202302L` (C++26) | 是否支援原生 `std::ranges::constant_range` (P2728R6) |
| `COMPAT_HAS_STD_VIEWS_CONCAT` | `__cpp_lib_ranges_concat >= 202403L` (C++26) | 是否支援原生 `std::views::concat` (P2542R8) |
| `COMPAT_HAS_STD_VIEWS_CACHE_LATEST` | `__cpp_lib_ranges_cache_latest >= 202403L` (C++26) | 是否支援原生 `std::views::cache_latest` (P3138R5) |
| `COMPAT_HAS_STD_RANGES_CONTAINS` | `__cpp_lib_ranges_contains >= 202207L` (C++23) | 是否支援原生 `std::ranges::contains` |
| `COMPAT_HAS_STD_RANGES_STARTS_WITH` | `__cpp_lib_ranges_starts_with >= 202207L` (C++23) | 是否支援原生 `std::ranges::starts_with` / `ends_with` |
| `COMPAT_HAS_STD_RANGES_FOLD` | `__cpp_lib_ranges_fold >= 202207L` (C++23) | 是否支援原生 `std::ranges::fold_left` (C++23) |
| `COMPAT_HAS_STD_RANGES_TO` | `__cpp_lib_ranges_to_container >= 202202L` (C++23) | 是否支援原生 `std::ranges::to` (P1206R7，規避 GCC 14 Bugzilla 115200) |
| `COMPAT_HAS_STD_RANGES_FIND_LAST` | `__cpp_lib_ranges_find_last >= 202207L` (C++23) | 是否支援原生 `std::ranges::find_last` / `find_last_if` |
| `COMPAT_HAS_STD_RANGES_IOTA` | `__cpp_lib_ranges_iota >= 202202L` (C++23) | 是否支援原生 `std::ranges::iota` 演算法 |
| `COMPAT_HAS_STD_RANGES_SHIFT` | `__cpp_lib_ranges_shift >= 202202L` (C++23) | 是否支援原生 `std::ranges::shift_left` / `shift_right` |
| `COMPAT_HAS_STD_RANGES_GENERATE_RANDOM` | `__cpp_lib_ranges_generate_random >= 202403L` (C++26) | 是否支援原生 `std::ranges::generate_random` |


---

## 例外處理與微架構最佳化 (Exceptions & Microarchitecture)

### 核心失敗策略不變量 (Failure Policy Invariant)

所有函式庫內部需要報告致命錯誤或終止流程之處，一律經由統一的失敗抽象介面（`COMPAT_THROW_OR_ABORT`）管控，禁止任何 detail 內部實作散落未受控的裸 `throw`。

- **`COMPAT_THROW_OR_ABORT(ex)`**：
  - **Exceptions Enabled 模式**（定義 `__cpp_exceptions` 或 MSVC `_CPPUNWIND`）：依 API 契約拋出對應例外型別 `throw (ex);`。
  - **Exceptions Disabled 模式**（`-fno-exceptions` 或 MSVC `/EHs-c-`）：一律執行 Fail-fast 終止策略 `std::abort();`。保證即便是 `#include <compat/Compat.hpp>` 在 C++11 `-fno-exceptions` 下編譯亦 100% 成功，無任何語法錯誤。

### `COMPAT_UNREACHABLE()`
- **語意**：提示編譯器該分支在邏輯上永遠不可達，消除分支開銷。
- **實作**：
  - MSVC: `__assume(0)`
  - GCC / Clang: `__builtin_unreachable()`
  - 通用降階: 呼叫 `::compat::detail::compat_unreachable_abort()`

### `compat::detail::compat_unreachable_abort()`
```cpp
[[noreturn]] inline void compat_unreachable_abort() noexcept;
```
- **說明**：跨平台終止函式。保證先觸發 `std::abort()` 產生 Core Dump / Crash Report，後續緊跟 `__builtin_unreachable()`，徹底杜絕編譯器因推導不可達而進行錯誤的死碼消除（Dead Code Elimination）。

### `COMPAT_CONSTEXPR_14` 與 `COMPAT_CONSTEXPR_20`
- `COMPAT_CONSTEXPR_14`：在 C++14 及以上環境展開為 `constexpr`，在 C++11 環境展開為 `inline`，用於修飾包含迴圈、區域變數或修改內部狀態的函式。
- `COMPAT_CONSTEXPR_20`：在 C++20 及以上環境展開為 `constexpr`，在 C++11/14/17 展開為 `inline`。

---

## 跨平臺與編譯器驗證矩陣 (Verified Toolchain Matrix)

`CPP-Compat` 實作保證通過以下平臺、編譯器與語言標準的建構與迴歸測試（全部啟用 `COMPAT_FORCE_SELF_IMPLEMENTATION=1`）：

| 平臺環境 | 編譯器版本 | C++11 | C++14 | C++17 | C++20 | C++23 |
| :--- | :--- | :---: | :---: | :---: | :---: | :---: |
| **Windows 11 x64** | MSVC 19.51 (Visual Studio 2026 Preview) | 不支援* | ✅ 通過 | ✅ 通過 | ✅ 通過 | ✅ 通過 |
| **Linux (WSL2 Ubuntu)** | GNU GCC 14.2 | ✅ 通過 | ✅ 通過 | ✅ 通過 | ✅ 通過 | ✅ 通過 |
| **Linux (WSL2 Ubuntu)** | Clang 18.1 | ✅ 通過 | ✅ 通過 | ✅ 通過 | ✅ 通過 | ✅ 通過 |

*\*註：MSVC 現代編譯器最低支援標準為 `/std:c++14`。*

---

## 範例程式碼 (Example)

```cpp
#include <compat/Config.hpp>
#include <iostream>

void DemonstrateConfig() {
    std::cout << "Detected C++ Version: " << COMPAT_CPLUSPLUS << "\n";

#if COMPAT_HAS_STD_EXPECTED
    std::cout << "Mode: Using Native std::expected\n";
#else
    std::cout << "Mode: Using Self-Contained compat::expected Fallback\n";
#endif

#if COMPAT_HAS_STD_RANGES
    std::cout << "Ranges Mode: Native std::ranges active\n";
#else
    std::cout << "Ranges Mode: Self-Contained Fallback active\n";
#endif
}
```

---

## 參閱 (See Also)
- [API 參考首頁](README.md)
- [架構說明書 (ARCHITECTURE.md)](../ARCHITECTURE.md)
