#include <boost/preprocessor.hpp>

#define gettypeinfo(z,n,text)  ti.push_back(Get_Type_Info<Param ## n>::get());
#define casthelper(z,n,text) ,dispatchkit::boxed_cast< Param ## n >(params[n])
#define comparetype(z,n,text)  && ((Get_Type_Info<Param ## n>::get() == params[n].get_type_info()))
#define trycast(z,n,text) dispatchkit::boxed_cast<Param ## n>(params[n]);


#ifndef  BOOST_PP_IS_ITERATING
#ifndef __proxy_functions_hpp__
#define __proxy_functions_hpp__

#include "boxed_value.hpp"
#include "type_info.hpp"
#include <string>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include <stdexcept>
#include <vector>

namespace dispatchkit
{
  // handle_return implementations
  template<typename Ret>
    struct Handle_Return
    {
      Boxed_Value operator()(const boost::function<Ret ()> &f)
      {
        return Boxed_Value(f());
      }
    };

  template<typename Ret>
    struct Handle_Return<Ret &>
    {
      Boxed_Value operator()(const boost::function<Ret &()> &f)
      {
        return Boxed_Value(boost::ref(f()));
      }
    };

  template<>
    struct Handle_Return<Boxed_Value>
    {
      Boxed_Value operator()(const boost::function<Boxed_Value ()> &f)
      {
        return f();
      }
    };

  template<>
    struct Handle_Return<Boxed_Value &>
    {
      Boxed_Value operator()(const boost::function<Boxed_Value &()> &f)
      {
        return f();
      }
    };

  template<>
    struct Handle_Return<void>
    {
      Boxed_Value operator()(const boost::function<void ()> &f)
      {
        f();
        return Boxed_Value(Boxed_Value::Void_Type());
      }
    };


  struct Param_List_Builder
  {
    Param_List_Builder &operator<<(const Boxed_Value &so)
    {
      objects.push_back(so);
      return *this;
    }

    template<typename T>
      Param_List_Builder &operator<<(T t)
      {
        objects.push_back(Boxed_Value(t));
        return *this;
      }

    operator const std::vector<Boxed_Value> &() const
    {
      return objects;
    }

    std::vector<Boxed_Value> objects;
  };

  struct arity_error : std::range_error
  {
    arity_error(int t_got, int t_expected)
      : std::range_error("Function dispatch arity mismatch"),
        got(t_got), expected(t_expected)
    {
    }

    virtual ~arity_error() throw() {}
    int got;
    int expected;
  };

}

#define BOOST_PP_ITERATION_LIMITS ( 0, 10 )
#define BOOST_PP_FILENAME_1 <chaiscript/dispatchkit/proxy_functions.hpp>
#include BOOST_PP_ITERATE()

namespace dispatchkit
{
  class Proxy_Function
  {
    public:
      virtual ~Proxy_Function() {}
      virtual Boxed_Value operator()(const std::vector<Boxed_Value> &params) = 0;
      virtual std::vector<Type_Info> get_param_types() const = 0;
      virtual bool operator==(const Proxy_Function &) const = 0;
      virtual bool types_match(const std::vector<Boxed_Value> &types) const = 0;
      virtual std::string annotation() const = 0;
  };

  class guard_error : public std::runtime_error
  {
    public:
      guard_error() throw()
        : std::runtime_error("Guard evaluation failed")
      { }

      virtual ~guard_error() throw()
      { }
  };

  class Dynamic_Proxy_Function : public Proxy_Function
  {
    public:
      Dynamic_Proxy_Function(
          const boost::function<Boxed_Value (const std::vector<Boxed_Value> &)> &t_f, 
          int t_arity=-1,
          const std::string &t_description = "",
          const boost::shared_ptr<Proxy_Function> &t_guard = boost::shared_ptr<Proxy_Function>())
        : m_f(t_f), m_arity(t_arity), m_description(t_description), m_guard(t_guard)
      {
      }

      virtual bool operator==(const Proxy_Function &) const
      {
        return false;
      }

      virtual bool types_match(const std::vector<Boxed_Value> &types) const
      {
          return (m_arity < 0 || types.size() == size_t(m_arity))
            && test_guard(types);
      }    

      virtual ~Dynamic_Proxy_Function() {}

      virtual Boxed_Value operator()(const std::vector<Boxed_Value> &params)
      {
        if (m_arity < 0 || params.size() == size_t(m_arity))
        {

          if (test_guard(params))
          {
            return m_f(params);
          } else {
            throw guard_error();
          }

        } else {
          throw arity_error(params.size(), m_arity);
        } 
      }

      virtual std::vector<Type_Info> get_param_types() const
      {
        std::vector<Type_Info> types;

        types.push_back(Get_Type_Info<Boxed_Value>::get());

        if (m_arity >= 0)
        {
          for (int i = 0; i < m_arity; ++i)
          {
            types.push_back(Get_Type_Info<Boxed_Value>::get());
          }
        } else {
          types.push_back(Get_Type_Info<std::vector<Boxed_Value> >::get());
        }

        return types;
      }

      virtual std::string annotation() const
      {
        return m_description;
      }

    private:
      bool test_guard(const std::vector<Boxed_Value> &params) const
      {
        if (m_guard)
        {
          try {
            return boxed_cast<bool>((*m_guard)(params));
          } catch (const arity_error &) {
            return false;
          } catch (const bad_boxed_cast &) {
            return false;
          }
        } else {
          return true;
        }
      }

      boost::function<Boxed_Value (const std::vector<Boxed_Value> &)> m_f;
      int m_arity;
      std::string m_description;
      boost::shared_ptr<Proxy_Function> m_guard;
  };

  struct Placeholder_Object
  {
  };

  class Bound_Function : public Proxy_Function
  {
    public:
      Bound_Function(const boost::shared_ptr<Proxy_Function> &t_f, 
                     const std::vector<Boxed_Value> &t_args)
        : m_f(t_f), m_args(t_args)
      {
      }

      virtual bool operator==(const Proxy_Function &) const
      {
        return false;
      }

      virtual ~Bound_Function() {}

      virtual bool types_match(const std::vector<Boxed_Value> &types) const
      {
        std::vector<Boxed_Value> params = build_param_list(types);
        return m_f->types_match(params);
      }

      virtual Boxed_Value operator()(const std::vector<Boxed_Value> &params)
      {
        return (*m_f)(build_param_list(params));
      }

      std::vector<Boxed_Value> build_param_list(const std::vector<Boxed_Value> &params) const
      {
        typedef std::vector<Boxed_Value>::const_iterator pitr;

        pitr parg = params.begin();
        pitr barg = m_args.begin();

        std::vector<Boxed_Value> args;

        while (true)
        {
          while (barg != m_args.end() 
                 && !(barg->get_type_info() == Get_Type_Info<Placeholder_Object>::get()))
          {
            args.push_back(*barg);
            ++barg;
          }

          if (parg != params.end())
          {
            args.push_back(*parg);
            ++parg;
          }

          if (barg != m_args.end() 
              && barg->get_type_info() == Get_Type_Info<Placeholder_Object>::get())
          {
            ++barg;
          } 

          if (parg == params.end() && barg == m_args.end())
          {
            break;
          }
        }

        return args;
      }

      virtual std::vector<Type_Info> get_param_types() const
      {
        return std::vector<Type_Info>();
      }

      virtual std::string annotation() const
      {
        return "";
      }


    private:
      boost::shared_ptr<Proxy_Function> m_f;
      std::vector<Boxed_Value> m_args;
  };

  template<typename Func>
    class Proxy_Function_Impl : public Proxy_Function
  {
    public:
      Proxy_Function_Impl(const Func &f)
        : m_f(f)
      {
      }

      virtual ~Proxy_Function_Impl() {}

      virtual bool operator==(const Proxy_Function &t_func) const
      {
        try {
          dynamic_cast<const Proxy_Function_Impl<Func> &>(t_func);
          return true; 
        } catch (const std::bad_cast &) {
          return false;
        }
      }
 
      virtual Boxed_Value operator()(const std::vector<Boxed_Value> &params)
      {
        return call_func(m_f, params);
      }

      virtual std::vector<Type_Info> get_param_types() const
      {
        return build_param_type_list(m_f);
      }

      virtual bool types_match(const std::vector<Boxed_Value> &types) const
      {
        return compare_types(m_f, types);
      }

      virtual std::string annotation() const
      {
        return "";
      }


    private:
      Func m_f;
  };

  struct dispatch_error : std::runtime_error
  {
    dispatch_error() throw()
      : std::runtime_error("No matching function to dispatch to")
    {
    }

    virtual ~dispatch_error() throw() {}
  };

  Boxed_Value dispatch(const std::vector<std::pair<std::string, boost::shared_ptr<Proxy_Function> > > &funcs,
      const std::vector<Boxed_Value> &plist)
  {
    for (std::vector<std::pair<std::string, boost::shared_ptr<Proxy_Function> > >::const_iterator itr = funcs.begin();
        itr != funcs.end();
        ++itr)
    {
      try {
        return (*itr->second)(plist);
      } catch (const bad_boxed_cast &) {
        //parameter failed to cast, try again
      } catch (const arity_error &) {
        //invalid num params, try again
      } catch (const guard_error &) {
        //guard failed to allow the function to execute,
        //try again
      }
    }

    throw dispatch_error();
  }
}

# endif
#else
# define n BOOST_PP_ITERATION()

namespace dispatchkit
{

  template<typename Ret BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param) >
    std::vector<Type_Info> build_param_type_list(const boost::function<Ret (BOOST_PP_ENUM_PARAMS(n, Param))> &)
    {
      std::vector<Type_Info> ti;
      ti.push_back(Get_Type_Info<Ret>::get());

      BOOST_PP_REPEAT(n, gettypeinfo, ~)

      return ti;
    }

  template<typename Ret BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param)>
    Boxed_Value call_func(const boost::function<Ret (BOOST_PP_ENUM_PARAMS(n, Param))> &f,
        const std::vector<Boxed_Value> &params)
    {
      if (params.size() != n)
      {
        throw arity_error(params.size(), n);
      } else {
        return Handle_Return<Ret>()(boost::bind(f BOOST_PP_REPEAT(n, casthelper, ~)));
      }
    }

  template<typename Ret BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param)>
    bool compare_types(const boost::function<Ret (BOOST_PP_ENUM_PARAMS(n, Param))> &,
        const std::vector<Boxed_Value> &params)
    {
      if (params.size() != n)
      {
        return false;
      } else {
        bool val = true BOOST_PP_REPEAT(n, comparetype, ~);
        if (val) return true;

        try {
          BOOST_PP_REPEAT(n, trycast, ~);
        } catch (const bad_boxed_cast &) {
          return false;
        }

        return true;
      }
    }

}

#endif

