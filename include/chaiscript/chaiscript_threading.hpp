#ifndef __chaiscript_threading_hpp__
#define __chaiscript_threading_hpp__

#ifndef CHAISCRIPT_NO_THREADS
#include <boost/thread.hpp>
#else
#pragma message ("ChaiScript is compiling without thread safety.")
#endif

namespace chaiscript
{
  namespace threading
  {

#ifndef CHAISCRIPT_NO_THREADS

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



#endif

