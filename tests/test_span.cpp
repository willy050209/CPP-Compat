#include "test_helpers.hpp"
#include <compat/Span.hpp>
#include <vector>
#include <string>
#include <array>

namespace {

#if (COMPAT_CPLUSPLUS >= COMPAT_CXX_14)
    static constexpr int kConstexprArr[] = {10, 20, 30, 40, 50};
#endif

    void test_span_constructors() {
        // 1. 預設建構子
        compat::span<int> empty_s;
        TEST_ASSERT(empty_s.empty());
        TEST_ASSERT(empty_s.size() == 0);
        TEST_ASSERT(empty_s.data() == nullptr);

        // 2. 指標與長度
        int raw[] = {1, 2, 3, 4, 5};
        compat::span<int> ptr_len(raw, 5);
        TEST_ASSERT(!ptr_len.empty());
        TEST_ASSERT(ptr_len.size() == 5);
        TEST_ASSERT(ptr_len.data() == raw);
        TEST_ASSERT(ptr_len[0] == 1 && ptr_len[4] == 5);

        // 3. 指標對 [first, last)
        compat::span<int> ptr_pair(raw + 1, raw + 4);
        TEST_ASSERT(ptr_pair.size() == 3);
        TEST_ASSERT(ptr_pair.front() == 2 && ptr_pair.back() == 4);

        // 4. 原生 C 陣列
        compat::span<int> arr_span(raw);
        TEST_ASSERT(arr_span.size() == 5);
        TEST_ASSERT(arr_span.size_bytes() == 5 * sizeof(int));

        // 5. std::array
        std::array<int, 4> std_arr = {{100, 200, 300, 400}};
        compat::span<int> std_arr_span(std_arr);
        TEST_ASSERT(std_arr_span.size() == 4);
        TEST_ASSERT(std_arr_span[1] == 200);

        // 6. std::vector
        std::vector<int> vec = {11, 22, 33};
        compat::span<int> vec_span(vec);
        TEST_ASSERT(vec_span.size() == 3);
        TEST_ASSERT(vec_span[0] == 11 && vec_span[2] == 33);

        // 7. 轉換建構子 (T -> const T)
        compat::span<const int> const_span = vec_span;
        TEST_ASSERT(const_span.size() == 3);
        TEST_ASSERT(const_span[1] == 22);
    }

    void test_span_access_and_iterators() {
        int nums[] = {10, 20, 30, 40};
        compat::span<int> sp(nums);

        // 元素存取
        TEST_ASSERT(sp[0] == 10);
        TEST_ASSERT(sp[3] == 40);
        TEST_ASSERT(sp.front() == 10);
        TEST_ASSERT(sp.back() == 40);

        // 正向迭代器
        int sum = 0;
        for (auto it = sp.begin(); it != sp.end(); ++it) {
            sum += *it;
        }
        TEST_ASSERT(sum == 100);

        // 反向迭代器
        int rsum = 0;
        for (auto rit = sp.rbegin(); rit != sp.rend(); ++rit) {
            rsum += *rit;
        }
        TEST_ASSERT(rsum == 100);
        TEST_ASSERT(*sp.rbegin() == 40);
    }

    void test_span_subspan_and_slicing() {
        int data[] = {10, 20, 30, 40, 50};
        compat::span<int> sp(data);

        // 1. 標準子區間切片
        auto sub1 = sp.subspan(1, 3);
        TEST_ASSERT(sub1.size() == 3);
        TEST_ASSERT(sub1[0] == 20 && sub1[1] == 30 && sub1[2] == 40);
        TEST_ASSERT(sub1.data() == sp.data() + 1);

        // 2. 預設 count (dynamic_extent) 切片至末端
        auto sub_tail = sp.subspan(2);
        TEST_ASSERT(sub_tail.size() == 3);
        TEST_ASSERT(sub_tail[0] == 30 && sub_tail[2] == 50);

        // 3. 長度為 0 的切片
        auto sub_zero = sp.subspan(2, 0);
        TEST_ASSERT(sub_zero.empty());
        TEST_ASSERT(sub_zero.size() == 0);
        TEST_ASSERT(sub_zero.data() == sp.data() + 2);

        // 4. 末端合法空切片 (offset == size())
        auto sub_end1 = sp.subspan(5);
        TEST_ASSERT(sub_end1.empty());
        TEST_ASSERT(sub_end1.size() == 0);
        TEST_ASSERT(sub_end1.data() == sp.data() + 5);

        auto sub_end2 = sp.subspan(5, 0);
        TEST_ASSERT(sub_end2.empty());
        TEST_ASSERT(sub_end2.size() == 0);
        TEST_ASSERT(sub_end2.data() == sp.data() + 5);

        // 5. 空 span 切片
        compat::span<int> empty_sp;
        auto sub_empty = empty_sp.subspan(0);
        TEST_ASSERT(sub_empty.empty());
        TEST_ASSERT(sub_empty.size() == 0);

        // 6. count 超出剩餘長度時的安全夾取 (Clamping count to maxCount)
        // offset = 3, 剩餘元素為 data[3], data[4] (2 個)
        // 傳入 count = 100，應安全夾取至 2，不發生緩衝區溢位
        auto clamped = sp.subspan(3, 100);
        TEST_ASSERT(clamped.size() == 2);
        TEST_ASSERT(clamped[0] == 40 && clamped[1] == 50);
        TEST_ASSERT(clamped.data() == sp.data() + 3);

        // 7. first() 與 last()
        auto f2 = sp.first(2);
        TEST_ASSERT(f2.size() == 2);
        TEST_ASSERT(f2[0] == 10 && f2[1] == 20);

        auto l2 = sp.last(2);
        TEST_ASSERT(l2.size() == 2);
        TEST_ASSERT(l2[0] == 40 && l2[1] == 50);

        // 8. first/last 夾取安全防護
        auto f_clamp = sp.first(999);
        TEST_ASSERT(f_clamp.size() == 5);
        auto l_clamp = sp.last(999);
        TEST_ASSERT(l_clamp.size() == 5);

        // 9. 編譯期模板切片
        auto tmpl_sub = sp.subspan<1, 2>();
        TEST_ASSERT(tmpl_sub.size() == 2);
        TEST_ASSERT(tmpl_sub[0] == 20 && tmpl_sub[1] == 30);

        auto tmpl_f = sp.first<2>();
        TEST_ASSERT(tmpl_f.size() == 2);
        TEST_ASSERT(tmpl_f[0] == 10 && tmpl_f[1] == 20);

        auto tmpl_l = sp.last<2>();
        TEST_ASSERT(tmpl_l.size() == 2);
        TEST_ASSERT(tmpl_l[0] == 40 && tmpl_l[1] == 50);
    }

    void test_span_bytes_conversion() {
        int data[] = {0x01020304, 0x05060708};
        compat::span<int> sp(data);

        auto bytes = compat::as_bytes(sp);
        TEST_ASSERT(bytes.size() == 2 * sizeof(int));
        TEST_ASSERT(bytes.size_bytes() == 2 * sizeof(int));

        auto wbytes = compat::as_writable_bytes(sp);
        TEST_ASSERT(wbytes.size() == 2 * sizeof(int));
        // 修改位元組驗證可寫
        wbytes[0] = static_cast<typename decltype(wbytes)::element_type>(0x7F);
        TEST_ASSERT(static_cast<unsigned char>(wbytes[0]) == 0x7F);
    }

    void test_span_constexpr() {
#if (COMPAT_CPLUSPLUS >= COMPAT_CXX_14)
        constexpr compat::span<const int> csp(kConstexprArr);
        static_assert(!csp.empty(), "");
        static_assert(csp.size() == 5, "");
        static_assert(csp[0] == 10, "");
        static_assert(csp.front() == 10, "");
        static_assert(csp.back() == 50, "");

        constexpr auto sub = csp.subspan(1, 3);
        static_assert(sub.size() == 3, "");
        static_assert(sub[0] == 20, "");

        constexpr auto sub_end = csp.subspan(5);
        static_assert(sub_end.empty(), "");

        constexpr auto f = csp.first(2);
        static_assert(f.size() == 2, "");
        static_assert(f[1] == 20, "");

        constexpr auto l = csp.last(2);
        static_assert(l.size() == 2, "");
        static_assert(l[0] == 40, "");
#endif
    }

} // namespace

void run_test_span() {
    std::cout << "[TEST] Running test_span (compat::span & subspan bounds)..." << std::endl;
    test_span_constructors();
    test_span_access_and_iterators();
    test_span_subspan_and_slicing();
    test_span_bytes_conversion();
    test_span_constexpr();
    std::cout << "[PASS] test_span passed." << std::endl;
}
