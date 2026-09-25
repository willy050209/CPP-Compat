# CPP-Compat（自研 C++ 標準庫向下相容層）

[![CI Build](https://github.com/Willy/CPP-Compat/actions/workflows/ci.yml/badge.svg)](https://github.com/Willy/CPP-Compat/actions/workflows/ci.yml)
[![Standard](https://img.shields.io/badge/C%2B%2B-11%20%7C%2014%20%7C%2017%20%7C%2020%20%7C%2023%20%7C%2026-blue.svg)](#)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](#)
[![Zero Dependencies](https://img.shields.io/badge/dependencies-zero-brightgreen.svg)](#)

純自研、零第三方函式庫相依性（Zero External Dependencies）的現代 C++ 標準庫向下相容層架構。  
在現代 C++20/23/26 支援環境下透明別名至原生 `std`，在 C++11/14/17 等舊環境下自動無縫切換至純自研 Fallback 實作。

---

## 📖 API 參考文件 (MSDN / cppreference 風格)

本專案提供比照 **Microsoft Learn (MSDN)** 與 **C++ cppreference** 規格之官方 API 參考手冊，歡迎深入查閱各模組詳細規格：

- 📚 [**API 參考首頁與導覽 (docs/README.md)**](docs/README.md)
- ⚙️ [**特性檢測與配置 (docs/config.md)**](docs/config.md)：標準方言常數、特性探測巨集、雙軌開關
- 🔤 [**字串檢視 (docs/string_view.md)**](docs/string_view.md)：`compat::string_view`、`std::hash` 特化
- 🎁 [**選用值容器 (docs/optional.md)**](docs/optional.md)：`compat::optional<T>`、`nullopt`、`bad_optional_access`
- 🎯 [**期望值與錯誤處理 (docs/expected.md)**](docs/expected.md)：`expected<T, E>`、`unexpected`、Monadic 操作、`emplace()` 就地建構與強例外安全保證
- 📝 [**格式化輸出 (docs/format.md)**](docs/format.md)：`format`、標準格式規格語法（`[[fill]align][sign][#][0][width][.precision][type]`）、Unicode 東亞寬度 (UAX #11 / P1868R2)、編譯期靜態檢查與執行期例外防護、`formatter<T>` 自訂型別擴充
- 🖨️ [**終端列印 (docs/print.md)**](docs/print.md)：`print`、`println`、`std::ostream&` 串流路由、Windows UTF-8 `WriteConsoleW` 直寫管線、東亞多欄表格對齊
- 🔢 [**強型別解析 (docs/parse.md)**](docs/parse.md)：`parse<T>`、零堆積 `from_chars`（整數 2~36 進位、浮點數、NaN/Inf）
- 🔄 [**範圍基礎與概念 (docs/ranges.md)**](docs/ranges.md)：Range Concepts、CPO (`begin`, `end`, `size` 等)、`subrange`、`dangling`、`enable_borrowed_range` 特化自訂點
- 🌊 [**視圖適配器 (docs/views.md)**](docs/views.md)：管道語法 (`|`)、C++20 視圖、C++23 `as_const`、C++26 `concat` 與 `cache_latest`
- ⚡ [**受約束範圍演算法 (docs/algorithms.md)**](docs/algorithms.md)：Niebloids、投影支援 (`&Item::id`)、標籤結果型別 (`in_out_result`)
- 📦 [**未初始化記憶體演算法 (docs/memory.md)**](docs/memory.md)：`construct_at`、`destroy_at`、`uninitialized_copy/fill/move` 等 RAII 物件生命週期演算法

---

## 核心特性

- **零外部第三方相依**：專案完全自給自足，不下載或連結任何外部函式庫（如 `fmt`、`range-v3`、`tl::expected` 等）。
- **透明條件編譯對接**：
  - 當編譯器支援現代標準（如 C++20/23/26）時，直接透傳至原生 `std`，享受極致零負擔編譯器最佳化。
  - 當編譯環境為舊版（C++11/14/17）或缺乏原生庫支援時，無縫切換至純自研、輕量級且型別安全之 Fallback 實作。
- **向下相容至 C++11**：舊標準缺乏 `<variant>`、`<string_view>`、`<format>`、`<ranges>`、`<optional>` 時，自動啟用基於 C++11 無限制聯合體（Tagged Unrestricted Union）之 `expected`、`optional`、自研 `string_view`、自研串流格式化引擎與純自研 Ranges / Algorithms 模組。
- **ISO C++23 & C++26 前沿特性**：
  - `compat::ranges::to` (C++23 / P1206R7)：將任意 Range 轉為容器（支援管線語法 `r | to<vector>()`、樣板引數推導與 Fallback Emplace/Insert 迭代）。
  - `compat::views::concat` (C++26 / P2542R8 / N4984)：前綴長度儲存、雙向狀態機、跳過空區間與 $O(1)$ 下標跳轉階梯。
  - `compat::views::cache_latest` (C++26 / P3138R5)：嚴格的 `non-propagating-cache` 快取語意。
  - `compat::ranges::constant_range` (C++26 / P2728R6)：唯讀範圍概念合約。
  - `compat::views::take_while` / `compat::views::drop_while` (C++20)：條件式截取與略過視圖適配器。
- **受約束範圍演算法全家族**：全套 50+ 個 `compat::ranges::*` 演算法，支援 Niebloid 呼叫防護、投影（PMF/PMD 成員指標）與標籤結果型別（`in_out_result` 等）；重載嚴格約束 `sentinel_for<S, I>`，徹底杜絕自訂 Lambda 與成員指針在 MSVC/GCC/Clang 上的多載解析衝突（Overload Ambiguity）。
- **標準格式化規格與 Unicode 東亞寬度 (East Asian Width)**：
  - `compat::format` 與 `compat::print` 完整支援 ISO C++20 格式規格語法（`[[fill]align][sign][#][0][width][.precision][type]`），涵蓋對齊（`<`/`>`/`^`）、自訂填充字元、前導零（`#010x`）、正負號（`+`/`-`/` `）、進位前綴與浮點數精度控制。
  - 嚴格實作 Unicode UAX #11 與 P1868R2 終端估計欄位寬度（Display Columns），CJK 全形字元精確計算為 2 欄位，保證多欄位終端表格完美對齊（如 `compat::println("{:8}{:8}", "一號", "二號")`）與精度安全截斷。
  - 原生支援寬字元與寬字串（`wchar_t`, `std::wstring`）即時轉譯至 UTF-8。
- **Windows UTF-8 終端支援與串流路由**：透過 `WriteConsoleW` 直寫通道正確輸出 Unicode，徹底杜絕亂碼，無需 `SetConsoleOutputCP(65001)`；`std::cout` 與 `std::cerr` 自動路由轉譯防護。
- **自訂型別格式化**：`compat::formatter<T>` 擴充介面，完全相容 `std::formatter<T>` 語意。
- **強型別字串解析 `parse<T>`**：純函數、無例外、Fail-fast 安全轉換（支援 `int8_t`~`uint64_t`、`float`、`double`、`bool`、`string`）。
- **`-fno-exceptions` 安全**：`COMPAT_THROW_OR_ABORT` 巨集在無例外環境中自動降階為 `std::abort()`。
- **ABI 隔離**：`COMPAT_ABI_TAG` inline namespace 防止跨編譯單元混用時的 ODR 衝突。
- **自動化發行工具**：
  - 一鍵打包單一標頭檔：`dist/compat.hpp`（14,332 行，含 `#undef` 清除）
  - 一鍵導出標準 C++20 Module：`dist/compat.ixx`（14,343 行，支援 `import compat;`）

---

## 模組對照表

| 功能組件 | 標頭檔 | 所屬命名空間 | 現代標準原生支援時 | 舊標準 (C++11/14/17) Fallback |
| :--- | :--- | :--- | :--- | :--- |
| **特性檢測** | `<compat/Config.hpp>` | `compat`, `compat::detail` | 探測 `__cpp_lib_*` | `COMPAT_FORCE_SELF_IMPLEMENTATION` / `COMPAT_FORCE_STD_IMPLEMENTATION` |
| **字串檢視** | `<compat/StringView.hpp>` | `compat` | `std::string_view` (C++17+) | 自研 `compat::string_view`（constexpr、`std::hash`）|
| **選用值容器** | `<compat/Optional.hpp>` | `compat` | `std::optional` (C++17+) | 自研 `compat::optional<T>` (Tagged Union, constexpr) |
| **期望值/錯誤** | `<compat/Expected.hpp>` | `compat` | `std::expected`, `std::unexpected` | C++17: `std::variant`<br>C++11/14: Tagged Unrestricted Union<br>（完整支援 `emplace()` 與強例外安全） |
| **格式化輸出** | `<compat/Format.hpp>` | `compat` | `std::format` (C++20+) | 自研格式化規格引擎（對齊/填充/進位/精度/東亞寬度）+ `compat::formatter<T>` |
| **終端列印** | `<compat/Print.hpp>` | `compat` | `std::print`, `std::println` (C++23+) | 自研引擎 + Windows UTF-8 WriteConsoleW 直寫 + `std::ostream` 路由 |
| **強型別解析** | `<compat/Parse.hpp>` | `compat` | 原生或自研 `from_chars` | 自研零堆積、零 locale、基數 2~36 數值解析 |
| **範圍概念/CPO**| `<compat/Ranges.hpp>` | `compat::ranges` | `std::ranges` (C++20+) | 自研 CPO (`begin`/`end`/`size`)、`subrange`、`enable_borrowed_range`、Concepts 萃取器 |
| **視圖適配器** | `<compat/View.hpp>` | `compat::views` | `std::views` (C++20+) | 自研管道 (`\|`)、核心 Views、C++26 `concat` 與 `cache_latest` |
| **範圍演算法** | `<compat/Algorithm.hpp>`| `compat::ranges` | `std::ranges::*` (C++20+) | 自研 Niebloids、投影支援、標籤結果型別 (`in_out_result` 等) |
| **記憶體演算法**| `<compat/Memory.hpp>` | `compat::ranges` | `std::ranges::*` (C++20+) | 自研未初始化物件建構/銷毀、RAII 回滾保護、標籤結果型別 |
| **總括標頭** | `<compat/Compat.hpp>` | 全部聚合 | 聚合所有模組 | 聚合所有模組 |

---

## 快速上手範例

### 1. 現代範圍管道 (Range Views Pipeline: `operator|`)

```cpp
#include <compat/Ranges.hpp>
#include <compat/Print.hpp>

int main() {
    using namespace compat::views;

    // 產生 1 到 50，篩選奇數，計算三次方，截取前 4 個
    auto stream = iota(1, 50)
        | filter([](int n) { return n % 2 != 0; })
        | transform([](int n) { return n * n * n; })
        | take(4);

    compat::print("Cube of odd numbers: ");
    for (int v : stream) {
        compat::print("{} ", v);
    }
    compat::println("");
    // 輸出: Cube of odd numbers: 1 27 125 343

    return 0;
}
```

### 2. 受約束範圍演算法與投影 (Ranges Algorithms with Projections)

```cpp
#include <compat/Algorithm.hpp>
#include <compat/Print.hpp>
#include <vector>
#include <string>

struct Task {
    std::string title;
    int priority;
};

int main() {
    std::vector<Task> tasks = {
        {"Refactor Architecture", 2},
        {"Fix Security Bug", 1},
        {"Write API Docs", 3}
    };

    // 透過成員變數指標投影，依照優先級升序排序
    compat::ranges::sort(tasks, {}, &Task::priority);

    for (const auto& task : tasks) {
        compat::println("[Priority {}] {}", task.priority, task.title);
    }
    // 輸出:
    // [Priority 1] Fix Security Bug
    // [Priority 2] Refactor Architecture
    // [Priority 3] Write API Docs

    return 0;
}
```

### 3. ISO C++26 `views::concat` 多區間串接

```cpp
#include <compat/Ranges.hpp>
#include <compat/Print.hpp>
#include <vector>
#include <array>

int main() {
    std::vector<int> prefix = {1, 2};
    std::vector<int> empty_mid;
    std::array<int, 3> suffix = {3, 4, 5};

    auto all = compat::views::concat(prefix, empty_mid, suffix);

    compat::println("Total length: {}", compat::ranges::size(all)); // 5
    compat::println("Random access element at [3]: {}", all[3]);   // 4

    return 0;
}
```

### 4. 安全純函數除法與 Monadic Operations

```cpp
#include <compat/Expected.hpp>
#include <compat/Print.hpp>
#include <string>

[[nodiscard]] compat::expected<int, std::string> SafeDivide(int a, int b) {
    if (b == 0) {
        return compat::unexpected<std::string>("Division by zero");
    }
    return a / b;
}

int main() {
    auto res = SafeDivide(100, 2)
        .and_then([](int v) -> compat::expected<int, std::string> {
            return SafeDivide(v, 5);
        })
        .transform([](int v) {
            return v + 1;
        });

    if (res) {
        compat::println("Result: {}", *res); // 11
    } else {
        compat::println("Error: {}", res.error());
    }

    return 0;
}
```

---

## 編譯模式開關

| CMake 選項 | 對應巨集 | 效果 |
| :--- | :--- | :--- |
| `-DCOMPAT_FORCE_SELF_IMPLEMENTATION=ON` | `COMPAT_FORCE_SELF_IMPLEMENTATION` | 強制所有模組使用自研 Fallback（相容 `COMPAT_FORCE_FALLBACK`）|
| `-DCOMPAT_FORCE_STD_IMPLEMENTATION=ON` | `COMPAT_FORCE_STD_IMPLEMENTATION` | 強制使用原生 std；標準庫不符時 `#error` |
| 兩者同時 | — | 編譯錯誤（互斥）|

### -fno-exceptions 支援

在 `/EHs-c-` 或 `-fno-exceptions` 環境中，`COMPAT_THROW_OR_ABORT(ex)` 自動展開為 `::std::abort()`，而非 `throw`：

```cmake
target_compile_options(my_target PRIVATE /EHs-c-)
target_compile_definitions(my_target PRIVATE COMPAT_FORCE_SELF_IMPLEMENTATION)
```

---

## 單一標頭檔與 C++20 Module 導出

本專案提供自動化 Python 發行工具：

```bash
# 1. 自動拓撲排序打包為單一標頭檔（含 #undef 清除內部巨集）
python scripts/bundle_header.py
# 產出: dist/compat.hpp (12,845 行，UTF-8 BOM 驗證)

# 2. 自動導出為標準 C++20 Module 介面單元
python scripts/export_module.py
# 產出: dist/compat.ixx (12,856 行，支援 import compat;)
```

---

## 建置與測試

### Windows (Visual Studio / MSVC 19.51+)

```powershell
# 設定 C++20 原生標準
cmake -B build -DCMAKE_CXX_STANDARD=20
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure

# 強制自研 Fallback 模式驗證
cmake -B build_fb -DCMAKE_CXX_STANDARD=20 -DCOMPAT_FORCE_SELF_IMPLEMENTATION=ON
cmake --build build_fb --config Release
ctest --test-dir build_fb -C Release --output-on-failure
```

### Linux (Ubuntu / GCC 14.2 & Clang 18)

```bash
# 驗證 C++11 (完整向下相容與 pedantic 嚴格模式)
cmake -B build-cxx11 -DCMAKE_CXX_STANDARD=11
cmake --build build-cxx11
./build-cxx11/compat_test

# 驗證 Clang 18 C++11 模式
cmake -B build-clang-cxx11 -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_STANDARD=11
cmake --build build-clang-cxx11
./build-clang-cxx11/compat_test
```

---

## 跨編譯器與方言驗證矩陣

專案在全平臺與主流工具鏈上，均經過 **Native 預設模式 (`default`)** 與 **強制自研回退模式 (`fallback`)** 雙軌完整迴歸驗證，達成 **100% 通過（0 警告、0 錯誤）**：

| 平臺環境 | 編譯器版本 | 測試模式 | C++11 | C++14 | C++17 | C++20 | C++23 | 測試狀態 |
| :--- | :--- | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| **Windows 11 x64** | MSVC 19.51 (VS 2026 Preview) | Default (Native) | 不支援* | ✅ 通過 | ✅ 通過 | ✅ 通過 | ✅ 通過 | **PASSED** |
| | | Fallback (Self) | 不支援* | ✅ 通過 | ✅ 通過 | ✅ 通過 | ✅ 通過 | **PASSED** |
| **Linux (Ubuntu 24.04)** | GNU GCC 14.2 | Default (Native) | ✅ 通過 | ✅ 通過 | ✅ 通過 | ✅ 通過 | ✅ 通過 | **PASSED** |
| | | Fallback (Self) | ✅ 通過 | ✅ 通過 | ✅ 通過 | ✅ 通過 | ✅ 通過 | **PASSED** |
| **Linux (Ubuntu 24.04)** | LLVM Clang 18.1 | Default (Native) | ✅ 通過 | ✅ 通過 | ✅ 通過 | ✅ 通過 | ✅ 通過 | **PASSED** |
| | | Fallback (Self) | ✅ 通過 | ✅ 通過 | ✅ 通過 | ✅ 通過 | ✅ 通過 | **PASSED** |
| **macOS (GitHub Actions)** | Apple Clang (Xcode) | Dual-Mode CI | ✅ 通過 | ✅ 通過 | ✅ 通過 | ✅ 通過 | ✅ 通過 | **PASSED** |

*\*註：MSVC 現代編譯器最低支援方言為 `/std:c++14`。*

---

## 規範標準

- 固定寬度整數型別（`int32_t`、`uint64_t` 等），不使用非固定寬度 `int`/`long`
- 所有公開 API 附 XML 文件註解（`<summary>`, `<param>`, `<returns>`）
- 純函數設計（Fail-fast）：`[[nodiscard]]`、`noexcept`、`constexpr` / `COMPAT_CONSTEXPR_14`
- 禁止 `using namespace std` 於標頭檔中污染使用者命名空間
- 全原始碼與標頭檔嚴格遵循 **UTF-8 BOM** 編碼與 `#pragma once`

---

## 授權條款 (License)

本專案採用 [MIT License](LICENSE) 授權。
