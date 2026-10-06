#pragma once

#include "Config.hpp"

#include <cstddef>
#include <type_traits>
#include <iterator>
#include <array>

#if COMPAT_HAS_STD_SPAN
#  include <span>
#endif

#if COMPAT_HAS_STD_SPAN && defined(COMPAT_FORCE_STD_IMPLEMENTATION)

namespace compat {
    using std::byte;
    using std::span;
    using std::dynamic_extent;
    using std::as_bytes;
    using std::as_writable_bytes;
} // namespace compat

#else

namespace compat {

#if defined(__cpp_lib_byte) || (COMPAT_CPLUSPLUS >= COMPAT_CXX_17)
    using byte = std::byte;
#else
    enum class byte : unsigned char {};
#endif

    /// <summary>
    /// ISO C++20 動態範圍標記常數 (對齊 std::dynamic_extent)。
    /// </summary>
    COMPAT_INLINE_VAR constexpr std::size_t dynamic_extent = static_cast<std::size_t>(-1);

    namespace detail {
        namespace span_detail {

            template <typename T>
            using remove_cv_t = typename std::remove_cv<T>::type;

            template <typename C, typename T>
            struct is_contiguous_container {
            private:
                template <typename U>
                static auto test(int) -> decltype(
                    std::declval<U>().data(),
                    std::declval<U>().size(),
                    std::true_type{}
                );

                template <typename>
                static std::false_type test(...);

            public:
                static constexpr bool value = decltype(test<C>(0))::value;
            };

        } // namespace span_detail
    } // namespace detail

    /// <summary>
    /// ISO C++20 連續記憶體切片視圖 (對齊 std::span)。
    /// 提供零複製、型別安全且向下相容 C++11 的連續緩衝區引用。
    /// </summary>
    /// <typeparam name="T">元素型別。</typeparam>
    /// <typeparam name="Extent">視圖長度；預設為 dynamic_extent (動態長度)。</typeparam>
    template <typename T, std::size_t Extent = dynamic_extent>
    class span {
    public:
        using element_type           = T;
        using value_type             = typename std::remove_cv<T>::type;
        using size_type              = std::size_t;
        using difference_type        = std::ptrdiff_t;
        using pointer                = T*;
        using const_pointer          = const T*;
        using reference              = T&;
        using const_reference        = const T&;
        using iterator               = T*;
        using const_iterator         = const T*;
        using reverse_iterator       = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        static constexpr std::size_t extent = Extent;

        // --------------------------------------------------------------------
        // 建構子 (Constructors)
        // --------------------------------------------------------------------

        /// <summary>
        /// 預設建構子，建立空視圖。
        /// </summary>
        COMPAT_CONSTEXPR_14 span() noexcept : m_data(nullptr), m_size(0) {
            static_assert(Extent == 0 || Extent == dynamic_extent, "Static extent must be 0 for default constructor");
        }

        /// <summary>
        /// 依據指標與元素數量建構視圖。
        /// </summary>
        /// <param name="ptr">起始指標。</param>
        /// <param name="count">元素個數。</param>
        COMPAT_CONSTEXPR_14 span(pointer ptr, size_type count) noexcept
            : m_data(ptr), m_size(count) {
            COMPAT_ASSERT(Extent == dynamic_extent || count == Extent);
        }

        /// <summary>
        /// 依據指標區間 [first, last) 建構視圖。
        /// </summary>
        /// <param name="first">起始指標。</param>
        /// <param name="last">末端哨兵指標。</param>
        COMPAT_CONSTEXPR_14 span(pointer first, pointer last) noexcept
            : m_data(first), m_size(static_cast<size_type>(last - first)) {
            COMPAT_ASSERT(last >= first);
            COMPAT_ASSERT(Extent == dynamic_extent || static_cast<size_type>(last - first) == Extent);
        }

        /// <summary>
        /// 依據原生 C 陣列參考建構視圖。
        /// </summary>
        template <std::size_t N,
                  typename = typename std::enable_if<(Extent == dynamic_extent || Extent == N)>::type>
        COMPAT_CONSTEXPR_14 span(element_type (&arr)[N]) noexcept
            : m_data(arr), m_size(N) {}

        /// <summary>
        /// 依據 std::array 建構視圖。
        /// </summary>
        template <std::size_t N,
                  typename = typename std::enable_if<(Extent == dynamic_extent || Extent == N)>::type>
        COMPAT_CONSTEXPR_14 span(std::array<value_type, N>& arr) noexcept
            : m_data(arr.data()), m_size(N) {}

        /// <summary>
        /// 依據 const std::array 建構唯讀視圖。
        /// </summary>
        template <std::size_t N,
                  typename = typename std::enable_if<(Extent == dynamic_extent || Extent == N) && std::is_const<element_type>::value>::type>
        COMPAT_CONSTEXPR_14 span(const std::array<value_type, N>& arr) noexcept
            : m_data(arr.data()), m_size(N) {}

        /// <summary>
        /// 依據具備連續記憶體的容器 (如 std::vector, std::string) 建構視圖。
        /// </summary>
        template <typename Container,
                  typename = typename std::enable_if<
                      !std::is_same<typename std::decay<Container>::type, span>::value &&
                      detail::span_detail::is_contiguous_container<Container, element_type>::value &&
                      std::is_convertible<decltype(std::declval<Container&>().data()), pointer>::value
                  >::type>
        COMPAT_CONSTEXPR_14 span(Container& cont) noexcept
            : m_data(cont.data()), m_size(cont.size()) {
            COMPAT_ASSERT(Extent == dynamic_extent || cont.size() == Extent);
        }

        /// <summary>
        /// 依據唯讀容器建構唯讀視圖。
        /// </summary>
        template <typename Container,
                  typename = typename std::enable_if<
                      !std::is_same<typename std::decay<Container>::type, span>::value &&
                      std::is_const<element_type>::value &&
                      detail::span_detail::is_contiguous_container<const Container, element_type>::value &&
                      std::is_convertible<decltype(std::declval<const Container&>().data()), pointer>::value
                  >::type>
        COMPAT_CONSTEXPR_14 span(const Container& cont) noexcept
            : m_data(cont.data()), m_size(cont.size()) {
            COMPAT_ASSERT(Extent == dynamic_extent || cont.size() == Extent);
        }

        /// <summary>
        /// 轉換建構子：允許從型別相容之 span (如非 const 至 const) 轉換。
        /// </summary>
        template <typename OtherType, std::size_t OtherExtent,
                  typename = typename std::enable_if<
                      (Extent == dynamic_extent || Extent == OtherExtent) &&
                      std::is_convertible<OtherType(*)[], element_type(*)[]>::value
                  >::type>
        COMPAT_CONSTEXPR_14 span(const span<OtherType, OtherExtent>& other) noexcept
            : m_data(other.data()), m_size(other.size()) {}

        constexpr span(const span&) noexcept = default;
        COMPAT_CONSTEXPR_14 span& operator=(const span&) noexcept = default;

#if COMPAT_HAS_STD_SPAN
        /// <summary>
        /// 與 C++20 std::span 互相轉換建構子。
        /// </summary>
        constexpr span(std::span<element_type, Extent> s) noexcept : m_data(s.data()), m_size(s.size()) {}

        /// <summary>
        /// 隱含轉換為 C++20 std::span。
        /// </summary>
        constexpr operator std::span<element_type, Extent>() const noexcept {
            return std::span<element_type, Extent>(m_data, size());
        }
#endif

        // --------------------------------------------------------------------
        // 觀察器 (Observers)
        // --------------------------------------------------------------------

        /// <summary>
        /// 取得指向底層資料緩衝區的原始指標。
        /// </summary>
        COMPAT_NODISCARD COMPAT_CONSTEXPR_14 pointer data() const noexcept { return m_data; }

        /// <summary>
        /// 取得視圖包含的元素個數。
        /// </summary>
        COMPAT_NODISCARD COMPAT_CONSTEXPR_14 size_type size() const noexcept {
            return (Extent == dynamic_extent) ? m_size : Extent;
        }

        /// <summary>
        /// 取得視圖佔用的總位元組大小。
        /// </summary>
        COMPAT_NODISCARD COMPAT_CONSTEXPR_14 size_type size_bytes() const noexcept {
            return size() * sizeof(element_type);
        }

        /// <summary>
        /// 檢查視圖是否為空。
        /// </summary>
        COMPAT_NODISCARD COMPAT_CONSTEXPR_14 bool empty() const noexcept { return size() == 0; }

        // --------------------------------------------------------------------
        // 元素存取 (Element Access)
        // --------------------------------------------------------------------

        COMPAT_NODISCARD COMPAT_CONSTEXPR_14 reference operator[](size_type idx) const noexcept {
            COMPAT_ASSERT(idx < size());
            return m_data[idx];
        }

        COMPAT_NODISCARD COMPAT_CONSTEXPR_14 reference front() const noexcept {
            COMPAT_ASSERT(!empty());
            return m_data[0];
        }

        COMPAT_NODISCARD COMPAT_CONSTEXPR_14 reference back() const noexcept {
            COMPAT_ASSERT(!empty());
            return m_data[size() - 1];
        }

        // --------------------------------------------------------------------
        // 迭代器支援 (Iterators)
        // --------------------------------------------------------------------

        COMPAT_NODISCARD COMPAT_CONSTEXPR_14 iterator begin() const noexcept { return m_data; }
        COMPAT_NODISCARD COMPAT_CONSTEXPR_14 iterator end() const noexcept { return m_data + size(); }
        COMPAT_NODISCARD COMPAT_CONSTEXPR_14 const_iterator cbegin() const noexcept { return m_data; }
        COMPAT_NODISCARD COMPAT_CONSTEXPR_14 const_iterator cend() const noexcept { return m_data + size(); }

        COMPAT_NODISCARD COMPAT_CONSTEXPR_14 reverse_iterator rbegin() const noexcept {
            return reverse_iterator(end());
        }
        COMPAT_NODISCARD COMPAT_CONSTEXPR_14 reverse_iterator rend() const noexcept {
            return reverse_iterator(begin());
        }
        COMPAT_NODISCARD COMPAT_CONSTEXPR_14 const_reverse_iterator crbegin() const noexcept {
            return const_reverse_iterator(cend());
        }
        COMPAT_NODISCARD COMPAT_CONSTEXPR_14 const_reverse_iterator crend() const noexcept {
            return const_reverse_iterator(cbegin());
        }

        // --------------------------------------------------------------------
        // 子視圖切片 (Subviews: subspan, first, last)
        // --------------------------------------------------------------------

        /// <summary>
        /// 取得從 offset 開始、長度為 count 的子視圖 (subspan)。
        /// 實作具備雙重保護：
        /// 1. Debug 模式驗證 offset &lt;= size() 與 count &lt;= size() - offset；
        /// 2. Release 模式自動箝位防止無號數下溢或越界緩衝區建構。
        /// </summary>
        /// <param name="offset">起始偏移量。</param>
        /// <param name="count">欲截取的元素個數；預設為 dynamic_extent，表示截取至末端。</param>
        /// <returns>子視圖 span 物件。</returns>
        COMPAT_NODISCARD COMPAT_CONSTEXPR_14 span<element_type, dynamic_extent> subspan(
            size_type offset, size_type count = dynamic_extent) const noexcept {
            // 合約前置條件檢驗 (Fail-Fast)
            COMPAT_ASSERT(offset <= size());

            // 邊界防禦：若 offset 越界則安全箝位至末端空區間
            if (COMPAT_UNLIKELY(offset > size())) {
                return span<element_type, dynamic_extent>(m_data ? (m_data + size()) : nullptr, static_cast<size_type>(0));
            }

            size_type max_count = size() - offset;

            // 安全計算子區間長度：未指定 (dynamic_extent) 或超過剩餘長度時安全夾取至 max_count
            size_type actual_count = (count == dynamic_extent || count > max_count) ? max_count : count;

            return span<element_type, dynamic_extent>(m_data + offset, actual_count);
        }

        /// <summary>
        /// 編譯期樣板子視圖 (Compile-time Template Subspan)。
        /// </summary>
        template <std::size_t Offset, std::size_t Count = dynamic_extent>
        COMPAT_NODISCARD COMPAT_CONSTEXPR_14 auto subspan() const noexcept
            -> span<element_type, (Count != dynamic_extent ? Count : (Extent != dynamic_extent ? Extent - Offset : dynamic_extent))> {
            static_assert(Extent == dynamic_extent || Offset <= Extent, "Offset out of bounds");
            static_assert(Extent == dynamic_extent || Count == dynamic_extent || Count <= Extent - Offset, "Count out of bounds");
            COMPAT_ASSERT(Offset <= size());
            size_type max_c = (Offset <= size()) ? (size() - Offset) : 0;
            size_type actual_c = (Count == dynamic_extent || Count > max_c) ? max_c : Count;
            return span<element_type, (Count != dynamic_extent ? Count : (Extent != dynamic_extent ? Extent - Offset : dynamic_extent))>(
                m_data + Offset, actual_c
            );
        }

        /// <summary>
        /// 取得視圖前 count 個元素的前綴子視圖。
        /// </summary>
        COMPAT_NODISCARD COMPAT_CONSTEXPR_14 span<element_type, dynamic_extent> first(size_type count) const noexcept {
            size_type safe_count = (count > size()) ? size() : count;
            return span<element_type, dynamic_extent>(m_data, safe_count);
        }

        /// <summary>
        /// 編譯期樣板前綴子視圖。
        /// </summary>
        template <std::size_t Count>
        COMPAT_NODISCARD COMPAT_CONSTEXPR_14 span<element_type, Count> first() const noexcept {
            static_assert(Extent == dynamic_extent || Count <= Extent, "Count out of bounds");
            COMPAT_ASSERT(Count <= size());
            return span<element_type, Count>(m_data, Count);
        }

        /// <summary>
        /// 取得視圖末尾 count 個元素的後綴子視圖。
        /// </summary>
        COMPAT_NODISCARD COMPAT_CONSTEXPR_14 span<element_type, dynamic_extent> last(size_type count) const noexcept {
            size_type safe_count = (count > size()) ? size() : count;
            return span<element_type, dynamic_extent>(m_data + (size() - safe_count), safe_count);
        }

        /// <summary>
        /// 編譯期樣板後綴子視圖。
        /// </summary>
        template <std::size_t Count>
        COMPAT_NODISCARD COMPAT_CONSTEXPR_14 span<element_type, Count> last() const noexcept {
            static_assert(Extent == dynamic_extent || Count <= Extent, "Count out of bounds");
            COMPAT_ASSERT(Count <= size());
            return span<element_type, Count>(m_data + (size() - Count), Count);
        }

    private:
        pointer   m_data = nullptr;
        size_type m_size = 0;
    };

    // ------------------------------------------------------------------------
    // as_bytes & as_writable_bytes
    // ------------------------------------------------------------------------

    /// <summary>
    /// 將任意型別的 span 轉換為唯讀位元組視圖 (對齊 std::as_bytes)。
    /// </summary>
    template <typename T, std::size_t Extent>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14
    span<const byte, (Extent == dynamic_extent ? dynamic_extent : Extent * sizeof(T))>
    as_bytes(span<T, Extent> s) noexcept {
        return span<const byte, (Extent == dynamic_extent ? dynamic_extent : Extent * sizeof(T))>(
            reinterpret_cast<const byte*>(s.data()),
            s.size_bytes()
        );
    }

    /// <summary>
    /// 將非常數型別的 span 轉換為可寫位元組視圖 (對齊 std::as_writable_bytes)。
    /// </summary>
    template <typename T, std::size_t Extent,
              typename = typename std::enable_if<!std::is_const<T>::value>::type>
    COMPAT_NODISCARD COMPAT_CONSTEXPR_14
    span<byte, (Extent == dynamic_extent ? dynamic_extent : Extent * sizeof(T))>
    as_writable_bytes(span<T, Extent> s) noexcept {
        return span<byte, (Extent == dynamic_extent ? dynamic_extent : Extent * sizeof(T))>(
            reinterpret_cast<byte*>(s.data()),
            s.size_bytes()
        );
    }

} // namespace compat

#endif
