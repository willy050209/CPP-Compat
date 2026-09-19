#pragma once

#include "Config.hpp"
#include "detail/SelfMemory.hpp"

#if COMPAT_HAS_STD_RANGES
#  include <memory>

namespace compat {
    namespace ranges {
        using std::ranges::uninitialized_copy_result;
        using std::ranges::uninitialized_copy_n_result;
        using std::ranges::uninitialized_move_result;
        using std::ranges::uninitialized_move_n_result;

        using std::ranges::construct_at;
        using std::ranges::destroy_at;
        using std::ranges::destroy;
        using std::ranges::destroy_n;
        using std::ranges::uninitialized_copy;
        using std::ranges::uninitialized_copy_n;
        using std::ranges::uninitialized_fill;
        using std::ranges::uninitialized_fill_n;
        using std::ranges::uninitialized_move;
        using std::ranges::uninitialized_move_n;
        using std::ranges::uninitialized_default_construct;
        using std::ranges::uninitialized_default_construct_n;
        using std::ranges::uninitialized_value_construct;
        using std::ranges::uninitialized_value_construct_n;
    } // namespace ranges
} // namespace compat
#else
namespace compat {
    namespace ranges {
        using namespace ::compat::detail::self_memory::ranges;
    } // namespace ranges
} // namespace compat
#endif
