# 檔案系統 (compat::filesystem)

定義於標頭檔 [`<compat/Filesystem.hpp>`](file:///D:/P/CPP/CPP-Compat/include/compat/Filesystem.hpp)。  
所屬命名空間：`compat::filesystem`。

`compat::filesystem` 提供現代 C++ 檔案系統函式庫的向下相容實作，完全對齊 **ISO C++17 `std::filesystem`** 標準規範。  
在支援 C++17 的編譯器下直接透明別名（Type Alias）至 `std::filesystem`；在舊版 C++11/14 環境或強制指定 `COMPAT_FORCE_SELF_IMPLEMENTATION=1` 時自動切換至純自研、零外部第三方依賴（Self-Contained）之向下相容實作引擎 `compat::detail::SelfFilesystem.hpp`。在 Windows 平台直接轉接 Win32 Unicode API（`*W` 函式家族），在 POSIX 平台直接轉接 POSIX 系統呼叫。

---

## 核心型別與常數 (Types & Constants)

### 1. 核心類別與結構

| 類別 / 結構 | 說明 |
| :--- | :--- |
| `compat::filesystem::path` | 檔案系統路徑物件。支援路徑語法分析、跨平台格式轉換、組合、分解、查詢、正規化與元件疊代。 |
| `compat::filesystem::filesystem_error` | 繼承自 `std::system_error`，為檔案系統操作失敗時拋出之標準例外，攜帶 0~2 個關聯路徑。 |
| `compat::filesystem::directory_entry` | 快取單一目錄項目（包含路徑與狀態屬性，如 `status`, `file_size`, `last_write_time`）。 |
| `compat::filesystem::directory_iterator` | 單層目錄項目走訪 InputIterator。 |
| `compat::filesystem::recursive_directory_iterator` | 遞迴目錄樹走訪 InputIterator，支援深度查詢與遞迴剪枝（`pop`, `disable_recursion_pending`）。 |
| `compat::filesystem::file_status` | 儲存檔案類型 (`file_type`) 與存取權限 (`perms`)。 |
| `compat::filesystem::space_info` | 儲存檔案系統磁碟容量資訊（`capacity`, `free`, `available`）。 |

### 2. 列舉與位元旗標

| 列舉型別 | 常數值 | 說明 |
| :--- | :--- | :--- |
| `file_type` | `none`, `not_found`, `regular`, `directory`, `symlink`, `block`, `character`, `fifo`, `socket`, `unknown` | 檔案類型列舉。 |
| `perms` | `none`, `owner_read`, `owner_write`, `owner_exec`, `owner_all`, `group_read`, `group_write`, `group_exec`, `group_all`, `others_read`, `others_write`, `others_exec`, `others_all`, `all`, `set_uid`, `set_gid`, `sticky_bit`, `mask`, `unknown` | POSIX 風格權限位元遮罩（支援 `&, \|, ^, ~, &=, \|=, ^=`）。 |
| `perm_options` | `replace`, `add`, `remove`, `nofollow` | 權限修改模式選項。 |
| `copy_options` | `none`, `skip_existing`, `overwrite_existing`, `update_existing`, `recursive`, `copy_symlinks`, `skip_symlinks`, `directories_only`, `create_symlinks`, `create_hard_links` | 檔案/目錄複製選項。 |
| `directory_options` | `none`, `follow_directory_symlink`, `skip_permission_denied` | 目錄走訪選項。 |

---

## `path` 核心操作 (Path Member Functions)

### 1. 建構與賦值
- 支援從 `std::string` (UTF-8)、`std::wstring`、`const char*`、`const wchar_t*`、字串範圍疊代器建構。
- Windows 原生 `value_type` 為 `wchar_t`，內部保持 Unicode 寬字串；POSIX 為 `char` (UTF-8)。

### 2. 串接與修改
- **路徑組合 (`/=`, `append`)**：依平台偏好分隔符號智慧組合路徑。若右側為絕對路徑，則直接覆寫當前路徑。
- **字串附加 (`+=`, `concat`)**：單純字串銜接，不加入分隔符。
- **修改器**：
  - `make_preferred()`：轉為平台偏好分隔符（Windows: `\`；POSIX: `/`）。
  - `remove_filename()`：移除檔案名稱部分。
  - `replace_filename(const path& p)`：替換檔名。
  - `replace_extension(const path& ext)`：替換副檔名。
  - `clear()`, `swap(path&)`。

### 3. 分解與查詢 (Decomposition & Queries)
- `root_name()`, `root_directory()`, `root_path()`, `relative_path()`, `parent_path()`, `filename()`, `stem()`, `extension()`。
- `empty()`, `is_absolute()`, `is_relative()`, `has_filename()`, `has_parent_path()`, `has_stem()`, `has_extension()`。

### 4. 正規化與疊代器 (Normalization & Iterators)
- `lexically_normal()`：依 ISO C++ 規格消除 `.` 與合法消除 `..`，正規化路徑字面值。
- `begin()`, `end()`：雙向疊代器，走訪路徑中各階層節點元件。

---

## 檔案系統操作 API (Operations)

所有操作函式皆提供「**拋出例外**」與「**接收 `std::error_code& ec`**」雙重載：

| 函式 | 說明 |
| :--- | :--- |
| `exists(p)` | 檢查檔案或目錄是否存在。 |
| `status(p)` / `symlink_status(p)` | 取得檔案狀態與權限資訊。 |
| `is_regular_file(p)` / `is_directory(p)` | 判定是否為普通檔案或目錄。 |
| `is_symlink(p)` / `is_empty(p)` | 判定是否為符號連結或空檔案/空目錄。 |
| `file_size(p)` | 取得普通檔案位元組大小。 |
| `current_path()` / `current_path(p)` | 取得或設定當前行程工作目錄。 |
| `temp_directory_path()` | 取得作業系統暫存目錄路徑。 |
| `create_directory(p)` / `create_directories(p)` | 建立單層目錄或遞迴建立多層目錄。 |
| `remove(p)` | 刪除單一檔案或空目錄。 |
| `remove_all(p)` | 遞迴刪除整個目錄樹及其所有子項目。 |
| `rename(old_p, new_p)` | 檔案或目錄重新命名/搬移。 |
| `copy_file(from, to, options)` | 複製單一檔案，支援覆寫與更新選項。 |
| `copy(from, to, options)` | 複製檔案或遞迴複製目錄樹。 |
| `space(p)` | 取得磁碟容量與可用空間資訊 (`space_info`)。 |
| `absolute(p)` / `canonical(p)` | 轉換為絕對路徑或規範化實體路徑。 |
| `equivalent(p1, p2)` | 檢查兩路徑是否指向相同實體檔案。 |

---

## 目錄走訪範例 (Directory Iteration Example)

```cpp
#include <compat/Filesystem.hpp>
#include <iostream>

namespace fs = compat::filesystem;

void list_directory(const fs::path& dir) {
    if (!fs::exists(dir) || !fs::is_directory(dir)) {
        return;
    }

    // 1. 單層目錄走訪
    for (const auto& entry : fs::directory_iterator(dir)) {
        std::cout << (entry.is_directory() ? "[DIR]  " : "[FILE] ")
                  << entry.path().filename().string() << '\n';
    }

    // 2. 遞迴走訪多層目錄樹
    for (const auto& entry : fs::recursive_directory_iterator(dir)) {
        if (entry.is_regular_file()) {
            std::cout << "Depth " << entry.path().string() << " (" 
                      << entry.file_size() << " bytes)\n";
        }
    }
}
```

---

## 雙軌切換機制 (Dual-Track Mechanism)

- **原生模式 (Native Mode)**：
  在 C++17/20/23 環境且標準庫具備 `<filesystem>` 時，自動使用 `std::filesystem`，零轉接開銷。
- **純自研降階模式 (Fallback Mode)**：
  在舊標準或指定 `-DCOMPAT_FORCE_SELF_IMPLEMENTATION=ON` 時，啟用 `SelfFilesystem.hpp`。完全使用 Win32 Unicode API（Windows）或原生 POSIX 系統呼叫（Linux/macOS），杜絕外部庫相依。
