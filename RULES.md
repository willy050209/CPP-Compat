# 核心開發原則與規範 (RULES.md)

本專案所有程式碼必須嚴格遵守以下規範：

## 1. 職責分離 (Separation of Concerns)
- 明確區分資料結構（Data Class / Struct）與業務邏輯（Logic），降低耦合度。
- 各標頭檔與模組必須具備單獨編譯與獨立進行單元測試之能力。

## 2. 註解規範 (Documentation Standards)
- **僅對函式 (Function/Method) 與資料結構/類別 (Class/Struct) 使用 XML 樣式說明文件註解**：
  - 必須包含 `<summary>`、`<typeparam>`、`<param>`、`<returns>`、`<exception>` 等標籤。
- **內部程式執行邏輯一律使用一般語言標準註解**（如 `//` 或 `/* */`），嚴禁在內部業務邏輯中使用 XML 標籤。

## 3. 架構偏好 (Architectural Preferences)
- **純函數 (Pure Function) 設計**：
  - 輸入即輸出、無外部副作用、不修改隱式全域狀態。
  - 優先使用 `[[nodiscard]]`、`constexpr`、`noexcept`、`const` 參考。
- **快速失敗 (Fail-Fast)**：
  - 函式進入點進行前置條件驗證，遇到非法參數或狀態異常時立即拋出型別安全例外或回傳 `unexpected`，禁止帶病執行。
- **固定寬度型別**：
  - 數值一律明確使用 `<cstdint>` 之固定寬度型別（如 `int8_t`, `int16_t`, `int32_t`, `int64_t`, `uint8_t`, `uint32_t`, `size_t`），杜絕不可預期的原生 `int`/`long` 平台差異。

## 4. 檔案與編碼規範
- 所有標頭檔一律使用 `#pragma once` 作為防衛巨集。
- 所有 C++ 源碼檔（`.hpp`, `.cpp`, `.ixx`）一律保存為 **UTF-8 with BOM** 編碼格式。

## 5. 測試驅動開發 (TDD)
- 嚴格落實「先寫測試 (Red) → 實作程式碼使測試通過 (Green) → 重構消除重複 (Refactor)」流程。
- 確保所有模組皆有完整的單元測試覆蓋，涵蓋正常路徑與邊界異常路徑。
