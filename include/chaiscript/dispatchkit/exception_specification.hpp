// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2017, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#ifndef CHAISCRIPT_EXCEPTION_SPECIFICATION_HPP_
#define CHAISCRIPT_EXCEPTION_SPECIFICATION_HPP_

#include <memory>

#include "../chaiscript_defines.hpp"
#include "boxed_cast.hpp"

namespace chaiscript {
class Boxed_Value;
namespace exception {
class bad_boxed_cast;
}  // namespace exception
}  // namespace chaiscript

namespace chaiscript
{

  namespace detail
  {
    struct Exception_Handler
    {
      virtual void operator()(const Boxed_Value &, const detail::Dispatch_Engine &) const
      {
      }

      Exception_Handler() = default;
      Exception_Handler(Exception_Handler &&) = default;
      Exception_Handler &operator=(Exception_Handler &&) = default;
      virtual ~Exception_Handler() noexcept = default;

      protected:
        Exception_Handler(const Exception_Handler &) = default;
        Exception_Handler &operator=(const Exception_Handler &) = default;

    };

     template<typename ... T>
      struct Exception_Handler_Impl final : Exception_Handler
      {
        template<typename ExceptionType>
        static void throw_type(const Boxed_Value &bv, const Dispatch_Engine &t_engine)
        {
          try { throw t_engine.boxed_cast<ExceptionType>(bv); } catch (const chaiscript::exception::bad_boxed_cast &) {}
        }

        void operator()(const Boxed_Value &bv, const Dispatch_Engine &t_engine) const final
        {
          (void)std::initializer_list<int>{(throw_type<T>(bv, t_engine), 0)...};
        }
      };
  }

  /// \brief Used in the automatic unboxing of exceptions thrown during script evaluation
  ///
  /// Exception specifications allow the user to tell ChaiScript what possible exceptions are expected from the script
  /// being executed. Exception_Handler objects are created with the chaiscript::exception_specification() function.
  ///
  /// Example:
  /// \code
  /// chaiscript::ChaiScript chai;
  ///
  /// try {
  ///   chai.eval("throw(runtime_error(\"error\"))", chaiscript::exception_specification<int, double, float, const std::string &, const std::exception &>());
  /// } catch (const double e) {
  /// } catch (int) {
  /// } catch (float) {
  /// } catch (const std::string &) {
  /// } catch (const std::exception &e) {
  ///   // This is the one what will be called in the specific throw() above
  /// }
  /// \endcode
  ///
  /// It is recommended that if catching the generic \c std::exception& type that you specifically catch
  /// the chaiscript::exception::eval_error type, so that there is no confusion.
  ///
  /// \code
  /// try {
  ///   chai.eval("throw(runtime_error(\"error\"))", chaiscript::exception_specification<const std::exception &>());
  /// } catch (const chaiscript::exception::eval_error &) {
  ///   // Error in script parsing / execution
  /// } catch (const std::exception &e) {
  ///   // Error explicitly thrown from script
  /// }
  /// \endcode
  ///
  /// Similarly, if you are using the ChaiScript::eval form that unboxes the return value, then chaiscript::exception::bad_boxed_cast
  /// should be handled as well.
  /// 
  /// \code
  /// try {
  ///   chai.eval<int>("1.0", chaiscript::exception_specification<const std::exception &>());
  /// } catch (const chaiscript::exception::eval_error &) {
  ///   // Error in script parsing / execution
  /// } catch (const chaiscript::exception::bad_boxed_cast &) {
  ///   // Error unboxing return value
  /// } catch (const std::exception &e) {
  ///   // Error explicitly thrown from script
  /// }
  /// \endcode
  ///
  /// \sa chaiscript::exception_specification for creation of chaiscript::Exception_Handler objects
  /// \sa \ref exceptions

  /// \brief creates a chaiscript::Exception_Handler which handles one type of exception unboxing
  /// \sa \ref exceptions
  template<typename ... T>
  auto exception_specification() noexcept
  {
    return detail::Exception_Handler_Impl<T...>();
  }
}


#endif

