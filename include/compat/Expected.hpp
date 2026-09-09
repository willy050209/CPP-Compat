#pragma once

#include "Config.hpp"

#if COMPAT_HAS_STD_EXPECTED
#  include <expected>
namespace compat {
    using std::expected;
    using std::unexpected;
    using std::unexpect_t;
    using std::unexpect;
    using std::bad_expected_access;
}
#elif COMPAT_HAS_STD_VARIANT
#  include "detail/SelfExpected.hpp"
namespace compat {
    using detail::expected;
    using detail::unexpected;
    using detail::unexpect_t;
    using detail::unexpect;
    using detail::bad_expected_access;
}
#else
#  include "detail/SelfUnionExpected.hpp"
namespace compat {
    using detail::expected;
    using detail::unexpected;
    using detail::unexpect_t;
    using detail::unexpect;
    using detail::bad_expected_access;
}
#endif
