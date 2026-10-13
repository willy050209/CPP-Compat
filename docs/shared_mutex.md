# 讀寫互斥鎖與共享鎖 (compat::shared_mutex & compat::shared_lock)

定義於標頭檔 [`<compat/SharedMutex.hpp>`](file:///D:/P/CPP/CPP-Compat/include/compat/SharedMutex.hpp)。  
亦可透過總括標頭檔 [`<compat/Compat.hpp>`](file:///D:/P/CPP/CPP-Compat/include/compat/Compat.hpp) 引入。  
所屬命名空間：`compat`。

`compat::shared_mutex` 與 `compat::shared_lock` 為現代 C++ 多執行緒讀寫並發控制提供跨標準之統一抽象，分別對齊 **C++17 `std::shared_mutex`** 與 **C++14 `std::shared_lock`** 規範。  
它解決了 C++14 下不同編譯器標準庫實作差異（如 GCC libstdc++ 缺少 `std::shared_mutex`），並提供向下相容至 **C++11** 的無餓死雙閘門讀寫鎖與完整 RAII 共享鎖包裝器。

---

## 核心痛點與設計動機

### C++14 標準庫演進落差

- **C++14 (`-std=c++14`)**：C++14 標準的 `<shared_mutex>` 僅引入了 `std::shared_timed_mutex` 與 `std::shared_lock`。更輕量的非定時讀寫互斥鎖 `std::shared_mutex` 遲至 **C++17** 才正式入標。
- **編譯器行為分歧**：
  - **MSVC STL**：在 `/std:c++14` 模式下即將 `std::shared_mutex` 作為全模式擴充提供。
  - **GNU libstdc++**：嚴格遵循 ISO 標準，透過 `#if __cplusplus >= 201703L` 守護 `std::shared_mutex`，在 `-std=c++14` 下宣告 `std::shared_mutex` 會直接觸發編譯錯誤。
  - **C++11**：標準庫完全不包含 `<shared_mutex>`。

### `compat::shared_mutex` 的解決方案

1. **C++17+ / MSVC C++14+**：原生透明別名至 `std::shared_mutex` 與 `std::shared_lock`，享受極致零成本原生效能。
2. **C++14 (GCC / Clang libstdc++)**：透明映射至 `<shared_mutex>` 中的 `std::shared_timed_mutex` 與 `std::shared_lock`。其完整符合 `SharedMutex` 概念契約，提供相同語意與執行期鎖定保障。
3. **C++11 / 強制 Fallback 模式**：
   - 採用基於 `<mutex>` 與 `<condition_variable>` 實作的雙閘門（Two-gate fair RWLock）讀寫鎖，杜絕讀取者餓死（Reader Starvation）與死鎖。
   - 提供 100% 符合 RAII 規範之 `compat::shared_lock` 模板類別，支援延遲鎖定 (`defer_lock`)、嘗試鎖定 (`try_to_lock`)、接管鎖定 (`adopt_lock`)、移動語意、`release()`、`swap()` 與狀態查詢。

---

## 類別與型別摘要

### `compat::shared_mutex`

滿足 C++ `SharedMutex` 概念要求：

| 成員函式 | 說明 | 複雜度 |
| :--- | :--- | :---: |
| `shared_mutex()` | 預設建構子，初始化讀寫鎖。 | $O(1)$ |
| `~shared_mutex()` | 解構子。 | $O(1)$ |
| `lock()` | 獲取排他（寫入）鎖。若其他執行緒持有排他鎖或共享鎖，當前執行緒阻塞。 | 阻塞直至獲取 |
| `try_lock()` | 嘗試獲取排他鎖，若無法立即取得則返回 `false`，不阻塞。 | $O(1)$ |
| `unlock()` | 釋放排他鎖並喚醒等待中的執行緒。 | $O(1)$ |
| `lock_shared()` | 獲取共享（讀取）鎖。若有寫入者持有排他鎖，當前執行緒阻塞。 | 阻塞直至獲取 |
| `try_lock_shared()` | 嘗試獲取共享鎖，若有寫入者持有則返回 `false`，不阻塞。 | $O(1)$ |
| `unlock_shared()` | 釋放共享鎖。當所有共享鎖釋放完畢且有寫入者等待時，喚醒寫入者。 | $O(1)$ |

*註：複製建構子與複製賦值運算子被顯式刪除 (`= delete`)。*

### `compat::shared_lock<Mutex>`

RAII 共享鎖管理器模板：

| 成員函式 / 運算子 | 說明 |
| :--- | :--- |
| `shared_lock() noexcept` | 預設建構子，不關聯任何互斥鎖。 |
| `explicit shared_lock(Mutex& m)` | 建構並立即獲取共享鎖 (`m.lock_shared()`)。 |
| `shared_lock(Mutex& m, std::defer_lock_t) noexcept` | 建構並延遲鎖定（不立即獲取鎖）。 |
| `shared_lock(Mutex& m, std::try_to_lock_t)` | 建構並嘗試非阻塞獲取共享鎖 (`m.try_lock_shared()`)。 |
| `shared_lock(Mutex& m, std::adopt_lock_t) noexcept` | 建構並接管已被當前執行緒鎖定的互斥鎖所有權。 |
| `~shared_lock()` | 解構子。若仍持有鎖則自動呼叫 `m->unlock_shared()`。 |
| `shared_lock(shared_lock&& other) noexcept` | 移動建構子。接管來源物件的鎖所有權。 |
| `operator=(shared_lock&& other) noexcept` | 移動賦值運算子。 |
| `lock()` | 獲取共享鎖。 |
| `try_lock()` | 嘗試獲取共享鎖。 |
| `unlock()` | 釋放共享鎖。 |
| `release() noexcept` | 放棄對互斥鎖的管理，不進行解鎖，返回底層互斥鎖指標。 |
| `owns_lock() const noexcept` | 查詢是否持有鎖。 |
| `explicit operator bool() const noexcept` | 等價於 `owns_lock()`。 |
| `mutex() const noexcept` | 獲取底層關聯之互斥鎖指標。 |
| `swap(shared_lock& other) noexcept` | 交換兩個 shared_lock 物件的狀態。 |

---

## 範例程式碼

### 典型讀寫鎖應用

```cpp
#include <compat/SharedMutex.hpp>
#include <vector>
#include <string>

class ThreadSafeCache {
public:
    std::string read(size_t index) const {
        // 多個執行緒可並行獲取共享鎖進行安全讀取
        compat::shared_lock<compat::shared_mutex> lock(m_mutex);
        if (index < m_data.size()) {
            return m_data[index];
        }
        return "";
    }

    void write(std::string value) {
        // 單一寫入者獲取排他鎖，阻擋所有其他讀取者與寫入者
        std::unique_lock<compat::shared_mutex> lock(m_mutex);
        m_data.push_back(std::move(value));
    }

private:
    mutable compat::shared_mutex m_mutex;
    std::vector<std::string> m_data;
};
```
