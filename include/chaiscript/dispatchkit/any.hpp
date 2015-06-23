// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// and Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_ANY_HPP_
#define CHAISCRIPT_ANY_HPP_

#include <utility>
#include <array>

namespace chaiscript {
  namespace detail {
    namespace exception
    {
      /// \brief Thrown in the event that an Any cannot be cast to the desired type
      ///
      /// It is used internally during function dispatch.
      ///
      /// \sa chaiscript::detail::Any
      class bad_any_cast : public std::bad_cast
      {
        public:
          bad_any_cast() CHAISCRIPT_NOEXCEPT
            : m_what("bad any cast")
          {
          }

          bad_any_cast(const bad_any_cast &) = default;

          virtual ~bad_any_cast() CHAISCRIPT_NOEXCEPT {}

          /// \brief Description of what error occurred
          virtual const char * what() const CHAISCRIPT_NOEXCEPT CHAISCRIPT_OVERRIDE
          {
            return m_what.c_str();
          }

        private:
          std::string m_what;
      };
    }
  

    class Any {
      private:


        template<class T>
          struct destruct {
            void operator()(T *t) const {
              t->~T();
            }
          };

        struct Data
        {
          Data(const std::type_info &t_type) 
            : m_type(t_type)
          {
          }

          Data &operator=(const Data &) = delete;

          virtual ~Data() {}

          virtual void *data() = 0;
          const std::type_info &type() const
          {
            return m_type;
          }

          virtual void clone(void *t_ptr) const = 0;
          const std::type_info &m_type;
        };

        template<typename T>
          struct Data_Impl : Data
          {
            explicit Data_Impl(T t_type)
              : Data(typeid(T)),
                m_data(std::move(t_type))
            {
            }

            virtual ~Data_Impl() {}

            virtual void *data() CHAISCRIPT_OVERRIDE
            {
              return &m_data;
            }

            void clone(void *t_ptr) const CHAISCRIPT_OVERRIDE
            {
              new (t_ptr) Data_Impl<T>(m_data);
            }

            Data_Impl &operator=(const Data_Impl&) = delete;

            T m_data;
          };

        static const size_t buffersize = sizeof(Data_Impl<std::shared_ptr<int>>)>sizeof(Data_Impl<std::reference_wrapper<int>>)?sizeof(Data_Impl<std::shared_ptr<int>>):sizeof(Data_Impl<std::reference_wrapper<int>>);
        bool m_constructed;
        mutable std::array<uint8_t, buffersize> m_data_holder;

        inline Data *data() const
        {
          return reinterpret_cast<Data*>(m_data_holder.data());
        }


        void call_destructor()
        {
          if (m_constructed)
          {
            m_constructed = false;
            data()->~Data();
          }
        }

      public:
        Any() 
          : m_constructed(false)
        {
        }

        // construct/copy/destruct
        Any(const Any &t_any)
          : m_constructed(false)
        {
          if (t_any.m_constructed) {
            t_any.data()->clone(m_data_holder.data());
            m_constructed = true;
          }
        }

        Any(Any &&t_any)
          : m_constructed(false)
        {
          if (t_any.m_constructed) {
            t_any.m_constructed = false;
            m_constructed = true;
            m_data_holder = std::move(t_any.m_data_holder);
          }
        }

        Any &operator=(Any &&t_any)
        {
          call_destructor();
          if (t_any.m_constructed) {
            t_any.m_constructed = false;
            m_constructed = true;
            m_data_holder = std::move(t_any.m_data_holder);
          }
          return *this;
        }


        template<typename ValueType,
          typename = typename std::enable_if<!std::is_same<Any, typename std::decay<ValueType>::type>::value>::type>
        explicit Any(ValueType &&t_value)
          : m_constructed(true)
        {
          static_assert(sizeof(Data_Impl<typename std::decay<ValueType>::type>) <= buffersize, "Buffer too small");
          (void)(new (m_data_holder.data()) Data_Impl<typename std::decay<ValueType>::type>(std::forward<ValueType>(t_value)));
        }


        Any & operator=(const Any &t_any)
        {
          Any copy(t_any);
          swap(copy);
          return *this; 
        }

        template<typename ToType>
          ToType &cast() const
          {
            if (m_constructed && typeid(ToType) == data()->type())
            {
              return *static_cast<ToType *>(data()->data());
            } else {
              throw chaiscript::detail::exception::bad_any_cast();
            }
          }


        ~Any()
        {
          call_destructor();
        }

        // modifiers
        Any & swap(Any &t_other)
        {
          std::swap(t_other.m_data_holder, m_data_holder);
          std::swap(t_other.m_constructed, m_constructed);
          return *this;
        }


        const std::type_info & type() const
        {
          if (m_constructed) {
            return data()->type();
          } else {
            return typeid(void);
          }
        }
    };

  }
}

#endif


