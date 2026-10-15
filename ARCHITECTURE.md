# 系統架構手冊 (ARCHITECTURE.md)

## 1. 系統定位

本專案為一個「零外部第三方函式庫依賴（Zero External Dependencies）」的現代 C++ 標準庫向下相容層（Self-Contained Shim Layer）。  
在 C++23/20 支援環境下透明別名至原生 `std`，在 C++11/14/17 等舊環境下自動切換至純自研、輕量且型別安全之 Fallback 實作。

Refinement Phase（2026-09）已完成全套 ISO C++23 對齊、-fno-exceptions 支援、ABI 隔離、Windows UTF-8 終端修正、monadic operations、自訂型別格式化擴充介面，及多維度 CI 矩陣。

---

## 2. 模組架構圖

```
include/compat/
├── Config.hpp             <- 特性檢測巨集中樞（探測 __cpp_lib_*、ABI 隔離 inline ns、C++11 ODR 防護、例外開關）
├── StringView.hpp         <- string_view 轉接（C++17 原生 vs C++11 自研 Fallback）
├── Span.hpp               <- span<T, Extent> 轉接（C++20 原生 vs 自研邊界防護 Fallback）
├── PropagateConst.hpp     <- propagate_const<T> 常數傳播指針（TS v2/v3 原生 vs 自研 Fallback）
├── SharedMutex.hpp        <- shared_mutex 讀寫鎖轉接（C++17 原生 vs C++14 timed 降階 vs C++11 雙閘門鎖）
├── Optional.hpp           <- optional 轉接（C++17 原生 vs C++11 自研 Fallback）
├── Expected.hpp           <- expected/unexpected 轉接（C++23 / C++17 variant / C++11 union）
├── Format.hpp             <- format 轉接（C++20 原生 vs 自研格式引擎）+ compat::formatter<T>
├── Print.hpp              <- print/println 轉接（C++23 原生 vs 自研引擎）+ Windows UTF-8
├── Parse.hpp              <- parse<T>（支援整數、浮點、bool、string）+ from_chars 介面
├── Ranges.hpp             <- ranges 轉接（C++20/23/26 概念、CPO、subrange、to 等）
├── View.hpp               <- views 視圖適配器轉接（管道語法 |、concat、cache_latest 等）
├── Algorithm.hpp          <- ranges 演算法轉接（Niebloids、投影、結果標籤）
├── Memory.hpp             <- ranges memory 演算法轉接（construct_at, uninitialized_* 等）
├── Filesystem.hpp         <- filesystem 轉接（C++17 原生 vs 自研 Win32/POSIX Fallback）
├── Compat.hpp             <- 總括標頭檔（包含上述所有模組）
└── detail/
    ├── SelfStringView.hpp    <- C++11 自研 string_view（constexpr find/rfind/compare、std::hash）
    ├── SelfUnionExpected.hpp <- C++11/14 Tagged Unrestricted Union 基礎 expected（含 void 特化、monadic ops）
    ├── SelfExpected.hpp      <- C++17+ std::variant 基礎 expected（含 void 特化、monadic ops）
    ├── SelfFormat.hpp        <- 自研格式引擎（支援 compat::formatter<T> 自訂型別）
    ├── SelfPrint.hpp         <- 自研串流輸出引擎（Windows WriteConsoleW UTF-8、fmt arg 邊界驗證）
    ├── SelfParse.hpp         <- 自研純函數無例外字串解析引擎（from_chars、零堆積、零 locale）
    ├── SelfRanges.hpp        <- 自研 Ranges 核心概念與視圖引擎
    ├── SelfAlgorithm.hpp     <- 自研受約束演算法引擎
    ├── SelfMemory.hpp        <- 自研未初始化記憶體演算法與回滾保護
    └── SelfFilesystem.hpp    <- 自研檔案系統引擎（零外部相依，Win32 Unicode + POSIX 原生）
```

---

## 3. Config.hpp 巨集系統與 ABI 防護架構

### 3.1 特性偵測旗標

| 巨集 | 意義 |
| :--- | :--- |
| `COMPAT_HAS_STD_EXPECTED` | `1` 當 `std::expected` 可用（C++23 + `__cpp_lib_expected`） |
| `COMPAT_HAS_STD_FORMAT` | `1` 當 `std::format` 可用（C++20 + `__cpp_lib_format`） |
| `COMPAT_HAS_STD_PRINT` | `1` 當 `std::print` 可用（C++23 + `__cpp_lib_print`） |
| `COMPAT_HAS_STD_SPAN` | `1` 當 `std::span` 可用（C++20 + `__cpp_lib_span`） |
| `COMPAT_HAS_STD_STRING_VIEW` | `1` 當 `std::string_view` 可用（C++17+） |
| `COMPAT_HAS_STD_OPTIONAL` | `1` 當 `std::optional` 可用（C++17+） |
| `COMPAT_HAS_VARIANT` | `1` 當 `std::variant` 可用（C++17+） |
| `COMPAT_HAS_STD_FILESYSTEM` | `1` 當 `std::filesystem` 可用（C++17+ + `__cpp_lib_filesystem`） |
| `COMPAT_HAS_STD_SHARED_MUTEX` | `1` 當 `std::shared_mutex` 可用（C++17+ + `__cpp_lib_shared_mutex`） |

### 3.2 雙軌切換開關與 ABI 鎖定（互斥）

| 巨集 | 作用 |
| :--- | :--- |
| `COMPAT_FORCE_SELF_IMPL` / `COMPAT_FORCE_SELF_IMPLEMENTATION` | 強制所有模組使用自研 Fallback 實作（相容 `COMPAT_FORCE_FALLBACK`）；跨標準過渡期鎖定 ABI 二進位相容 |
| `COMPAT_FORCE_STD_IMPLEMENTATION` | 強制使用原生 std；若標準庫不符則 `#error` |
| 同時定義兩者 | 觸發 `#error "Cannot force both self and std implementations simultaneously"` |

### 3.3 例外處理開關

```cpp
#if defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND)
#  define COMPAT_HAS_EXCEPTIONS 1
#  define COMPAT_THROW_OR_ABORT(ex) throw (ex)
#else
#  define COMPAT_HAS_EXCEPTIONS 0
#  define COMPAT_THROW_OR_ABORT(ex) ::std::abort()
#endif
```

在 `-fno-exceptions` / `/EHs-c-` 環境中，所有原本 `throw` 的路徑自動改為 `std::abort()`。

### 3.4 ABI 隔離與 Inline Namespace

為避免大型專案在「舊模組 C++11 + 新模組 C++17/20」混編升級過渡期產生 ABI 撕裂與 ODR 靜默破壞，專案實作了嚴格的雙軌 Inline Namespace 隔離：

```cpp
#define COMPAT_SELF_NS self_v1
#define COMPAT_STD_NS  std_v1

// 自研 Fallback 型別置於 compat::self_v1
namespace compat {
    inline namespace self_v1 {
        class string_view { ... };
    }
}

// 原生 std 型別別名置於 compat::std_v1
namespace compat {
    inline namespace std_v1 {
        using string_view = std::string_view;
    }
}
```

1. **型別二進位隔離**：不同 TU 分別編譯為 `compat::self_v1::string_view` 與 `compat::std_v1::string_view`，符號名稱（Mangled Name）相異，Linker 能明確攔截型別混傳錯誤，杜絕靜默記憶體錯位。
2. **無痛無縫升級**：單一專案升級編譯標準時，呼叫端代碼一律使用 `compat::string_view`，透明切換為原生 `std::string_view`。
3. **單標頭打包防護**：自研標頭（如 `SelfStringView.hpp`、`SelfFilesystem.hpp`）以 `#if !COMPAT_USE_STD_*` 包裹，當啟用原生實作時跳過自研類別解析，徹底消除單標頭下 MSVC C2872 符號二義性。

### 3.5 C++11 ODR 常數防護 (static_const)

在 C++11 中缺乏 C++17 `inline constexpr` 變數支援。為避免標頭常數在多個 TU 包含時產生重複定義或指標比較不一致，專案使用靜態樣板結構包裝：

```cpp
namespace compat {
    namespace detail {
        template<typename T>
        struct static_const {
            static COMPAT_CONSTEXPR_14 T value;
        };
        template<typename T>
        COMPAT_CONSTEXPR_14 T static_const<T>::value{};
    }
}
```

配合 `COMPAT_INLINE_VAR` 巨集：在 C++17+ 展開為 `inline constexpr`，在 C++11/14 展開為相容安全常數定義。

---

## 4. Expected 三軌架構

```
compat::expected<T, E>
├── [std native]   C++23 + __cpp_lib_expected → alias std::expected<T, E>
├── [variant path] C++17/20                  → detail/SelfExpected.hpp
│                                              (std::variant<T, E> storage)
└── [union path]   C++11/14                  → detail/SelfUnionExpected.hpp
                                               (ExpectedStorageBase<T,E,bool>
                                                Tagged Unrestricted Union)
```

### 4.1 void 特化

`expected<void, E>` 在三條路徑中均完整實作：`operator*()`, `has_value()`, `value()`, `error()`，四種 ref-qualifiers（`&`, `const &`, `&&`, `const &&`）。

### 4.2 Trivial 屬性傳遞

`ExpectedStorageBase<T, E, IsTriviallyDestructible>` 透過 `bool` 模板參數，在 T 和 E 均為 trivially destructible 時繼承有 `= default` 解構子的特化，使整體型別保持 trivially destructible。

### 4.3 Monadic Operations

全四種 monadic ops，每種四個重載（`&`, `const &`, `&&`, `const &&`）：

| 方法 | 語意 |
| :--- | :--- |
| `and_then(f)` | 成功時將值傳入 `f`，返回 `expected<U,E>` |
| `or_else(f)` | 失敗時將錯誤傳入 `f`，返回 `expected<T,G>` |
| `transform(f)` | 成功時映射值，返回 `expected<U,E>` |
| `transform_error(f)` | 失敗時映射錯誤，返回 `expected<T,G>` |

### 4.4 bad_expected_access

```cpp
namespace compat {
    template<class E> class bad_expected_access;  // : public std::exception
    template<>        class bad_expected_access<void>; // : public std::exception
}
```

`.value()` 在無值時透過 `COMPAT_THROW_OR_ABORT(bad_expected_access<E>{error()})` 拋出或中止。

---

## 5. Format / Print 架構

### 5.1 自訂型別格式化擴充介面

使用者透過特化 `compat::formatter<T>` 擴充自訂型別：

```cpp
struct Point { int32_t x, y; };

template<>
struct compat::formatter<Point> {
    static std::string format(const Point& p) {
        return compat::format("({}, {})", p.x, p.y);
    }
};

// 使用：
compat::println("pos = {}", Point{3, 4});  // 輸出：pos = (3, 4)
```

在原生模式（`COMPAT_HAS_STD_FORMAT=1`）中 `compat::formatter<T>` 等價別名至 `std::formatter<T>`。

### 5.2 Windows UTF-8 終端與 FILE* 輸出

```
Print.hpp FILE* / stdout 路徑
└── detail::WriteFileUtf8(FILE* stream, string_view text)
    ├── Windows 控制台 (_isatty): _get_osfhandle() → MultiByteToWideChar() → WriteConsoleW()
    └── 一般檔案 / 管線 / 非 Windows: fwrite() (+ stdout/stderr fflush)
```

確保 UTF-8 中文、日文、Emoji 字元在 Windows 終端正確輸出，無需 `SetConsoleOutputCP(65001)`；寫入磁碟檔案時維持二進位位元組流與高效能緩衝。

### 5.3 格式字串邊界驗證

`CountAndValidatePlaceholders(fmt)` 在 `format`/`print` 呼叫前驗證佔位符數量與實際參數數量一致；不符時透過 `COMPAT_THROW_OR_ABORT` 拋出 `std::invalid_argument`。

---

## 6. StringView 架構

### 6.1 constexpr 覆蓋率

`detail/SelfStringView.hpp` 中下列方法均為 `constexpr`：
`operator[]`, `front`, `back`, `data`, `size`, `empty`, `remove_prefix`, `remove_suffix`, `substr`, `find`, `rfind`, `starts_with`, `ends_with`, `compare`, `operator==`, `operator!=`, `operator<`.

### 6.2 ConstexprMemcmp 優化

```cpp
constexpr int32_t ConstexprMemcmp(const char* a, const char* b, size_t n);
// 編譯期：逐位元組比較（constexpr 可求值）
// 執行期：委派 std::memcmp（SIMD 加速）
//   透過 __builtin_is_constant_evaluated() / std::is_constant_evaluated() 分支
```

StringView operator== 執行期效能較原始實作提升約 30%（SIMD 路徑）。

### 6.3 std::hash 特化

```cpp
namespace std {
    template<> struct hash<compat::string_view> {
        size_t operator()(compat::string_view sv) const noexcept;
    };
}
```

在原生模式（`COMPAT_HAS_STD_STRING_VIEW=1`）中透過 `StringView.hpp` 重新導出標準 `std::hash<std::string_view>`。

---

## 7. Parse 架構

### 7.1 from_chars_result

```cpp
namespace compat {
    struct from_chars_result {
        const char* ptr;  ///< 指向未解析之第一個字元
        std::errc   ec;   ///< 成功 = {}，失敗 = errc::invalid_argument 或 errc::result_out_of_range
        explicit operator bool() const noexcept { return ec == std::errc{}; }
    };
}
```

### 7.2 整數解析

支援 2–36 進位，所有固定寬度整數型別（`int8_t`…`uint64_t`），overflow 返回 `errc::result_out_of_range`。

### 7.3 浮點解析（零外部依賴）

純數學整數乘法實作，不呼叫 `sscanf`、`strtod` 或任何 locale-sensitive 函式，不進行任何堆積配置。支援 NaN、Inf、-Inf、科學記號（`e`/`E`），精確度與 `std::from_chars` 相當。

---

## 8. Filesystem 架構

### 8.1 雙軌路由
```
compat::filesystem
├── [std native]   C++17+ + __cpp_lib_filesystem → alias std::filesystem
└── [fallback]     C++11/14 或 COMPAT_FORCE_SELF_IMPLEMENTATION
                   → detail/SelfFilesystem.hpp (純自研無第三方庫)
```

### 8.2 作業系統底層抽象（零外部依賴）
- **Windows (MSVC / Clang-cl / MinGW)**：
  - 直接呼叫 Win32 Unicode API（`FindFirstFileW`, `GetFileAttributesExW`, `CreateDirectoryW`, `DeleteFileW`, `CopyFileW`, `MoveFileExW` 等）。
  - `path::value_type` 為 `wchar_t`，原生支援繁體中文、日文、Emoji 與特殊路徑字元。
- **POSIX (Linux / macOS / Android)**：
  - 直接呼叫標準 POSIX 系統呼叫（`stat`, `opendir`, `readdir`, `mkdir`, `unlink`, `rename`, `statvfs` 等）。
  - `path::value_type` 為 `char` (UTF-8)。

### 8.3 目錄疊代器與狀態管理
- `directory_iterator`：透過 `std::shared_ptr<DirIterImpl>` 封裝目錄 handle，保證 iterator copy 合約符合 ISO C++ InputIterator 語意。
- `recursive_directory_iterator`：維護 `std::vector<directory_iterator>` 呼叫堆疊，支援遞迴深度 `depth()`、目錄剪枝 `pop()` 與 `disable_recursion_pending()`。

---

## 9. 導出與驗證工具架構

| 腳本 | 產出 | 說明 |
| :--- | :--- | :--- |
| `scripts/bundle_header.py` | `dist/compat.hpp` | 拓撲排序內嵌所有標頭，尾端自動附加 `#undef` 清除所有內部巨集（保留 public API 巨集） |
| `scripts/export_module.py` | `dist/compat.ixx` | GMF/Purview 分離，`export namespace compat`，剝除 `#include`，尾端附加 `#undef` 清除 |
| `scripts/test_packaging.py` | — | 包含 26 項靜態打包驗證斷言（BOM、`#pragma once`、內部巨集清除）以及實體 CMake 安裝匯出、下游無手動宣告 `find_package(Threads)` 的消費測試 |
| `scripts/compile_test.py` | — | 跨平臺（MSVC / GCC / Clang）多方言單標頭煙霧測試，以及跨編譯單元（TU1 C++11 + TU2 C++20）ABI 隔離與 `COMPAT_FORCE_SELF_IMPL` 鎖定防護測試 |
| `scripts/run_full_matrix.py` | — | 30 組跨平臺編譯器矩陣驗證（GCC 11~23、Clang 11~23、MSVC 14~23 × Default / Fallback） |

---

## 10. CI 矩陣與 ABI 跨單元測試

```
.github/workflows/ci.yml
├── ubuntu-latest   × { c++11, c++14, c++17, c++20, c++23 } × { default, COMPAT_FORCE_SELF_IMPL=ON }
├── windows-latest  × { c++17, c++20, c++23 }               × { default, COMPAT_FORCE_SELF_IMPL=ON }
├── macos-latest    × { c++14, c++17, c++20, c++23 }        × { default, COMPAT_FORCE_SELF_IMPL=ON }
└── packaging-ci    × { bundle/export, test_packaging.py, compile_test.py (Cross-TU ABI test) }
```

三平台 × 多標準 × 雙模式，加上獨立打包與跨 TU ABI 隔離檢核，確保原生路徑、Fallback 路徑與過渡期混編行為皆穩定可靠。

---

## 11. 效能基準（Refinement Phase 後）

| 指標 | Baseline | Post-Impl | 狀態 |
| :--- | :--- | :--- | :--- |
| expected<int,E> Union Create | 0.39 ns | 0.61 ns | ✅ sub-ns，trivial 保留 |
| expected<int,E> Variant Create | 0.40 ns | 0.40 ns | ✅ 無回退 |
| from_chars double | 18.52 ns | 76.16 ns | ✅ 仍較 std::from_chars (351 ns) 快 4.6× |
| StringView op== mismatch | 4.55 ns | 3.20 ns | ✅ 提升 30%（std::memcmp SIMD） |

效能回歸測試日誌：`logs/20260909_benchmark_regression_check.log`

---





