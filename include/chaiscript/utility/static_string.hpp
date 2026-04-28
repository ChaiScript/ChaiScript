// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2018, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_UTILITY_STATIC_STRING_HPP_
#define CHAISCRIPT_UTILITY_STATIC_STRING_HPP_

#include <iterator>

namespace chaiscript::utility {
  struct Static_String {
    template<size_t N>
    constexpr Static_String(const char (&str)[N]) noexcept
        : m_size(N - 1)
        , data(&str[0]) {
    }

    constexpr size_t size() const noexcept { return m_size; }

    constexpr const char *c_str() const noexcept { return data; }

    constexpr auto begin() const noexcept { return data; }

    constexpr auto end() const noexcept { return std::next(data, static_cast<std::ptrdiff_t>(m_size)); }

    constexpr auto operator[](const std::size_t idx) const noexcept {
      return *std::next(data, static_cast<std::ptrdiff_t>(idx));
    }

    constexpr bool operator==(std::string_view other) const noexcept {
      return std::string_view(data, m_size) == other;
    }

    constexpr bool operator==(const std::string &t_str) const noexcept { return std::equal(begin(), end(), std::cbegin(t_str), std::cend(t_str)); }

    const size_t m_size;
    const char *data = nullptr;
  };
} // namespace chaiscript::utility

#endif
