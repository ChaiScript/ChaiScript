// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2014, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_BOXED_CAST_HPP_
#define CHAISCRIPT_BOXED_CAST_HPP_

#include "../chaiscript_defines.hpp"

#include "type_info.hpp"
#include "boxed_value.hpp"
#include "boxed_cast_helper.hpp"
#include "dynamic_cast_conversion.hpp"

#include "../chaiscript_threading.hpp"

namespace chaiscript 
{
 
  /// \brief Function for extracting a value stored in a Boxed_Value object
  /// \tparam Type The type to extract from the Boxed_Value
  /// \param[in] bv The Boxed_Value to extract a typed value from
  /// \returns Type equivalent to the requested type 
  /// \throws exception::bad_boxed_cast If the requested conversion is not possible
  /// 
  /// boxed_cast will attempt to make conversions between value, &, *, std::shared_ptr, std::reference_wrapper,
  /// and std::function (const and non-const) where possible. boxed_cast is used internally during function
  /// dispatch. This means that all of these conversions will be attempted automatically for you during
  /// ChaiScript function calls.
  ///
  /// \li non-const values can be extracted as const or non-const
  /// \li const values can be extracted only as const
  /// \li Boxed_Value constructed from pointer or std::reference_wrapper can be extracted as reference,
  ///     pointer or value types
  /// \li Boxed_Value constructed from std::shared_ptr or value types can be extracted as reference,
  ///     pointer, value, or std::shared_ptr types
  ///
  /// Conversions to std::function objects are attempted as well
  ///
  /// Example:
  /// \code
  /// // All of the following should succeed
  /// chaiscript::Boxed_Value bv(1);
  /// std::shared_ptr<int> spi = chaiscript::boxed_cast<std::shared_ptr<int> >(bv);
  /// int i = chaiscript::boxed_cast<int>(bv);
  /// int *ip = chaiscript::boxed_cast<int *>(bv);
  /// int &ir = chaiscript::boxed_cast<int &>(bv);
  /// std::shared_ptr<const int> cspi = chaiscript::boxed_cast<std::shared_ptr<const int> >(bv);
  /// const int ci = chaiscript::boxed_cast<const int>(bv);
  /// const int *cip = chaiscript::boxed_cast<const int *>(bv);
  /// const int &cir = chaiscript::boxed_cast<const int &>(bv);
  /// \endcode
  ///
  /// std::function conversion example
  /// \code
  /// chaiscript::ChaiScript chai;
  /// Boxed_Value bv = chai.eval("`+`"); // Get the functor for the + operator which is built in 
  /// std::function<int (int, int)> f = chaiscript::boxed_cast<std::function<int (int, int)> >(bv);
  /// int i = f(2,3);
  /// assert(i == 5);
  /// \endcode
  template<typename Type>
  typename detail::Cast_Helper<Type>::Result_Type boxed_cast(const Boxed_Value &bv, const Dynamic_Cast_Conversions *t_conversions = nullptr)
  {
    try {
      return detail::Cast_Helper<Type>::cast(bv, t_conversions);
    } catch (const chaiscript::detail::exception::bad_any_cast &) {


#ifdef CHAISCRIPT_MSVC
      //Thank you MSVC, yes we know that a constant value is being used in the if
      // statment in THIS VERSION of the template instantiation
#pragma warning(push)
#pragma warning(disable : 4127)
#endif

      if (std::is_polymorphic<typename detail::Stripped_Type<Type>::type>::value && t_conversions)
      {
        try {
          // We will not catch any bad_boxed_dynamic_cast that is thrown, let the user get it
          // either way, we are not responsible if it doesn't work
          return detail::Cast_Helper<Type>::cast(t_conversions->boxed_dynamic_cast<Type>(bv), t_conversions);
        } catch (const chaiscript::detail::exception::bad_any_cast &) {
          throw exception::bad_boxed_cast(bv.get_type_info(), typeid(Type));
        }
      } else {
        // If it's not polymorphic, just throw the error, don't waste the time on the 
        // attempted dynamic_cast
        throw exception::bad_boxed_cast(bv.get_type_info(), typeid(Type));
      }

#ifdef CHAISCRIPT_MSVC
#pragma warning(pop)
#endif


    } 
  }

}



#endif

