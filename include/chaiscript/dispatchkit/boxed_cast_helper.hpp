// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2011, Jonathan Turner (jonathan@emptycrate.com)
// and Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_BOXED_CAST_HELPER_HPP_
#define CHAISCRIPT_BOXED_CAST_HELPER_HPP_

#include "type_info.hpp"
#include "boxed_value.hpp"

#include <boost/any.hpp>

namespace chaiscript 
{
  namespace detail
  {
    // Cast_Helper_Inner helper classes

    /**
     * Generic Cast_Helper_Inner, for casting to any type
     */
    template<typename Result>
      struct Cast_Helper_Inner
      {
        typedef typename std::reference_wrapper<typename std::add_const<Result>::type > Result_Type;

        static Result_Type cast(const Boxed_Value &ob)
        {
          if (ob.is_ref())
          {
            if (!ob.get_type_info().is_const())
            {
              return std::cref((boost::any_cast<std::reference_wrapper<Result> >(ob.get())).get());
            } else {
              return boost::any_cast<std::reference_wrapper<const Result> >(ob.get());
            }
          } else {
            if (!ob.get_type_info().is_const())
            {
              return std::cref(*(boost::any_cast<std::shared_ptr<Result> >(ob.get())));   
            } else {
              return std::cref(*(boost::any_cast<std::shared_ptr<const Result> >(ob.get())));   
            }
          }
        }
      };

	template<typename Result>
	  struct Cast_Helper_Inner<const Result> : Cast_Helper_Inner<Result>
	  {
	  };

    /**
     * Cast_Helper_Inner for casting to a const & type
     */
    template<typename Result>
      struct Cast_Helper_Inner<const Result &> : Cast_Helper_Inner<Result>
      {
      };

    /**
     * Cast_Helper_Inner for casting to a const * type
     */
    template<typename Result>
      struct Cast_Helper_Inner<const Result *>
      {
        typedef const Result * Result_Type;

        static Result_Type cast(const Boxed_Value &ob)
        {
          if (ob.is_ref())
          {
            if (!ob.get_type_info().is_const())
            {
              return &(boost::any_cast<std::reference_wrapper<Result> >(ob.get())).get();
            } else {
              return &(boost::any_cast<std::reference_wrapper<const Result> >(ob.get())).get();
            }
          } else {
            if (!ob.get_type_info().is_const())
            {
              return (boost::any_cast<std::shared_ptr<Result> >(ob.get())).get();
            } else {
              return (boost::any_cast<std::shared_ptr<const Result> >(ob.get())).get();
            }
          }
        }
      };

    /**
     * Cast_Helper_Inner for casting to a * type
     */
    template<typename Result>
      struct Cast_Helper_Inner<Result *>
      {
        typedef Result * Result_Type;

        static Result_Type cast(const Boxed_Value &ob)
        {
          if (ob.is_ref())
          {
            return &(boost::any_cast<std::reference_wrapper<Result> >(ob.get())).get();
          } else {
            return (boost::any_cast<std::shared_ptr<Result> >(ob.get())).get();
          }
        }
      };

    /**
     * Cast_Helper_Inner for casting to a & type
     */
    template<typename Result>
      struct Cast_Helper_Inner<Result &>
      {
        typedef Result& Result_Type;

        static Result &cast(const Boxed_Value &ob)
        {
          if (ob.is_ref())
          {
            return boost::any_cast<std::reference_wrapper<Result> >(ob.get());
          } else {
            Result &r = *(boost::any_cast<std::shared_ptr<Result> >(ob.get()));
            return r;
          }
        }
      };

    /**
     * Cast_Helper_Inner for casting to a std::shared_ptr<> type
     */
    template<typename Result>
      struct Cast_Helper_Inner<typename std::shared_ptr<Result> >
      {
        typedef typename std::shared_ptr<Result> Result_Type;

        static Result_Type cast(const Boxed_Value &ob)
        {
          return boost::any_cast<std::shared_ptr<Result> >(ob.get());
        }
      };

    /**
     * Cast_Helper_Inner for casting to a std::shared_ptr<const> type
     */
    template<typename Result>
      struct Cast_Helper_Inner<typename std::shared_ptr<const Result> >
      {
        typedef typename std::shared_ptr<const Result> Result_Type;

        static Result_Type cast(const Boxed_Value &ob)
        {
          if (!ob.get_type_info().is_const())
          {
            return std::const_pointer_cast<const Result>(boost::any_cast<std::shared_ptr<Result> >(ob.get()));
          } else {
            return boost::any_cast<std::shared_ptr<const Result> >(ob.get());
          }
        }
      };

    /**
     * Cast_Helper_Inner for casting to a const std::shared_ptr<> & type
     */
    template<typename Result>
      struct Cast_Helper_Inner<const std::shared_ptr<Result> > : Cast_Helper_Inner<std::shared_ptr<Result> >
      {
      };

    template<typename Result>
      struct Cast_Helper_Inner<const std::shared_ptr<Result> &> : Cast_Helper_Inner<std::shared_ptr<Result> >
      {
      };


    /**
     * Cast_Helper_Inner for casting to a const std::shared_ptr<const> & type
     */
    template<typename Result>
      struct Cast_Helper_Inner<const std::shared_ptr<const Result> > : Cast_Helper_Inner<std::shared_ptr<const Result> >
      {
      };

    template<typename Result>
      struct Cast_Helper_Inner<const std::shared_ptr<const Result> &> : Cast_Helper_Inner<std::shared_ptr<const Result> >
      {
      };



    /**
     * Cast_Helper_Inner for casting to a Boxed_Value type
     */
    template<>
      struct Cast_Helper_Inner<Boxed_Value>
      {
        typedef const Boxed_Value & Result_Type;

        static Result_Type cast(const Boxed_Value &ob)
        {
          return ob;    
        }
      };

    /**
     * Cast_Helper_Inner for casting to a const Boxed_Value & type
     */
    template<>
      struct Cast_Helper_Inner<const Boxed_Value> : Cast_Helper_Inner<Boxed_Value>
      {
      };

    template<>
      struct Cast_Helper_Inner<const Boxed_Value &> : Cast_Helper_Inner<Boxed_Value>
      {
      };
    

	/**
     * Cast_Helper_Inner for casting to a std::reference_wrapper type
     */
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

    /**
     * The exposed Cast_Helper object that by default just calls the Cast_Helper_Inner
     */
    template<typename T>
      struct Cast_Helper
      {
        typedef typename Cast_Helper_Inner<T>::Result_Type Result_Type;
        
        static Result_Type cast(const Boxed_Value &ob)
        {
          return Cast_Helper_Inner<T>::cast(ob);
        }
      };
  }
  
}

#endif
