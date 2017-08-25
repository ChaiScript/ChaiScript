// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2017, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_UTILITY_FNV1A_HPP_
#define CHAISCRIPT_UTILITY_FNV1A_HPP_


#include <cstdint>
#include "../chaiscript_defines.hpp"


namespace chaiscript
{


  namespace utility
  {
    template<typename Itr>
    static constexpr std::uint32_t fnv1a_32(Itr begin, Itr end) noexcept {
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif

#ifdef CHAISCRIPT_MSVC
#pragma warning(push)
#pragma warning(disable : 4307)
#endif
      std::uint32_t h = 0x811c9dc5;

      while (begin != end) {
        h = (h ^ (*begin)) * 0x01000193;
        ++begin;
      }
      return h;

#ifdef CHAISCRIPT_MSVC
#pragma warning(pop)
#endif

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

    }


    template<size_t N>
      static constexpr std::uint32_t fnv1a_32(const char (&str)[N]) noexcept {
        return fnv1a_32(std::begin(str), std::end(str)-1);
      }

    static constexpr std::uint32_t fnv1a_32(const std::string_view &sv) noexcept {
      return fnv1a_32(sv.begin(), sv.end());
    }

    static std::uint32_t fnv1a_32(const std::string &s) noexcept {
      return fnv1a_32(s.begin(), s.end());
    }

  }

}

#endif
