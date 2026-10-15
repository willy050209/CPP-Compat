#pragma once

#include "Config.hpp"

#if COMPAT_USE_STD_FILESYSTEM
#include <filesystem>

namespace compat {
namespace filesystem {
inline namespace COMPAT_STD_NS {

    using namespace ::std::filesystem;

} // inline namespace COMPAT_STD_NS
} // namespace filesystem
} // namespace compat

#else

#include "detail/SelfFilesystem.hpp"

#endif
