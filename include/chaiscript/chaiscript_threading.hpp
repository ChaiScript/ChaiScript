// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2014, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_THREADING_HPP_
#define CHAISCRIPT_THREADING_HPP_

#include <unordered_map>

#ifndef CHAISCRIPT_NO_THREADS
#include <thread>
#include <mutex>
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
    /// If threading is enabled, then this namespace contains std thread classes.
    /// If threading is not enabled, then stubbed in wrappers that do nothing are provided.
    /// This allows us to avoid \#ifdef code in the sections that need thread safety.
    namespace threading
    {

#ifndef CHAISCRIPT_NO_THREADS

      template<typename T>
      class unique_lock : public std::unique_lock<T>
      {
        public:
          unique_lock(T &t) : std::unique_lock<T>(t) {}
      };

      template<typename T>
      class shared_lock : public std::unique_lock<T>
      {
        public:
          shared_lock(T &t) : std::unique_lock<T>(t) {}
          void unlock() {}
      };

      template<typename T>
      class lock_guard : public std::lock_guard<T>
      {
        public:
          lock_guard(T &t) : std::lock_guard<T>(t) {}
      };

      class shared_mutex : public std::mutex { };

      using std::mutex;

      using std::recursive_mutex;



      /// Typesafe thread specific storage. If threading is enabled, this class uses a mutex protected map. If
      /// threading is not enabled, the class always returns the same data, regardless of which thread it is called from.
      /// 
      /// \todo move to thread_local when it exists 
      template<typename T>
        class Thread_Storage
        {
          public:
            inline T *operator->() const
            {
              return get_tls().get();
            }

            inline T &operator*() const
            {
              return *get_tls();
            }


          private:
            std::shared_ptr<T> get_tls() const
            {
              
              unique_lock<mutex> lock(m_mutex);
             
              auto itr = m_instances.find(std::this_thread::get_id());

              if (itr != m_instances.end()) { return itr->second; }
             
              std::shared_ptr<T> new_instance(new T());

              m_instances.insert(std::make_pair(std::this_thread::get_id(), new_instance));

              return new_instance;
              

              /*
              static __thread std::shared_ptr<T> *m_data = 0;
              
              if (!m_data) { m_data = new std::shared_ptr<T>(new T()); }

              return *m_data;
              */
            }


            mutable mutex m_mutex;
            mutable std::unordered_map<std::thread::id, std::shared_ptr<T> > m_instances;
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

