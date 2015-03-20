// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2015, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_DYNAMIC_OBJECT_HPP_
#define CHAISCRIPT_DYNAMIC_OBJECT_HPP_

#include <cassert>
#include <map>
#include <memory>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>

#include "../chaiscript_defines.hpp"
#include "boxed_cast.hpp"
#include "boxed_cast_helper.hpp"
#include "boxed_value.hpp"
#include "type_info.hpp"

namespace chaiscript {
class Type_Conversions;
namespace dispatch {
class Proxy_Function_Base;
}  // namespace dispatch
}  // namespace chaiscript

namespace chaiscript
{
  namespace dispatch
  {
    class Dynamic_Object
    {
      public:
        Dynamic_Object(std::string t_type_name)
          : m_type_name(std::move(t_type_name))
        {
        }

        std::string get_type_name() const
        {
          return m_type_name;
        }

        Boxed_Value get_attr(const std::string &t_attr_name)
        {
          return m_attrs[t_attr_name];
        }

        std::map<std::string, Boxed_Value> get_attrs() const
        {
          return m_attrs;
        }

      private:
        std::string m_type_name;

        std::map<std::string, Boxed_Value> m_attrs;
    };

  }
}
#endif

