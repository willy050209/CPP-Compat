# 系統架構手冊 (ARCHITECTURE.md)

## 1. 系統定位
本專案為一個「零外部第三方函式庫依賴（Zero External Dependencies）」的現代 C++ 標準庫向下相容層（Self-Contained Shim Layer）。
在 C++23/20 支援環境下透明別名至原生 `std`，在 C++11/14/17 等舊環境下自動切換至純自研、輕量且型別安全之 Fallback 實作。

## 2. 模組架構圖

```
include/compat/
├── Config.hpp           <- 特性檢測巨集 (探測 __cpp_lib_* 與 C++ 標準版本)
├── StringView.hpp       <- string_view 轉接 (C++17 原生 vs C++11 自研 Fallback)
├── Expected.hpp         <- expected/unexpected 轉接 (C++23 原生 vs 自研 Fallback)
├── Format.hpp           <- format 轉接 (C++20 原生 vs 自研串流輸出引擎)
├── Print.hpp            <- print/println 轉接 (C++23 原生 vs 自研串流輸出引擎)
├── Parse.hpp            <- parse<T> 型態轉換 (支援固定寬度整數、浮點數、布林、字串)
├── Compat.hpp           <- 總括標頭檔 (包含上述所有模組)
└── detail/
    ├── SelfStringView.hpp   <- C++11 自研 string_view (支援 data, size, substr, find)
    ├── SelfUnionExpected.hpp<- C++11/14 自研 Tagged Unrestricted Union 基礎之 expected
    ├── SelfExpected.hpp     <- C++17+ 自研 std::variant 基礎之 expected
    ├── SelfFormat.hpp       <- 自研字串格式化引擎 (輸出至 std::string)
    ├── SelfPrint.hpp        <- 自研串流格式化輸出引擎 (輸出至 std::ostream)
    └── SelfParse.hpp        <- 自研純函數無例外字串型態解析引擎
```

## 3. 導出工具架構
- `scripts/bundle_header.py`：將所有模組拓撲排序內嵌，生成單一發行標頭檔 `dist/compat.hpp`。
- `scripts/export_module.py`：將相容層封裝為 C++20 Module 介面單元 `dist/compat.ixx`。
