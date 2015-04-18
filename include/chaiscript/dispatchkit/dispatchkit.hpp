// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2015, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_DISPATCHKIT_HPP_
#define CHAISCRIPT_DISPATCHKIT_HPP_

#include <algorithm>
#include <cassert>
#include <deque>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>

#include "../chaiscript_defines.hpp"
#include "../chaiscript_threading.hpp"
#include "boxed_cast.hpp"
#include "boxed_cast_helper.hpp"
#include "boxed_value.hpp"
#include "type_conversions.hpp"
#include "dynamic_object.hpp"
#include "proxy_constructors.hpp"
#include "proxy_functions.hpp"
#include "type_info.hpp"

namespace chaiscript {
class Boxed_Number;
}  // namespace chaiscript

namespace chaiscript {
namespace dispatch {
class Dynamic_Proxy_Function;
class Proxy_Function_Base;
struct Placeholder_Object;
}  // namespace dispatch
}  // namespace chaiscript


/// \namespace chaiscript::dispatch
/// \brief Classes and functions specific to the runtime dispatch side of ChaiScript. Some items may be of use to the end user.

namespace chaiscript
{
  namespace exception
  {
    /// Exception thrown in the case that an object name is invalid because it is a reserved word
    class reserved_word_error : public std::runtime_error
    {
      public:
        reserved_word_error(const std::string &t_word) CHAISCRIPT_NOEXCEPT
          : std::runtime_error("Reserved word not allowed in object name: " + t_word), m_word(t_word)
        {
        }

        reserved_word_error(const reserved_word_error &) = default;

        virtual ~reserved_word_error() CHAISCRIPT_NOEXCEPT {}

        std::string word() const
        {
          return m_word;
        }

      private:
        std::string m_word;
    };

    /// Exception thrown in the case that an object name is invalid because it contains illegal characters
    class illegal_name_error : public std::runtime_error
    {
      public:
        illegal_name_error(const std::string &t_name) CHAISCRIPT_NOEXCEPT
          : std::runtime_error("Reserved name not allowed in object name: " + t_name), m_name(t_name)
        {
        }

        illegal_name_error(const illegal_name_error &) = default;

        virtual ~illegal_name_error() CHAISCRIPT_NOEXCEPT {}

        std::string name() const
        {
          return m_name;
        }

      private:
        std::string m_name;
    };


    /// Exception thrown in the case that an object name is invalid because it already exists in current context
    class name_conflict_error : public std::runtime_error
    {
      public:
        name_conflict_error(const std::string &t_name) CHAISCRIPT_NOEXCEPT
          : std::runtime_error("Name already exists in current context " + t_name), m_name(t_name)
        {
        }

        name_conflict_error(const name_conflict_error &) = default;

        virtual ~name_conflict_error() CHAISCRIPT_NOEXCEPT {}

        std::string name() const
        {
          return m_name;
        }

      private:
        std::string m_name;

    };


    /// Exception thrown in the case that a non-const object was added as a shared object
    class global_non_const : public std::runtime_error
    {
      public:
        global_non_const() CHAISCRIPT_NOEXCEPT
          : std::runtime_error("a global object must be const")
        {
        }

        global_non_const(const global_non_const &) = default;
        virtual ~global_non_const() CHAISCRIPT_NOEXCEPT {}
    };
  }


  /// \brief Holds a collection of ChaiScript settings which can be applied to the ChaiScript runtime.
  ///        Used to implement loadable module support.
  class Module
  {
    public:
      Module &add(Type_Info ti, std::string name)
      {
        m_typeinfos.emplace_back(std::move(ti), std::move(name));
        return *this;
      }

      Module &add(Type_Conversion d)
      {
        m_conversions.push_back(std::move(d));
        return *this;
      }

      Module &add(Proxy_Function f, std::string name)
      {
        m_funcs.emplace_back(std::move(f), std::move(name));
        return *this;
      }

      Module &add_global_const(Boxed_Value t_bv, std::string t_name)
      {
        if (!t_bv.is_const())
        {
          throw chaiscript::exception::global_non_const();
        }

        m_globals.emplace_back(std::move(t_bv), std::move(t_name));
        return *this;
      }


      //Add a bit of ChaiScript to eval during module implementation
      Module &eval(const std::string &str)
      {
        m_evals.push_back(str);
        return *this;
      }

      Module &add(const std::shared_ptr<Module> &m)
      {
        m->apply(*this, *this);
        return *m;
      }

      template<typename Eval, typename Engine>
        void apply(Eval &t_eval, Engine &t_engine) const
        {
          apply(m_typeinfos.begin(), m_typeinfos.end(), t_engine);
          apply(m_funcs.begin(), m_funcs.end(), t_engine);
          apply_eval(m_evals.begin(), m_evals.end(), t_eval);
          apply_single(m_conversions.begin(), m_conversions.end(), t_engine);
          apply_globals(m_globals.begin(), m_globals.end(), t_engine);
        }

      ~Module()
      {
      }

      bool has_function(const Proxy_Function &new_f, const std::string &name)
      {
        return std::any_of(m_funcs.begin(), m_funcs.end(), [&](const std::pair<Proxy_Function, std::string> &existing_f) {
          return existing_f.second == name && *(existing_f.first) == *(new_f);
          });
      }
      

    private:
      std::vector<std::pair<Type_Info, std::string> > m_typeinfos;
      std::vector<std::pair<Proxy_Function, std::string> > m_funcs;
      std::vector<std::pair<Boxed_Value, std::string> > m_globals;
      std::vector<std::string> m_evals;
      std::vector<Type_Conversion> m_conversions;

      template<typename T, typename InItr>
        static void apply(InItr begin, const InItr end, T &t) 
        {
          for_each(begin, end, [&t](typename std::iterator_traits<InItr>::reference obj)
              {
                try {
                  t.add(obj.first, obj.second);
                } catch (const chaiscript::exception::name_conflict_error &) {
                  /// \todo Should we throw an error if there's a name conflict 
                  ///       while applying a module?
                }
              }
            );
        }

      template<typename T, typename InItr>
        static void apply_globals(InItr begin, InItr end, T &t)
        {
          while (begin != end)
          {
            t.add_global_const(begin->first, begin->second);
            ++begin;
          }
        }

      template<typename T, typename InItr>
        static void apply_single(InItr begin, InItr end, T &t)
        {
          while (begin != end)
          {
            t.add(*begin);
            ++begin;
          }
        }

      template<typename T, typename InItr>
        static void apply_eval(InItr begin, InItr end, T &t)
        {
          while (begin != end)
          {
            t.eval(*begin);
            ++begin;
          }
        }
  };

  /// Convenience typedef for Module objects to be added to the ChaiScript runtime
  typedef std::shared_ptr<Module> ModulePtr;

  namespace detail
  {
    /// A Proxy_Function implementation that is able to take
    /// a vector of Proxy_Functions and perform a dispatch on them. It is 
    /// used specifically in the case of dealing with Function object variables
    class Dispatch_Function : public dispatch::Proxy_Function_Base
    {
      public:
        Dispatch_Function(std::vector<Proxy_Function> t_funcs)
          : Proxy_Function_Base(build_type_infos(t_funcs), calculate_arity(t_funcs)),
            m_funcs(std::move(t_funcs))
        {
        }

        virtual bool operator==(const dispatch::Proxy_Function_Base &rhs) const CHAISCRIPT_OVERRIDE
        {
          try {
            const auto &dispatchfun = dynamic_cast<const Dispatch_Function &>(rhs);
            return m_funcs == dispatchfun.m_funcs;
          } catch (const std::bad_cast &) {
            return false;
          }
        }

        virtual ~Dispatch_Function() {}

        virtual std::vector<Const_Proxy_Function> get_contained_functions() const CHAISCRIPT_OVERRIDE
        {
          return std::vector<Const_Proxy_Function>(m_funcs.begin(), m_funcs.end());
        }


        static int calculate_arity(const std::vector<Proxy_Function> &t_funcs)
        {
          if (t_funcs.empty()) {
            return -1;
          }

          const auto arity = t_funcs.front()->get_arity();

          for (const auto &func : t_funcs)
          {
            if (arity != func->get_arity())
            {
              // The arities in the list do not match, so it's unspecified
              return -1;
            }
          }

          return arity;
        }

        virtual bool call_match(const std::vector<Boxed_Value> &vals, const Type_Conversions &t_conversions) const CHAISCRIPT_OVERRIDE
        {
          return std::any_of(m_funcs.cbegin(), m_funcs.cend(),
                             [&vals, &t_conversions](const Proxy_Function &f){ return f->call_match(vals, t_conversions); });
        }

        virtual std::string annotation() const CHAISCRIPT_OVERRIDE
        {
          return "Multiple method dispatch function wrapper.";
        }

      protected:
        virtual Boxed_Value do_call(const std::vector<Boxed_Value> &params, const Type_Conversions &t_conversions) const CHAISCRIPT_OVERRIDE
        {
          return dispatch::dispatch(m_funcs, params, t_conversions);
        }

      private:
        std::vector<Proxy_Function> m_funcs;

        static std::vector<Type_Info> build_type_infos(const std::vector<Proxy_Function> &t_funcs)
        {
          auto begin = t_funcs.cbegin();
          const auto &end = t_funcs.cend();

          if (begin != end)
          {
            std::vector<Type_Info> type_infos = (*begin)->get_param_types();

            ++begin;

            bool size_mismatch = false;

            while (begin != end)
            {
              std::vector<Type_Info> param_types = (*begin)->get_param_types();

              if (param_types.size() != type_infos.size())
              {
                size_mismatch = true;
              }

              for (size_t i = 0; i < type_infos.size() && i < param_types.size(); ++i)
              {
                if (!(type_infos[i] == param_types[i]))
                {
                  type_infos[i] = detail::Get_Type_Info<Boxed_Value>::get();
                }
              }

              ++begin;
            }

            assert(type_infos.size() > 0 && " type_info vector size is < 0, this is only possible if something else is broken");

            if (size_mismatch)
            {
              type_infos.resize(1);
            }

            return type_infos;
          }

          return std::vector<Type_Info>();
        }
    };
  }


  namespace detail
  {
    /// Main class for the dispatchkit. Handles management
    /// of the object stack, functions and registered types.
    class Dispatch_Engine
    {
      public:
        typedef std::map<std::string, chaiscript::Type_Info> Type_Name_Map;
        typedef std::map<std::string, Boxed_Value> Scope;
        typedef std::vector<Scope> StackData;

        struct State
        {
          std::map<std::string, std::vector<Proxy_Function> > m_functions;
          std::map<std::string, Proxy_Function> m_function_objects;
          std::map<std::string, Boxed_Value> m_global_objects;
          Type_Name_Map m_types;
          std::set<std::string> m_reserved_words;

          State &operator=(const State &) = default;
          State() = default;
          State(const State &) = default;
        };

        Dispatch_Engine()
          : m_stack_holder(this),
            m_place_holder(std::make_shared<dispatch::Placeholder_Object>())
        {
        }

        ~Dispatch_Engine()
        {
        }

        /// \brief casts an object while applying any Dynamic_Conversion available
        template<typename Type>
          typename detail::Cast_Helper<Type>::Result_Type boxed_cast(const Boxed_Value &bv) const
          {
            return chaiscript::boxed_cast<Type>(bv, &m_conversions);
          }

        /// Add a new conversion for upcasting to a base class
        void add(const Type_Conversion &d)
        {
          m_conversions.add_conversion(d);
        }

        /// Add a new named Proxy_Function to the system
        void add(const Proxy_Function &f, const std::string &name)
        {
          validate_object_name(name);
          add_function(f, name);
        }

        /// Set the value of an object, by name. If the object
        /// is not available in the current scope it is created
        void add(const Boxed_Value &obj, const std::string &name)
        {
          validate_object_name(name);
          auto &stack = get_stack_data();

          for (auto stack_elem = stack.rbegin(); stack_elem != stack.rend(); ++stack_elem)
          {
            auto itr = stack_elem->find(name);
            if (itr != stack_elem->end())
            {
              itr->second = std::move(obj);
              return;
            }
          }

          add_object(name, std::move(obj));
        }


        /// Adds a named object to the current scope
        /// \warning This version does not check the validity of the name
        /// it is meant for internal use only
        void add_object(const std::string &name, const Boxed_Value &obj)
        {
          if (!get_stack_data().back().insert(std::make_pair(name, obj)).second)
          {
            throw chaiscript::exception::name_conflict_error(name);
          }
        }

        /// Adds a new global shared object, between all the threads
        void add_global_const(const Boxed_Value &obj, const std::string &name)
        {
          validate_object_name(name);
          if (!obj.is_const())
          {
            throw chaiscript::exception::global_non_const();
          }

          chaiscript::detail::threading::unique_lock<chaiscript::detail::threading::shared_mutex> l(m_global_object_mutex);

          if (m_state.m_global_objects.find(name) != m_state.m_global_objects.end())
          {
            throw chaiscript::exception::name_conflict_error(name);
          } else {
            m_state.m_global_objects.insert(std::make_pair(name, obj));
          }
        }


        /// Adds a new global (non-const) shared object, between all the threads
        void add_global(const Boxed_Value &obj, const std::string &name)
        {
          validate_object_name(name);

          chaiscript::detail::threading::unique_lock<chaiscript::detail::threading::shared_mutex> l(m_global_object_mutex);

          if (m_state.m_global_objects.find(name) != m_state.m_global_objects.end())
          {
            throw chaiscript::exception::name_conflict_error(name);
          } else {
            m_state.m_global_objects.insert(std::make_pair(name, obj));
          }
        }


        /// Adds a new scope to the stack
        void new_scope()
        {
          get_stack_data().emplace_back();
          m_stack_holder->call_params.emplace_back();
        }

        /// Pops the current scope from the stack
        void pop_scope()
        {
          m_stack_holder->call_params.pop_back();
          StackData &stack = get_stack_data();
          if (stack.size() > 1)
          {
            stack.pop_back();
          } else {
            throw std::range_error("Unable to pop global stack");
          }
        }


        /// Pushes a new stack on to the list of stacks
        void new_stack()
        {
          // add a new Stack with 1 element
          m_stack_holder->stacks.emplace_back(1);
        }

        void pop_stack()
        {
          m_stack_holder->stacks.pop_back();
        }

        /// Searches the current stack for an object of the given name
        /// includes a special overload for the _ place holder object to
        /// ensure that it is always in scope.
        Boxed_Value get_object(const std::string &name) const
        {
          // Is it a placeholder object?
          if (name == "_")
          {
            return m_place_holder;
          }

          auto &stack = get_stack_data();

          // Is it in the stack?
          for (auto stack_elem = stack.rbegin(); stack_elem != stack.rend(); ++stack_elem)
          {
            const auto stackitr = stack_elem->find(name);
            if (stackitr != stack_elem->end())
            {
              return stackitr->second;
            }
          }

          // Is the value we are looking for a global?
          {
            chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l(m_global_object_mutex);

            const auto itr = m_state.m_global_objects.find(name);
            if (itr != m_state.m_global_objects.end())
            {
              return itr->second;
            }
          }

          // If all that failed, then check to see if it's a function
          return get_function_object(name);
        }

        /// Registers a new named type
        void add(const Type_Info &ti, const std::string &name)
        {
          add_global_const(const_var(ti), name + "_type");

          chaiscript::detail::threading::unique_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);

          m_state.m_types.insert(std::make_pair(name, ti));
        }

        /// Returns the type info for a named type
        Type_Info get_type(const std::string &name, bool t_throw = true) const
        {
          chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);

          const auto itr = m_state.m_types.find(name);

          if (itr != m_state.m_types.end())
          {
            return itr->second;
          }

          if (t_throw) {
            throw std::range_error("Type Not Known");
          } else {
            return Type_Info();
          }
        }

        /// Returns the registered name of a known type_info object
        /// compares the "bare_type_info" for the broadest possible
        /// match
        std::string get_type_name(const Type_Info &ti) const
        {
          chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);

          for (const auto & elem : m_state.m_types)
          {
            if (elem.second.bare_equal(ti))
            {
              return elem.first;
            }
          }

          return ti.bare_name();
        }

        /// Return all registered types
        std::vector<std::pair<std::string, Type_Info> > get_types() const
        {
          chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);

          return std::vector<std::pair<std::string, Type_Info> >(m_state.m_types.begin(), m_state.m_types.end());
        }

        /// Return a function by name
        std::vector< Proxy_Function > get_function(const std::string &t_name) const
        {
          chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);

          const auto &funs = get_functions_int();

          auto itr = funs.find(t_name);

          if (itr != funs.end())
          {
            return itr->second;
          } else {
            return std::vector<Proxy_Function>();
          }
        }

        /// \returns a function object (Boxed_Value wrapper) if it exists
        /// \throws std::range_error if it does not
        Boxed_Value get_function_object(const std::string &t_name) const
        {
//          std::cout << "Getting function object: " << t_name << '\n';
          chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);

          const auto &funs = get_function_objects_int();

          auto itr = funs.find(t_name);

          if (itr != funs.end())
          {
            return const_var(itr->second);
          } else {
            throw std::range_error("Object not found: " + t_name);
          }
        }

        /// Return true if a function exists
        bool function_exists(const std::string &name) const
        {
          chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);

          const auto &functions = get_functions_int();
          return functions.find(name) != functions.end();
        }

        /// \returns All values in the local thread state in the parent scope, or if it doesn't exist,
        ///          the current scope.
        std::map<std::string, Boxed_Value> get_parent_locals() const
        {
          auto &stack = get_stack_data();
          if (stack.size() > 1)
          {
            return stack[1];
          } else {
            return stack[0];
          }
        }

        /// \returns All values in the local thread state, added through the add() function
        std::map<std::string, Boxed_Value> get_locals() const
        {
          auto &stack = get_stack_data();
          auto &scope = stack.front();
          return scope;
        }

        /// \brief Sets all of the locals for the current thread state.
        ///
        /// \param[in] t_locals The map<name, value> set of variables to replace the current state with
        ///
        /// Any existing locals are removed and the given set of variables is added
        void set_locals(const std::map<std::string, Boxed_Value> &t_locals)
        {
          auto &stack = get_stack_data();
          auto &scope = stack.front();
          scope = t_locals;
        }



        ///
        /// Get a map of all objects that can be seen from the current scope in a scripting context
        ///
        std::map<std::string, Boxed_Value> get_scripting_objects() const
        {
          const Stack_Holder &s = *m_stack_holder;

          // We don't want the current context, but one up if it exists
          const StackData &stack = (s.stacks.size()==1)?(s.stacks.back()):(s.stacks[s.stacks.size()-2]);

          std::map<std::string, Boxed_Value> retval;

          // note: map insert doesn't overwrite existing values, which is why this works
          for (auto itr = stack.rbegin(); itr != stack.rend(); ++itr)
          {
            retval.insert(itr->begin(), itr->end());
          } 

          // add the global values
          {
            chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l(m_global_object_mutex);

            retval.insert(m_state.m_global_objects.begin(), m_state.m_global_objects.end());
          }

          return retval;
        }


        ///
        /// Get a map of all functions that can be seen from a scripting context
        ///
        std::map<std::string, Boxed_Value> get_function_objects() const
        {
          chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);

          const auto &funs = get_function_objects_int();

          std::map<std::string, Boxed_Value> objs;

          for (const auto & fun : funs)
          {
            objs.insert(std::make_pair(fun.first, const_var(fun.second)));
          }

          return objs;
        }


        /// Get a vector of all registered functions
        std::vector<std::pair<std::string, Proxy_Function > > get_functions() const
        {
          chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);

          std::vector<std::pair<std::string, Proxy_Function> > rets;

          const auto &functions = get_functions_int();

          for (const auto & function : functions)
          {
            for (const auto & internal_func : function.second)
            {
              rets.emplace_back(function.first, internal_func);
            }
          }

          return rets;
        }

        void add_reserved_word(const std::string &name)
        {
          chaiscript::detail::threading::unique_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);

          m_state.m_reserved_words.insert(name);
        }

        const Type_Conversions &conversions() const
        {
          return m_conversions;
        }

        bool is_attribute_call(const std::vector<Proxy_Function> &t_funs, const std::vector<Boxed_Value> &t_params,
            bool t_has_params) const
        {
          if (!t_has_params || t_params.empty()) {
            return false;
          }

          for (const auto &fun : t_funs) {
            if (fun->is_attribute_function()) {
              if (fun->compare_first_type(t_params[0], m_conversions)) {
                return true;
              }
            }
          }

          return false;
        }

        Boxed_Value call_member(const std::string &t_name, const std::vector<Boxed_Value> &params, bool t_has_params) const
        {
          auto funs = get_function(t_name);

          if (is_attribute_call(funs, params, t_has_params)) {
            std::vector<Boxed_Value> attr_params{params[0]};
            std::vector<Boxed_Value> remaining_params{params.begin() + 1, params.end()};
            Boxed_Value bv = dispatch::dispatch(funs, attr_params, m_conversions);
            if (!remaining_params.empty() || bv.get_type_info().bare_equal(user_type<dispatch::Proxy_Function_Base>())) {
              return (*boxed_cast<const dispatch::Proxy_Function_Base *>(bv))(remaining_params, m_conversions);
            } else {
              return bv;
            }
          } else {
            return dispatch::dispatch(funs, params, m_conversions);
          }
        }

        Boxed_Value call_function(const std::string &t_name, const std::vector<Boxed_Value> &params) const
        {
          Boxed_Value bv = dispatch::dispatch(get_function(t_name), params, m_conversions);
          // the result of a clone is never to be marked as a return_value
          if (t_name == "clone") {
            bv.reset_return_value();
          }
          return bv;
        }

        Boxed_Value call_function(const std::string &t_name) const
        {
          return call_function(t_name, std::vector<Boxed_Value>());
        }

        Boxed_Value call_function(const std::string &t_name, Boxed_Value p1) const
        {
          return call_function(t_name, std::vector<Boxed_Value>({std::move(p1)}));
        }

        Boxed_Value call_function(const std::string &t_name, Boxed_Value p1, Boxed_Value p2) const
        {
          return call_function(t_name, std::vector<Boxed_Value>({std::move(p1), std::move(p2)}));
        }

        /// Dump object info to stdout
        void dump_object(const Boxed_Value &o) const
        {
          std::cout << (o.is_const()?"const ":"") << type_name(o) << '\n';
        }

        /// Dump type info to stdout
        void dump_type(const Type_Info &type) const
        {
          std::cout << (type.is_const()?"const ":"") << get_type_name(type);
        }

        /// Dump function to stdout
        void dump_function(const std::pair<const std::string, Proxy_Function > &f) const
        {
          std::vector<Type_Info> params = f.second->get_param_types();
          std::string annotation = f.second->annotation();

          if (annotation.size() > 0) {
            std::cout << annotation;
          }
          dump_type(params.front());
          std::cout << " " << f.first << "(";

          for (std::vector<Type_Info>::const_iterator itr = params.begin() + 1;
              itr != params.end();
              )
          {
            dump_type(*itr);
            ++itr;

            if (itr != params.end())
            {
              std::cout << ", ";
            }
          }

          std::cout << ") \n";
        }

        /// Returns true if a call can be made that consists of the first parameter
        /// (the function) with the remaining parameters as its arguments.
        Boxed_Value call_exists(const std::vector<Boxed_Value> &params)
        {
          if (params.empty())
          {
            throw chaiscript::exception::arity_error(static_cast<int>(params.size()), 1);
          }

          const Const_Proxy_Function &f = this->boxed_cast<Const_Proxy_Function>(params[0]);

          return Boxed_Value(f->call_match(std::vector<Boxed_Value>(params.begin() + 1, params.end()), m_conversions));
        }

        /// Dump all system info to stdout
        void dump_system() const
        {
          std::cout << "Registered Types: \n";
          std::vector<std::pair<std::string, Type_Info> > types = get_types();
          for (std::vector<std::pair<std::string, Type_Info> >::const_iterator itr = types.begin();
              itr != types.end();
              ++itr)
          {
            std::cout << itr->first << ": ";
            std::cout << itr->second.bare_name();
            std::cout << '\n';
          }

          std::cout << '\n';  
          std::vector<std::pair<std::string, Proxy_Function > > funcs = get_functions();

          std::cout << "Functions: \n";
          for (std::vector<std::pair<std::string, Proxy_Function > >::const_iterator itr = funcs.begin();
              itr != funcs.end();
              ++itr)
          {
            dump_function(*itr);
          }
          std::cout << '\n';
        }

        /// return true if the Boxed_Value matches the registered type by name
        bool is_type(const Boxed_Value &r, const std::string &user_typename) const
        {
          try {
            if (get_type(user_typename).bare_equal(r.get_type_info()))
            {
              return true;
            }
          } catch (const std::range_error &) {
          }

          try {
            const dispatch::Dynamic_Object &d = boxed_cast<const dispatch::Dynamic_Object &>(r);
            return d.get_type_name() == user_typename;
          } catch (const std::bad_cast &) {
          }

          return false;
        }

        std::string type_name(const Boxed_Value &obj) const
        {
          return get_type_name(obj.get_type_info());
        }

        State get_state() const
        {
          chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);
          chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l2(m_global_object_mutex);

          return m_state;
        }

        void set_state(const State &t_state)
        {
          chaiscript::detail::threading::unique_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);
          chaiscript::detail::threading::unique_lock<chaiscript::detail::threading::shared_mutex> l2(m_global_object_mutex);

          m_state = t_state;
        }

        void save_function_params(std::initializer_list<Boxed_Value> t_params)
        {
          Stack_Holder &s = *m_stack_holder;
          s.call_params.back().insert(s.call_params.back().begin(), std::move(t_params));
        }

        void save_function_params(std::vector<Boxed_Value> &&t_params)
        {
          Stack_Holder &s = *m_stack_holder;

          for (auto &&param : t_params)
          {
            s.call_params.back().insert(s.call_params.back().begin(), std::move(param));
          }
        }

        void save_function_params(const std::vector<Boxed_Value> &t_params)
        {
          Stack_Holder &s = *m_stack_holder;
          s.call_params.back().insert(s.call_params.back().begin(), t_params.begin(), t_params.end());
        }

        void new_function_call()
        {
          Stack_Holder &s = *m_stack_holder;
          if (s.call_depth == 0)
          {
            m_conversions.enable_conversion_saves(true);
          }

          ++s.call_depth;

          save_function_params(m_conversions.take_saves());
        }

        void pop_function_call()
        {
          Stack_Holder &s = *m_stack_holder;
          --s.call_depth;

          assert(s.call_depth >= 0);

          if (s.call_depth == 0)
          {
            s.call_params.back().clear();
            m_conversions.enable_conversion_saves(false);
          }
        }

      private:
        /// Returns the current stack
        /// make const/non const versions
        const StackData &get_stack_data() const
        {
          return m_stack_holder->stacks.back();
        }

        StackData &get_stack_data()
        {
          return m_stack_holder->stacks.back();
        }

        const std::map<std::string, Proxy_Function> &get_function_objects_int() const
        {
          return m_state.m_function_objects;
        }

        std::map<std::string, Proxy_Function> &get_function_objects_int() 
        {
          return m_state.m_function_objects;
        }

        const std::map<std::string, std::vector<Proxy_Function> > &get_functions_int() const
        {
          return m_state.m_functions;
        }

        std::map<std::string, std::vector<Proxy_Function> > &get_functions_int() 
        {
          return m_state.m_functions;
        }

        static bool function_less_than(const Proxy_Function &lhs, const Proxy_Function &rhs)
        {

          auto dynamic_lhs(std::dynamic_pointer_cast<const dispatch::Dynamic_Proxy_Function>(lhs));
          auto dynamic_rhs(std::dynamic_pointer_cast<const dispatch::Dynamic_Proxy_Function>(rhs));

          if (dynamic_lhs && dynamic_rhs)
          {
            if (dynamic_lhs->get_guard())
            {
              if (dynamic_rhs->get_guard())
              {
                return false;
              } else {
                return true;
              }
            } else {
              return false;
            }
          }

          if (dynamic_lhs && !dynamic_rhs)
          {
            return false;
          }

          if (!dynamic_lhs && dynamic_rhs)
          {
            return true;
          }

          const auto &lhsparamtypes = lhs->get_param_types();
          const auto &rhsparamtypes = rhs->get_param_types();

          const auto lhssize = lhsparamtypes.size();
          const auto rhssize = rhsparamtypes.size();

          CHAISCRIPT_CONSTEXPR auto boxed_type = user_type<Boxed_Value>();
          CHAISCRIPT_CONSTEXPR auto boxed_pod_type = user_type<Boxed_Number>();

          for (size_t i = 1; i < lhssize && i < rhssize; ++i)
          {
            const Type_Info &lt = lhsparamtypes[i];
            const Type_Info &rt = rhsparamtypes[i];

            if (lt.bare_equal(rt) && lt.is_const() == rt.is_const())
            {
              continue; // The first two types are essentially the same, next iteration
            }

            // const is after non-const for the same type
            if (lt.bare_equal(rt) && lt.is_const() && !rt.is_const())
            {
              return false;
            }

            if (lt.bare_equal(rt) && !lt.is_const())
            {
              return true;
            }

            // boxed_values are sorted last
            if (lt.bare_equal(boxed_type))
            {
              return false;
            }

            if (rt.bare_equal(boxed_type))
            {
              if (lt.bare_equal(boxed_pod_type))
              {
                return true;
              }
              return true;
            }

            if (lt.bare_equal(boxed_pod_type))
            {
              return false;
            }

            if (rt.bare_equal(boxed_pod_type))
            {
              return true;
            }

            // otherwise, we want to sort by typeid
            return lt < rt;
          }

          return false;
        }


        /// Throw a reserved_word exception if the name is not allowed
        void validate_object_name(const std::string &name) const
        {
          if (name.find("::") != std::string::npos) {
            throw chaiscript::exception::illegal_name_error(name);
          }

          chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);

          if (m_state.m_reserved_words.find(name) != m_state.m_reserved_words.end())
          {
            throw chaiscript::exception::reserved_word_error(name);
          }
        }

        /// Implementation detail for adding a function. 
        /// \throws exception::name_conflict_error if there's a function matching the given one being added
        void add_function(const Proxy_Function &t_f, const std::string &t_name)
        {
          chaiscript::detail::threading::unique_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);

          auto &funcs = get_functions_int();

          auto itr = funcs.find(t_name);

          auto &func_objs = get_function_objects_int();

          if (itr != funcs.end())
          {
            auto &vec = itr->second;
            for (const auto &func : vec)
            {
              if ((*t_f) == *(func))
              {
                throw chaiscript::exception::name_conflict_error(t_name);
              }
            }

            vec.push_back(t_f);
            std::stable_sort(vec.begin(), vec.end(), &function_less_than);
            func_objs[t_name] = std::make_shared<Dispatch_Function>(vec);
          } else if (t_f->has_arithmetic_param()) {
            // if the function is the only function but it also contains
            // arithmetic operators, we must wrap it in a dispatch function
            // to allow for automatic arithmetic type conversions
            std::vector<Proxy_Function> vec({t_f});
            funcs.insert(std::make_pair(t_name, vec));
            func_objs[t_name] = std::make_shared<Dispatch_Function>(std::move(vec));
          } else {
            funcs.insert(std::make_pair(t_name, std::vector<Proxy_Function>{t_f}));
            func_objs[t_name] = t_f;
          }


        }

        mutable chaiscript::detail::threading::shared_mutex m_mutex;
        mutable chaiscript::detail::threading::shared_mutex m_global_object_mutex;

        struct Stack_Holder
        {
          Stack_Holder()
            : call_depth(0)
          {
            stacks.emplace_back(1);
            call_params.emplace_back();
          }

          std::deque<StackData> stacks;

          std::deque<std::list<Boxed_Value>> call_params;
          int call_depth;
        };

        Type_Conversions m_conversions;
        chaiscript::detail::threading::Thread_Storage<Stack_Holder> m_stack_holder;


        State m_state;

        Boxed_Value m_place_holder;
    };
  }
}

#endif

