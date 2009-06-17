#ifndef __bootstrap_stl_hpp
#define __bootstrap_stl_hpp__

#include "dispatchkit.hpp"
#include "register_function.hpp"


namespace dispatchkit
{
  template<typename Container>
    struct Input_Range
    {
      Input_Range(Container &c)
        : m_begin(c.begin()), m_end(c.end())
      {
      }

      bool empty() const
      {
        return m_begin == m_end;
      }

      void popFront()
      {
        if (empty())
        {
          throw std::range_error("Range empty");
        }
        ++m_begin;
      }

      typename std::iterator_traits<typename Container::iterator>::reference front() const
      {
        if (empty())
        {
          throw std::range_error("Range empty");
        }
        return *m_begin;
      }

      typename Container::iterator m_begin;
      typename Container::iterator m_end;
    };

  template<typename ContainerType>
    void bootstrap_input_range(Dispatch_Engine &system, const std::string &type)
    {
      system.register_function(build_constructor<Input_Range<ContainerType>, ContainerType &>(), "range");

      register_function(system, &Input_Range<ContainerType>::empty, "empty");
      register_function(system, &Input_Range<ContainerType>::popFront, "popFront");
      register_function(system, &Input_Range<ContainerType>::front, "front");
    } 
  
  template<typename ContainerType>
  void bootstrap_reversible_container(Dispatch_Engine &system, const std::string &type)
  {
  }

  template<typename ContainerType>
  void bootstrap_random_access_container(Dispatch_Engine &system, const std::string &type)
  {
    bootstrap_reversible_container<ContainerType>(system, type);

    typedef typename ContainerType::reference(ContainerType::*indexoper)(size_t);

    //In the interest of runtime safety for the system, we prefer the at() method for [] access,
    //to throw an exception in an out of bounds condition.
    system.register_function(
        boost::function<typename ContainerType::reference (ContainerType *, int)>(indexoper(&ContainerType::at)), "[]");
    system.register_function(
        boost::function<typename ContainerType::reference (ContainerType *, int)>(indexoper(&ContainerType::operator[])), "at");
  }

  template<typename Assignable>
  void bootstrap_assignable(Dispatch_Engine &system, const std::string &type)
  {
    system.register_function(
        boost::function<Assignable &(Assignable*, const Assignable&)>(&Assignable::operator=), "=");
  }

  template<typename ContainerType>
  void bootstrap_container(Dispatch_Engine &system, const std::string &type)
  {
    bootstrap_assignable<ContainerType>(system, type);

    system.register_function(
        boost::function<size_t (ContainerType *)>(&ContainerType::size), "size");
    system.register_function(
        boost::function<size_t (ContainerType *)>(&ContainerType::size), "maxsize");
  }

  template<typename ContainerType>
  void bootstrap_forward_container(Dispatch_Engine &system, const std::string &type)
  {
    bootstrap_input_range<ContainerType>(system, type);
    bootstrap_container<ContainerType>(system, type);
  }

  template<typename Type>
  void bootstrap_default_constructible(Dispatch_Engine &system, const std::string &type)
  {
    system.register_function(build_constructor<Type>(), type);
  }

  template<typename SequenceType>
  void bootstrap_sequence(Dispatch_Engine &system, const std::string &type)
  {
    bootstrap_forward_container<SequenceType>(system, type);
    bootstrap_default_constructible<SequenceType>(system, type);
  }

  template<typename SequenceType>
  void bootstrap_back_insertion_sequence(Dispatch_Engine &system, const std::string &type)
  {
    bootstrap_sequence<SequenceType>(system, type);


    typedef typename SequenceType::reference (SequenceType::*backptr)();

    system.register_function(boost::function<typename SequenceType::reference (SequenceType *)>(backptr(&SequenceType::back)), "back");
    system.register_function(boost::function<void (SequenceType *,typename SequenceType::value_type)>(&SequenceType::push_back), "push_back");
    system.register_function(boost::function<void (SequenceType *)>(&SequenceType::pop_back), "pop_back");
  }

  template<typename VectorType>
  void bootstrap_vector(Dispatch_Engine &system, const std::string &type)
  {
    system.register_type<VectorType>(type);
    bootstrap_random_access_container<VectorType>(system, type);
    bootstrap_back_insertion_sequence<VectorType>(system, type);
  }

  template<typename ContainerType>
    void bootstrap_associative_container(Dispatch_Engine &system, const std::string &type)
    {
      bootstrap_forward_container<ContainerType>(system, type);
      bootstrap_default_constructible<ContainerType>(system, type);
    }

  template<typename ContainerType>
    void bootstrap_pair_associative_container(Dispatch_Engine &system, const std::string &type)
    {
      bootstrap_associative_container<ContainerType>(system, type);
    }

  template<typename ContainerType>
    void bootstrap_unique_associative_container(Dispatch_Engine &system, const std::string &type)
    {
      bootstrap_associative_container<ContainerType>(system, type);
    }

  template<typename ContainerType>
    void bootstrap_sorted_associative_container(Dispatch_Engine &system, const std::string &type)
    {
      bootstrap_reversible_container<ContainerType>(system, type);
    }

  template<typename ContainerType>
    void bootstrap_unique_sorted_associative_container(Dispatch_Engine &system, const std::string &type)
    {
      bootstrap_sorted_associative_container<ContainerType>(system, type);
      bootstrap_unique_associative_container<ContainerType>(system, type);
    }

  template<typename MapType>
    void bootstrap_map(Dispatch_Engine &system, const std::string &type)
    {
      system.register_type<MapType>(type);
      register_function(system, &MapType::operator[], "[]");
      bootstrap_unique_sorted_associative_container<MapType>(system, type);
      bootstrap_pair_associative_container<MapType>(system, type);
     }
}

#endif
