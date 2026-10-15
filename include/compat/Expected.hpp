#pragma once

#include "Config.hpp"

#if COMPAT_USE_STD_EXPECTED
#  include <expected>
namespace compat {
inline namespace COMPAT_STD_NS {
    using std::expected;
    using std::unexpected;
    using std::unexpect_t;
    using std::unexpect;
    using std::bad_expected_access;
} // inline namespace COMPAT_STD_NS
} // namespace compat
#elif COMPAT_USE_STD_VARIANT
#  include "detail/SelfExpected.hpp"
namespace compat {
inline namespace COMPAT_SELF_NS {
    using detail::expected;
    using detail::unexpected;
    using detail::unexpect_t;
    using detail::unexpect;
    using detail::bad_expected_access;
} // inline namespace COMPAT_SELF_NS
} // namespace compat
#else
#  include "detail/SelfUnionExpected.hpp"
namespace compat {
inline namespace COMPAT_SELF_NS {
    using detail::expected;
    using detail::unexpected;
    using detail::unexpect_t;
    using detail::unexpect;
    using detail::bad_expected_access;
} // inline namespace COMPAT_SELF_NS
} // namespace compat
#endif
