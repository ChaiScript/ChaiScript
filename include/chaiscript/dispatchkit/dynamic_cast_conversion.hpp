// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2014, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_DYNAMIC_CAST_CONVERSION_HPP_
#define CHAISCRIPT_DYNAMIC_CAST_CONVERSION_HPP_

#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>

#include "../chaiscript_threading.hpp"
#include "bad_boxed_cast.hpp"
#include "boxed_cast_helper.hpp"
#include "boxed_value.hpp"
#include "type_info.hpp"

namespace chaiscript
{
  namespace exception
  {
    class bad_boxed_dynamic_cast : public bad_boxed_cast
    {
      public:
        bad_boxed_dynamic_cast(const Type_Info &t_from, const std::type_info &t_to,
            const std::string &t_what) CHAISCRIPT_NOEXCEPT
          : bad_boxed_cast(t_from, t_to, t_what)
        {
        }

        bad_boxed_dynamic_cast(const Type_Info &t_from, const std::type_info &t_to) CHAISCRIPT_NOEXCEPT
          : bad_boxed_cast(t_from, t_to)
        {
        }

        bad_boxed_dynamic_cast(const std::string &w) CHAISCRIPT_NOEXCEPT
          : bad_boxed_cast(w)
        {
        }

        virtual ~bad_boxed_dynamic_cast() CHAISCRIPT_NOEXCEPT {}
    };
  }

  namespace detail
  {
    class Dynamic_Conversion
    {
      public:
        virtual Boxed_Value convert(const Boxed_Value &derived) const = 0;
        virtual Boxed_Value convert_down(const Boxed_Value &base) const = 0;

        const Type_Info &base() const
        {
          return m_base;
        }
        const Type_Info &derived() const
        {
          return m_derived;
        }

      protected:
        Dynamic_Conversion(const Type_Info &t_base, const Type_Info &t_derived)
          : m_base(t_base), m_derived(t_derived)
        {
        }

        virtual ~Dynamic_Conversion() {} 

      private:
        Type_Info m_base;
        Type_Info m_derived;

    };

    template<typename From, typename To> 
      class Dynamic_Caster
      {
        public: 
          static Boxed_Value cast(const Boxed_Value &t_from)
          {
            if (t_from.get_type_info().bare_equal(user_type<From>()))
            {
              if (t_from.is_pointer())
              {
                // Dynamic cast out the contained boxed value, which we know is the type we want
                if (t_from.is_const())
                {
                  std::shared_ptr<const To> data
                    = std::dynamic_pointer_cast<const To>(detail::Cast_Helper<std::shared_ptr<const From> >::cast(t_from, nullptr));
                  if (!data)
                  {
                    throw std::bad_cast();
                  }

                  return Boxed_Value(data);
                } else {
                  std::shared_ptr<To> data
                    = std::dynamic_pointer_cast<To>(detail::Cast_Helper<std::shared_ptr<From> >::cast(t_from, nullptr));

                  if (!data)
                  {
                    throw std::bad_cast();
                  }

                  return Boxed_Value(data);
                }
              } else {
                // Pull the reference out of the contained boxed value, which we know is the type we want
                if (t_from.is_const())
                {
                  const From &d = detail::Cast_Helper<const From &>::cast(t_from, nullptr);
                  const To &data = dynamic_cast<const To &>(d);
                  return Boxed_Value(std::cref(data));
                } else {
                  From &d = detail::Cast_Helper<From &>::cast(t_from, nullptr);
                  To &data = dynamic_cast<To &>(d);
                  return Boxed_Value(std::ref(data));
                }
              }
            } else {
              throw chaiscript::exception::bad_boxed_dynamic_cast(t_from.get_type_info(), typeid(To), "Unknown dynamic_cast_conversion");
            }
          }
      };

    template<typename Base, typename Derived>
      class Dynamic_Conversion_Impl : public Dynamic_Conversion
    {
      public:
        Dynamic_Conversion_Impl()
          : Dynamic_Conversion(user_type<Base>(), user_type<Derived>())
        {
        }

        virtual Boxed_Value convert_down(const Boxed_Value &t_base) const CHAISCRIPT_OVERRIDE
        {
          return Dynamic_Caster<Base, Derived>::cast(t_base);
        }

        virtual Boxed_Value convert(const Boxed_Value &t_derived) const CHAISCRIPT_OVERRIDE
        {
          return Dynamic_Caster<Derived, Base>::cast(t_derived);
        }
    };
  }

  class Dynamic_Cast_Conversions
  {
    public:
      Dynamic_Cast_Conversions()
      {
      }

      Dynamic_Cast_Conversions(const Dynamic_Cast_Conversions &t_other)
        : m_conversions(t_other.get_conversions())
      {
      }

      void add_conversion(const std::shared_ptr<detail::Dynamic_Conversion> &conversion)
      {
        chaiscript::detail::threading::unique_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);
        m_conversions.insert(conversion);
      }

      template<typename Base, typename Derived>
        bool dynamic_cast_converts() const
        {
          return dynamic_cast_converts(user_type<Base>(), user_type<Derived>());
        }

      bool dynamic_cast_converts(const Type_Info &base, const Type_Info &derived) const
      {
        return has_conversion(base, derived) || has_conversion(derived, base);
      }

      template<typename Base>
        Boxed_Value boxed_dynamic_cast(const Boxed_Value &derived) const
        {
          try {
            return get_conversion(user_type<Base>(), derived.get_type_info())->convert(derived);
          } catch (const std::out_of_range &) {
            throw exception::bad_boxed_dynamic_cast(derived.get_type_info(), typeid(Base), "No known conversion");
          } catch (const std::bad_cast &) {
            throw exception::bad_boxed_dynamic_cast(derived.get_type_info(), typeid(Base), "Unable to perform dynamic_cast operation");
          }
        }

      template<typename Derived>
        Boxed_Value boxed_dynamic_down_cast(const Boxed_Value &base) const
        {
          try {
            return get_conversion(base.get_type_info(), user_type<Derived>())->convert_down(base);
          } catch (const std::out_of_range &) {
            throw exception::bad_boxed_dynamic_cast(base.get_type_info(), typeid(Derived), "No known conversion");
          } catch (const std::bad_cast &) {
            throw exception::bad_boxed_dynamic_cast(base.get_type_info(), typeid(Derived), "Unable to perform dynamic_cast operation");
          }
        }


      bool has_conversion(const Type_Info &base, const Type_Info &derived) const
      {
        chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);
        return find(base, derived) != m_conversions.end();
      }

      std::shared_ptr<detail::Dynamic_Conversion> get_conversion(const Type_Info &base, const Type_Info &derived) const
      {
        chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);

        auto itr =
          find(base, derived);

        if (itr != m_conversions.end())
        {
          return *itr;
        } else {
          throw std::out_of_range("No such conversion exists from " + derived.bare_name() + " to " + base.bare_name());
        }
      }

    private:
      std::set<std::shared_ptr<detail::Dynamic_Conversion> >::const_iterator find(
          const Type_Info &base, const Type_Info &derived) const
      {
        for (auto itr = m_conversions.begin();
            itr != m_conversions.end();
            ++itr)
        {
          if ((*itr)->base().bare_equal(base) && (*itr)->derived().bare_equal(derived))
          {
            return itr;
          }
        }

        return m_conversions.end();
      }

      std::set<std::shared_ptr<detail::Dynamic_Conversion> > get_conversions() const
      {
        chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);

        return m_conversions;
      }

      mutable chaiscript::detail::threading::shared_mutex m_mutex;
      std::set<std::shared_ptr<detail::Dynamic_Conversion> > m_conversions;
  };

  typedef std::shared_ptr<chaiscript::detail::Dynamic_Conversion> Dynamic_Cast_Conversion;

  /// \brief Used to register a base / parent class relationship with ChaiScript. Necessary if you
  ///        want automatic conversions up your inheritance hierarchy.
  ///
  /// Create a new base class registration for applying to a module or to the chaiscript engine
  /// Currently, due to limitations in module loading on Windows, and for the sake of portability,
  /// if you have a type that is introduced in a loadable module and is used by multiple modules
  /// (through a tertiary dll that is shared between the modules, static linking the new type
  /// into both loadable modules would not be portable), you need to register the base type
  /// relationship in all modules that use the newly added type in a polymorphic way.
  ///
  /// Example:
  /// \code
  /// class Base
  /// {};
  /// class Derived : public Base
  /// {};
  ///
  /// chaiscript::ChaiScript chai;
  /// chai.add(chaiscript::base_class<Base, Derived>());
  /// \endcode
  /// 
  template<typename Base, typename Derived>
  Dynamic_Cast_Conversion base_class()
  {
    //Can only be used with related polymorphic types
    //may be expanded some day to support conversions other than child -> parent
    static_assert(std::is_base_of<Base,Derived>::value, "Classes are not related by inheritance");
    static_assert(std::is_polymorphic<Base>::value, "Base class must be polymorphic");
    static_assert(std::is_polymorphic<Derived>::value, "Derived class must be polymorphic");

    return std::shared_ptr<detail::Dynamic_Conversion>(new detail::Dynamic_Conversion_Impl<Base, Derived>());
  }

}


#endif
