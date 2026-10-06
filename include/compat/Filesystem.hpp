#pragma once

#include "Config.hpp"

#if COMPAT_HAS_STD_FILESYSTEM && !defined(COMPAT_FORCE_SELF_IMPLEMENTATION)
#include <filesystem>

namespace compat {
namespace filesystem {

    using namespace ::std::filesystem;

} // namespace filesystem
} // namespace compat

#else

#include "detail/SelfFilesystem.hpp"

#endif
