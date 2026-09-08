#pragma once

#include "Config.hpp"
#include "StringView.hpp"
#include "Expected.hpp"
#include "detail/SelfParse.hpp"

namespace compat {

/// <summary>
/// Parses a string_view into the specified type T with fail-fast validation.
/// Fails fast on invalid characters, empty string, or boundary overflow without throwing exceptions.
/// </summary>
/// <typeparam name="T">Target type to parse into.</typeparam>
/// <param name="str">Input string view.</param>
/// <returns>expected containing parsed value on success, or unexpected error description.</returns>
template <typename T>
COMPAT_NODISCARD inline compat::expected<T, compat::string_view> parse(compat::string_view str) noexcept {
    return detail::Parser<T>::parse(str);
}

} // namespace compat
