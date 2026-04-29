// This file is distributed under the BSD License.
// See "license.txt" for details.
// http://www.chaiscript.com

#ifndef CHAISCRIPT_UTILITY_UNICODE_HPP_
#define CHAISCRIPT_UTILITY_UNICODE_HPP_

#include <cstddef>
#include <cstdint>
#include <string>

namespace chaiscript {
  namespace utility {
    namespace unicode {

      inline constexpr std::uint32_t max_codepoint = 0x10FFFF;

      constexpr bool is_surrogate(std::uint32_t cp) noexcept { return cp >= 0xD800 && cp <= 0xDFFF; }

      // Append cp to out as UTF-8. Returns bytes written, or 0 if cp >= 0x200000.
      // Surrogates are not rejected here; callers that care check is_surrogate() first.
      inline std::size_t append_utf8(std::string &out, std::uint32_t cp) {
        if (cp < 0x80) {
          out += static_cast<char>(cp);
          return 1;
        }
        if (cp < 0x800) {
          out += static_cast<char>(0xC0 | (cp >> 6));
          out += static_cast<char>(0x80 | (cp & 0x3F));
          return 2;
        }
        if (cp < 0x10000) {
          out += static_cast<char>(0xE0 | (cp >> 12));
          out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
          out += static_cast<char>(0x80 | (cp & 0x3F));
          return 3;
        }
        if (cp < 0x200000) {
          out += static_cast<char>(0xF0 | (cp >> 18));
          out += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
          out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
          out += static_cast<char>(0x80 | (cp & 0x3F));
          return 4;
        }
        return 0;
      }

      // Append cp to out as UTF-16. Returns code units written, or 0 if cp is
      // a surrogate or > max_codepoint.
      template<typename CharT>
      inline std::size_t append_utf16(std::basic_string<CharT> &out, std::uint32_t cp) {
        if (is_surrogate(cp) || cp > max_codepoint) {
          return 0;
        }
        if (cp < 0x10000) {
          out += static_cast<CharT>(cp);
          return 1;
        }
        const std::uint32_t v = cp - 0x10000;
        out += static_cast<CharT>(0xD800 | (v >> 10));
        out += static_cast<CharT>(0xDC00 | (v & 0x3FF));
        return 2;
      }

      // Append cp to a basic_string<CharT>. Dispatches on sizeof(CharT):
      // 1 byte -> UTF-8, 2 bytes -> UTF-16, 4 bytes -> UTF-32.
      // Returns code units written, or 0 if the codepoint is invalid.
      template<typename CharT>
      inline std::size_t append_codepoint(std::basic_string<CharT> &out, std::uint32_t cp) {
        if constexpr (sizeof(CharT) == 1) {
          std::string tmp;
          const auto n = append_utf8(tmp, cp);
          out.append(tmp.begin(), tmp.end());
          return n;
        } else if constexpr (sizeof(CharT) == 2) {
          return append_utf16(out, cp);
        } else {
          static_assert(sizeof(CharT) == 4, "append_codepoint: unsupported CharT size");
          if (is_surrogate(cp) || cp > max_codepoint) {
            return 0;
          }
          out += static_cast<CharT>(cp);
          return 1;
        }
      }

    } // namespace unicode
  } // namespace utility
} // namespace chaiscript

#endif
