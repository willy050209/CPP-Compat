#include "test_helpers.hpp"
#include <compat/Algorithm.hpp>
#include <compat/Ranges.hpp>

#include <vector>
#include <string>
#include <numeric>
#include <utility>

namespace {

    /// <summary>
    /// 用於測試投影與自訂型別的結構體。
    /// </summary>
    struct Item {
        int id;
        std::string name;

        bool operator==(const Item& other) const {
            return id == other.id && name == other.name;
        }
    };

    void test_non_modifying_algorithms() {
        std::vector<int> vec = {1, 2, 3, 4, 5, 6};

        // all_of, any_of, none_of
        TEST_ASSERT(compat::ranges::all_of(vec, [](int x) { return x > 0; }));
        TEST_ASSERT(!compat::ranges::all_of(vec, [](int x) { return x > 3; }));
        TEST_ASSERT(compat::ranges::any_of(vec, [](int x) { return x == 4; }));
        TEST_ASSERT(!compat::ranges::any_of(vec, [](int x) { return x == 10; }));
        TEST_ASSERT(compat::ranges::none_of(vec, [](int x) { return x < 0; }));
        TEST_ASSERT(!compat::ranges::none_of(vec, [](int x) { return x == 2; }));

        // for_each, for_each_n
        int sum = 0;
        compat::ranges::for_each(vec, [&sum](int x) { sum += x; });
        TEST_ASSERT(sum == 21);

        sum = 0;
        compat::ranges::for_each_n(vec.begin(), 3, [&sum](int x) { sum += x; });
        TEST_ASSERT(sum == 6);

        // count, count_if
        std::vector<int> dup = {1, 2, 2, 3, 2, 4};
        TEST_ASSERT(compat::ranges::count(dup, 2) == 3);
        TEST_ASSERT(compat::ranges::count(dup, 5) == 0);
        TEST_ASSERT(compat::ranges::count_if(dup, [](int x) { return x % 2 == 0; }) == 4);

        // find, find_if, find_if_not
        auto it1 = compat::ranges::find(vec, 3);
        TEST_ASSERT(it1 != vec.end() && *it1 == 3);
        auto it2 = compat::ranges::find_if(vec, [](int x) { return x > 4; });
        TEST_ASSERT(it2 != vec.end() && *it2 == 5);
        auto it3 = compat::ranges::find_if_not(vec, [](int x) { return x < 4; });
        TEST_ASSERT(it3 != vec.end() && *it3 == 4);

        // adjacent_find
        std::vector<int> adj = {1, 2, 3, 3, 4, 5};
        auto it_adj = compat::ranges::adjacent_find(adj);
        TEST_ASSERT(it_adj != adj.end() && *it_adj == 3);

        // mismatch & equal
        std::vector<int> v1 = {1, 2, 3, 4};
        std::vector<int> v2 = {1, 2, 9, 4};
        auto mis = compat::ranges::mismatch(v1, v2);
        TEST_ASSERT(*mis.in1 == 3 && *mis.in2 == 9);
        TEST_ASSERT(!compat::ranges::equal(v1, v2));
        std::vector<int> v3 = {1, 2, 3, 4};
        TEST_ASSERT(compat::ranges::equal(v1, v3));

        // search
        std::vector<int> hay = {10, 20, 30, 40, 50};
        std::vector<int> needle = {30, 40};
        auto s_res = compat::ranges::search(hay, needle);
        TEST_ASSERT(s_res.begin() != hay.end() && *s_res.begin() == 30);

        // contains, starts_with, ends_with
        TEST_ASSERT(compat::ranges::contains(hay, 30));
        TEST_ASSERT(!compat::ranges::contains(hay, 99));

        std::vector<int> prefix = {10, 20};
        std::vector<int> suffix = {40, 50};
        TEST_ASSERT(compat::ranges::starts_with(hay, prefix));
        TEST_ASSERT(compat::ranges::ends_with(hay, suffix));

        // fold_left
        int fold_res = compat::ranges::fold_left(vec, 0, [](int acc, int x) { return acc + x; });
        TEST_ASSERT(fold_res == 21);
    }

    void test_modifying_algorithms() {
        std::vector<int> vec = {1, 2, 3, 4, 5};
        std::vector<int> out(5, 0);

        // copy & copy_n
        compat::ranges::copy(vec, out.begin());
        TEST_ASSERT(out == vec);

        std::vector<int> out_n(3, 0);
        compat::ranges::copy_n(vec.begin(), 3, out_n.begin());
        TEST_ASSERT(out_n[0] == 1 && out_n[1] == 2 && out_n[2] == 3);

        // copy_if
        std::vector<int> evens;
        evens.resize(2);
        compat::ranges::copy_if(vec, evens.begin(), [](int x) { return x % 2 == 0; });
        TEST_ASSERT(evens[0] == 2 && evens[1] == 4);

        // fill & fill_n
        std::vector<int> fv(4, 0);
        compat::ranges::fill(fv, 7);
        TEST_ASSERT(fv[0] == 7 && fv[3] == 7);
        compat::ranges::fill_n(fv.begin(), 2, 9);
        TEST_ASSERT(fv[0] == 9 && fv[1] == 9 && fv[2] == 7);

        // transform unary
        std::vector<int> sq(5, 0);
        compat::ranges::transform(vec, sq.begin(), [](int x) { return x * x; });
        TEST_ASSERT(sq[0] == 1 && sq[1] == 4 && sq[4] == 25);

        // transform binary
        std::vector<int> added(5, 0);
        compat::ranges::transform(vec, sq, added.begin(), [](int a, int b) { return a + b; });
        TEST_ASSERT(added[0] == 2 && added[1] == 6 && added[4] == 30);

        // generate & generate_n
        std::vector<int> gen_vec(3, 0);
        int counter = 10;
        compat::ranges::generate(gen_vec, [&counter]() { return counter++; });
        TEST_ASSERT(gen_vec[0] == 10 && gen_vec[1] == 11 && gen_vec[2] == 12);

        // replace & replace_if
        std::vector<int> rep_vec = {1, 2, 3, 2, 5};
        compat::ranges::replace(rep_vec, 2, 99);
        TEST_ASSERT(rep_vec[1] == 99 && rep_vec[3] == 99);
        compat::ranges::replace_if(rep_vec, [](int x) { return x > 50; }, 0);
        TEST_ASSERT(rep_vec[1] == 0 && rep_vec[3] == 0);

        // reverse
        std::vector<int> rev_vec = {1, 2, 3, 4};
        compat::ranges::reverse(rev_vec);
        TEST_ASSERT(rev_vec[0] == 4 && rev_vec[3] == 1);

        // rotate
        std::vector<int> rot_vec = {1, 2, 3, 4, 5};
        compat::ranges::rotate(rot_vec, rot_vec.begin() + 2);
        TEST_ASSERT(rot_vec[0] == 3 && rot_vec[1] == 4 && rot_vec[2] == 5 && rot_vec[3] == 1 && rot_vec[4] == 2);

        // unique
        std::vector<int> u_vec = {1, 1, 2, 2, 2, 3, 4, 4, 5};
        auto u_res = compat::ranges::unique(u_vec);
        u_vec.erase(u_res.begin(), u_res.end());
        TEST_ASSERT(u_vec.size() == 5);
        TEST_ASSERT(u_vec[0] == 1 && u_vec[1] == 2 && u_vec[4] == 5);
    }

    void test_sorting_and_partitioning() {
        std::vector<int> part_vec = {1, 3, 5, 2, 4, 6};
        TEST_ASSERT(compat::ranges::is_partitioned(part_vec, [](int x) { return x % 2 != 0; }));

        std::vector<int> unpart = {1, 2, 3, 4, 5, 6};
        compat::ranges::partition(unpart, [](int x) { return x % 2 == 0; });
        TEST_ASSERT(compat::ranges::is_partitioned(unpart, [](int x) { return x % 2 == 0; }));

        // is_sorted & sort
        std::vector<int> s_vec = {5, 2, 4, 1, 3};
        TEST_ASSERT(!compat::ranges::is_sorted(s_vec));
        compat::ranges::sort(s_vec);
        TEST_ASSERT(compat::ranges::is_sorted(s_vec));
        TEST_ASSERT(s_vec[0] == 1 && s_vec[4] == 5);

        // lower_bound, upper_bound, binary_search
        TEST_ASSERT(compat::ranges::binary_search(s_vec, 3));
        TEST_ASSERT(!compat::ranges::binary_search(s_vec, 9));
        auto lb = compat::ranges::lower_bound(s_vec, 3);
        TEST_ASSERT(lb != s_vec.end() && *lb == 3);
        auto ub = compat::ranges::upper_bound(s_vec, 3);
        TEST_ASSERT(ub != s_vec.end() && *ub == 4);
    }

    void test_projections() {
        std::vector<Item> items = {
            {3, "Charlie"},
            {1, "Alice"},
            {2, "Bob"}
        };

        // Projection with member pointer &Item::id
        auto min_item = compat::ranges::min_element(items, {}, &Item::id);
        TEST_ASSERT(min_item != items.end() && min_item->id == 1 && min_item->name == "Alice");

        auto max_item = compat::ranges::max_element(items, {}, &Item::id);
        TEST_ASSERT(max_item != items.end() && max_item->id == 3 && max_item->name == "Charlie");

        // Sort by id
        compat::ranges::sort(items, {}, &Item::id);
        TEST_ASSERT(items[0].id == 1 && items[1].id == 2 && items[2].id == 3);

        // Binary search with projection
        bool found = compat::ranges::binary_search(items, 2, {}, &Item::id);
        TEST_ASSERT(found);

        bool not_found = compat::ranges::binary_search(items, 99, {}, &Item::id);
        TEST_ASSERT(!not_found);

        // Find with projection
        auto it = compat::ranges::find(items, "Bob", &Item::name);
        TEST_ASSERT(it != items.end() && it->id == 2);
    }

    void test_min_max_algorithms() {
        std::vector<int> nums = {9, 2, 7, 4, 8, 1};

        TEST_ASSERT(compat::ranges::min(nums) == 1);
        TEST_ASSERT(compat::ranges::max(nums) == 9);

        auto mm = compat::ranges::minmax(nums);
        TEST_ASSERT(mm.min == 1 && mm.max == 9);

        TEST_ASSERT(compat::ranges::clamp(5, 1, 10) == 5);
        TEST_ASSERT(compat::ranges::clamp(0, 1, 10) == 1);
        TEST_ASSERT(compat::ranges::clamp(15, 1, 10) == 10);
    }

    void test_views_pipeline_and_composition() {
        // empty_view
        compat::ranges::empty_view<int> ev;
        TEST_ASSERT(ev.empty());
        TEST_ASSERT(ev.size() == 0);

        // single_view
        auto sv = compat::views::single(42);
        TEST_ASSERT(sv.size() == 1);
        TEST_ASSERT(*sv.begin() == 42);

        // iota_view
        auto iv = compat::views::iota(1, 6);
        TEST_ASSERT(iv.size() == 5);
        TEST_ASSERT(*iv.begin() == 1);

        // filter_view, transform_view, take_view, drop_view, reverse_view
        std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

        // Pipeline: filter evens -> transform double -> take 3
        auto pipeline = numbers
            | compat::views::filter([](int x) { return x % 2 == 0; })
            | compat::views::transform([](int x) { return x * 2; })
            | compat::views::take(3);

        std::vector<int> result;
        for (auto it = pipeline.begin(); it != pipeline.end(); ++it) {
            result.push_back(*it);
        }

        TEST_ASSERT(result.size() == 3);
        TEST_ASSERT(result[0] == 4);  // 2 * 2
        TEST_ASSERT(result[1] == 8);  // 4 * 2
        TEST_ASSERT(result[2] == 12); // 6 * 2

        // drop and reverse
        auto drop_rev = numbers | compat::views::drop(7) | compat::views::reverse;
        std::vector<int> dr_res;
        for (auto it = drop_rev.begin(); it != drop_rev.end(); ++it) {
            dr_res.push_back(*it);
        }
        TEST_ASSERT(dr_res.size() == 3);
        TEST_ASSERT(dr_res[0] == 10);
        TEST_ASSERT(dr_res[1] == 9);
        TEST_ASSERT(dr_res[2] == 8);
    }

} // namespace

void run_test_algorithm() {
    std::cout << "Running test_algorithm (compat::ranges algorithms & views)..." << std::endl;
    test_non_modifying_algorithms();
    test_modifying_algorithms();
    test_sorting_and_partitioning();
    test_projections();
    test_min_max_algorithms();
    test_views_pipeline_and_composition();
    std::cout << "  test_algorithm passed." << std::endl;
}
