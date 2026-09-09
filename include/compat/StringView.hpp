#pragma once

#include "Config.hpp"
#include <functional>

#if COMPAT_HAS_STD_STRING_VIEW
#  include <string_view>
namespace compat {
    using string_view = std::string_view;
}
#else
#  if defined(COMPAT_FORCE_STD_IMPLEMENTATION)
#    error "Standard C++ library does not support requested modern features"
#  endif
#  include "detail/SelfStringView.hpp"
namespace compat {
    using string_view = detail::string_view;
}
#endif
