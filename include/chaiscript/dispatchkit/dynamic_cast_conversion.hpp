#ifndef __chaiscriptdispatchkit_dynamic_cast_conversion_hpp__
#define __chaiscriptdispatchkit_dynamic_cast_conversion_hpp__

#include "type_info.hpp"
#include "boxed_value.hpp"
#include "boxed_cast_helper.hpp"
#include "bad_boxed_cast.hpp"

namespace chaiscript
{
  class bad_boxed_dynamic_cast : public bad_boxed_cast
  {
    public:
      bad_boxed_dynamic_cast(const Type_Info &t_from, const std::type_info &t_to,
          const std::string &t_what)
        : bad_boxed_cast(t_from, t_to, t_what)
      {
      }

      bad_boxed_dynamic_cast(const Type_Info &t_from, const std::type_info &t_to) throw()
        : bad_boxed_cast(t_from, t_to)
      {
      }

      bad_boxed_dynamic_cast(const std::string &w) throw()
        : bad_boxed_cast(w)
      {
      }
  };


  namespace detail
  {
    class Dynamic_Conversion
    {
      public:
        virtual Boxed_Value convert(const Boxed_Value &derived) = 0;
        const Type_Info &base()
        {
          return m_base;
        }
        const Type_Info &derived()
        {
          return m_derived;
        }

      protected:
        Dynamic_Conversion(const Type_Info &t_base, const Type_Info &t_derived)
          : m_base(t_base), m_derived(t_derived)
        {
        }

      private:
        Type_Info m_base;
        Type_Info m_derived;

    };

    template<typename Base, typename Derived>
    class Dynamic_Conversion_Impl : public Dynamic_Conversion
    {
      public:
        Dynamic_Conversion_Impl()
          : Dynamic_Conversion(user_type<Base>(), user_type<Derived>())
        {
        }

        virtual Boxed_Value convert(const Boxed_Value &derived)
        {
          if (derived.get_type_info().bare_equal(user_type<Derived>()))
          {
            if (derived.is_pointer())
            {
              // Dynamic cast out the contained boxed value, which we know is the type we want
              if (derived.is_const())
              {
                boost::shared_ptr<const Base> data 
                  = boost::dynamic_pointer_cast<const Base>(detail::Cast_Helper<boost::shared_ptr<const Derived> >::cast(derived));
                if (!data)
                {
                  throw std::bad_cast();
                }

                return Boxed_Value(data);
              } else {
                std::cout << "performing boxed_dynamic_cast" << std::endl;
                boost::shared_ptr<Base> data 
                  = boost::dynamic_pointer_cast<Base>(detail::Cast_Helper<boost::shared_ptr<Derived> >::cast(derived));

                if (!data)
                {
                  throw std::bad_cast();
                }

                std::cout << "typeinfo " << typeid(data).name() << std::endl;
                Boxed_Value ret(data);
                std::cout << " It worked " << ret.get_type_info().name() << std::endl;

                return ret;
              }
            } else {
              // Pull the reference out of the contained boxed value, which we know is the type we want
              if (derived.is_const())
              {
                const Derived &d = detail::Cast_Helper<const Derived &>::cast(derived);
                const Base &data = dynamic_cast<const Base &>(d);
                return Boxed_Value(boost::cref(data));
              } else {
                Derived &d = detail::Cast_Helper<Derived &>::cast(derived);
                Base &data = dynamic_cast<Base &>(d);
                return Boxed_Value(boost::ref(data));
              }
            }
          } else {
            throw bad_boxed_dynamic_cast(derived.get_type_info(), typeid(Base), "Unknown dynamic_cast_conversion");
          }

        }
    };

    class Dynamic_Conversions
    {
      public:
        template<typename Base, typename Derived>
          static void add_conversion()
          {
            get_conversions().push_back(
                boost::shared_ptr<Dynamic_Conversion>(new Dynamic_Conversion_Impl<Base, Derived>())
              );
          }

        static bool has_conversion(const Type_Info &base, const Type_Info &derived)
        {
          const std::vector<boost::shared_ptr<Dynamic_Conversion> > &convs
            = get_conversions();

          for (std::vector<boost::shared_ptr<Dynamic_Conversion> >::const_iterator itr = convs.begin();
               itr != convs.end();
               ++itr)
          {
            if ((*itr)->base().bare_equal(base) && (*itr)->derived().bare_equal(derived))
            {
              return true;
            }
          }

          return false;
        }

        static boost::shared_ptr<Dynamic_Conversion> get_conversion(const Type_Info &base, const Type_Info &derived)
        {
          const std::vector<boost::shared_ptr<Dynamic_Conversion> > &convs
            = get_conversions();

          for (std::vector<boost::shared_ptr<Dynamic_Conversion> >::const_iterator itr = convs.begin();
               itr != convs.end();
               ++itr)
          {
            if ((*itr)->base().bare_equal(base) && (*itr)->derived().bare_equal(derived))
            {
              return *itr; 
            }
          }

          throw std::out_of_range("No such conversion exists from " + derived.bare_name() + " to " + base.bare_name());
        }

    private:
        static std::vector<boost::shared_ptr<Dynamic_Conversion> > &get_conversions()
        {
          static std::vector<boost::shared_ptr<Dynamic_Conversion> > convs;
          return convs;
        }
    };
  }

  template<typename Base, typename Derived>
  void register_base_class()
  {
    detail::Dynamic_Conversions::add_conversion<Base, Derived>();
  }

  template<typename Base, typename Derived>
  bool dynamic_cast_converts()
  {
    return dynamic_cast_converts(user_type<Base>(), user_type<Derived>());
  }

  bool dynamic_cast_converts(const Type_Info &base, const Type_Info &derived)
  {
    return detail::Dynamic_Conversions::has_conversion(base, derived);
  }

  template<typename Base>
  Boxed_Value boxed_dynamic_cast(const Boxed_Value &derived)
  {
    std::cout << " Attempting conversion from " << derived.get_type_info().bare_name() << " to "<< typeid(Base).name()  
      << std::endl;
    try {
      return detail::Dynamic_Conversions::get_conversion(user_type<Base>(), derived.get_type_info())->convert(derived);
    } catch (const std::out_of_range &) {
      throw bad_boxed_dynamic_cast(derived.get_type_info(), typeid(Base), "No known conversion");
    } catch (const std::bad_cast &) {
      throw bad_boxed_dynamic_cast(derived.get_type_info(), typeid(Base), "Unable to perform dynamic_cast operation");
    }
  }

}


#endif
