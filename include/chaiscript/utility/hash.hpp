// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2017, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_UTILITY_FNV1A_HPP_
#define CHAISCRIPT_UTILITY_FNV1A_HPP_

#include "../chaiscript_defines.hpp"
#include <cstdint>

namespace chaiscript {
  namespace utility {
    namespace fnv1a {
      template<typename Itr>
      static constexpr std::uint32_t hash(Itr begin, Itr end) noexcept {
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
      static constexpr std::uint32_t hash(const char (&str)[N]) noexcept {
        return hash(std::begin(str), std::end(str) - 1);
      }

      static constexpr std::uint32_t hash(std::string_view sv) noexcept {
        return hash(sv.begin(), sv.end());
      }

      static inline std::uint32_t hash(const std::string &s) noexcept {
        return hash(s.begin(), s.end());
      }
    } // namespace fnv1a

    namespace jenkins_one_at_a_time {
      template<typename Itr>
      static constexpr std::uint32_t hash(Itr begin, Itr end) noexcept {
        std::uint32_t hash = 0;

        while (begin != end) {
          hash += std::uint32_t(*begin);
          hash += hash << 10;
          hash ^= hash >> 6;
          ++begin;
        }

        hash += hash << 3;
        hash ^= hash >> 11;
        hash += hash << 15;
        return hash;
      }

      template<size_t N>
      static constexpr std::uint32_t hash(const char (&str)[N]) noexcept {
        return hash(std::begin(str), std::end(str) - 1);
      }

      static constexpr std::uint32_t hash(std::string_view sv) noexcept {
        return hash(sv.begin(), sv.end());
      }

      static inline std::uint32_t hash(const std::string &s) noexcept {
        return hash(s.begin(), s.end());
      }
    } // namespace jenkins_one_at_a_time

    using fnv1a::hash;
  } // namespace utility
} // namespace chaiscript

#endif
