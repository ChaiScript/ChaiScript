// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2010, Jonathan Turner (jonathan@emptycrate.com)
// and Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef __boxed_cast_helper_hpp__
#define __boxed_cast_helper_hpp__

#include "type_info.hpp"
#include "boxed_value.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>
#include <boost/ref.hpp>
#include <boost/type_traits/add_const.hpp>

namespace chaiscript 
{
  namespace detail
  {
    // Cast_Helper helper classes

    /**
     * Generic Cast_Helper, for casting to any type
     */
    template<typename Result>
      struct Cast_Helper
      {
        typedef typename boost::reference_wrapper<typename boost::add_const<Result>::type > Result_Type;

        static Result_Type cast(const Boxed_Value &ob)
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

    /**
     * Cast_Helper for casting to a const & type
     */
    template<typename Result>
      struct Cast_Helper<const Result &>
      {
        typedef typename boost::reference_wrapper<typename boost::add_const<Result>::type > Result_Type;

        static Result_Type cast(const Boxed_Value &ob)
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

    /**
     * Cast_Helper for casting to a const * type
     */
    template<typename Result>
      struct Cast_Helper<const Result *>
      {
        typedef const Result * Result_Type;

        static Result_Type cast(const Boxed_Value &ob)
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
     * Cast_Helper for casting to a * type
     */
    template<typename Result>
      struct Cast_Helper<Result *>
      {
        typedef Result * Result_Type;

        static Result_Type cast(const Boxed_Value &ob)
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
     * Cast_Helper for casting to a & type
     */
    template<typename Result>
      struct Cast_Helper<Result &>
      {
        typedef typename boost::reference_wrapper<Result> Result_Type;

        static Result_Type cast(const Boxed_Value &ob)
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
     * Cast_Helper for casting to a boost::shared_ptr<> type
     */
    template<typename Result>
      struct Cast_Helper<typename boost::shared_ptr<Result> >
      {
        typedef typename boost::shared_ptr<Result> Result_Type;

        static Result_Type cast(const Boxed_Value &ob)
        {
          return boost::any_cast<boost::shared_ptr<Result> >(ob.get());
        }
      };

    /**
     * Cast_Helper for casting to a boost::shared_ptr<const> type
     */
    template<typename Result>
      struct Cast_Helper<typename boost::shared_ptr<const Result> >
      {
        typedef typename boost::shared_ptr<const Result> Result_Type;

        static Result_Type cast(const Boxed_Value &ob)
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
     * Cast_Helper for casting to a const boost::shared_ptr<> & type
     */
    template<typename Result>
      struct Cast_Helper<const boost::shared_ptr<Result> &>
      {
        typedef typename boost::shared_ptr<Result> Result_Type;

        static Result_Type cast(const Boxed_Value &ob)
        {
          return boost::any_cast<boost::shared_ptr<Result> >(ob.get());
        }
      };

    /**
     * Cast_Helper for casting to a const boost::shared_ptr<const> & type
     */
    template<typename Result>
      struct Cast_Helper<const boost::shared_ptr<const Result> &>
      {
        typedef typename boost::shared_ptr<const Result> Result_Type;

        static Result_Type cast(const Boxed_Value &ob)
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
     * Cast_Helper for casting to a Boxed_Value type
     */
    template<>
      struct Cast_Helper<Boxed_Value>
      {
        typedef const Boxed_Value & Result_Type;

        static Result_Type cast(const Boxed_Value &ob)
        {
          return ob;    
        }
      };

    /**
     * Cast_Helper for casting to a const Boxed_Value & type
     */
    template<>
      struct Cast_Helper<const Boxed_Value &>
      {
        typedef const Boxed_Value & Result_Type;

        static Result_Type cast(const Boxed_Value &ob)
        {
          return ob;    
        }
      };
  }

  
}



#endif

