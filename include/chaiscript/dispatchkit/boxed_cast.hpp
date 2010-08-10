// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2010, Jonathan Turner (jonathan@emptycrate.com)
// and Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef __boxed_cast_hpp__
#define __boxed_cast_hpp__

#include "type_info.hpp"
#include "boxed_value.hpp"
#include "boxed_cast_helper.hpp"
#include "dynamic_cast_conversion.hpp"

#include "../chaiscript_threading.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>
#include <boost/function.hpp>
#include <boost/ref.hpp>
#include <boost/cstdint.hpp>
#include <boost/type_traits/add_const.hpp>
#include <boost/type_traits/is_polymorphic.hpp>
#include <boost/integer_traits.hpp>

namespace chaiscript 
{
  
  /**
   * boxed_cast function for casting a Boxed_Value into a given type
   * example:
   * int &i = boxed_cast<int &>(boxedvalue);
   */
  template<typename Type>
  typename detail::Cast_Helper<Type>::Result_Type boxed_cast(const Boxed_Value &bv)
  {
    try {
      return detail::Cast_Helper<Type>::cast(bv);
    } catch (const boost::bad_any_cast &) {

#ifdef BOOST_MSVC
      //Thank you MSVC, yes we know that a constant value is being used in the if
      // statment in THIS VERSION of the template instantiation
#pragma warning(push)
#pragma warning(disable : 4127)
#endif

      if (boost::is_polymorphic<typename Stripped_Type<Type>::type>::value)
      {
        try {
          // We will not catch any bad_boxed_dynamic_cast that is thrown, let the user get it
          // either way, we are not responsible if it doesn't work
          return detail::Cast_Helper<Type>::cast(boxed_dynamic_cast<Type>(bv));
        } catch (const boost::bad_any_cast &) {
          throw bad_boxed_cast(bv.get_type_info(), typeid(Type));
        }
      } else {
        // If it's not polymorphic, just throw the error, don't waste the time on the 
        // attempted dynamic_cast
        throw bad_boxed_cast(bv.get_type_info(), typeid(Type));
      }

#ifdef BOOST_MSVC
#pragma warning(pop)
#endif


    } 
  }

}



#endif

