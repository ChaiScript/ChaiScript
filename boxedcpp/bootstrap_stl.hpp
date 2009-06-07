#ifndef __bootstrap_stl_hpp
#define __bootstrap_stl_hpp__

#include "boxedcpp.hpp"

template<typename ContainerType>
void bootstrap_reversible_container(BoxedCPP_System &system, const std::string &type)
{
}

template<typename ContainerType>
void bootstrap_random_access_container(BoxedCPP_System &system, const std::string &type)
{
  bootstrap_reversible_container<ContainerType>(system, type);

  typedef typename ContainerType::reference(ContainerType::*indexoper)(size_t);

  system.register_function(
      boost::function<typename ContainerType::reference (ContainerType *, int)>(indexoper(&ContainerType::operator[])), "[]");
  system.register_function(
      boost::function<typename ContainerType::reference (ContainerType *, int)>(indexoper(&ContainerType::at)), "at");
}

template<typename Assignable>
void bootstrap_assignable(BoxedCPP_System &system, const std::string &type)
{
  /*
  system.register_function(
      boost::function<Assignable &(Assignable*, Assignable&)>(&Assignable::operator=), "=");
      */
}

template<typename ContainerType>
void bootstrap_container(BoxedCPP_System &system, const std::string &type)
{
  bootstrap_assignable<ContainerType>(system, type);

  system.register_function(
      boost::function<size_t (ContainerType *)>(&ContainerType::size), "size");
  system.register_function(
      boost::function<size_t (ContainerType *)>(&ContainerType::size), "maxsize");
}

template<typename ContainerType>
void bootstrap_forward_container(BoxedCPP_System &system, const std::string &type)
{
  bootstrap_container<ContainerType>(system, type);
}

template<typename Type>
void bootstrap_default_constructible(BoxedCPP_System &system, const std::string &type)
{
  system.register_function(build_constructor<Type>(), type);
}

template<typename SequenceType>
void bootstrap_sequence(BoxedCPP_System &system, const std::string &type)
{
  bootstrap_forward_container<SequenceType>(system, type);
  bootstrap_default_constructible<SequenceType>(system, type);
}

template<typename SequenceType>
void bootstrap_back_insertion_sequence(BoxedCPP_System &system, const std::string &type)
{
  bootstrap_sequence<SequenceType>(system, type);


  typedef typename SequenceType::reference (SequenceType::*backptr)();

  system.register_function(boost::function<typename SequenceType::reference (SequenceType *)>(backptr(&SequenceType::back)), "back");
  system.register_function(boost::function<void (SequenceType *,typename SequenceType::value_type)>(&SequenceType::push_back), "push_back");
  system.register_function(boost::function<void (SequenceType *)>(&SequenceType::pop_back), "pop_back");
}

template<typename VectorType>
void bootstrap_vector(BoxedCPP_System &system, const std::string &type)
{
  system.register_type<VectorType>(type);
  bootstrap_random_access_container<VectorType>(system, type);
  bootstrap_back_insertion_sequence<VectorType>(system, type);
}

#endif
