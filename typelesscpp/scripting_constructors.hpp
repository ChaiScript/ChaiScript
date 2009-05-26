#ifndef __scripting_constructors_hpp__
#define __scripting_constrcutors_hpp__

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

//Constructors, to be made variadic
template<typename Class>
boost::shared_ptr<Class> constructor()
{
  return boost::shared_ptr<Class>(new Class());
}

template<typename Class, typename Param1>
boost::shared_ptr<Class> constructor(Param1 p1)
{
  return boost::shared_ptr<Class>(new Class(p1));
}
  
template<typename Class, typename Param1, typename Param2>
boost::shared_ptr<Class> constructor(Param1 p1, Param2 p2)
{
  return boost::shared_ptr<Class>(new Class(p1, p2));
}

template<typename Class>
boost::function<boost::shared_ptr<Class> ()> build_constructor()
{
  typedef boost::shared_ptr<Class> (*func)();
  return boost::function<boost::shared_ptr<Class> ()>(func(&(constructor<Class>)));
}

template<typename Class, typename Param1>
boost::function<boost::shared_ptr<Class> (Param1)> build_constructor()
{
  typedef boost::shared_ptr<Class> (*func)(Param1);
  return boost::function<boost::shared_ptr<Class> (Param1)>(func(&(constructor<Class, Param1>)));
}

template<typename Class, typename Param1, typename Param2>
boost::function<boost::shared_ptr<Class> (Param1, Param2)> build_constructor()
{
  typedef boost::shared_ptr<Class> (*func)(Param1, Param2);
  return boost::function<boost::shared_ptr<Class> (Param1, Param2)>(func(&(constructor<Class, Param1, Param2>)));
}


#endif

