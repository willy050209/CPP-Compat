#pragma once

#include "Config.hpp"
#include "detail/SelfRanges.hpp"

#if COMPAT_HAS_STD_RANGES
#  include <ranges>
namespace compat {
    namespace ranges {
        using std::ranges::begin;
        using std::ranges::end;
        using std::ranges::cbegin;
        using std::ranges::cend;
        using std::ranges::rbegin;
        using std::ranges::rend;
        using std::ranges::crbegin;
        using std::ranges::crend;
        using std::ranges::size;
        using std::ranges::ssize;
        using std::ranges::empty;
        using std::ranges::data;
        using std::ranges::cdata;
        using detail::self_ranges::ranges::default_sentinel_t;
        using detail::self_ranges::ranges::default_sentinel;
        using std::ranges::dangling;
        using std::ranges::subrange_kind;
        using std::ranges::view_base;
        using std::ranges::view_interface;
        using std::ranges::subrange;
        using std::ranges::owning_view;
        using detail::self_ranges::ranges::range_adaptor_closure;
        using std::iter_difference_t;
        using std::iter_value_t;
        using std::iter_reference_t;

        using std::ranges::iterator_t;
        using std::ranges::sentinel_t;
        using std::ranges::range_difference_t;
        using std::ranges::range_value_t;
        using std::ranges::range_reference_t;
        using std::ranges::range_rvalue_reference_t;
        using std::ranges::range_size_t;

        template <typename R> using range = detail::self_ranges::ranges::range<R>;
        template <typename R> using sized_range = detail::self_ranges::ranges::sized_range<R>;
        template <typename R> using common_range = detail::self_ranges::ranges::common_range<R>;
        template <typename R> using input_range = detail::self_ranges::ranges::input_range<R>;
        template <typename R> using forward_range = detail::self_ranges::ranges::forward_range<R>;
        template <typename R> using bidirectional_range = detail::self_ranges::ranges::bidirectional_range<R>;
        template <typename R> using random_access_range = detail::self_ranges::ranges::random_access_range<R>;
        template <typename R> using constant_range = detail::self_ranges::ranges::constant_range<R>;
        template <typename R> using borrowed_range = detail::self_ranges::ranges::borrowed_range<R>;
        template <typename R> using view = detail::self_ranges::ranges::view<R>;
        template <typename R> using viewable_range = detail::self_ranges::ranges::viewable_range<R>;

        using std::ranges::dangling;
        using std::ranges::borrowed_iterator_t;
        using std::ranges::borrowed_subrange_t;
        using std::ranges::enable_borrowed_range;

        using std::ranges::empty_view;
        using std::ranges::single_view;
        using std::ranges::iota_view;
        using std::ranges::filter_view;
        using std::ranges::transform_view;
        using std::ranges::take_view;
        using std::ranges::take_while_view;
        using std::ranges::drop_view;
        using std::ranges::drop_while_view;
        using std::ranges::reverse_view;

        using detail::self_ranges::ranges::sentinel_for;
        using detail::self_ranges::ranges::sized_sentinel_for;

#  if COMPAT_HAS_STD_VIEWS_CONCAT
        using std::ranges::concat_view;
#  else
        using detail::self_ranges::ranges::concat_view;
#  endif
#  if COMPAT_HAS_STD_VIEWS_CACHE_LATEST
        using std::ranges::cache_latest_view;
#  else
        using detail::self_ranges::ranges::cache_latest_view;
#  endif
#  if COMPAT_HAS_STD_VIEWS_AS_CONST
        using std::ranges::as_const_view;
#  else
        using detail::self_ranges::ranges::as_const_view;
#  endif
#  if COMPAT_HAS_STD_RANGES_TO
        using std::ranges::to;
#  else
        using detail::self_ranges::ranges::to;
#  endif
    } // namespace ranges

    namespace views {
        using std::views::all;
        using std::views::all_t;
        using std::views::empty;
        using std::views::single;
        using std::views::iota;
        using std::views::filter;
        using std::views::transform;
        using std::views::take;
        using std::views::take_while;
        using std::views::drop;
        using std::views::drop_while;
        using std::views::reverse;

#  if COMPAT_HAS_STD_VIEWS_CONCAT
        using std::views::concat;
#  else
        using detail::self_ranges::views::concat;
#  endif
#  if COMPAT_HAS_STD_VIEWS_CACHE_LATEST
        using std::views::cache_latest;
#  else
        using detail::self_ranges::views::cache_latest;
#  endif
#  if COMPAT_HAS_STD_VIEWS_AS_CONST
        using std::views::as_const;
#  else
        using detail::self_ranges::views::as_const;
#  endif
    } // namespace views

    namespace ranges {
        namespace views = compat::views;
    }

#  if COMPAT_HAS_STD_RANGES_TO
    using std::from_range_t;
    using std::from_range;
#  else
    using ::compat::detail::self_ranges::ranges::from_range_t;
    using ::compat::detail::self_ranges::ranges::from_range;
#  endif

    namespace ranges {
        using ::compat::from_range_t;
        using ::compat::from_range;
    }

#  if (COMPAT_CPLUSPLUS >= COMPAT_CXX_20) && defined(__cpp_lib_common_reference)
    using std::common_reference;
    using std::common_reference_t;
#  else
    using detail::self_ranges::common_reference;
    using detail::self_ranges::common_reference_t;
#  endif
} // namespace compat
#else
namespace compat {
    using ::compat::detail::self_ranges::ranges::from_range_t;
    using ::compat::detail::self_ranges::ranges::from_range;

    namespace ranges {
        using namespace ::compat::detail::self_ranges::ranges;
    }
    namespace views {
        using namespace ::compat::detail::self_ranges::views;
    }
    namespace ranges {
        namespace views = compat::views;
    }

    using detail::self_ranges::common_reference;
    using detail::self_ranges::common_reference_t;
} // namespace compat
#endif
