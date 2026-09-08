#pragma once

#include "Config.hpp"

#if COMPAT_HAS_STD_STRING_VIEW
#  include <string_view>
namespace compat {
    using string_view = std::string_view;
}
#else
#  include "detail/SelfStringView.hpp"
namespace compat {
    using string_view = detail::string_view;
}
#endif
