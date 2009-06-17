#include <boost/preprocessor.hpp>

#define gettypeinfo(z,n,text)  ti.push_back(Get_Type_Info<Param ## n>::get());
#define casthelper(z,n,text) ,Cast_Helper<Param ## n>()(params[n])


#ifndef  BOOST_PP_IS_ITERATING
#ifndef __proxy_functions_hpp__
#define __proxy_functions_hpp__

#include "boxed_value.hpp"
#include "type_info.hpp"
#include <string>
#include <boost/function.hpp>
#include <boost/bind.hpp>
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
}

#define BOOST_PP_ITERATION_LIMITS ( 0, 10 )
#define BOOST_PP_FILENAME_1 "proxy_functions.hpp"
#include BOOST_PP_ITERATE()

namespace dispatchkit
{
  class Proxy_Function
  {
    public:
      virtual ~Proxy_Function() {}
      virtual Boxed_Value operator()(const std::vector<Boxed_Value> &params) = 0;
      virtual std::vector<Type_Info> get_param_types() = 0;

  };

  class Dynamic_Proxy_Function : public Proxy_Function
  {
    public:
      Dynamic_Proxy_Function(const boost::function<Boxed_Value (const std::vector<Boxed_Value> &)> &t_f, int arity=-1)
        : m_f(t_f), m_arity(arity)
      {
      }

      virtual ~Dynamic_Proxy_Function() {}

      virtual Boxed_Value operator()(const std::vector<Boxed_Value> &params)
      {
        if (m_arity < 0 || params.size() == size_t(m_arity))
        {
          return m_f(params);
        } else {
          throw std::range_error("Incorrect number of parameters");
        } 
      }

      virtual std::vector<Type_Info> get_param_types()
      {
        return build_param_type_list(m_f);
      }

    private:
      boost::function<Boxed_Value (const std::vector<Boxed_Value> &)> m_f;
      int m_arity;
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


      virtual Boxed_Value operator()(const std::vector<Boxed_Value> &params)
      {
        return call_func(m_f, params);
      }

      virtual std::vector<Type_Info> get_param_types()
      {
        return build_param_type_list(m_f);
      }

    private:
      Func m_f;
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
      } catch (const std::bad_cast &) {
        //try again
      } catch (const std::range_error &) {
        //invalid num params, try again
      }
    }

    throw std::runtime_error("No matching function to dispatch to");
  }
}

# endif
#else
# define n BOOST_PP_ITERATION()

namespace dispatchkit
{

  template<typename Ret BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param) >
    std::vector<Type_Info> build_param_type_list(const boost::function<Ret (BOOST_PP_ENUM_PARAMS(n, Param))> &f)
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
        throw std::range_error("Incorrect number of parameters");
      } else {
        return Handle_Return<Ret>()(boost::bind(f BOOST_PP_REPEAT(n, casthelper, ~)));
      }
    }
}

#endif

