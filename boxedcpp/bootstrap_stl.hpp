#ifndef __bootstrap_stl_hpp
#define __bootstrap_stl_hpp__

#include "boxedcpp.hpp"

template<typename ContainerType>
void bootstrap_reversible_container(BoxedCPP_System &system)
{
}

template<typename ContainerType>
void bootstrap_random_access_container(BoxedCPP_System &system)
{
  bootstrap_reversible_container<ContainerType>(system);

  typedef typename ContainerType::reference(ContainerType::*indexoper)(size_t);

  system.register_function(
      boost::function<typename ContainerType::reference (ContainerType *, int)>(indexoper(&ContainerType::operator[])), "[]");
}

template<typename ContainerType>
void bootstrap_container(BoxedCPP_System &system)
{
}

template<typename ContainerType>
void bootstrap_forward_container(BoxedCPP_System &system)
{
  bootstrap_container<ContainerType>(system);
}

template<typename Type>
void bootstrap_default_constructible(BoxedCPP_System &system)
{
}

template<typename SequenceType>
void bootstrap_sequence(BoxedCPP_System &system)
{
  bootstrap_forward_container<SequenceType>(system);
  bootstrap_default_constructible<SequenceType>(system);
}

template<typename SequenceType>
void bootstrap_back_insertion_sequence(BoxedCPP_System &system)
{
  bootstrap_sequence<SequenceType>(system);


  typedef typename SequenceType::reference (SequenceType::*backptr)();

  system.register_function(boost::function<typename SequenceType::reference (SequenceType *)>(backptr(&SequenceType::back)), "back");
  system.register_function(boost::function<void (SequenceType *,typename SequenceType::value_type)>(&SequenceType::push_back), "push_back");
  system.register_function(boost::function<void (SequenceType *)>(&SequenceType::pop_back), "pop_back");
}

template<typename VectorType>
void bootstrap_vector(BoxedCPP_System &system)
{
  bootstrap_random_access_container<VectorType>(system);
  bootstrap_back_insertion_sequence<VectorType>(system);
}

#endif
