#pragma once

#include "Config.hpp"
#include "Ranges.hpp"
#include "detail/SelfAlgorithm.hpp"

#if COMPAT_HAS_STD_RANGES
#  include <algorithm>
#  include <functional>

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

#  if COMPAT_HAS_STD_RANGES_CONTAINS
        using std::ranges::contains;
#  else
        using detail::self_algo::ranges::contains;
#  endif

#  if COMPAT_HAS_STD_RANGES_STARTS_WITH
        using std::ranges::starts_with;
        using std::ranges::ends_with;
#  else
        using detail::self_algo::ranges::starts_with;
        using detail::self_algo::ranges::ends_with;
#  endif

#  if COMPAT_HAS_STD_RANGES_FOLD
        using std::ranges::fold_left;
#  else
        using detail::self_algo::ranges::fold_left;
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

        // Partitioning
        using std::ranges::is_partitioned;
        using std::ranges::partition;
        using std::ranges::partition_point;

        // Sorting & Searching
        using std::ranges::is_sorted;
        using std::ranges::is_sorted_until;
        using std::ranges::sort;
        using std::ranges::stable_sort;
        using std::ranges::lower_bound;
        using std::ranges::upper_bound;
        using std::ranges::equal_range;
        using std::ranges::binary_search;

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
