#ifndef CHAISCRIPT_FUNCTION_SIGNATURE_HPP
#define CHAISCRIPT_FUNCTION_SIGNATURE_HPP

#include <type_traits>

namespace chaiscript::dispatch::detail {
  template<typename... Param>
  struct Function_Params {
  };

  template<typename Ret, typename Params, bool IsNoExcept = false, bool IsMember = false, bool IsMemberObject = false, bool IsObject = false>
  struct Function_Signature {
    using Param_Types = Params;
    using Return_Type = Ret;
    constexpr static const bool is_object = IsObject;
    constexpr static const bool is_member_object = IsMemberObject;
    constexpr static const bool is_noexcept = IsNoExcept;
    template<typename T>
    constexpr Function_Signature(T &&) noexcept {
    }
    constexpr Function_Signature() noexcept = default;
  };

  // Free functions

  template<typename Ret, typename... Param>
  Function_Signature(Ret (*f)(Param...)) -> Function_Signature<Ret, Function_Params<Param...>>;

  template<typename Ret, typename... Param>
  Function_Signature(Ret (*f)(Param...) noexcept) -> Function_Signature<Ret, Function_Params<Param...>, true>;

  // no reference specifier

  template<typename Ret, typename Class, typename... Param>
  Function_Signature(Ret (Class::*f)(Param...) volatile) -> Function_Signature<Ret, Function_Params<volatile Class &, Param...>, false, true>;

  template<typename Ret, typename Class, typename... Param>
  Function_Signature(Ret (Class::*f)(Param...) volatile noexcept)
      -> Function_Signature<Ret, Function_Params<volatile Class &, Param...>, true, true>;

  template<typename Ret, typename Class, typename... Param>
  Function_Signature(Ret (Class::*f)(Param...) volatile const)
      -> Function_Signature<Ret, Function_Params<volatile const Class &, Param...>, false, true>;

  template<typename Ret, typename Class, typename... Param>
  Function_Signature(Ret (Class::*f)(Param...) volatile const noexcept)
      -> Function_Signature<Ret, Function_Params<volatile const Class &, Param...>, true, true>;

  template<typename Ret, typename Class, typename... Param>
  Function_Signature(Ret (Class::*f)(Param...)) -> Function_Signature<Ret, Function_Params<Class &, Param...>, false, true>;

  template<typename Ret, typename Class, typename... Param>
  Function_Signature(Ret (Class::*f)(Param...) noexcept) -> Function_Signature<Ret, Function_Params<Class &, Param...>, true, true>;

  template<typename Ret, typename Class, typename... Param>
  Function_Signature(Ret (Class::*f)(Param...) const) -> Function_Signature<Ret, Function_Params<const Class &, Param...>, false, true>;

  template<typename Ret, typename Class, typename... Param>
  Function_Signature(Ret (Class::*f)(Param...) const noexcept) -> Function_Signature<Ret, Function_Params<const Class &, Param...>, true, true>;

  // & reference specifier

  template<typename Ret, typename Class, typename... Param>
  Function_Signature(Ret (Class::*f)(Param...) volatile &) -> Function_Signature<Ret, Function_Params<volatile Class &, Param...>, false, true>;

  template<typename Ret, typename Class, typename... Param>
  Function_Signature(Ret (Class::*f)(Param...) volatile &noexcept)
      -> Function_Signature<Ret, Function_Params<volatile Class &, Param...>, true, true>;

  template<typename Ret, typename Class, typename... Param>
  Function_Signature(Ret (Class::*f)(Param...) volatile const &)
      -> Function_Signature<Ret, Function_Params<volatile const Class &, Param...>, false, true>;

  template<typename Ret, typename Class, typename... Param>
  Function_Signature(Ret (Class::*f)(Param...) volatile const &noexcept)
      -> Function_Signature<Ret, Function_Params<volatile const Class &, Param...>, true, true>;

  template<typename Ret, typename Class, typename... Param>
  Function_Signature(Ret (Class::*f)(Param...) &) -> Function_Signature<Ret, Function_Params<Class &, Param...>, false, true>;

  template<typename Ret, typename Class, typename... Param>
  Function_Signature(Ret (Class::*f)(Param...) &noexcept) -> Function_Signature<Ret, Function_Params<Class &, Param...>, true, true>;

  template<typename Ret, typename Class, typename... Param>
  Function_Signature(Ret (Class::*f)(Param...) const &) -> Function_Signature<Ret, Function_Params<const Class &, Param...>, false, true>;

  template<typename Ret, typename Class, typename... Param>
  Function_Signature(Ret (Class::*f)(Param...) const &noexcept) -> Function_Signature<Ret, Function_Params<const Class &, Param...>, true, true>;

  // && reference specifier

  template<typename Ret, typename Class, typename... Param>
  Function_Signature(Ret (Class::*f)(Param...) volatile &&) -> Function_Signature<Ret, Function_Params<volatile Class &&, Param...>, false, true>;

  template<typename Ret, typename Class, typename... Param>
  Function_Signature(Ret (Class::*f)(Param...) volatile &&noexcept)
      -> Function_Signature<Ret, Function_Params<volatile Class &&, Param...>, true, true>;

  template<typename Ret, typename Class, typename... Param>
  Function_Signature(Ret (Class::*f)(Param...) volatile const &&)
      -> Function_Signature<Ret, Function_Params<volatile const Class &&, Param...>, false, true>;

  template<typename Ret, typename Class, typename... Param>
  Function_Signature(Ret (Class::*f)(Param...) volatile const &&noexcept)
      -> Function_Signature<Ret, Function_Params<volatile const Class &&, Param...>, true, true>;

  template<typename Ret, typename Class, typename... Param>
  Function_Signature(Ret (Class::*f)(Param...) &&) -> Function_Signature<Ret, Function_Params<Class &&, Param...>, false, true>;

  template<typename Ret, typename Class, typename... Param>
  Function_Signature(Ret (Class::*f)(Param...) &&noexcept) -> Function_Signature<Ret, Function_Params<Class &&, Param...>, true, true>;

  template<typename Ret, typename Class, typename... Param>
  Function_Signature(Ret (Class::*f)(Param...) const &&) -> Function_Signature<Ret, Function_Params<const Class &&, Param...>, false, true>;

  template<typename Ret, typename Class, typename... Param>
  Function_Signature(Ret (Class::*f)(Param...) const &&noexcept)
      -> Function_Signature<Ret, Function_Params<const Class &&, Param...>, true, true>;

  template<typename Ret, typename Class>
  Function_Signature(Ret Class::*f) -> Function_Signature<Ret, Function_Params<Class &>, true, true, true>;

  // primary template handles types that have no nested ::type member:
  template<class, class = std::void_t<>>
  struct has_call_operator : std::false_type {
  };

  // specialization recognizes types that do have a nested ::type member:
  template<class T>
  struct has_call_operator<T, std::void_t<decltype(&T::operator())>> : std::true_type {
  };

  template<typename Func>
  auto function_signature(const Func &f) {
    if constexpr (has_call_operator<Func>::value) {
      return Function_Signature<typename decltype(Function_Signature{&std::decay_t<Func>::operator()})::Return_Type,
                                typename decltype(Function_Signature{&std::decay_t<Func>::operator()})::Param_Types,
                                decltype(Function_Signature{&std::decay_t<Func>::operator()})::is_noexcept,
                                false,
                                false,
                                true>{};
    } else {
      return Function_Signature{f};
    }
  }

} // namespace chaiscript::dispatch::detail

#endif
