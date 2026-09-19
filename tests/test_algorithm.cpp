#include "test_helpers.hpp"
#include <compat/Algorithm.hpp>
#include <compat/Ranges.hpp>
#include <compat/Memory.hpp>
#include <compat/Optional.hpp>

#include <vector>
#include <string>
#include <numeric>
#include <utility>
#include <random>

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

    void test_search_and_find_algorithms() {
        std::vector<int> haystack = {1, 2, 3, 2, 3, 4};
        std::vector<int> needle = {2, 3};

        TEST_ASSERT(compat::ranges::contains_subrange(haystack, needle));

        std::vector<int> not_found = {9, 8};
        TEST_ASSERT(!compat::ranges::contains_subrange(haystack, not_found));

        auto last_sub = compat::ranges::find_end(haystack, needle);
        TEST_ASSERT(!last_sub.empty() && std::distance(haystack.begin(), last_sub.begin()) == 3);

        auto fl = compat::ranges::find_last(haystack, 3);
        TEST_ASSERT(!fl.empty() && *fl.begin() == 3 && std::distance(haystack.begin(), fl.begin()) == 4);

        auto fl_if = compat::ranges::find_last_if(haystack, [](int x) { return x < 3; });
        TEST_ASSERT(!fl_if.empty() && *fl_if.begin() == 2 && std::distance(haystack.begin(), fl_if.begin()) == 3);

        auto fl_not = compat::ranges::find_last_if_not(haystack, [](int x) { return x >= 3; });
        TEST_ASSERT(!fl_not.empty() && *fl_not.begin() == 2 && std::distance(haystack.begin(), fl_not.begin()) == 3);

        std::vector<int> targets = {9, 3};
        auto fo = compat::ranges::find_first_of(haystack, targets);
        TEST_ASSERT(fo != haystack.end() && *fo == 3 && std::distance(haystack.begin(), fo) == 2);

        std::vector<int> repeat_vec = {1, 2, 2, 2, 3};
        auto sn = compat::ranges::search_n(repeat_vec, 3, 2);
        TEST_ASSERT(!sn.empty() && std::distance(repeat_vec.begin(), sn.begin()) == 1);
    }

    void test_folds() {
        std::vector<int> vals = {1, 2, 3, 4};

        auto fl_res = compat::ranges::fold_left(vals, 0, [](int a, int b) { return a + b; });
        TEST_ASSERT(fl_res == 10);

        auto flf_res = compat::ranges::fold_left_first(vals, [](int a, int b) { return a + b; });
        TEST_ASSERT(flf_res.has_value() && *flf_res == 10);

        std::vector<int> empty_vals;
        auto flf_empty = compat::ranges::fold_left_first(empty_vals, [](int a, int b) { return a + b; });
        TEST_ASSERT(!flf_empty.has_value());

        // fold_right: 1 - (2 - (3 - (4 - 0))) = 1 - (2 - (-1)) = 1 - 3 = -2
        auto fr_res = compat::ranges::fold_right(vals, 0, [](int a, int b) { return a - b; });
        TEST_ASSERT(fr_res == -2);

        auto frl_res = compat::ranges::fold_right_last(vals, [](int a, int b) { return a - b; });
        TEST_ASSERT(frl_res.has_value() && *frl_res == -2);

        auto flw_res = compat::ranges::fold_left_with_iter(vals, 0, [](int a, int b) { return a + b; });
        TEST_ASSERT(flw_res.in == vals.end() && flw_res.value == 10);

        auto flfw_res = compat::ranges::fold_left_first_with_iter(vals, [](int a, int b) { return a + b; });
        TEST_ASSERT(flfw_res.in == vals.end() && flfw_res.value.has_value() && *flfw_res.value == 10);
    }

    void test_copy_and_modifying() {
        std::vector<int> src = {1, 2, 3, 2, 1};
        std::vector<int> dst(5);

        compat::ranges::replace_copy(src, dst.begin(), 2, 99);
        TEST_ASSERT(dst[1] == 99 && dst[3] == 99 && dst[0] == 1);

        compat::ranges::replace_copy_if(src, dst.begin(), [](int x) { return x > 2; }, 0);
        TEST_ASSERT(dst[2] == 0 && dst[0] == 1);

        std::vector<int> rem_dst(3);
        compat::ranges::remove_copy(src, rem_dst.begin(), 2);
        TEST_ASSERT(rem_dst[0] == 1 && rem_dst[1] == 3 && rem_dst[2] == 1);

        std::vector<int> rem_if_dst(3);
        compat::ranges::remove_copy_if(src, rem_if_dst.begin(), [](int x) { return x % 2 == 0; });
        TEST_ASSERT(rem_if_dst[0] == 1 && rem_if_dst[1] == 3 && rem_if_dst[2] == 1);

        std::vector<int> dup = {1, 1, 2, 2, 3, 3};
        std::vector<int> u_dst(3);
        compat::ranges::unique_copy(dup, u_dst.begin());
        TEST_ASSERT(u_dst[0] == 1 && u_dst[1] == 2 && u_dst[2] == 3);

        std::vector<int> r_src = {1, 2, 3, 4, 5};
        std::vector<int> r_dst(5);
        compat::ranges::rotate_copy(r_src, r_src.begin() + 2, r_dst.begin());
        TEST_ASSERT(r_dst[0] == 3 && r_dst[1] == 4 && r_dst[2] == 5 && r_dst[3] == 1 && r_dst[4] == 2);

        std::vector<int> sh_vec = {1, 2, 3, 4, 5};
        auto sh_it = compat::ranges::shift_left(sh_vec, 2);
        TEST_ASSERT(std::distance(sh_it.begin(), sh_it.end()) == 3 && sh_vec[0] == 3 && sh_vec[1] == 4 && sh_vec[2] == 5);

        std::vector<int> sh_r_vec = {1, 2, 3, 4, 5};
        auto sh_r_it = compat::ranges::shift_right(sh_r_vec, 2);
        TEST_ASSERT(std::distance(sh_r_it.begin(), sh_r_it.end()) == 3 && sh_r_vec[2] == 1 && sh_r_vec[3] == 2 && sh_r_vec[4] == 3);

        std::vector<int> shuf_vec = {1, 2, 3, 4, 5};
        std::mt19937 g(1234);
        compat::ranges::shuffle(shuf_vec, g);
        TEST_ASSERT(compat::ranges::is_permutation(shuf_vec, std::vector<int>{1, 2, 3, 4, 5}));

        std::vector<int> sample_src = {10, 20, 30, 40, 50};
        std::vector<int> sample_out(3);
        compat::ranges::sample(sample_src, sample_out.begin(), 3, g);
        TEST_ASSERT(sample_out.size() == 3);
    }

    void test_partition_and_partial_sort() {
        std::vector<int> nums = {1, 2, 3, 4, 5, 6};
        std::vector<int> evens(3);
        std::vector<int> odds(3);
        compat::ranges::partition_copy(nums, evens.begin(), odds.begin(), [](int x) { return x % 2 == 0; });
        TEST_ASSERT(evens[0] == 2 && evens[1] == 4 && evens[2] == 6);
        TEST_ASSERT(odds[0] == 1 && odds[1] == 3 && odds[2] == 5);

        std::vector<int> sp_vec = {1, 2, 3, 4, 5, 6};
        compat::ranges::stable_partition(sp_vec, [](int x) { return x % 2 == 0; });
        TEST_ASSERT(sp_vec[0] == 2 && sp_vec[1] == 4 && sp_vec[2] == 6 && sp_vec[3] == 1 && sp_vec[4] == 3 && sp_vec[5] == 5);

        std::vector<int> ps_vec = {5, 7, 4, 2, 8, 6, 1, 9, 0, 3};
        compat::ranges::partial_sort(ps_vec, ps_vec.begin() + 3);
        TEST_ASSERT(ps_vec[0] == 0 && ps_vec[1] == 1 && ps_vec[2] == 2);

        std::vector<int> psc_src = {5, 7, 4, 2, 8, 6, 1, 9, 0, 3};
        std::vector<int> psc_dst(3);
        compat::ranges::partial_sort_copy(psc_src, psc_dst);
        TEST_ASSERT(psc_dst[0] == 0 && psc_dst[1] == 1 && psc_dst[2] == 2);

        std::vector<int> nth_vec = {5, 6, 4, 3, 2, 6, 7, 9, 3};
        compat::ranges::nth_element(nth_vec, nth_vec.begin() + 4);
        TEST_ASSERT(nth_vec[4] == 5);
    }

    void test_heaps() {
        std::vector<int> heap = {3, 1, 4, 1, 5, 9};
        compat::ranges::make_heap(heap);
        TEST_ASSERT(compat::ranges::is_heap(heap));

        heap.push_back(6);
        compat::ranges::push_heap(heap);
        TEST_ASSERT(compat::ranges::is_heap(heap));

        compat::ranges::pop_heap(heap);
        TEST_ASSERT(heap.back() == 9);
        heap.pop_back();

        compat::ranges::sort_heap(heap);
        TEST_ASSERT(compat::ranges::is_sorted(heap));
    }

    void test_sets_and_merges() {
        std::vector<int> s1 = {1, 2, 3, 4, 5};
        std::vector<int> s2 = {2, 4};
        TEST_ASSERT(compat::ranges::includes(s1, s2));

        std::vector<int> a = {1, 2, 4};
        std::vector<int> b = {2, 3, 5};

        std::vector<int> u(5);
        auto u_res = compat::ranges::set_union(a, b, u.begin());
        TEST_ASSERT(std::distance(u.begin(), u_res.out) == 5);
        TEST_ASSERT(u[0] == 1 && u[1] == 2 && u[2] == 3 && u[3] == 4 && u[4] == 5);

        std::vector<int> inter(1);
        auto i_res = compat::ranges::set_intersection(a, b, inter.begin());
        TEST_ASSERT(std::distance(inter.begin(), i_res.out) == 1 && inter[0] == 2);

        std::vector<int> diff(2);
        auto d_res = compat::ranges::set_difference(a, b, diff.begin());
        TEST_ASSERT(std::distance(diff.begin(), d_res.out) == 2 && diff[0] == 1 && diff[1] == 4);

        std::vector<int> symm(4);
        auto sd_res = compat::ranges::set_symmetric_difference(a, b, symm.begin());
        TEST_ASSERT(std::distance(symm.begin(), sd_res.out) == 4 && symm[0] == 1 && symm[1] == 3 && symm[2] == 4 && symm[3] == 5);

        std::vector<int> m_out(6);
        compat::ranges::merge(a, b, m_out.begin());
        TEST_ASSERT(compat::ranges::is_sorted(m_out));

        std::vector<int> ipm = {1, 3, 5, 2, 4, 6};
        compat::ranges::inplace_merge(ipm, ipm.begin() + 3);
        TEST_ASSERT(compat::ranges::is_sorted(ipm));
    }

    void test_permutations() {
        std::vector<int> p1 = {1, 2, 3};
        std::vector<int> p2 = {3, 1, 2};
        TEST_ASSERT(compat::ranges::is_permutation(p1, p2));

        auto np = compat::ranges::next_permutation(p1);
        TEST_ASSERT(np.found && p1[0] == 1 && p1[1] == 3 && p1[2] == 2);

        auto pp = compat::ranges::prev_permutation(p1);
        TEST_ASSERT(pp.found && p1[0] == 1 && p1[1] == 2 && p1[2] == 3);
    }

    void test_numeric_and_random() {
        std::vector<int> iota_v(5);
        compat::ranges::iota(iota_v, 10);
        TEST_ASSERT(iota_v[0] == 10 && iota_v[4] == 14);

        std::vector<int> rand_v(5);
        std::mt19937 rng(42);
        compat::ranges::generate_random(rand_v, rng);
        TEST_ASSERT(rand_v.size() == 5);
    }

    void test_uninitialized_memory() {
        alignas(alignof(std::string)) char buf[sizeof(std::string) * 3];
        std::string* p = reinterpret_cast<std::string*>(buf);

        compat::ranges::construct_at(p, "hello");
        TEST_ASSERT(*p == "hello");
        compat::ranges::destroy_at(p);

        std::vector<std::string> src_strs;
        src_strs.push_back("alpha");
        src_strs.push_back("beta");
        src_strs.push_back("gamma");

        compat::ranges::uninitialized_copy(src_strs.begin(), src_strs.end(), p, p + 3);
        TEST_ASSERT(p[0] == "alpha" && p[1] == "beta" && p[2] == "gamma");
        compat::ranges::destroy_n(p, 3);
    }

    struct User {
        int id;
        std::string name;
        int score;
    };

    void test_user_overload_resolution() {
        std::vector<User> users = {
            {1, "Alice", 85},
            {2, "Bob", 95},
            {3, "Charlie", 70}
        };

        // User's exact case: sort(users, lambda, &User::score)
        compat::ranges::sort(users, [](int a, int b) { return a > b; }, &User::score);
        TEST_ASSERT(users[0].name == "Bob" && users[0].score == 95);
        TEST_ASSERT(users[1].name == "Alice" && users[1].score == 85);
        TEST_ASSERT(users[2].name == "Charlie" && users[2].score == 70);

        // find_if(users, lambda, &User::score)
        auto it = compat::ranges::find_if(users, [](int s) { return s == 85; }, &User::score);
        TEST_ASSERT(it != users.end() && it->name == "Alice");

        // Raw array with lambda and projection
        User arr[3] = {
            {1, "Alice", 85},
            {2, "Bob", 95},
            {3, "Charlie", 70}
        };
        compat::ranges::sort(arr, [](int a, int b) { return a < b; }, &User::score);
        TEST_ASSERT(arr[0].score == 70 && arr[2].score == 95);
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
    test_search_and_find_algorithms();
    test_folds();
    test_copy_and_modifying();
    test_partition_and_partial_sort();
    test_heaps();
    test_sets_and_merges();
    test_permutations();
    test_numeric_and_random();
    test_uninitialized_memory();
    test_user_overload_resolution();
    std::cout << "  test_algorithm passed." << std::endl;
}
