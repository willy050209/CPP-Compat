#pragma once

#include "Config.hpp"
#include "Ranges.hpp"
#include "Optional.hpp"
#include "detail/SelfAlgorithm.hpp"

#if COMPAT_HAS_STD_RANGES
#  include <algorithm>
#  include <functional>
#  include <numeric>

namespace compat {
    namespace ranges {
        // Result Types
        using std::ranges::in_fun_result;
        using std::ranges::in_in_result;
        using std::ranges::in_out_result;
        using std::ranges::in_in_out_result;
        using std::ranges::in_out_out_result;
        using std::ranges::min_max_result;
        using std::ranges::in_found_result;

        using std::ranges::for_each_result;
        using std::ranges::copy_result;
        using std::ranges::copy_n_result;
        using std::ranges::copy_backward_result;
        using std::ranges::move_result;
        using std::ranges::move_backward_result;
        using std::ranges::mismatch_result;
        using std::ranges::unary_transform_result;
        using std::ranges::binary_transform_result;
        using std::ranges::swap_ranges_result;
        using std::ranges::reverse_copy_result;
        using std::ranges::rotate_copy_result;
        using std::ranges::unique_copy_result;
        using std::ranges::partition_copy_result;
        using std::ranges::minmax_result;
        using std::ranges::minmax_element_result;
        using std::ranges::partial_sort_copy_result;
        using std::ranges::next_permutation_result;
        using std::ranges::prev_permutation_result;

#  if COMPAT_HAS_STD_RANGES_FOLD
        using std::ranges::in_value_result;
        using std::ranges::fold_left_with_iter_result;
        using std::ranges::fold_left_first_with_iter_result;
#  else
        using detail::self_algo::ranges::in_value_result;
        using detail::self_algo::ranges::fold_left_with_iter_result;
        using detail::self_algo::ranges::fold_left_first_with_iter_result;
#  endif

#  if COMPAT_HAS_STD_RANGES_IOTA
        using std::ranges::out_value_result;
        using std::ranges::iota_result;
#  else
        using detail::self_algo::ranges::out_value_result;
        using detail::self_algo::ranges::iota_result;
#  endif

        // Non-modifying sequence operations
        using std::ranges::all_of;
        using std::ranges::any_of;
        using std::ranges::none_of;
        using std::ranges::for_each;
        using std::ranges::for_each_n;
        using std::ranges::count;
        using std::ranges::count_if;
        using std::ranges::mismatch;
        using std::ranges::equal;
        using std::ranges::lexicographical_compare;
        using std::ranges::find;
        using std::ranges::find_if;
        using std::ranges::find_if_not;
        using std::ranges::adjacent_find;
        using std::ranges::search;
        using std::ranges::find_end;
        using std::ranges::find_first_of;
        using std::ranges::search_n;

#  if COMPAT_HAS_STD_RANGES_CONTAINS
        using std::ranges::contains;
        using std::ranges::contains_subrange;
#  else
        using detail::self_algo::ranges::contains;
        using detail::self_algo::ranges::contains_subrange;
#  endif

#  if COMPAT_HAS_STD_RANGES_STARTS_WITH
        using std::ranges::starts_with;
        using std::ranges::ends_with;
#  else
        using detail::self_algo::ranges::starts_with;
        using detail::self_algo::ranges::ends_with;
#  endif

#  if COMPAT_HAS_STD_RANGES_FIND_LAST
        using std::ranges::find_last;
        using std::ranges::find_last_if;
        using std::ranges::find_last_if_not;
#  else
        using detail::self_algo::ranges::find_last;
        using detail::self_algo::ranges::find_last_if;
        using detail::self_algo::ranges::find_last_if_not;
#  endif

#  if COMPAT_HAS_STD_RANGES_FOLD
        using std::ranges::fold_left;
        using std::ranges::fold_left_first;
        using std::ranges::fold_right;
        using std::ranges::fold_right_last;
        using std::ranges::fold_left_with_iter;
        using std::ranges::fold_left_first_with_iter;
#  else
        using detail::self_algo::ranges::fold_left;
        using detail::self_algo::ranges::fold_left_first;
        using detail::self_algo::ranges::fold_right;
        using detail::self_algo::ranges::fold_right_last;
        using detail::self_algo::ranges::fold_left_with_iter;
        using detail::self_algo::ranges::fold_left_first_with_iter;
#  endif

        // Modifying sequence operations
        using std::ranges::copy;
        using std::ranges::copy_if;
        using std::ranges::copy_n;
        using std::ranges::copy_backward;
        using std::ranges::move;
        using std::ranges::move_backward;
        using std::ranges::fill;
        using std::ranges::fill_n;
        using std::ranges::transform;
        using std::ranges::generate;
        using std::ranges::generate_n;
        using std::ranges::remove;
        using std::ranges::remove_if;
        using std::ranges::replace;
        using std::ranges::replace_if;
        using std::ranges::swap_ranges;
        using std::ranges::reverse;
        using std::ranges::reverse_copy;
        using std::ranges::rotate;
        using std::ranges::unique;
        using std::ranges::replace_copy;
        using std::ranges::replace_copy_if;
        using std::ranges::remove_copy;
        using std::ranges::remove_copy_if;
        using std::ranges::unique_copy;
        using std::ranges::rotate_copy;
        using std::ranges::shuffle;
        using std::ranges::sample;

#  if COMPAT_HAS_STD_RANGES_SHIFT
        using std::ranges::shift_left;
        using std::ranges::shift_right;
#  else
        using detail::self_algo::ranges::shift_left;
        using detail::self_algo::ranges::shift_right;
#  endif

        // Partitioning
        using std::ranges::is_partitioned;
        using std::ranges::partition;
        using std::ranges::partition_point;
        using std::ranges::partition_copy;
        using std::ranges::stable_partition;

        // Sorting & Searching
        using std::ranges::is_sorted;
        using std::ranges::is_sorted_until;
        using std::ranges::sort;
        using std::ranges::stable_sort;
        using std::ranges::partial_sort;
        using std::ranges::partial_sort_copy;
        using std::ranges::nth_element;
        using std::ranges::lower_bound;
        using std::ranges::upper_bound;
        using std::ranges::equal_range;
        using std::ranges::binary_search;

        // Heaps
        using std::ranges::is_heap;
        using std::ranges::is_heap_until;
        using std::ranges::push_heap;
        using std::ranges::pop_heap;
        using std::ranges::make_heap;
        using std::ranges::sort_heap;

        // Sets
        using std::ranges::includes;
        using std::ranges::set_union;
        using std::ranges::set_intersection;
        using std::ranges::set_difference;
        using std::ranges::set_symmetric_difference;

        // Merges
        using std::ranges::merge;
        using std::ranges::inplace_merge;

        // Permutations
        using std::ranges::is_permutation;
        using std::ranges::next_permutation;
        using std::ranges::prev_permutation;

        // Numeric / Random
#  if COMPAT_HAS_STD_RANGES_IOTA
        using std::ranges::iota;
#  else
        using detail::self_algo::ranges::iota;
#  endif

#  if COMPAT_HAS_STD_RANGES_GENERATE_RANDOM
        using std::ranges::generate_random;
#  else
        using detail::self_algo::ranges::generate_random;
#  endif

        // Min/Max
        using std::ranges::min_element;
        using std::ranges::max_element;
        using std::ranges::minmax_element;
        using std::ranges::min;
        using std::ranges::max;
        using std::ranges::minmax;
        using std::ranges::clamp;

    } // namespace ranges
} // namespace compat
#else
namespace compat {
    namespace ranges {
        using namespace ::compat::detail::self_algo::ranges;
    }
} // namespace compat
#endif
