// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// and Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_ANY_HPP_
#define CHAISCRIPT_ANY_HPP_

#include <utility>

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
        struct Data
        {

          Data &operator=(const Data &) = delete;

          virtual ~Data() {}

          virtual void *data() = 0;
          virtual std::unique_ptr<Data> clone() const = 0;
        };

        template<typename T>
          struct Data_Impl : Data
          {
            explicit Data_Impl(T t_type)
              : m_data(std::move(t_type))
            {
            }

            virtual ~Data_Impl() {}

            virtual void *data() CHAISCRIPT_OVERRIDE
            {
              return &m_data;
            }

            std::unique_ptr<Data> clone() const CHAISCRIPT_OVERRIDE
            {
              return std::unique_ptr<Data>(new Data_Impl<T>(m_data));
            }

            Data_Impl &operator=(const Data_Impl&) = delete;

            T m_data;
          };

        std::unique_ptr<Data> m_data;
        mutable std::array<uint8_t, 15> m_smallSize;
        bool m_isSmall = false;
        const std::type_info *m_type = &typeid(void);

      public:
        // construct/copy/destruct
        Any() = default;

        Any(const Any &t_any) 
          : m_data(t_any.m_data?t_any.m_data->clone():nullptr),
            m_smallSize(t_any.m_smallSize),
            m_isSmall(t_any.m_isSmall),
            m_type(t_any.m_type)
        {

        }

#if _MSC_VER  != 1800
        Any(Any &&) = default;
        Any &operator=(Any &&t_any) = default;
#endif

        template<typename ValueType,
          typename = typename std::enable_if<!std::is_same<Any, typename std::decay<ValueType>::type>::value>::type,
          typename = typename std::enable_if< std::is_trivial<typename std::decay<ValueType>::type>::value>::type,
          typename = typename std::enable_if<sizeof(typename std::decay<ValueType>::type) <= sizeof(decltype(m_smallSize)) >::type>
        explicit Any(ValueType &&t_value)
          : m_isSmall(true), m_type(&typeid(typename std::decay<ValueType>::type))
        {
          m_smallSize.fill(0);
          *(static_cast<typename std::decay<ValueType>::type *>(static_cast<void *>(m_smallSize.data()))) = t_value;
          // std::cout << "Setting type: " << typeid(typename std::decay<ValueType>::type).name() << " "  << t_value << " actual val: " << *(static_cast<typename std::decay<ValueType>::type *>(static_cast<void *>(m_smallSize.data()))) << " cast: " << cast<typename std::decay<ValueType>::type>() << "\n";
        }

        template<typename ValueType,
          typename = typename std::enable_if<!std::is_same<Any, typename std::decay<ValueType>::type>::value>::type,
          typename = typename std::enable_if< 
            !std::is_trivial<typename std::decay<ValueType>::type>::value
            || !(sizeof(typename std::decay<ValueType>::type) <= sizeof(decltype(m_smallSize))) >::type>
        explicit Any(ValueType &&t_value)
          : m_data(std::unique_ptr<Data>(new Data_Impl<typename std::decay<ValueType>::type>(std::forward<ValueType>(t_value)))),
            m_isSmall(false),
            m_type(&typeid(typename std::decay<ValueType>::type))
        {
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
            if (m_isSmall && typeid(ToType) == *m_type)
            {
              return *static_cast<ToType *>(static_cast<void *>(m_smallSize.data()));
            } else if (!m_isSmall && m_data && typeid(ToType) == *m_type) {
              return *static_cast<ToType *>(m_data->data());
            } else {
              throw chaiscript::detail::exception::bad_any_cast();
            }
          }

        const std::type_info &type() const
        {
          return *m_type;
        }

        ~Any()
        {
        }

        // modifiers
        Any & swap(Any &t_other)
        {
          std::swap(t_other.m_smallSize, m_smallSize);
          std::swap(t_other.m_isSmall, m_isSmall);
          std::swap(t_other.m_data, m_data);
          std::swap(t_other.m_type, m_type);
          return *this;
        }

        // queries
        bool empty() const
        {
          return !bool(m_data) && !m_isSmall;
        }

        void *data() const
        {
          if (m_isSmall)
          {
            return static_cast<void *>(m_smallSize.data());
          } else if (m_data) {
            return m_data->data();
          } else {
            return nullptr;
          }
        }

    };

  }
}

#endif


