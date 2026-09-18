# 終端列印 (compat::print / println)

定義於標頭檔 [`<compat/Print.hpp>`](file:///D:/program/C++/CPP-Compat/include/compat/Print.hpp)。  
所屬命名空間：`compat`。

`compat::print` 與 `compat::println` 完全遵循 **ISO C++23 `std::print` 與 `std::println`** 標準規格。提供極速、型別安全且自動處理 Unicode 編碼的控制台與串流輸出能力。  
在 Windows 平台上，它透過自研的 UTF-8 直寫通道直接調用 Win32 `WriteConsoleW` API，徹底終結了 Windows 終端長久以來的亂碼（Mojibake）問題，**完全無需**侵入性修改全域 Code Page（如 `SetConsoleOutputCP(65001)`）。

---

## 函式樣板宣告 (Declaration)

```cpp
namespace compat {

    // 格式化輸出至標準輸出 stdout (不換行)
    template <typename... Args>
    void print(const char* fmt, Args&&... args);

    template <typename... Args>
    void print(compat::string_view fmt, Args&&... args);

    // 格式化輸出至標準輸出 stdout，並追加換行符 '\n'
    template <typename... Args>
    void println(const char* fmt, Args&&... args);

    template <typename... Args>
    void println(compat::string_view fmt, Args&&... args);

    // 格式化輸出至指定 C 檔案串流 (FILE* stream)
    template <typename... Args>
    void print(std::FILE* stream, const char* fmt, Args&&... args);

    template <typename... Args>
    void print(std::FILE* stream, compat::string_view fmt, Args&&... args);

    // 格式化輸出至指定 C 檔案串流，並追加換行符 '\n'
    template <typename... Args>
    void println(std::FILE* stream, const char* fmt, Args&&... args);

    template <typename... Args>
    void println(std::FILE* stream, compat::string_view fmt, Args&&... args);

} // namespace compat
```

---

## 核心架構：Windows Unicode 直寫管線 (Windows Terminal Architecture)

在傳統 C++ 中，於 Windows 終端印出 UTF-8 多國語言（中文、日文、Emoji 等）極易發生亂碼，或因管道重定向導致輸出損壞。`compat::print` 採用以下狀態機處理：

```mermaid
flowchart TD
    Call["compat::print / println 呼叫"] --> CheckOS{作業系統平臺}
    CheckOS -- "Linux / macOS" --> UnixWrite["fwrite(buffer, 1, len, stream)<br>(原生 POSIX UTF-8 輸出)"]
    CheckOS -- "Windows" --> CheckStream{輸出標的是否為終端 Console？<br>(_isatty && GetConsoleMode)}
    CheckStream -- "是 (真實終端控制台)" --> WinConvert["轉換 UTF-8 為 UTF-16 (MultiByteToWideChar)"]
    WinConvert --> WinWrite["WriteConsoleW(hConsole, wide_buf, ...)<br>(零亂碼直接渲染)"]
    CheckStream -- "否 (管道/檔案重定向)" --> WinFile["fwrite(buffer, 1, len, stream)<br>(保留完整 UTF-8 Byte Stream)"]
```

### 技術優勢
1. **無全域副作用**：不呼叫 `SetConsoleOutputCP`，不影響應用程式內其他依賴當前 Code Page 的模組或第三方函式庫。
2. **自動探測重定向**：當輸出被重定向至檔案（如 `app.exe > log.txt`）或 CI 管道時，自動切換至純 Byte Stream 寫入，保證日誌檔案內容為乾淨合法的 UTF-8，不插入額外的寬字元編碼。

---

## 範例程式碼 (Example)

```cpp
#include <compat/Print.hpp>
#include <cstdio>

int main() {
    // 1. 標準輸出與 println
    compat::println("Welcome to CPP-Compat {}!", "v2.0");

    // 2. 多國語言與 Emoji 終端輸出測試
    compat::println("繁體中文測試: 現代 C++ 格式化輸出 🚀");
    compat::println("日本語テスト: こんにちは世界 🌸");
    compat::println("Accented text: Déjà vu, façade, café ☕");

    // 3. 輸出至 stderr (錯誤串流)
    compat::println(stderr, "[ERROR] Critical error code: {:#x}, file: {}", 0x80070005, "data.bin");

    // 4. 連續 print 不換行
    for (int i = 1; i <= 5; ++i) {
        compat::print("{} ", i);
    }
    compat::println(""); // 換行

    return 0;
}
```

---

## 參閱 (See Also)
- [API 參考首頁](README.md)
- [格式化輸出 (Format)](format.md)
- [字串檢視 (String View)](string_view.md)
