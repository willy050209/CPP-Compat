#pragma once

// ============================================================================
// CPP-Compat: Zero-Dependency Modern C++ Backward Compatibility Shim Layer
// SharedMutex.hpp - Reader-Writer Mutex and Shared Lock Abstraction
// ============================================================================

#include "Config.hpp"

#include <cstddef>
#include <utility>

#if COMPAT_HAS_STD_SHARED_MUTEX && !defined(COMPAT_FORCE_SELF_IMPLEMENTATION)
#  include <shared_mutex>

namespace compat {

    /// <summary>
    /// 提供支援多讀取者與單一寫入者之讀寫互斥鎖（對齊 C++17 std::shared_mutex）。
    /// </summary>
    using shared_mutex = std::shared_mutex;

    /// <summary>
    /// 共享鎖 RAII 包裝器模板（對齊 C++14 std::shared_lock）。
    /// </summary>
    /// <typeparam name="Mutex">互斥鎖型別，需滿足 SharedMutex 概念。</typeparam>
    template <typename Mutex>
    using shared_lock = std::shared_lock<Mutex>;

} // namespace compat

#elif COMPAT_HAS_STD_SHARED_TIMED_MUTEX && !defined(COMPAT_FORCE_SELF_IMPLEMENTATION)
#  include <shared_mutex>

namespace compat {

    /// <summary>
    /// 提供支援多讀取者與單一寫入者之讀寫互斥鎖（於 C++14 下對齊 std::shared_timed_mutex）。
    /// </summary>
    using shared_mutex = std::shared_timed_mutex;

    /// <summary>
    /// 共享鎖 RAII 包裝器模板（對齊 C++14 std::shared_lock）。
    /// </summary>
    /// <typeparam name="Mutex">互斥鎖型別，需滿足 SharedMutex 概念。</typeparam>
    template <typename Mutex>
    using shared_lock = std::shared_lock<Mutex>;

} // namespace compat

#else

#  include <mutex>
#  include <condition_variable>
#  include <system_error>

namespace compat {

    /// <summary>
    /// C++11 向下相容之無餓死雙閘門讀寫互斥鎖（對齊 C++17 std::shared_mutex 概念）。
    /// </summary>
    class shared_mutex {
    public:
        /// <summary>
        /// 預設建構子，初始化讀寫鎖。
        /// </summary>
        shared_mutex() : m_readers(0), m_writer(false) {}

        /// <summary>
        /// 解構子。
        /// </summary>
        ~shared_mutex() = default;

        shared_mutex(const shared_mutex&) = delete;
        shared_mutex& operator=(const shared_mutex&) = delete;

        /// <summary>
        /// 獲取排他（寫入）鎖。若其他執行緒持有排他鎖或共享鎖，當前執行緒將阻塞直至取得鎖。
        /// </summary>
        void lock() {
            std::unique_lock<std::mutex> lk(m_mutex);
            m_gate1.wait(lk, [this]() { return !m_writer; });
            m_writer = true;
            m_gate2.wait(lk, [this]() { return m_readers == 0; });
        }

        /// <summary>
        /// 嘗試獲取排他鎖，若無法立即取得則返回 false，不阻塞當前執行緒。
        /// </summary>
        /// <returns>若成功取得鎖返回 true，否則返回 false。</returns>
        COMPAT_NODISCARD bool try_lock() {
            std::unique_lock<std::mutex> lk(m_mutex);
            if (m_writer || m_readers != 0) {
                return false;
            }
            m_writer = true;
            return true;
        }

        /// <summary>
        /// 釋放排他鎖並喚醒等待中的執行緒。
        /// </summary>
        void unlock() {
            {
                std::lock_guard<std::mutex> lk(m_mutex);
                m_writer = false;
            }
            m_gate1.notify_all();
        }

        /// <summary>
        /// 獲取共享（讀取）鎖。若其他執行緒持有排他鎖，當前執行緒將阻塞直至取得鎖。
        /// </summary>
        void lock_shared() {
            std::unique_lock<std::mutex> lk(m_mutex);
            m_gate1.wait(lk, [this]() { return !m_writer; });
            ++m_readers;
        }

        /// <summary>
        /// 嘗試獲取共享鎖，若有寫入者持有或正在請求鎖則返回 false。
        /// </summary>
        /// <returns>若成功取得共享鎖返回 true，否則返回 false。</returns>
        COMPAT_NODISCARD bool try_lock_shared() {
            std::unique_lock<std::mutex> lk(m_mutex);
            if (m_writer) {
                return false;
            }
            ++m_readers;
            return true;
        }

        /// <summary>
        /// 釋放共享鎖。當所有共享鎖釋放完畢且有寫入者等待時，喚醒等待中的寫入者。
        /// </summary>
        void unlock_shared() {
            std::unique_lock<std::mutex> lk(m_mutex);
            --m_readers;
            if (m_writer && m_readers == 0) {
                m_gate2.notify_one();
            }
        }

    private:
        std::mutex m_mutex;
        std::condition_variable m_gate1;
        std::condition_variable m_gate2;
        std::size_t m_readers;
        bool m_writer;
    };

    /// <summary>
    /// C++11 相容之共享鎖 RAII 包裝器模板（對齊 C++14 std::shared_lock）。
    /// </summary>
    /// <typeparam name="Mutex">互斥鎖型別，需滿足 SharedMutex 概念。</typeparam>
    template <typename Mutex>
    class shared_lock {
    public:
        using mutex_type = Mutex;

        /// <summary>
        /// 預設建構子，不關聯任何互斥鎖。
        /// </summary>
        shared_lock() noexcept : m_pm(nullptr), m_owns(false) {}

        /// <summary>
        /// 建構並立即獲取共享鎖。
        /// </summary>
        /// <param name="m">目標互斥鎖參考。</param>
        explicit shared_lock(mutex_type& m) : m_pm(&m), m_owns(true) {
            m_pm->lock_shared();
        }

        /// <summary>
        /// 建構並延遲鎖定（不立即獲取鎖）。
        /// </summary>
        /// <param name="m">目標互斥鎖參考。</param>
        /// <param name="tag">延遲標籤 std::defer_lock。</param>
        shared_lock(mutex_type& m, std::defer_lock_t tag) noexcept : m_pm(&m), m_owns(false) {
            (void)tag;
        }

        /// <summary>
        /// 建構並嘗試獲取共享鎖（非阻塞）。
        /// </summary>
        /// <param name="m">目標互斥鎖參考。</param>
        /// <param name="tag">嘗試鎖定標籤 std::try_to_lock。</param>
        shared_lock(mutex_type& m, std::try_to_lock_t tag) : m_pm(&m), m_owns(m_pm->try_lock_shared()) {
            (void)tag;
        }

        /// <summary>
        /// 接管已被當前執行緒鎖定的互斥鎖所有權。
        /// </summary>
        /// <param name="m">目標互斥鎖參考。</param>
        /// <param name="tag">接管標籤 std::adopt_lock。</param>
        shared_lock(mutex_type& m, std::adopt_lock_t tag) noexcept : m_pm(&m), m_owns(true) {
            (void)tag;
        }

        /// <summary>
        /// 解構子。若仍持有鎖則自動釋放共享鎖。
        /// </summary>
        ~shared_lock() {
            if (m_owns && m_pm) {
                m_pm->unlock_shared();
            }
        }

        shared_lock(const shared_lock&) = delete;
        shared_lock& operator=(const shared_lock&) = delete;

        /// <summary>
        /// 移動建構子。接管來源物件的鎖所有權。
        /// </summary>
        /// <param name="other">來源 shared_lock 物件。</param>
        shared_lock(shared_lock&& other) noexcept : m_pm(other.m_pm), m_owns(other.m_owns) {
            other.m_pm = nullptr;
            other.m_owns = false;
        }

        /// <summary>
        /// 移動賦值運算子。
        /// </summary>
        /// <param name="other">來源 shared_lock 物件。</param>
        /// <returns>自我參考。</returns>
        shared_lock& operator=(shared_lock&& other) noexcept {
            if (this != &other) {
                if (m_owns && m_pm) {
                    m_pm->unlock_shared();
                }
                m_pm = other.m_pm;
                m_owns = other.m_owns;
                other.m_pm = nullptr;
                other.m_owns = false;
            }
            return *this;
        }

        /// <summary>
        /// 獲取共享鎖。
        /// </summary>
        /// <exception cref="std::system_error">若未關聯互斥鎖或已持有鎖時擲出。</exception>
        void lock() {
            if (!m_pm) {
                throw std::system_error(std::make_error_code(std::errc::operation_not_permitted));
            }
            if (m_owns) {
                throw std::system_error(std::make_error_code(std::errc::resource_deadlock_would_occur));
            }
            m_pm->lock_shared();
            m_owns = true;
        }

        /// <summary>
        /// 嘗試獲取共享鎖。
        /// </summary>
        /// <returns>若成功取得鎖返回 true，否則返回 false。</returns>
        /// <exception cref="std::system_error">若未關聯互斥鎖或已持有鎖時擲出。</exception>
        bool try_lock() {
            if (!m_pm) {
                throw std::system_error(std::make_error_code(std::errc::operation_not_permitted));
            }
            if (m_owns) {
                throw std::system_error(std::make_error_code(std::errc::resource_deadlock_would_occur));
            }
            m_owns = m_pm->try_lock_shared();
            return m_owns;
        }

        /// <summary>
        /// 釋放共享鎖。
        /// </summary>
        /// <exception cref="std::system_error">若未關聯互斥鎖或未持有鎖時擲出。</exception>
        void unlock() {
            if (!m_owns || !m_pm) {
                throw std::system_error(std::make_error_code(std::errc::operation_not_permitted));
            }
            m_pm->unlock_shared();
            m_owns = false;
        }

        /// <summary>
        /// 與另一個 shared_lock 交換內部狀態。
        /// </summary>
        /// <param name="other">目標 shared_lock 物件。</param>
        void swap(shared_lock& other) noexcept {
            std::swap(m_pm, other.m_pm);
            std::swap(m_owns, other.m_owns);
        }

        /// <summary>
        /// 釋放所管理的互斥鎖指標並放棄所有權，不進行解鎖。
        /// </summary>
        /// <returns>原關聯之互斥鎖指標。</returns>
        mutex_type* release() noexcept {
            mutex_type* res = m_pm;
            m_pm = nullptr;
            m_owns = false;
            return res;
        }

        /// <summary>
        /// 查詢當前是否持有鎖。
        /// </summary>
        /// <returns>若持有鎖返回 true，否則返回 false。</returns>
        COMPAT_NODISCARD bool owns_lock() const noexcept {
            return m_owns;
        }

        /// <summary>
        /// 布林轉換運算子，相當於 owns_lock()。
        /// </summary>
        explicit operator bool() const noexcept {
            return owns_lock();
        }

        /// <summary>
        /// 獲取關聯之互斥鎖指標。
        /// </summary>
        /// <returns>互斥鎖指標，若未關聯則為 nullptr。</returns>
        COMPAT_NODISCARD mutex_type* mutex() const noexcept {
            return m_pm;
        }

    private:
        mutex_type* m_pm;
        bool m_owns;
    };

    /// <summary>
    /// 非成員 swap 函式。
    /// </summary>
    /// <typeparam name="Mutex">互斥鎖型別。</typeparam>
    /// <param name="x">第一個 shared_lock 物件。</param>
    /// <param name="y">第二個 shared_lock 物件。</param>
    template <typename Mutex>
    inline void swap(shared_lock<Mutex>& x, shared_lock<Mutex>& y) noexcept {
        x.swap(y);
    }

} // namespace compat

#endif
