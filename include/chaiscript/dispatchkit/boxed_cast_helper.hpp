// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2014, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_BOXED_CAST_HELPER_HPP_
#define CHAISCRIPT_BOXED_CAST_HELPER_HPP_

#include "type_info.hpp"
#include "boxed_value.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>
#include <boost/ref.hpp>
#include <boost/type_traits/add_const.hpp>

namespace chaiscript 
{
  class Dynamic_Cast_Conversions;

  namespace detail
  {
    // Cast_Helper_Inner helper classes

    /**
     * Generic Cast_Helper_Inner, for casting to any type
     */
    template<typename Result>
      struct Cast_Helper_Inner
      {
        typedef typename boost::reference_wrapper<typename boost::add_const<Result>::type > Result_Type;

        static Result_Type cast(const Boxed_Value &ob, const Dynamic_Cast_Conversions *)
        {
          if (ob.is_ref())
          {
            if (!ob.get_type_info().is_const())
            {
              return boost::cref((boost::any_cast<boost::reference_wrapper<Result> >(ob.get())).get());
            } else {
              return boost::any_cast<boost::reference_wrapper<const Result> >(ob.get());
            }
          } else {
            if (!ob.get_type_info().is_const())
            {
              return boost::cref(*(boost::any_cast<boost::shared_ptr<Result> >(ob.get())));   
            } else {
              return boost::cref(*(boost::any_cast<boost::shared_ptr<const Result> >(ob.get())));   
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

        static Result_Type cast(const Boxed_Value &ob, const Dynamic_Cast_Conversions *)
        {
          if (ob.is_ref())
          {
            if (!ob.get_type_info().is_const())
            {
              return (boost::any_cast<boost::reference_wrapper<Result> >(ob.get())).get_pointer();
            } else {
              return (boost::any_cast<boost::reference_wrapper<const Result> >(ob.get())).get_pointer();
            }
          } else {
            if (!ob.get_type_info().is_const())
            {
              return (boost::any_cast<boost::shared_ptr<Result> >(ob.get())).get();
            } else {
              return (boost::any_cast<boost::shared_ptr<const Result> >(ob.get())).get();
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

        static Result_Type cast(const Boxed_Value &ob, const Dynamic_Cast_Conversions *)
        {
          if (ob.is_ref())
          {
            return (boost::any_cast<boost::reference_wrapper<Result> >(ob.get())).get_pointer();
          } else {
            return (boost::any_cast<boost::shared_ptr<Result> >(ob.get())).get();
          }
        }
      };

    /**
     * Cast_Helper_Inner for casting to a & type
     */
    template<typename Result>
      struct Cast_Helper_Inner<Result &>
      {
        typedef typename boost::reference_wrapper<Result> Result_Type;

        static Result_Type cast(const Boxed_Value &ob, const Dynamic_Cast_Conversions *)
        {
          if (ob.is_ref())
          {
            return boost::any_cast<boost::reference_wrapper<Result> >(ob.get());
          } else {
            return boost::ref(*(boost::any_cast<boost::shared_ptr<Result> >(ob.get())));
          }
        }
      };

    /**
     * Cast_Helper_Inner for casting to a boost::shared_ptr<> type
     */
    template<typename Result>
      struct Cast_Helper_Inner<typename boost::shared_ptr<Result> >
      {
        typedef typename boost::shared_ptr<Result> Result_Type;

        static Result_Type cast(const Boxed_Value &ob, const Dynamic_Cast_Conversions *)
        {
          return boost::any_cast<boost::shared_ptr<Result> >(ob.get());
        }
      };

    /**
     * Cast_Helper_Inner for casting to a boost::shared_ptr<const> type
     */
    template<typename Result>
      struct Cast_Helper_Inner<typename boost::shared_ptr<const Result> >
      {
        typedef typename boost::shared_ptr<const Result> Result_Type;

        static Result_Type cast(const Boxed_Value &ob, const Dynamic_Cast_Conversions *)
        {
          if (!ob.get_type_info().is_const())
          {
            return boost::const_pointer_cast<const Result>(boost::any_cast<boost::shared_ptr<Result> >(ob.get()));
          } else {
            return boost::any_cast<boost::shared_ptr<const Result> >(ob.get());
          }
        }
      };

    /**
     * Cast_Helper_Inner for casting to a const boost::shared_ptr<> & type
     */
    template<typename Result>
      struct Cast_Helper_Inner<const boost::shared_ptr<Result> > : Cast_Helper_Inner<boost::shared_ptr<Result> >
      {
      };

    template<typename Result>
      struct Cast_Helper_Inner<const boost::shared_ptr<Result> &> : Cast_Helper_Inner<boost::shared_ptr<Result> >
      {
      };


    /**
     * Cast_Helper_Inner for casting to a const boost::shared_ptr<const> & type
     */
    template<typename Result>
      struct Cast_Helper_Inner<const boost::shared_ptr<const Result> > : Cast_Helper_Inner<boost::shared_ptr<const Result> >
      {
      };

    template<typename Result>
      struct Cast_Helper_Inner<const boost::shared_ptr<const Result> &> : Cast_Helper_Inner<boost::shared_ptr<const Result> >
      {
      };



    /**
     * Cast_Helper_Inner for casting to a Boxed_Value type
     */
    template<>
      struct Cast_Helper_Inner<Boxed_Value>
      {
        typedef const Boxed_Value & Result_Type;

        static Result_Type cast(const Boxed_Value &ob, const Dynamic_Cast_Conversions *)
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
     * Cast_Helper_Inner for casting to a boost::reference_wrapper type
     */
    template<typename Result>
      struct Cast_Helper_Inner<boost::reference_wrapper<Result> > : Cast_Helper_Inner<Result &>
      {
      };

    template<typename Result>
      struct Cast_Helper_Inner<const boost::reference_wrapper<Result> > : Cast_Helper_Inner<Result &>
      {
      };

    template<typename Result>
      struct Cast_Helper_Inner<const boost::reference_wrapper<Result> &> : Cast_Helper_Inner<Result &>
      {
      };

    template<typename Result>
      struct Cast_Helper_Inner<boost::reference_wrapper<const Result> > : Cast_Helper_Inner<const Result &>
      {
      };

    template<typename Result>
      struct Cast_Helper_Inner<const boost::reference_wrapper<const Result> > : Cast_Helper_Inner<const Result &>
      {
      };

    template<typename Result>
      struct Cast_Helper_Inner<const boost::reference_wrapper<const Result> & > : Cast_Helper_Inner<const Result &>
      {
      };

    /**
     * The exposed Cast_Helper object that by default just calls the Cast_Helper_Inner
     */
    template<typename T>
      struct Cast_Helper
      {
        typedef typename Cast_Helper_Inner<T>::Result_Type Result_Type;
        
        static Result_Type cast(const Boxed_Value &ob, const Dynamic_Cast_Conversions *t_conversions)
        {
          return Cast_Helper_Inner<T>::cast(ob, t_conversions);
        }
      };
  }
  
}

#endif
