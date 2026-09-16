# 系統架構手冊 (ARCHITECTURE.md)

## 1. 系統定位

本專案為一個「零外部第三方函式庫依賴（Zero External Dependencies）」的現代 C++ 標準庫向下相容層（Self-Contained Shim Layer）。  
在 C++23/20 支援環境下透明別名至原生 `std`，在 C++11/14/17 等舊環境下自動切換至純自研、輕量且型別安全之 Fallback 實作。

Refinement Phase（2026-09）已完成全套 ISO C++23 對齊、-fno-exceptions 支援、ABI 隔離、Windows UTF-8 終端修正、monadic operations、自訂型別格式化擴充介面，及多維度 CI 矩陣。

---

## 2. 模組架構圖

```
include/compat/
├── Config.hpp             <- 特性檢測巨集中樞（探測 __cpp_lib_*、ABI 隔離、例外開關）
├── StringView.hpp         <- string_view 轉接（C++17 原生 vs C++11 自研 Fallback）
├── Expected.hpp           <- expected/unexpected 轉接（C++23 / C++17 variant / C++11 union）
├── Format.hpp             <- format 轉接（C++20 原生 vs 自研格式引擎）+ compat::formatter<T>
├── Print.hpp              <- print/println 轉接（C++23 原生 vs 自研引擎）+ Windows UTF-8
├── Parse.hpp              <- parse<T>（支援整數、浮點、bool、string）+ from_chars 介面
├── Compat.hpp             <- 總括標頭檔（包含上述所有模組）
└── detail/
    ├── SelfStringView.hpp    <- C++11 自研 string_view（constexpr find/rfind/compare、std::hash）
    ├── SelfUnionExpected.hpp <- C++11/14 Tagged Unrestricted Union 基礎 expected（含 void 特化、monadic ops）
    ├── SelfExpected.hpp      <- C++17+ std::variant 基礎 expected（含 void 特化、monadic ops）
    ├── SelfFormat.hpp        <- 自研格式引擎（支援 compat::formatter<T> 自訂型別）
    ├── SelfPrint.hpp         <- 自研串流輸出引擎（Windows WriteConsoleW UTF-8、fmt arg 邊界驗證）
    └── SelfParse.hpp         <- 自研純函數無例外字串解析引擎（from_chars、零堆積、零 locale）
```

---

## 3. Config.hpp 巨集系統

### 3.1 特性偵測旗標

| 巨集 | 意義 |
| :--- | :--- |
| `COMPAT_HAS_STD_EXPECTED` | `1` 當 `std::expected` 可用（C++23 + `__cpp_lib_expected`） |
| `COMPAT_HAS_STD_FORMAT` | `1` 當 `std::format` 可用（C++20 + `__cpp_lib_format`） |
| `COMPAT_HAS_STD_PRINT` | `1` 當 `std::print` 可用（C++23 + `__cpp_lib_print`） |
| `COMPAT_HAS_STD_STRING_VIEW` | `1` 當 `std::string_view` 可用（C++17+） |
| `COMPAT_HAS_VARIANT` | `1` 當 `std::variant` 可用（C++17+） |

### 3.2 雙軌切換開關（互斥）

| 巨集 | 作用 |
| :--- | :--- |
| `COMPAT_FORCE_SELF_IMPLEMENTATION` | 強制所有模組使用自研 Fallback 實作（別名 `COMPAT_FORCE_FALLBACK`） |
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

### 3.4 ABI 隔離

```cpp
#define COMPAT_ABI_TAG compat_v1_fallback   // (or compat_v1_std)
```

`detail/SelfStringView.hpp` 中的自研型別封裝在 `inline namespace COMPAT_ABI_TAG` 內，確保同一程式中不同 Fallback/原生翻譯單元的 `string_view` 不產生 ODR 衝突。

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

## 8. 導出工具架構

| 腳本 | 產出 | 說明 |
| :--- | :--- | :--- |
| `scripts/bundle_header.py` | `dist/compat.hpp` | 拓撲排序內嵌所有標頭，尾端自動附加 `#undef` 清除所有內部巨集（保留 public API 巨集） |
| `scripts/export_module.py` | `dist/compat.ixx` | GMF/Purview 分離，`export namespace compat`，剝除 `#include`，尾端附加 `#undef` 清除 |
| `scripts/test_packaging.py` | — | 26 項打包驗證斷言（BOM、`#pragma once`、無內部 include、模組標頭、`#undef` 清除、public 巨集保留） |

---

## 9. CI 矩陣

```
.github/workflows/ci.yml
├── ubuntu-latest   × { c++11, c++14, c++17, c++20, c++23 } × { auto, COMPAT_FORCE_SELF_IMPLEMENTATION=ON }
├── windows-latest  × { c++17, c++20, c++23 }               × { auto, COMPAT_FORCE_SELF_IMPLEMENTATION=ON }
└── macos-latest    × { c++14, c++17, c++20, c++23 }        × { auto, COMPAT_FORCE_SELF_IMPLEMENTATION=ON }
```

三平台 × 多標準 × 雙模式，確保原生路徑與 Fallback 路徑行為一致。

---

## 10. 效能基準（Refinement Phase 後）

| 指標 | Baseline | Post-Impl | 狀態 |
| :--- | :--- | :--- | :--- |
| expected<int,E> Union Create | 0.39 ns | 0.61 ns | ✅ sub-ns，trivial 保留 |
| expected<int,E> Variant Create | 0.40 ns | 0.40 ns | ✅ 無回退 |
| from_chars double | 18.52 ns | 76.16 ns | ✅ 仍較 std::from_chars (351 ns) 快 4.6× |
| StringView op== mismatch | 4.55 ns | 3.20 ns | ✅ 提升 30%（std::memcmp SIMD） |

效能回歸測試日誌：`logs/20260909_benchmark_regression_check.log`

---

## 11. BigInt 與 Decimal 高精度數值架構

### 11.1 BigInt (compat::bigint)
- **儲存層 (BigIntStorage)**：
  - 128 位元 SBO（內建 2 個 64-bit limbs 緩衝區）。
  - 當數值大小在 128 位元（約 $3.4 \times 10^{38}$）以內時，達成 **0 次動態記憶體配置 (0 Heap Allocations)**。
  - 超出 128 位元時，透明升級至動態配置陣列。
- **演算法核心 (BigIntCore)**：
  - 加減法：多精度帶進位/借位整數運算。
  - 乘法：基礎 Schoolbook 乘法與 Karatsuba 演算法。
  - 除法與模運算：Knuth Algorithm D 長除法。
  - 位元運算：`&`, `|`, `^`, `~`, `<<`, `>>`。
- **型別互通與邏輯運算**：
  - 自動轉型：支援所有有號/無號整數型別雙向隱式建構與對稱運算。
  - 邏輯運算：符合 C 語言非 0 為 true、0 為 false 語意；提供 `explicit operator bool()` 保持短路求值特性，並重載 `operator!`、`operator&&`、`operator||`。

### 11.2 Decimal (compat::decimal)
- **儲存層 (DecimalStorage)**：
  - 數值結構：未縮放整數 `m_unscaled` (`compat::bigint`) + 縮放因子 `m_scale` (`int64_t`) + 特殊旗標（`m_is_nan`, `m_is_infinity`）。
  - 得益於 `m_unscaled` 的 SBO 機制，38 位十進位有效數字以內的小數運算同樣具備零堆疊配置優勢。
- **演算法核心 (DecimalCore)**：
  - 精度標準：預設對齊 IEEE 754-2008 decimal128（34 位有效十進位數字）。
  - 捨入模式：除不盡或超出目標精度時，採用銀行家捨入法 (Half-Even Rounding)。
  - 科學記號解析：支援 `1.23e-10`、`inf`、`-infinity`、`nan` 等標準輸入解析。
- **整合生態**：
  - 格式化：特化 `compat::formatter<bigint>` 與 `compat::formatter<decimal>`，支援 `{:.2f}` 等精度格式化。
  - 雜湊：特化 `std::hash<compat::bigint>` 與 `std::hash<compat::decimal>`。

