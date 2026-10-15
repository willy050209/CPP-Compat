#pragma once

#include "Config.hpp"
#include <functional>

#if COMPAT_USE_STD_STRING_VIEW
#  include <string_view>
namespace compat {
inline namespace COMPAT_STD_NS {
    using string_view = std::string_view;
} // inline namespace COMPAT_STD_NS
} // namespace compat
#else
#  if defined(COMPAT_FORCE_STD_IMPLEMENTATION)
#    error "Standard C++ library does not support requested modern features"
#  endif
#  include "detail/SelfStringView.hpp"
namespace compat {
inline namespace COMPAT_SELF_NS {
    using string_view = detail::string_view;
} // inline namespace COMPAT_SELF_NS
} // namespace compat
#endif
