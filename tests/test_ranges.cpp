#include "test_helpers.hpp"
#include <compat/Ranges.hpp>
#include <compat/View.hpp>

#include <vector>
#include <string>
#include <list>
#include <set>
#include <map>
#include <utility>
#include <type_traits>
#include <memory>

namespace {

    struct MoveOnly {
        int id{0};
        MoveOnly() = default;
        explicit MoveOnly(int i) : id(i) {}
        MoveOnly(const MoveOnly&) = delete;
        MoveOnly& operator=(const MoveOnly&) = delete;
        MoveOnly(MoveOnly&&) noexcept = default;
        MoveOnly& operator=(MoveOnly&&) noexcept = default;
    };

    void test_cpos_and_concepts() {
        int arr[5] = {1, 2, 3, 4, 5};
        TEST_ASSERT(compat::ranges::begin(arr) == arr + 0);
        TEST_ASSERT(compat::ranges::end(arr) == arr + 5);
        TEST_ASSERT(compat::ranges::size(arr) == 5);
        TEST_ASSERT(!compat::ranges::empty(arr));
        TEST_ASSERT(compat::ranges::data(arr) == arr);

        std::vector<int> vec = {10, 20, 30};
        TEST_ASSERT(*compat::ranges::begin(vec) == 10);
        TEST_ASSERT(compat::ranges::size(vec) == 3);
        TEST_ASSERT(compat::ranges::ssize(vec) == 3);
        TEST_ASSERT(!compat::ranges::empty(vec));
        TEST_ASSERT(compat::ranges::data(vec) == vec.data());

        // Concepts / Traits checks
        TEST_ASSERT(compat::ranges::range<std::vector<int>>::value);
        TEST_ASSERT(compat::ranges::sized_range<std::vector<int>>::value);
        TEST_ASSERT(compat::ranges::common_range<std::vector<int>>::value);
        TEST_ASSERT(compat::ranges::bidirectional_range<std::vector<int>>::value);
        TEST_ASSERT(compat::ranges::random_access_range<std::vector<int>>::value);

        // Constant range trait (P2728R6)
        TEST_ASSERT(!compat::ranges::constant_range<std::vector<int>>::value);
        TEST_ASSERT(compat::ranges::constant_range<const std::vector<int>>::value);

        // TEST-RNG-001: single_view must NOT be a borrowed range
#if (COMPAT_CPLUSPLUS >= COMPAT_CXX_14)
        static_assert(!compat::ranges::enable_borrowed_range<compat::ranges::single_view<int>>,
                      "single_view must not be a borrowed range");
#endif
        static_assert(!compat::ranges::borrowed_range<compat::ranges::single_view<int>>::value,
                      "single_view must not satisfy borrowed_range");
    }

    void test_views_all_and_owning() {
        std::vector<int> vec = {1, 2, 3};
        auto all_view = compat::views::all(vec);
        TEST_ASSERT(!all_view.empty());
        TEST_ASSERT(all_view.front() == 1);

        // Owning view with rvalue
        auto own = compat::views::all(std::vector<int>{4, 5, 6});
        TEST_ASSERT(own.size() == 3);
        TEST_ASSERT(own.front() == 4);
        TEST_ASSERT(own.back() == 6);
    }

    void test_views_concat_basic() {
        std::vector<int> v1 = {1, 2, 3};
        std::vector<int> v2 = {4, 5};
        std::vector<int> v3 = {6, 7, 8, 9};

        auto cv = compat::views::concat(v1, v2, v3);
        TEST_ASSERT(cv.size() == 9);

        int expected[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
        size_t idx = 0;
        for (auto it = cv.begin(); it != cv.end(); ++it, ++idx) {
            TEST_ASSERT(*it == expected[idx]);
        }
        TEST_ASSERT(idx == 9);

        // Subscript operator[]
        for (size_t i = 0; i < 9; ++i) {
            TEST_ASSERT(cv[static_cast<std::ptrdiff_t>(i)] == expected[i]);
        }

        // Const view access
        const auto& const_cv = cv;
        TEST_ASSERT(const_cv.size() == 9);
        TEST_ASSERT(const_cv[4] == 5);
        TEST_ASSERT(*const_cv.begin() == 1);
    }

    void test_views_concat_step_backward() {
        // Plan 36 Core Fix 1: Bidirectional step_backward across boundaries and empty subranges
        std::vector<int> v1 = {10, 20};
        std::vector<int> v_empty1;
        std::vector<int> v2 = {30};
        std::vector<int> v_empty2;
        std::vector<int> v3 = {40, 50};

        auto cv = compat::views::concat(v1, v_empty1, v2, v_empty2, v3);
        TEST_ASSERT(cv.size() == 5);

        // Reverse iteration from end()
        auto it = cv.end();
        --it;
        TEST_ASSERT(*it == 50);
        --it;
        TEST_ASSERT(*it == 40);
        --it;
        TEST_ASSERT(*it == 30); // crossed v_empty2
        --it;
        TEST_ASSERT(*it == 20); // crossed v_empty1
        --it;
        TEST_ASSERT(*it == 10);
        TEST_ASSERT(it == cv.begin());
    }

    void test_views_concat_advance_random() {
        // Plan 36 Core Fix 1: Random access jumps (it += n, it -= n, it + n, it - it2)
        std::vector<int> v1 = {1, 2};
        std::vector<int> v2 = {3, 4, 5};
        std::vector<int> v3 = {6, 7};

        auto cv = compat::views::concat(v1, v2, v3);
        auto it = cv.begin();

        it += 4;
        TEST_ASSERT(*it == 5);

        it -= 3;
        TEST_ASSERT(*it == 2);

        auto it2 = it + 5;
        TEST_ASSERT(*it2 == 7);

        TEST_ASSERT(it2 - it == 5);
        TEST_ASSERT(it - it2 == -5);
        TEST_ASSERT(cv.end() - it == 6);

        TEST_ASSERT(it < it2);
        TEST_ASSERT(it2 > it);
        TEST_ASSERT(it <= it2);
        TEST_ASSERT(it2 >= it);

        // Subscript from iterator
        TEST_ASSERT(it[0] == 2);
        TEST_ASSERT(it[2] == 4);
    }

    void test_views_concat_empty_subranges() {
        // Edge cases: empty at beginning, all empty
        std::vector<int> v_empty1;
        std::vector<int> v1 = {100, 200};
        std::vector<int> v_empty2;

        auto cv1 = compat::views::concat(v_empty1, v1, v_empty2);
        TEST_ASSERT(cv1.size() == 2);
        TEST_ASSERT(cv1[0] == 100);
        TEST_ASSERT(cv1[1] == 200);
        TEST_ASSERT(*cv1.begin() == 100);

        // All empty ranges
        std::vector<int> e1, e2, e3;
        auto cv_all_empty = compat::views::concat(e1, e2, e3);
        TEST_ASSERT(cv_all_empty.size() == 0);
        TEST_ASSERT(cv_all_empty.begin() == cv_all_empty.end());
    }

    void test_views_concat_heterogeneous_types() {
        // Plan 36 Core Fix 5: value_type is common_type_t<Views...> (P2542R8)
        std::vector<int> v_int = {1, 2};
        std::vector<double> v_double = {3.5, 4.5};

        auto cv = compat::views::concat(v_int, v_double);
        using concat_t = decltype(cv);
        static_assert(std::is_same<typename concat_t::value_type, double>::value, "value_type must be double");

        TEST_ASSERT(cv.size() == 4);
        TEST_ASSERT(cv[0] == 1.0);
        TEST_ASSERT(cv[2] == 3.5);
    }

    void test_views_cache_latest() {
        // Plan 36 Section 3.2: ISO non-propagating-cache
        std::vector<int> vec = {10, 20, 30};
        auto cached = compat::views::cache_latest(vec);

        auto it = cached.begin();
        TEST_ASSERT(it != cached.end());

        // Dereferencing twice produces same value without side effects
        TEST_ASSERT(*it == 10);
        TEST_ASSERT(*it == 10);

        // Arrow operator on lvalue reference
        TEST_ASSERT(it.operator->() != nullptr);
        TEST_ASSERT(*it.operator->() == 10);

        // Advancing updates cache
        ++it;
        TEST_ASSERT(*it == 20);
        ++it;
        TEST_ASSERT(*it == 30);
        ++it;
        TEST_ASSERT(it == cached.end());

        // Non-propagating move semantics: moving cache_latest_view produces empty cache
        auto cached2 = std::move(cached);
        auto it2 = cached2.begin();
        TEST_ASSERT(*it2 == 10);
    }

    void test_views_as_const() {
        // ISO C++23 / P2278R4 views::as_const
        std::vector<int> vec = {1, 2, 3};
        auto const_v = compat::views::as_const(vec);

        TEST_ASSERT(const_v.size() == 3);
        TEST_ASSERT(*const_v.begin() == 1);
        static_assert(std::is_const<typename std::remove_reference<decltype(*const_v.begin())>::type>::value,
                      "Elements must be const reference");

        // Already constant range returns itself / all
        const std::vector<int> c_vec = {4, 5};
        auto c_res = compat::views::as_const(c_vec);
        TEST_ASSERT(c_res.size() == 2);
        TEST_ASSERT(c_res[0] == 4);
    }

    void test_pipeline_syntax() {
        // Pipeline operator| with views::as_const and views::cache_latest
        std::vector<int> vec = {100, 200, 300};

        auto piped1 = vec | compat::views::as_const;
        TEST_ASSERT(piped1.size() == 3);
        TEST_ASSERT(piped1[0] == 100);

        auto piped2 = vec | compat::views::cache_latest;
        TEST_ASSERT(*piped2.begin() == 100);

        // Composition of closures: (c1 | c2)
        auto comp = compat::views::as_const | compat::views::cache_latest;
        auto piped3 = vec | comp;
        TEST_ASSERT(*piped3.begin() == 100);

        // Chaining: vec | c1 | c2
        auto piped4 = vec | compat::views::as_const | compat::views::cache_latest;
        TEST_ASSERT(*piped4.begin() == 100);
    }

    void test_views_take_while_and_drop_while() {
        std::vector<int> v = {1, 2, 3, 4, 5, 6};

        // take_while
        auto tw = v | compat::views::take_while([](int x) { return x < 4; });
        std::vector<int> tw_res;
        for (auto it = tw.begin(); it != tw.end(); ++it) {
            tw_res.push_back(*it);
        }
        TEST_ASSERT(tw_res.size() == 3);
        TEST_ASSERT(tw_res[0] == 1 && tw_res[1] == 2 && tw_res[2] == 3);

        // drop_while
        auto dw = v | compat::views::drop_while([](int x) { return x < 4; });
        std::vector<int> dw_res;
        for (auto it = dw.begin(); it != dw.end(); ++it) {
            dw_res.push_back(*it);
        }
        TEST_ASSERT(dw_res.size() == 3);
        TEST_ASSERT(dw_res[0] == 4 && dw_res[1] == 5 && dw_res[2] == 6);
    }

    void test_ranges_to() {
        std::vector<int> v = {1, 2, 3, 4, 5};
        auto tw = v | compat::views::take_while([](int x) { return x <= 3; });

        // 1. Concrete container: to<std::vector<int>>(r)
        auto vec1 = compat::ranges::to<std::vector<int>>(tw);
        TEST_ASSERT(vec1.size() == 3);
        TEST_ASSERT(vec1[0] == 1 && vec1[1] == 2 && vec1[2] == 3);

        // 2. Concrete container pipe: r | to<std::vector<int>>()
        auto vec2 = tw | compat::ranges::to<std::vector<int>>();
        TEST_ASSERT(vec2.size() == 3);
        TEST_ASSERT(vec2[0] == 1 && vec2[1] == 2 && vec2[2] == 3);

        // 3. Template template deduction: to<std::vector>(r)
        auto vec3 = compat::ranges::to<std::vector>(tw);
        TEST_ASSERT(vec3.size() == 3);
        TEST_ASSERT(vec3[0] == 1 && vec3[1] == 2 && vec3[2] == 3);

        // 4. Template template pipe: r | to<std::vector>()
        auto vec4 = tw | compat::ranges::to<std::vector>();
        TEST_ASSERT(vec4.size() == 3);
        TEST_ASSERT(vec4[0] == 1 && vec4[1] == 2 && vec4[2] == 3);

        // 5. Deduce std::list
        auto lst = tw | compat::ranges::to<std::list>();
        TEST_ASSERT(lst.size() == 3);
        TEST_ASSERT(lst.front() == 1 && lst.back() == 3);

        // 6. Deduce std::set
        auto st = tw | compat::ranges::to<std::set>();
        TEST_ASSERT(st.size() == 3);
        TEST_ASSERT(st.count(1) == 1 && st.count(3) == 1);

        // 7. Deduce std::map from key-value pairs
        std::vector<std::pair<int, std::string>> pairs = {
            {10, "ten"},
            {20, "twenty"}
        };
        auto m = compat::ranges::to<std::map>(pairs);
        TEST_ASSERT(m.size() == 2);
        TEST_ASSERT(m[10] == "ten");
        TEST_ASSERT(m[20] == "twenty");

        // 8. Pipe to std::map
        auto m_pipe = pairs | compat::ranges::to<std::map>();
        TEST_ASSERT(m_pipe.size() == 2);
        TEST_ASSERT(m_pipe[20] == "twenty");
    }

} // namespace

void run_test_ranges() {
    std::cout << "Running test_ranges (ISO C++26 ranges & views)..." << std::endl;
    test_cpos_and_concepts();
    test_views_all_and_owning();
    test_views_concat_basic();
    test_views_concat_step_backward();
    test_views_concat_advance_random();
    test_views_concat_empty_subranges();
    test_views_concat_heterogeneous_types();
    test_views_cache_latest();
    test_views_as_const();
    test_pipeline_syntax();
    test_views_take_while_and_drop_while();
    test_ranges_to();
    std::cout << "  test_ranges passed." << std::endl;
}
