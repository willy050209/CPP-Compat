#pragma once

#include "Config.hpp"
#include "detail/SelfMemory.hpp"

#if COMPAT_USE_STD_RANGES
#  include <memory>

namespace compat {
    namespace ranges {
        inline namespace COMPAT_STD_NS {
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
        } // inline namespace COMPAT_STD_NS
    } // namespace ranges
} // namespace compat
#else
namespace compat {
    namespace ranges {
        inline namespace COMPAT_SELF_NS {
            using namespace ::compat::detail::self_memory::ranges;

            using ::compat::detail::self_memory::ranges::uninitialized_copy_result;
            using ::compat::detail::self_memory::ranges::uninitialized_copy_n_result;
            using ::compat::detail::self_memory::ranges::uninitialized_move_result;
            using ::compat::detail::self_memory::ranges::uninitialized_move_n_result;

            using ::compat::detail::self_memory::ranges::construct_at;
            using ::compat::detail::self_memory::ranges::destroy_at;
            using ::compat::detail::self_memory::ranges::destroy;
            using ::compat::detail::self_memory::ranges::destroy_n;
            using ::compat::detail::self_memory::ranges::uninitialized_copy;
            using ::compat::detail::self_memory::ranges::uninitialized_copy_n;
            using ::compat::detail::self_memory::ranges::uninitialized_fill;
            using ::compat::detail::self_memory::ranges::uninitialized_fill_n;
            using ::compat::detail::self_memory::ranges::uninitialized_move;
            using ::compat::detail::self_memory::ranges::uninitialized_move_n;
            using ::compat::detail::self_memory::ranges::uninitialized_default_construct;
            using ::compat::detail::self_memory::ranges::uninitialized_default_construct_n;
            using ::compat::detail::self_memory::ranges::uninitialized_value_construct;
            using ::compat::detail::self_memory::ranges::uninitialized_value_construct_n;
        } // inline namespace COMPAT_SELF_NS
    } // namespace ranges
} // namespace compat
#endif
