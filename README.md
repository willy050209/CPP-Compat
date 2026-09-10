# CPP-Compat（自研 C++ 標準庫向下相容層）

[![CI Build](https://github.com/Willy/CPP-Compat/actions/workflows/ci.yml/badge.svg)](https://github.com/Willy/CPP-Compat/actions/workflows/ci.yml)
[![Standard](https://img.shields.io/badge/C%2B%2B-11%20%7C%2014%20%7C%2017%20%7C%2020%20%7C%2023-blue.svg)](#)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](#)
[![Zero Dependencies](https://img.shields.io/badge/dependencies-zero-brightgreen.svg)](#)

純自研、零第三方函式庫相依性（Zero External Dependencies）的現代 C++ 標準庫向下相容層架構。  
在 C++23/20 支援環境下透明別名至原生 `std`，在 C++11/14/17 等舊環境下自動切換至輕量自研 Fallback 實作。

---

## 核心特性

- **零外部第三方相依**：專案完全自給自足，不下載或連結任何外部函式庫（如 `fmt`、`tl::expected` 等）。
- **透明條件編譯對接**：
  - 當編譯器支援現代標準（如 C++23/C++20）時，直接透傳至原生 `std`，享受極致零負擔編譯器最佳化。
  - 當編譯環境為舊版（C++11/14/17）或缺乏原生庫支援時，無縫切換至純自研、輕量級且型別安全之 Fallback 實作。
- **向下相容 C++11**：舊標準缺乏 `<variant>`、`<string_view>`、`<format>` 時，自動啟用基於 C++11 無限制聯合體（Tagged Unrestricted Union）之 `expected`、自研 `string_view` 與自研串流格式化引擎。
- **ISO C++23 對齊**：Refinement Phase 已完成 `expected<void, E>`、Trivial 屬性傳遞、完整 Monadic Operations、`bad_expected_access`。
- **自訂型別格式化**：`compat::formatter<T>` 擴充介面，相容 `std::formatter<T>` 語意。
- **Windows UTF-8 終端支援**：透過 `WriteConsoleW` 正確輸出 Unicode，無需 `SetConsoleOutputCP(65001)`。
- **強型別字串解析 `parse<T>`**：純函數、無例外、Fail-fast 安全轉換（支援 `int8_t`~`uint64_t`、`float`、`double`、`bool`、`string`）。
- **`-fno-exceptions` 安全**：`COMPAT_THROW_OR_ABORT` 巨集在無例外環境中自動改為 `std::abort()`。
- **ABI 隔離**：`COMPAT_ABI_TAG` inline namespace 防止雙軌混用時的 ODR 衝突。
- **自動化發行工具**：
  - 一鍵打包單一標頭檔：`dist/compat.hpp`（5,139 行，含 `#undef` 清除）
  - 一鍵導出標準 C++20 Module：`dist/compat.ixx`
- **嚴格規範標準**：全數遵循純函數、固定寬度型別（`<cstdint>`）、XML 文件註解、Fail-fast 原則、UTF-8 BOM 編碼與 `#pragma once`。

---

## 模組對照表

| 功能組件 | 標頭檔 | C++23 原生支援時 | 舊標準 (C++11/14/17/20) Fallback |
| :--- | :--- | :--- | :--- |
| **特性檢測** | `compat/Config.hpp` | 探測 `__cpp_lib_*` | `COMPAT_FORCE_SELF_IMPLEMENTATION` / `COMPAT_FORCE_STD_IMPLEMENTATION` |
| **字串檢視** | `compat/StringView.hpp` | `std::string_view` (C++17+) | 自研 `compat::detail::string_view`（constexpr、`std::hash`）|
| **期望值/錯誤** | `compat/Expected.hpp` | `std::expected`, `std::unexpected` | C++17: `std::variant`<br>C++11/14: Tagged Unrestricted Union |
| **格式化輸出** | `compat/Format.hpp` | `std::format` (C++20+) | 自研 `{}` 佔位符替換引擎 + `compat::formatter<T>` |
| **終端列印** | `compat/Print.hpp` | `std::print`, `std::println` | 自研引擎 + Windows UTF-8 WriteConsoleW |
| **型態解析** | `compat/Parse.hpp` | `compat::parse<T>` + `compat::from_chars` | 純函數無例外解析，零堆積、零 locale |
| **總括標頭** | `compat/Compat.hpp` | 聚合所有模組 | 聚合所有模組 |

---

## 快速上手範例

### 基本用法

```cpp
#include <cstdint>
#include <compat/Compat.hpp>

/// <summary>
/// 安全純函數除法計算：驗證除數，除數為零時返回錯誤。
/// </summary>
[[nodiscard]] static constexpr compat::expected<int32_t, compat::string_view> SafeDivide(
    const int32_t dividend,
    const int32_t divisor
) noexcept {
    if (divisor == 0) {
        return compat::unexpected<compat::string_view>("Division by zero is not permitted.");
    }
    return dividend / divisor;
}

int main() {
    // 1. 純函數安全除法
    const auto result = SafeDivide(42, 0);
    if (!result.has_value()) {
        compat::println("Calculation Failed: {}", result.error());
    }

    // 2. parse<T> 強型別解析
    const auto parsedInt = compat::parse<int32_t>("1024");
    if (parsedInt.has_value()) {
        compat::println("Parsed integer: {}", parsedInt.value());
    }

    // 3. 相容 std::format 格式化
    const std::string formatted = compat::format("Formatted value: {}, PI: {}", 42, 3.14159);
    compat::println("{}", formatted);

    return 0;
}
```

### Monadic Operations

```cpp
#include <compat/Expected.hpp>

const auto result = compat::expected<int32_t, std::string>{42}
    .and_then([](int32_t v) -> compat::expected<int32_t, std::string> {
        return v * 2;
    })
    .transform([](int32_t v) { return v + 1; })
    .or_else([](const std::string& e) -> compat::expected<int32_t, std::string> {
        return 0;
    });

// result.value() == 85
```

### 自訂型別格式化（compat::formatter<T>）

```cpp
#include <compat/Format.hpp>
#include <compat/Print.hpp>

struct Point { int32_t x, y; };

template<>
struct compat::formatter<Point> {
    static std::string format(const Point& p) {
        return compat::format("({}, {})", p.x, p.y);
    }
};

int main() {
    compat::println("Position: {}", Point{3, 4});
    // 輸出：Position: (3, 4)
}
```

### from_chars 解析介面

```cpp
#include <compat/Parse.hpp>

// 整數（支援 2–36 進位）
int32_t val = 0;
const auto r1 = compat::from_chars("255", nullptr, val, 16);  // hex
// r1.ec == std::errc{}, val == 255

// 浮點（零堆積、零 locale、零 sscanf）
double d = 0.0;
const auto r2 = compat::from_chars("3.14e2", nullptr, d);
// r2.ec == std::errc{}, d == 314.0

// NaN / Inf
double nan_val = 0.0;
compat::from_chars("NaN", nullptr, nan_val);  // std::isnan(nan_val) == true
```

### expected<void, E>

```cpp
#include <compat/Expected.hpp>

[[nodiscard]] compat::expected<void, std::string> CheckRange(int32_t v) {
    if (v < 0 || v > 100) {
        return compat::unexpected<std::string>("Out of range");
    }
    return {};
}

const auto ok = CheckRange(50);
if (!ok) compat::println("Error: {}", ok.error());
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

本專案提供 Python 導出工具：

```bash
# 1. 自動拓撲排序打包為單一標頭檔（含 #undef 清除內部巨集）
python scripts/bundle_header.py
# 產出: dist/compat.hpp

# 2. 自動導出為 C++20 Module 介面單元
python scripts/export_module.py
# 產出: dist/compat.ixx (支援 import compat;)

# 3. 打包驗證（26 項斷言）
python scripts/test_packaging.py
```

---

## 建置與測試

### Windows (Visual Studio 18 Insiders / MSVC)

```powershell
# 設定 C++23 原生標準
cmake -B build -DCMAKE_CXX_STANDARD=23
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure

# 強制 Fallback 模式（驗證自研路徑）
cmake -B build_fb -DCMAKE_CXX_STANDARD=23 -DCOMPAT_FORCE_SELF_IMPLEMENTATION=ON
cmake --build build_fb --config Release
ctest --test-dir build_fb -C Release --output-on-failure
```

### Linux (Ubuntu / GCC)

```bash
# 驗證 C++11 (完整向下相容測試)
cmake -B build-cxx11 -DCMAKE_CXX_STANDARD=11
cmake --build build-cxx11
ctest --test-dir build-cxx11 --output-on-failure

# 驗證 C++23 原生標準
cmake -B build-cxx23 -DCMAKE_CXX_STANDARD=23
cmake --build build-cxx23
ctest --test-dir build-cxx23 --output-on-failure
```

### macOS (Apple Clang)

```bash
cmake -B build-mac -DCMAKE_CXX_STANDARD=20
cmake --build build-mac
ctest --test-dir build-mac --output-on-failure
```

---

## CI 矩陣

| 平台 | C++ 標準 | 模式 |
| :--- | :--- | :--- |
| Ubuntu (GCC) | C++11, 14, 17, 20, 23 | Auto + Forced Fallback |
| Windows (MSVC) | C++17, 20, 23 | Auto + Forced Fallback |
| macOS (Apple Clang) | C++14, 17, 20, 23 | Auto + Forced Fallback |

---

## 規範標準

- 固定寬度整數型別（`int32_t`、`uint64_t` 等），不使用 `int`/`long`
- 所有公開 API 附 XML 文件註解（`<summary>`, `<param>`, `<returns>`）
- 純函數設計（Fail-fast）：`[[nodiscard]]`、`noexcept`、`constexpr`
- 禁止 `using namespace std`
- UTF-8 BOM 編碼、`#pragma once`
- 禁止包含 `<iostream>` 於任何 `*.hpp` 標頭檔

---

## 授權條款 (License)

本專案採用 [MIT License](LICENSE) 授權。

---

## 詳細架構說明

請參閱 [ARCHITECTURE.md](ARCHITECTURE.md)。

