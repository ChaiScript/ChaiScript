// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2016, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_BOXED_CAST_HELPER_HPP_
#define CHAISCRIPT_BOXED_CAST_HELPER_HPP_

#include <memory>
#include <type_traits>

#include "boxed_value.hpp"
#include "type_info.hpp"


namespace chaiscript 
{
  class Type_Conversions_State;

  namespace detail
  {
    // Cast_Helper_Inner helper classes

    template<typename T>
      T* throw_if_null(T *t)
      {
        if (t) return t;
        throw std::runtime_error("Attempted to dereference null Boxed_Value");
      }

    /// Generic Cast_Helper_Inner, for casting to any type
    template<typename Result>
      struct Cast_Helper_Inner
      {
        typedef typename std::add_const<Result>::type Result_Type;

        static Result_Type cast(const Boxed_Value &ob, const Type_Conversions_State *)
        {
          if (ob.get_type_info().bare_equal_type_info(typeid(Result)))
          {
            auto p = throw_if_null(ob.get_const_ptr());
            return *static_cast<const Result *>(p);
          } else {
            throw chaiscript::detail::exception::bad_any_cast();
          }
        }
      };

    template<typename Result>
      struct Cast_Helper_Inner<const Result> : Cast_Helper_Inner<Result>
      {
      };


    /// Cast_Helper_Inner for casting to a const * type
    template<typename Result>
      struct Cast_Helper_Inner<const Result *>
      {
        typedef const Result * Result_Type;
        static Result_Type cast(const Boxed_Value &ob, const Type_Conversions_State *)
        {
          if (ob.get_type_info().bare_equal_type_info(typeid(Result)))
          {
            return static_cast<const Result *>(ob.get_const_ptr());
          } else {
            throw chaiscript::detail::exception::bad_any_cast();
          }
        }
      };

    /// Cast_Helper_Inner for casting to a * type
    template<typename Result>
      struct Cast_Helper_Inner<Result *>
      {
        typedef Result * Result_Type;
        static Result_Type cast(const Boxed_Value &ob, const Type_Conversions_State *)
        {
          if (!ob.get_type_info().is_const() && ob.get_type_info() == typeid(Result))
          {
            return static_cast<Result *>(ob.get_ptr());
          } else {
            throw chaiscript::detail::exception::bad_any_cast();
          }
        }
      };

    template<typename Result>
    struct Cast_Helper_Inner<Result * const &> : public Cast_Helper_Inner<Result *>
    {
    };

    template<typename Result>
    struct Cast_Helper_Inner<const Result * const &> : public Cast_Helper_Inner<const Result *>
    {
    };


    /// Cast_Helper_Inner for casting to a & type
    template<typename Result>
      struct Cast_Helper_Inner<const Result &>
      {
        typedef const Result& Result_Type;

        static Result_Type cast(const Boxed_Value &ob, const Type_Conversions_State *)
        {
          if (ob.get_type_info().bare_equal_type_info(typeid(Result)))
          {
            auto p = throw_if_null(ob.get_const_ptr());
            return *static_cast<const Result *>(p);
          } else {
            throw chaiscript::detail::exception::bad_any_cast();
          }
        }
      };



    /// Cast_Helper_Inner for casting to a & type
    template<typename Result>
      struct Cast_Helper_Inner<Result &>
      {
        typedef Result& Result_Type;

        static Result_Type cast(const Boxed_Value &ob, const Type_Conversions_State *)
        {
          if (!ob.get_type_info().is_const() && ob.get_type_info().bare_equal_type_info(typeid(Result)))
          {
            return *(static_cast<Result *>(throw_if_null(ob.get_ptr())));
          } else {
            throw chaiscript::detail::exception::bad_any_cast();
          }
        }
      };

    /// Cast_Helper_Inner for casting to a std::shared_ptr<> type
    template<typename Result>
      struct Cast_Helper_Inner<std::shared_ptr<Result> >
      {
        typedef std::shared_ptr<Result> Result_Type;

        static Result_Type cast(const Boxed_Value &ob, const Type_Conversions_State *)
        {
          return ob.get().cast<std::shared_ptr<Result> >();
        }
      };

    /// Cast_Helper_Inner for casting to a std::shared_ptr<const> type
    template<typename Result>
      struct Cast_Helper_Inner<std::shared_ptr<const Result> >
      {
        typedef std::shared_ptr<const Result> Result_Type;

        static Result_Type cast(const Boxed_Value &ob, const Type_Conversions_State *)
        {
          if (!ob.get_type_info().is_const())
          {
            return std::const_pointer_cast<const Result>(ob.get().cast<std::shared_ptr<Result> >());
          } else {
            return ob.get().cast<std::shared_ptr<const Result> >();
          }
        }
      };

    /// Cast_Helper_Inner for casting to a const std::shared_ptr<> & type
    template<typename Result>
      struct Cast_Helper_Inner<const std::shared_ptr<Result> > : Cast_Helper_Inner<std::shared_ptr<Result> >
      {
      };

    template<typename Result>
      struct Cast_Helper_Inner<const std::shared_ptr<Result> &> : Cast_Helper_Inner<std::shared_ptr<Result> >
      {
      };

    /// Cast_Helper_Inner for casting to a const std::shared_ptr<const> & type
    template<typename Result>
      struct Cast_Helper_Inner<const std::shared_ptr<const Result> > : Cast_Helper_Inner<std::shared_ptr<const Result> >
      {
      };

    template<typename Result>
      struct Cast_Helper_Inner<const std::shared_ptr<const Result> &> : Cast_Helper_Inner<std::shared_ptr<const Result> >
      {
      };


    /// Cast_Helper_Inner for casting to a Boxed_Value type
    template<>
      struct Cast_Helper_Inner<Boxed_Value>
      {
        typedef Boxed_Value Result_Type;

        static Result_Type cast(const Boxed_Value &ob, const Type_Conversions_State *)
        {
          return ob;
        }
      };

    /// Cast_Helper_Inner for casting to a Boxed_Value & type
    template<>
      struct Cast_Helper_Inner<Boxed_Value &>
      {
        typedef std::reference_wrapper<Boxed_Value> Result_Type;

        static Result_Type cast(const Boxed_Value &ob, const Type_Conversions_State *)
        {
          return std::ref(const_cast<Boxed_Value &>(ob));
        }
      };


    /// Cast_Helper_Inner for casting to a const Boxed_Value & type
    template<>
      struct Cast_Helper_Inner<const Boxed_Value> : Cast_Helper_Inner<Boxed_Value>
      {
      };

    template<>
      struct Cast_Helper_Inner<const Boxed_Value &> : Cast_Helper_Inner<Boxed_Value>
      {
      };


    /// Cast_Helper_Inner for casting to a std::reference_wrapper type
    template<typename Result>
      struct Cast_Helper_Inner<std::reference_wrapper<Result> > : Cast_Helper_Inner<Result &>
      {
      };

    template<typename Result>
      struct Cast_Helper_Inner<const std::reference_wrapper<Result> > : Cast_Helper_Inner<Result &>
      {
      };

    template<typename Result>
      struct Cast_Helper_Inner<const std::reference_wrapper<Result> &> : Cast_Helper_Inner<Result &>
      {
      };

    template<typename Result>
      struct Cast_Helper_Inner<std::reference_wrapper<const Result> > : Cast_Helper_Inner<const Result &>
      {
      };

    template<typename Result>
      struct Cast_Helper_Inner<const std::reference_wrapper<const Result> > : Cast_Helper_Inner<const Result &>
      {
      };

    template<typename Result>
      struct Cast_Helper_Inner<const std::reference_wrapper<const Result> & > : Cast_Helper_Inner<const Result &>
      {
      };

    /// The exposed Cast_Helper object that by default just calls the Cast_Helper_Inner
    template<typename T>
      struct Cast_Helper
      {
        typedef typename Cast_Helper_Inner<T>::Result_Type Result_Type;

        static Result_Type cast(const Boxed_Value &ob, const Type_Conversions_State *t_conversions)
        {
          return Cast_Helper_Inner<T>::cast(ob, t_conversions);
        }
      };
  }
  
}

#endif
