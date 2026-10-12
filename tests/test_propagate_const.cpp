#include "test_helpers.hpp"
#include <compat/PropagateConst.hpp>

#include <memory>
#include <type_traits>
#include <functional>

namespace {

#if (COMPAT_CPLUSPLUS >= COMPAT_CXX_14)
    static constexpr int kConstexprVal = 999;
    constexpr compat::propagate_const<const int*> ce_p(&kConstexprVal);
    static_assert(*ce_p == 999, "constexpr propagate_const dereference failed");
    static_assert(ce_p.get() == &kConstexprVal, "constexpr propagate_const get failed");
    static_assert(bool(ce_p), "constexpr propagate_const bool failed");
#endif

    // 測試原生裸指標之常數傳播與存取
    void test_propagate_const_raw_pointer() {
        int x = 42;

        // 1. 預設建構子
        compat::propagate_const<int*> empty_p;
        TEST_ASSERT(empty_p.get() == nullptr);
        TEST_ASSERT(!empty_p);
        TEST_ASSERT(empty_p == nullptr);

        // 2. 數值建構子
        compat::propagate_const<int*> p(&x);
        TEST_ASSERT(bool(p));
        TEST_ASSERT(p != nullptr);
        TEST_ASSERT(p.get() == &x);
        TEST_ASSERT(*p == 42);

        // 非常數存取：可修改指向之物件
        *p = 100;
        TEST_ASSERT(x == 100);

        // 3. 常數路徑存取：驗證型別與常數傳播
        const compat::propagate_const<int*> cp(&x);
        TEST_ASSERT(bool(cp));
        TEST_ASSERT(*cp == 100);
        TEST_ASSERT(cp.get() == &x);

        // 編譯期型別檢驗：常數存取路徑必須傳回 const int& 與 const int*
        static_assert(std::is_same<decltype(*p), int&>::value,
                      "non-const propagate_const operator* must return int&");
        static_assert(std::is_same<decltype(p.get()), int*>::value,
                      "non-const propagate_const get must return int*");
        static_assert(std::is_same<decltype(*cp), const int&>::value,
                      "const propagate_const operator* must return const int&");
        static_assert(std::is_same<decltype(cp.get()), const int*>::value,
                      "const propagate_const get must return const int*");

        // 4. 裸指標型別之隱含轉換
        int* raw = p;
        TEST_ASSERT(raw == &x);
        const int* craw = cp;
        TEST_ASSERT(craw == &x);
    }

    // 測試智慧指標 (std::unique_ptr)
    void test_propagate_const_unique_ptr() {
        compat::propagate_const<std::unique_ptr<int>> up(std::unique_ptr<int>(new int(55)));
        TEST_ASSERT(bool(up));
        TEST_ASSERT(*up == 55);

        // 非常數寫入
        *up = 88;
        TEST_ASSERT(*up == 88);

        // 移動建構
        compat::propagate_const<std::unique_ptr<int>> up2(std::move(up));
        TEST_ASSERT(!up);
        TEST_ASSERT(bool(up2));
        TEST_ASSERT(*up2 == 88);

        // 移動賦值
        compat::propagate_const<std::unique_ptr<int>> up3;
        up3 = std::move(up2);
        TEST_ASSERT(!up2);
        TEST_ASSERT(bool(up3));
        TEST_ASSERT(*up3 == 88);

        // 複製語意必須顯式被禁用 (ISO TS 契約)
        static_assert(!std::is_copy_constructible<compat::propagate_const<std::unique_ptr<int>>>::value,
                      "propagate_const<unique_ptr> must not be copy constructible");
        static_assert(!std::is_copy_assignable<compat::propagate_const<std::unique_ptr<int>>>::value,
                      "propagate_const<unique_ptr> must not be copy assignable");
        static_assert(!std::is_copy_constructible<compat::propagate_const<int*>>::value,
                      "propagate_const<int*> must not be copy constructible per ISO TS");
        static_assert(!std::is_copy_assignable<compat::propagate_const<int*>>::value,
                      "propagate_const<int*> must not be copy assignable per ISO TS");
    }

    // 測試 Pimpl (Pointer to Implementation) 慣用語法
    struct PimplWidgetImpl {
        int val = 10;
        void mutate(int v) { val = v; }
        int query() const { return val; }
    };

    struct PimplWidget {
        compat::propagate_const<std::unique_ptr<PimplWidgetImpl>> impl;

        PimplWidget() : impl(std::unique_ptr<PimplWidgetImpl>(new PimplWidgetImpl())) {}

        void set_value(int v) { impl->mutate(v); }
        int get_value() const { return impl->query(); }
    };

    void test_propagate_const_pimpl() {
        PimplWidget w;
        TEST_ASSERT(w.get_value() == 10);
        w.set_value(77);
        TEST_ASSERT(w.get_value() == 77);

        const PimplWidget cw(std::move(w));
        TEST_ASSERT(cw.get_value() == 77);
        TEST_ASSERT(cw.impl->query() == 77);
    }

    // 測試 get_underlying
    void test_propagate_const_underlying() {
        int val = 123;
        compat::propagate_const<int*> p(&val);
        TEST_ASSERT(compat::get_underlying(p) == &val);

        const compat::propagate_const<int*> cp(&val);
        TEST_ASSERT(compat::get_underlying(cp) == &val);
    }

    // 測試關係比較運算子
    void test_propagate_const_relational() {
        compat::propagate_const<int*> p_null(nullptr);
        TEST_ASSERT(p_null == nullptr);
        TEST_ASSERT(nullptr == p_null);
        TEST_ASSERT(!(p_null != nullptr));
        TEST_ASSERT(!(nullptr != p_null));

        int a = 10;
        int b = 20;
        compat::propagate_const<int*> pa(&a);
        compat::propagate_const<int*> pb(&b);

        TEST_ASSERT(pa != nullptr);
        TEST_ASSERT(pa != pb);
        TEST_ASSERT(pa == &a);
        TEST_ASSERT(&a == pa);
        TEST_ASSERT(pa != &b);

        if (&a < &b) {
            TEST_ASSERT(pa < pb);
            TEST_ASSERT(pa <= pb);
            TEST_ASSERT(pb > pa);
            TEST_ASSERT(pb >= pa);
            TEST_ASSERT(pa < &b);
            TEST_ASSERT(&a < pb);
        } else {
            TEST_ASSERT(pb < pa);
            TEST_ASSERT(pb <= pa);
            TEST_ASSERT(pa > pb);
            TEST_ASSERT(pa >= pb);
            TEST_ASSERT(pb < &a);
            TEST_ASSERT(&b < pa);
        }
    }

    // 測試交換 (swap)
    void test_propagate_const_swap() {
        int v1 = 111, v2 = 222;
        compat::propagate_const<int*> s1(&v1);
        compat::propagate_const<int*> s2(&v2);

        s1.swap(s2);
        TEST_ASSERT(*s1 == 222);
        TEST_ASSERT(*s2 == 111);

        using std::swap;
        swap(s1, s2);
        TEST_ASSERT(*s1 == 111);
        TEST_ASSERT(*s2 == 222);
    }

    // 測試 std::hash 與比較函式物件
    void test_propagate_const_hash_and_functional() {
        int val = 555;
        compat::propagate_const<int*> p(&val);

        std::hash<compat::propagate_const<int*>> h;
        std::hash<int*> rh;
        TEST_ASSERT(h(p) == rh(&val));

        compat::propagate_const<int*> p2(&val);
        std::equal_to<compat::propagate_const<int*>> eq;
        TEST_ASSERT(eq(p, p2));

        std::not_equal_to<compat::propagate_const<int*>> neq;
        TEST_ASSERT(!neq(p, p2));
    }

} // namespace

void run_test_propagate_const() {
    std::cout << "[TEST] Running test_propagate_const (std::experimental::propagate_const)..." << std::endl;
    test_propagate_const_raw_pointer();
    test_propagate_const_unique_ptr();
    test_propagate_const_pimpl();
    test_propagate_const_underlying();
    test_propagate_const_relational();
    test_propagate_const_swap();
    test_propagate_const_hash_and_functional();
    std::cout << "[PASS] test_propagate_const passed." << std::endl;
}
