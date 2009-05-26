#ifndef __scripting_type_info_hpp__
#define __scripting_type_info_hpp__

#include <boost/type_traits.hpp>

struct Type_Info
{
  Type_Info(bool t_is_const, bool t_is_reference, bool t_is_pointer, bool t_is_void, 
      const std::type_info *t_ti, const std::type_info *t_bareti)
    : m_is_const(t_is_const), m_is_reference(t_is_reference), m_is_pointer(t_is_pointer),
      m_type_info(t_ti), m_bare_type_info(t_bareti)
  {
  }

  bool m_is_const;
  bool m_is_reference;
  bool m_is_pointer;
  bool m_is_void;
  const std::type_info *m_type_info;
  const std::type_info *m_bare_type_info;
};

template<typename T>
struct Get_Type_Info
{
  Type_Info operator()()
  {
    return Type_Info(boost::is_const<T>::value, boost::is_reference<T>::value, boost::is_pointer<T>::value, 
        boost::is_void<T>::value,
        &typeid(T), 
        &typeid(typename boost::remove_const<typename boost::remove_pointer<typename boost::remove_reference<T>::type>::type>::type));

  }
};

#endif
