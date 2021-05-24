// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2017, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#ifndef CHAISCRIPT_FUNCTION_PARAMS_HPP
#define CHAISCRIPT_FUNCTION_PARAMS_HPP

#include "boxed_value.hpp"

namespace chaiscript {
  class Function_Params {
  public:
    constexpr Function_Params(const Boxed_Value *const t_begin, const Boxed_Value *const t_end)
        : m_begin(t_begin)
        , m_end(t_end) {
    }

    explicit Function_Params(const Boxed_Value &bv)
        : m_begin(&bv)
        , m_end(m_begin + 1) {
    }

    explicit Function_Params(const std::vector<Boxed_Value> &vec)
        : m_begin(vec.empty() ? nullptr : &vec.front())
        , m_end(vec.empty() ? nullptr : &vec.front() + vec.size()) {
    }

    template<size_t Size>
    constexpr explicit Function_Params(const std::array<Boxed_Value, Size> &a)
        : m_begin(&a.front())
        , m_end(&a.front() + Size) {
    }

    [[nodiscard]] constexpr const Boxed_Value &operator[](const std::size_t t_i) const noexcept { return m_begin[t_i]; }

    [[nodiscard]] constexpr const Boxed_Value *begin() const noexcept { return m_begin; }

    [[nodiscard]] constexpr const Boxed_Value &front() const noexcept { return *m_begin; }

    [[nodiscard]] constexpr const Boxed_Value *end() const noexcept { return m_end; }

    [[nodiscard]] constexpr std::size_t size() const noexcept { return std::size_t(m_end - m_begin); }

    [[nodiscard]] std::vector<Boxed_Value> to_vector() const { return std::vector<Boxed_Value>{m_begin, m_end}; }

    [[nodiscard]] constexpr bool empty() const noexcept { return m_begin == m_end; }

  private:
    const Boxed_Value *m_begin = nullptr;
    const Boxed_Value *m_end = nullptr;
  };

  // Constructor specialization for array of size 0
  template<>
  constexpr Function_Params::Function_Params(const std::array<Boxed_Value, size_t{0}> & /* a */)
      : m_begin(nullptr)
      , m_end(nullptr) {
  }

} // namespace chaiscript

#endif
