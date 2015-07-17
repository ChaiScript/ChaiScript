// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2015, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_DYNAMIC_CAST_CONVERSION_HPP_
#define CHAISCRIPT_DYNAMIC_CAST_CONVERSION_HPP_

#include <atomic>
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

        bad_boxed_dynamic_cast(const bad_boxed_dynamic_cast &) = default;

        virtual ~bad_boxed_dynamic_cast() CHAISCRIPT_NOEXCEPT {}
    };

    class bad_boxed_type_cast : public bad_boxed_cast
    {
      public:
        bad_boxed_type_cast(const Type_Info &t_from, const std::type_info &t_to,
            const std::string &t_what) CHAISCRIPT_NOEXCEPT
          : bad_boxed_cast(t_from, t_to, t_what)
        {
        }

        bad_boxed_type_cast(const Type_Info &t_from, const std::type_info &t_to) CHAISCRIPT_NOEXCEPT
          : bad_boxed_cast(t_from, t_to)
        {
        }

        bad_boxed_type_cast(const std::string &w) CHAISCRIPT_NOEXCEPT
          : bad_boxed_cast(w)
        {
        }

        bad_boxed_type_cast(const bad_boxed_type_cast &) = default;

        virtual ~bad_boxed_type_cast() CHAISCRIPT_NOEXCEPT {}
    };
  }


  namespace detail
  {
    class Type_Conversion_Base
    {
      public:
        virtual Boxed_Value convert(const Boxed_Value &from) const = 0;
        virtual Boxed_Value convert_down(const Boxed_Value &to) const = 0;

        const Type_Info &to() const
        {
          return m_to;
        }
        const Type_Info &from() const
        {
          return m_from;
        }

        virtual bool bidir() const
        {
          return true;
        }

        virtual ~Type_Conversion_Base() {}

      protected:
        Type_Conversion_Base(const Type_Info &t_to, const Type_Info &t_from)
          : m_to(t_to), m_from(t_from)
        {
        }


      private:
        Type_Info m_to;
        Type_Info m_from;

    };

    template<typename From, typename To> 
      class Static_Caster
      {
        public: 
          static Boxed_Value cast(const Boxed_Value &t_from)
          {
            if (t_from.get_type_info().bare_equal(chaiscript::user_type<From>()))
            {
              if (t_from.is_pointer())
              {
                // Dynamic cast out the contained boxed value, which we know is the type we want
                if (t_from.is_const())
                {
                  return Boxed_Value(
                      [&]()->std::shared_ptr<const To>{
                        if (auto data = std::static_pointer_cast<const To>(detail::Cast_Helper<std::shared_ptr<const From> >::cast(t_from, nullptr)))
                        {
                          return data;
                        } else {
                          throw std::bad_cast();
                        }
                      }()
                      );
                } else {
                  return Boxed_Value(
                      [&]()->std::shared_ptr<To>{
                        if (auto data = std::static_pointer_cast<To>(detail::Cast_Helper<std::shared_ptr<From> >::cast(t_from, nullptr)))
                        {
                          return data;
                        } else {
                          throw std::bad_cast();
                        }
                      }()
                      );
                }
              } else {
                // Pull the reference out of the contained boxed value, which we know is the type we want
                if (t_from.is_const())
                {
                  const From &d = detail::Cast_Helper<const From &>::cast(t_from, nullptr);
                  const To &data = static_cast<const To &>(d);
                  return Boxed_Value(std::cref(data));
                } else {
                  From &d = detail::Cast_Helper<From &>::cast(t_from, nullptr);
                  To &data = static_cast<To &>(d);
                  return Boxed_Value(std::ref(data));
                }
              }
            } else {
              throw chaiscript::exception::bad_boxed_dynamic_cast(t_from.get_type_info(), typeid(To), "Unknown dynamic_cast_conversion");
            }
          }
 
      };


    template<typename From, typename To> 
      class Dynamic_Caster
      {
        public: 
          static Boxed_Value cast(const Boxed_Value &t_from)
          {
            if (t_from.get_type_info().bare_equal(chaiscript::user_type<From>()))
            {
              if (t_from.is_pointer())
              {
                // Dynamic cast out the contained boxed value, which we know is the type we want
                if (t_from.is_const())
                {
                  return Boxed_Value(
                      [&]()->std::shared_ptr<const To>{
                        if (auto data = std::dynamic_pointer_cast<const To>(detail::Cast_Helper<std::shared_ptr<const From> >::cast(t_from, nullptr)))
                        {
                          return data;
                        } else {
                          throw std::bad_cast();
                        }
                      }()
                      );
                } else {
                  return Boxed_Value(
                      [&]()->std::shared_ptr<To>{
                        if (auto data = std::dynamic_pointer_cast<To>(detail::Cast_Helper<std::shared_ptr<From> >::cast(t_from, nullptr)))
                        {
                          return data;
                        } else {
#ifdef CHAISCRIPT_LIBCPP
                          /// \todo fix this someday after libc++ is fixed.
                          if (std::string(typeid(To).name()).find("Assignable_Proxy_Function") != std::string::npos) {
                            auto from = detail::Cast_Helper<std::shared_ptr<From> >::cast(t_from, nullptr);
                            if (std::string(typeid(*from).name()).find("Assignable_Proxy_Function_Impl") != std::string::npos) {
                              return std::static_pointer_cast<To>(from);
                            }
                          }
#endif
                          throw std::bad_cast();
                        }
                      }()
                      );
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
      class Dynamic_Conversion_Impl : public Type_Conversion_Base
    {
      public:
        Dynamic_Conversion_Impl()
          : Type_Conversion_Base(chaiscript::user_type<Base>(), chaiscript::user_type<Derived>())
        {
        }

        virtual Boxed_Value convert_down(const Boxed_Value &t_base) const CHAISCRIPT_OVERRIDE
        {
          return Dynamic_Caster<Base, Derived>::cast(t_base);
        }

        virtual Boxed_Value convert(const Boxed_Value &t_derived) const CHAISCRIPT_OVERRIDE
        {
          return Static_Caster<Derived, Base>::cast(t_derived);
        }
    };

    template<typename Base, typename Derived>
      class Static_Conversion_Impl : public Type_Conversion_Base
    {
      public:
        Static_Conversion_Impl()
          : Type_Conversion_Base(chaiscript::user_type<Base>(), chaiscript::user_type<Derived>())
        {
        }

        virtual Boxed_Value convert_down(const Boxed_Value &t_base) const CHAISCRIPT_OVERRIDE
        {
          throw chaiscript::exception::bad_boxed_dynamic_cast(t_base.get_type_info(), typeid(Derived), "Unable to cast down inheritance hierarchy with non-polymorphic types");
        }

        virtual bool bidir() const CHAISCRIPT_OVERRIDE
        {
          return false;
        }

        virtual Boxed_Value convert(const Boxed_Value &t_derived) const CHAISCRIPT_OVERRIDE
        {
          return Static_Caster<Derived, Base>::cast(t_derived);
        }
    };



    template<typename Callable>
    class Type_Conversion_Impl : public Type_Conversion_Base
    {
      public:
        Type_Conversion_Impl(Type_Info t_from, Type_Info t_to, Callable t_func)
          : Type_Conversion_Base(std::move(t_to), std::move(t_from)),
            m_func(std::move(t_func))
        {
        }

        virtual Boxed_Value convert_down(const Boxed_Value &) const CHAISCRIPT_OVERRIDE
        {
          throw chaiscript::exception::bad_boxed_type_cast("No conversion exists");
        }

        virtual Boxed_Value convert(const Boxed_Value &t_from) const CHAISCRIPT_OVERRIDE
        {
          /// \todo better handling of errors from the conversion function
          return m_func(t_from);
        }

        virtual bool bidir() const CHAISCRIPT_OVERRIDE
        {
          return false;
        }


      private:
        Callable m_func;
    };

  }

  class Type_Conversions
  {
    public:
      struct Less_Than
      {
        bool operator()(const std::type_info *t_lhs, const std::type_info *t_rhs) const
        {
          return *t_lhs != *t_rhs && t_lhs->before(*t_rhs);
        }
      };

      Type_Conversions()
        : m_mutex(),
          m_conversions(),
          m_convertableTypes(),
          m_num_types(0),
          m_thread_cache(this),
          m_conversion_saves(this)
      {
      }

      Type_Conversions(const Type_Conversions &t_other)
        : m_mutex(),
          m_conversions(t_other.get_conversions()),
          m_convertableTypes(t_other.m_convertableTypes),
          m_num_types(m_conversions.size()),
          m_thread_cache(this),
          m_conversion_saves(this)

      {
      }

      const std::set<const std::type_info *, Less_Than> &thread_cache() const
      {
        auto &cache = *m_thread_cache;
        if (cache.size() != m_num_types)
        {
          chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);
          cache = m_convertableTypes;
        }

        return cache;
      }

      void add_conversion(const std::shared_ptr<detail::Type_Conversion_Base> &conversion)
      {
        chaiscript::detail::threading::unique_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);
        /// \todo error if a conversion already exists
        m_conversions.insert(conversion);
        m_convertableTypes.insert({conversion->to().bare_type_info(), conversion->from().bare_type_info()});
        m_num_types = m_convertableTypes.size();
      }

      template<typename T>
        bool convertable_type() const
        {
          return thread_cache().count(user_type<T>().bare_type_info()) != 0;
        }

      template<typename To, typename From>
        bool converts() const
        {
          return converts(user_type<To>(), user_type<From>());
        }

      bool converts(const Type_Info &to, const Type_Info &from) const
      {
        const auto &types = thread_cache();
        if (types.count(to.bare_type_info()) != 0 && types.count(from.bare_type_info()) != 0)
        {
          return has_conversion(to, from);
        } else {
          return false;
        }

      }

      template<typename To>
        Boxed_Value boxed_type_conversion(const Boxed_Value &from) const
        {
          try {
            Boxed_Value ret = get_conversion(user_type<To>(), from.get_type_info())->convert(from);
            if (m_conversion_saves->enabled) m_conversion_saves->saves.push_back(ret);
            return ret;
          } catch (const std::out_of_range &) {
            throw exception::bad_boxed_dynamic_cast(from.get_type_info(), typeid(To), "No known conversion");
          } catch (const std::bad_cast &) {
            throw exception::bad_boxed_dynamic_cast(from.get_type_info(), typeid(To), "Unable to perform dynamic_cast operation");
          }
        }

      template<typename From>
        Boxed_Value boxed_type_down_conversion(const Boxed_Value &to) const
        {
          try {
            Boxed_Value ret = get_conversion(to.get_type_info(), user_type<From>())->convert_down(to);
            if (m_conversion_saves->enabled) m_conversion_saves->saves.push_back(ret);
            return ret;
          } catch (const std::out_of_range &) {
            throw exception::bad_boxed_dynamic_cast(to.get_type_info(), typeid(From), "No known conversion");
          } catch (const std::bad_cast &) {
            throw exception::bad_boxed_dynamic_cast(to.get_type_info(), typeid(From), "Unable to perform dynamic_cast operation");
          }
        }

      void enable_conversion_saves(bool t_val) 
      {
        m_conversion_saves->enabled = t_val;
      }

      std::vector<Boxed_Value> take_saves()
      {
        std::vector<Boxed_Value> ret;
        std::swap(ret, m_conversion_saves->saves);
        return ret;
      }

      bool has_conversion(const Type_Info &to, const Type_Info &from) const
      {
        chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);
        return find_bidir(to, from) != m_conversions.end();
      }

      std::shared_ptr<detail::Type_Conversion_Base> get_conversion(const Type_Info &to, const Type_Info &from) const
      {
        chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);

        auto itr = find(to, from);

        if (itr != m_conversions.end())
        {
          return *itr;
        } else {
          throw std::out_of_range("No such conversion exists from " + from.bare_name() + " to " + to.bare_name());
        }
      }

    private:
      std::set<std::shared_ptr<detail::Type_Conversion_Base> >::const_iterator find_bidir(
          const Type_Info &to, const Type_Info &from) const
      {
        return std::find_if(m_conversions.begin(), m_conversions.end(),
              [&to, &from](const std::shared_ptr<detail::Type_Conversion_Base> &conversion) -> bool
              {
                return  (conversion->to().bare_equal(to) && conversion->from().bare_equal(from))
                     || (conversion->bidir() && conversion->from().bare_equal(to) && conversion->to().bare_equal(from));
;
              }
        );
      }

      std::set<std::shared_ptr<detail::Type_Conversion_Base> >::const_iterator find(
          const Type_Info &to, const Type_Info &from) const
      {
        return std::find_if(m_conversions.begin(), m_conversions.end(),
              [&to, &from](const std::shared_ptr<detail::Type_Conversion_Base> &conversion)
              {
                return conversion->to().bare_equal(to) && conversion->from().bare_equal(from);
              }
        );
      }

      std::set<std::shared_ptr<detail::Type_Conversion_Base>> get_conversions() const
      {
        chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);

        return m_conversions;
      }


      struct Conversion_Saves
      {
        Conversion_Saves()
          : enabled(false)
        {}

        bool enabled;
        std::vector<Boxed_Value> saves;
      };

      mutable chaiscript::detail::threading::shared_mutex m_mutex;
      std::set<std::shared_ptr<detail::Type_Conversion_Base>> m_conversions;
      std::set<const std::type_info *, Less_Than> m_convertableTypes;
      std::atomic_size_t m_num_types;
      mutable chaiscript::detail::threading::Thread_Storage<std::set<const std::type_info *, Less_Than>> m_thread_cache;
      mutable chaiscript::detail::threading::Thread_Storage<Conversion_Saves> m_conversion_saves;
  };

  typedef std::shared_ptr<chaiscript::detail::Type_Conversion_Base> Type_Conversion;

  /// \brief Used to register a to / parent class relationship with ChaiScript. Necessary if you
  ///        want automatic conversions up your inheritance hierarchy.
  ///
  /// Create a new to class registration for applying to a module or to the ChaiScript engine
  /// Currently, due to limitations in module loading on Windows, and for the sake of portability,
  /// if you have a type that is introduced in a loadable module and is used by multiple modules
  /// (through a tertiary dll that is shared between the modules, static linking the new type
  /// into both loadable modules would not be portable), you need to register the type
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
  /// chai.add(chaiscript::to_class<Base, Derived>());
  /// \endcode
  /// 
  template<typename Base, typename Derived>
  Type_Conversion base_class(typename std::enable_if<std::is_polymorphic<Base>::value && std::is_polymorphic<Derived>::value>::type* = nullptr)
  {
    //Can only be used with related polymorphic types
    //may be expanded some day to support conversions other than child -> parent
    static_assert(std::is_base_of<Base,Derived>::value, "Classes are not related by inheritance");

    return chaiscript::make_shared<detail::Type_Conversion_Base, detail::Dynamic_Conversion_Impl<Base, Derived>>();
  }

  template<typename Base, typename Derived>
  Type_Conversion base_class(typename std::enable_if<!std::is_polymorphic<Base>::value || !std::is_polymorphic<Derived>::value>::type* = nullptr)
  {
    //Can only be used with related polymorphic types
    //may be expanded some day to support conversions other than child -> parent
    static_assert(std::is_base_of<Base,Derived>::value, "Classes are not related by inheritance");

    return chaiscript::make_shared<detail::Type_Conversion_Base, detail::Static_Conversion_Impl<Base, Derived>>();
  }


  template<typename Callable>
    Type_Conversion type_conversion(const Type_Info &t_from, const Type_Info &t_to, 
        const Callable &t_func)
    {
      return chaiscript::make_shared<detail::Type_Conversion_Base, detail::Type_Conversion_Impl<Callable>>(t_from, t_to, t_func);
    }

  template<typename From, typename To, typename Callable>
    Type_Conversion type_conversion(const Callable &t_function)
    {
      auto func = [t_function](const Boxed_Value &t_bv) -> Boxed_Value {
            // not even attempting to call boxed_cast so that we don't get caught in some call recursion
            return chaiscript::Boxed_Value(t_function(detail::Cast_Helper<const From &>::cast(t_bv, nullptr)));
          };

      return chaiscript::make_shared<detail::Type_Conversion_Base, detail::Type_Conversion_Impl<decltype(func)>>(user_type<From>(), user_type<To>(), func);
    }

  template<typename From, typename To>
    Type_Conversion type_conversion()
    {
      static_assert(std::is_convertible<From, To>::value, "Types are not automatically convertible");
      auto func = [](const Boxed_Value &t_bv) -> Boxed_Value {
            // not even attempting to call boxed_cast so that we don't get caught in some call recursion
            return chaiscript::Boxed_Value(To(detail::Cast_Helper<From>::cast(t_bv, nullptr)));
          };

      return chaiscript::make_shared<detail::Type_Conversion_Base, detail::Type_Conversion_Impl<decltype(func)>>(user_type<From>(), user_type<To>(), func);
    }

  template<typename To>
    Type_Conversion vector_conversion()
    {
      auto func = [](const Boxed_Value &t_bv) -> Boxed_Value {
        const std::vector<Boxed_Value> &from_vec = detail::Cast_Helper<const std::vector<Boxed_Value> &>::cast(t_bv, nullptr);

        To vec;

        for (const Boxed_Value &bv : from_vec) {
          vec.push_back(detail::Cast_Helper<typename To::value_type>::cast(bv, nullptr));
        }

        return Boxed_Value(std::move(vec));
      };

      return chaiscript::make_shared<detail::Type_Conversion_Base, detail::Type_Conversion_Impl<decltype(func)>>(user_type<std::vector<Boxed_Value>>(), user_type<To>(), func);
    }

}


#endif
