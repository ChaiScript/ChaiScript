// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2014, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_UTILITY_UTILITY_HPP_
#define CHAISCRIPT_UTILITY_UTILITY_HPP_

#include "../chaiscript.hpp"
#include <boost/preprocessor.hpp>
#include <string>

#define CHAISCRIPT_MODULE(_info) BOOST_PP_SEQ_ELEM(0, _info)

#define CHAISCRIPT_CLASS_ELEM(_info) BOOST_PP_SEQ_ELEM(1, _info)

#define CHAISCRIPT_METHOD(_info, _method) & CHAISCRIPT_CLASS_ELEM(_info) :: BOOST_PP_SEQ_ELEM(0, _method)

#define CHAISCRIPT_METHOD_NAME(_info, _method) \
  BOOST_PP_SEQ_ELEM(3, _info) (BOOST_PP_STRINGIZE(BOOST_PP_SEQ_ELEM(0, _method ) ) )

#define CHAISCRIPT_CLASS_NAME(_info) \
  BOOST_PP_SEQ_ELEM(2, _info) (BOOST_PP_STRINGIZE(CHAISCRIPT_CLASS_ELEM(_info) ) )

#define CHAISCRIPT_METHOD_SIGNATURE_PART(_r, _info, _i, _method_part) \
  BOOST_PP_EXPR_IF(BOOST_PP_EQUAL(_i, 1), < _method_part > )

#define CHAISCRIPT_METHOD_SIGNATURE(_info, _method) \
  BOOST_PP_SEQ_FOR_EACH_I(CHAISCRIPT_METHOD_SIGNATURE_PART, _info, _method)

#define CHAISCRIPT_CLASS_CONSTRUCTOR(_r, _info, _constructor) \
  CHAISCRIPT_MODULE(_info) ->add BOOST_PP_LPAREN() chaiscript::constructor<_constructor>() BOOST_PP_COMMA() CHAISCRIPT_CLASS_NAME(_info) BOOST_PP_RPAREN() ;

#define CHAISCRIPT_CLASS_METHOD(_r, _info, _method) \
  CHAISCRIPT_MODULE(_info) ->add BOOST_PP_LPAREN() chaiscript::fun CHAISCRIPT_METHOD_SIGNATURE(_info, _method) \
      BOOST_PP_LPAREN() CHAISCRIPT_METHOD(_info, _method) BOOST_PP_RPAREN() BOOST_PP_COMMA() CHAISCRIPT_METHOD_NAME(_info, _method)BOOST_PP_RPAREN() ;
  
#define CHAISCRIPT_CLASS_CONSTRUCTORS(_info, _constructors) \
  BOOST_PP_SEQ_FOR_EACH(CHAISCRIPT_CLASS_CONSTRUCTOR, _info, _constructors)

#define CHAISCRIPT_CLASS_METHODS(_info, _methods) \
  BOOST_PP_SEQ_FOR_EACH(CHAISCRIPT_CLASS_METHOD, _info, _methods)

#define CHAISCRIPT_CLASS_EX(_module, _class_name, _class_name_translator, _method_name_translator, _constructors, _methods) \
  { \
    _module->add(chaiscript::user_type<_class_name>(), _class_name_translator (BOOST_PP_STRINGIZE(_class_name))); \
    CHAISCRIPT_CLASS_CONSTRUCTORS((_module)(_class_name)(_class_name_translator), _constructors) \
    CHAISCRIPT_CLASS_METHODS((_module)(_class_name)(_class_name_translator)(_method_name_translator), _methods) \
  }

#define CHAISCRIPT_CLASS_NO_CONSTRUCTOR_EX(_module, _class_name, _class_name_translator, _method_name_translator, _methods) \
  { \
    _module->add(chaiscript::user_type<_class_name>(), _class_name_translator (BOOST_PP_STRINGIZE(_class_name))); \
    CHAISCRIPT_CLASS_METHODS((_module)(_class_name)(_class_name_translator)(_method_name_translator), _methods) \
  }

#define CHAISCRIPT_CLASS(_module, _class_name, _constructors, _methods) \
  CHAISCRIPT_CLASS_EX(_module, _class_name, chaiscript::utility::class_name_translator, \
      chaiscript::utility::method_name_translator, _constructors, _methods)

#define CHAISCRIPT_CLASS_NO_CONSTRUCTOR(_module, _class_name, _methods) \
  CHAISCRIPT_CLASS_NO_CONSTRUCTOR_EX(_module, _class_name, chaiscript::utility::class_name_translator, \
      chaiscript::utility::method_name_translator, _methods)

namespace chaiscript 
{
  /// \brief Classes and functions which provide general utility to the end user.
  namespace utility
  {
    inline std::string class_name_translator(const std::string &t_name)
    {
      size_t colon = t_name.rfind("::");
      if (colon != std::string::npos)
      {
        return t_name.substr(colon+2, std::string::npos);
      } else {
        return t_name;
      }
    }

    inline std::string method_name_translator(const std::string &t_name)
    {
      size_t namestart = t_name.rfind("operator");
      namestart = (namestart == std::string::npos)?0:namestart+strlen("operator");

      if (namestart == 0)
      {
        namestart = t_name.rfind("::");
        namestart = (namestart == std::string::npos)?0:namestart+strlen("::");
      } 

      return t_name.substr(namestart, std::string::npos);
    }
  }
}

#endif

