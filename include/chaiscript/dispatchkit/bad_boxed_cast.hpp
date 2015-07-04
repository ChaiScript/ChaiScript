// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2015, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_BAD_BOXED_CAST_HPP_
#define CHAISCRIPT_BAD_BOXED_CAST_HPP_

#include <string>
#include <typeinfo>

#include "../chaiscript_defines.hpp"
#include "type_info.hpp"

namespace chaiscript {
class Type_Info;
}  // namespace chaiscript

namespace chaiscript 
{
  namespace exception
  {
    /// \brief Thrown in the event that a Boxed_Value cannot be cast to the desired type
    ///
    /// It is used internally during function dispatch and may be used by the end user.
    ///
    /// \sa chaiscript::boxed_cast
    class bad_boxed_cast : public std::bad_cast
    {
      public:
        bad_boxed_cast(Type_Info t_from, const std::type_info &t_to,
            std::string t_what) CHAISCRIPT_NOEXCEPT
          : from(std::move(t_from)), to(&t_to), m_what(std::move(t_what))
        {
        }

        bad_boxed_cast(Type_Info t_from, const std::type_info &t_to)
          : from(std::move(t_from)), to(&t_to), m_what("Cannot perform boxed_cast: " + t_from.name() + " to: " + t_to.name())
        {
        }

        bad_boxed_cast(std::string t_what) CHAISCRIPT_NOEXCEPT
          : to(nullptr), m_what(std::move(t_what))
        {
        }

        bad_boxed_cast(const bad_boxed_cast &) = default;
        virtual ~bad_boxed_cast() CHAISCRIPT_NOEXCEPT {}

        /// \brief Description of what error occurred
        virtual const char * what() const CHAISCRIPT_NOEXCEPT CHAISCRIPT_OVERRIDE
        {
          return m_what.c_str();
        }

        Type_Info from; ///< Type_Info contained in the Boxed_Value
        const std::type_info *to; ///< std::type_info of the desired (but failed) result type

      private:
        std::string m_what;
    };
  }
}



#endif

