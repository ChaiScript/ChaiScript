// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2014, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_THREADING_HPP_
#define CHAISCRIPT_THREADING_HPP_

#ifndef CHAISCRIPT_NO_THREADS

#ifdef __llvm__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++11-long-long"
#pragma clang diagnostic ignored "-Wshadow"
#endif
#include <boost/thread.hpp>
#ifdef __llvm__
#pragma clang diagnostic pop
#endif
#else
#pragma message ("ChaiScript is compiling without thread safety.")
#endif

/// \file
///
/// This file contains code necessary for thread support in ChaiScript.
/// If the compiler definition CHAISCRIPT_NO_THREADS is defined then thread safety
/// is disabled in ChaiScript. This has the result that some code is faster, because mutex locks are not required.
/// It also has the side effect that the chaiscript::ChaiScript object may not be accessed from more than
/// one thread simultaneously.

namespace chaiscript
{
  namespace detail
  {
    /// If threading is enabled, then this namespace contains boost::thread classes.
    /// If threading is not enabled, then stubbed in wrappers that do nothing are provided.
    /// This allows us to avoid \#ifdef code in the sections that need thread safety.
    namespace threading
    {

#ifndef CHAISCRIPT_NO_THREADS
      using boost::unique_lock; 
      using boost::shared_lock;
      using boost::lock_guard;
      using boost::shared_mutex;
      using boost::recursive_mutex;


      /// Typesafe thread specific storage. If threading is enabled, this class uses boost::thread_specific_ptr<T>. If
      /// threading is not enabled, the class always returns the same data, regardless of which thread it is called from.
      template<typename T>
        class Thread_Storage
        {
          public:
            ~Thread_Storage()
            {
              m_thread_storage.reset();
            }

            inline T *operator->() const
            {
              if (!m_thread_storage.get())
              {
                m_thread_storage.reset(new T());
              }

              return m_thread_storage.get();
            }

            inline T &operator*() const
            {
              return *(this->operator->());
            }

          private:
            mutable boost::thread_specific_ptr<T> m_thread_storage;
        };

#else
      template<typename T>
      class unique_lock 
      {
        public:
          unique_lock(T &) {}
          void lock() {}
          void unlock() {}
      };

      template<typename T>
      class shared_lock 
      {
        public:
          shared_lock(T &) {}
          void lock() {}
          void unlock() {}
      };

      template<typename T>
      class lock_guard 
      {
        public:
          lock_guard(T &) {}
      };

      class shared_mutex { };

      class recursive_mutex {};


      template<typename T>
        class Thread_Storage
        {
          public:
            inline T *operator->() const
            {
              return &obj;
            }

            inline T &operator*() const
            {
              return obj;
            }

          private:
            mutable T obj;
        };

#endif
    }
  }
}



#endif

