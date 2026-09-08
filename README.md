# CPP-Compat (自研 C++ 標準庫向下相容層)

[![CI Build](https://github.com/Willy/CPP-Compat/actions/workflows/ci.yml/badge.svg)](https://github.com/Willy/CPP-Compat/actions/workflows/ci.yml)
[![Standard](https://img.shields.io/badge/C%2B%2B-11%20%7C%2014%20%7C%2017%20%7C%2020%20%7C%2023-blue.svg)](#)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](#)
[![Zero Dependencies](https://img.shields.io/badge/dependencies-zero-brightgreen.svg)](#)

純自研、零第三方函式庫相依性（Zero External Dependencies）的現代 C++ 標準庫向下相容層架構。

---

## 核心特性

- **零外部第三方相依**：專案完全自給自足，不下載或連結任何外部函式庫（如 `fmt`、`tl::expected` 等）。
- **透明條件編譯對接**：
  - 當編譯器支援現代標準（如 C++23/C++20）時，直接透傳至原生 `std`，享受極致零負擔編譯器最佳化。
  - 當編譯環境為舊版（C++11/14/17）或缺乏原生庫支援時，無縫切換至純自研、輕量級且型別安全之 Fallback 實作。
- **向下相容 C++11**：舊標準缺乏 `<variant>`、`<string_view>`、`<format>` 時，自動啟用基於 C++11 無限制聯合體（Tagged Unrestricted Union）之 `expected`、自研 `string_view` 與自研串流格式化引擎。
- **強型別字串解析 `parse<T>`**：純函數、無例外、Fail-fast 安全轉換（支援 `int8_t`~`uint64_t`、`float`、`double`、`bool`、`string`）。
- **自動化發行工具**：
  - 一鍵打包單一標頭檔：`dist/compat.hpp`
  - 一鍵導出標準 C++20 Module：`dist/compat.ixx`
- **嚴格規範標準**：全數遵循純函數、固定寬度型別（`<cstdint>`）、XML 文件註解、Fail-fast 原則、UTF-8 BOM 編碼與 `#pragma once`。

---

## 模組對照表

| 功能組件 | 標頭檔 | C++23 原生支援時 | 舊標準 (C++11/14/17/20) Fallback |
| :--- | :--- | :--- | :--- |
| **特性檢測** | `compat/Config.hpp` | 探測 `__cpp_lib_*` | 提供 `COMPAT_FORCE_FALLBACK` 開關 |
| **字串檢視** | `compat/StringView.hpp` | `std::string_view` (C++17+) | 自研 `compat::detail::string_view` (C++11/14) |
| **期望值/錯誤**| `compat/Expected.hpp` | `std::expected`, `std::unexpected` | C++17: `std::variant`<br>C++11/14: Tagged Unrestricted Union |
| **格式化輸出** | `compat/Format.hpp` | `std::format` (C++20+) | 自研 `{}` 佔位符替換引擎 (返回 `std::string`) |
| **終端列印** | `compat/Print.hpp` | `std::print`, `std::println` | 自研串流輸出引擎 (輸出至 `std::ostream` / `std::cout`) |
| **型態解析** | `compat/Parse.hpp` | `compat::parse<T>` | 純函數無例外解析，邊界溢位/無效格式 Fail-fast |
| **總括標頭** | `compat/Compat.hpp` | 聚合所有模組 | 聚合所有模組 |

---

## 快速上手範例

```cpp
#include <cstdint>
#include <compat/Compat.hpp>

/// <summary>
/// 安全純函數除法計算：驗證除數，除數為零時返回錯誤。
/// </summary>
/// <param name="dividend">被除數。</param>
/// <param name="divisor">除數。</param>
/// <returns>計算結果或錯誤訊息。</returns>
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

---

## 單一標頭檔與 C++20 Module 導出

本專案提供 Python 導出工具：

```bash
# 1. 自動拓撲排序打包為單一標頭檔
python scripts/bundle_header.py
# 產出: dist/compat.hpp

# 2. 自動導出為 C++20 Module 介面單元
python scripts/export_module.py
# 產出: dist/compat.ixx (支援 import compat;)
```

---

## 建置與測試

### Windows (Visual Studio 18 Insiders / MSVC)
```powershell
# 設定 C++23 原生標準
cmake -B build -DCMAKE_CXX_STANDARD=23
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure

# 驗證舊版 C++14 與強制 Fallback 模式
cmake -B build-cxx14 -DCMAKE_CXX_STANDARD=14 -DCOMPAT_FORCE_FALLBACK=ON
cmake --build build-cxx14 --config Release
ctest --test-dir build-cxx14 -C Release --output-on-failure
```

### Linux (Ubuntu / WSL GCC)
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
