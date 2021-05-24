// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2017, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#ifndef CHAISCRIPT_STACK_VECTOR_HPP_
#define CHAISCRIPT_STACK_VECTOR_HPP_

#include <cstdint>
#include <string>
#include <type_traits>

template<typename T, std::size_t MaxSize>
struct Stack_Vector {
  constexpr static auto aligned_size = sizeof(T) + (sizeof(T) & std::alignment_of_v<T>) > 0 ? std::alignment_of_v<T> : 0;

  alignas(std::alignment_of_v<T>) char data[aligned_size * MaxSize];

  [[nodiscard]] T &operator[](const std::size_t idx) noexcept { return *reinterpret_cast<T *>(&data + aligned_size * idx); }

  [[nodiscard]] const T &operator[](const std::size_t idx) const noexcept {
    return *reinterpret_cast<const T *>(&data + aligned_size * idx);
  }

  template<typename... Param>
  T &emplace_back(Param &&...param) {
    auto *p = new (&(*this)[m_size++]) T(std::forward<Param>(param)...);
    return *p;
  };

  auto size() const noexcept { return m_size; };

  void pop_back() noexcept(std::is_nothrow_destructible_v<T>) { (*this)[--m_size].~T(); }

  ~Stack_Vector() noexcept(std::is_nothrow_destructible_v<T>) {
    auto loc = m_size - 1;
    for (std::size_t pos = 0; pos < m_size; ++pos) {
      (*this)[loc--].~T();
    }
  }

  std::size_t m_size{0};
};

#endif CHAISCRIPT_STACK_VECTOR_HPP_
