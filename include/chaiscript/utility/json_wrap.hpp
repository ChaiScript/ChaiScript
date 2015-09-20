#ifndef CHAISCRIPT_SIMPLEJSON_WRAP_HPP
#define CHAISCRIPT_SIMPLEJSON_WRAP_HPP

#include "json.hpp"

namespace chaiscript
{
  class json_wrap
  {
    public:

      static ModulePtr library(ModulePtr m = std::make_shared<Module>())
      {

        m->add(chaiscript::fun([](const std::string &t_str) { return json_wrap::from_json(t_str); }), "from_json");
//        m->add(chaiscript::fun(&json_wrap::to_json), "to_json");

        return m;

      }

    private:

      static Boxed_Value from_json(const json::JSON &t_json)
      {
        switch( t_json.JSONType() ) {
          case json::JSON::Class::Null:
            return Boxed_Value();
          case json::JSON::Class::Object:
            {
              std::map<std::string, Boxed_Value> m;

              for (const auto &p : t_json.ObjectRange())
              {
                m.emplace(p.first, from_json(p.second));
              }

              return Boxed_Value(m);
            }
          case json::JSON::Class::Array:
            {
              std::vector<Boxed_Value> vec;

              for (const auto &p : t_json.ArrayRange()) 
              {
                vec.emplace_back(from_json(p));
              }

              return Boxed_Value(vec);
            }
          case json::JSON::Class::String:
            return Boxed_Value(t_json.ToString());
          case json::JSON::Class::Floating:
            return Boxed_Value(t_json.ToFloat());
          case json::JSON::Class::Integral:
            return Boxed_Value(t_json.ToInt());
          case json::JSON::Class::Boolean:
            return Boxed_Value(t_json.ToBool());
        }

        throw std::runtime_error("Unknown JSON type");
      }

      static Boxed_Value from_json(const std::string &t_json)
      {
        return from_json( json::JSON::Load(t_json) );
      }

  };


}

#endif
