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

      Input_Range(typename Container::iterator itr)
        : m_begin(itr), m_end(itr)
      {
      }

      Input_Range(const std::pair<typename Container::iterator, typename Container::iterator> &t_p)
        : m_begin(t_p.first), m_end(t_p.second)
      {
      }

      bool empty() const
      {
        return m_begin == m_end;
      }

      void pop_front()
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
      system.register_function(build_constructor<Input_Range<ContainerType>, 
          typename ContainerType::iterator>(), "range");
      system.register_function(build_constructor<Input_Range<ContainerType>, 
          const std::pair<typename ContainerType::iterator, typename ContainerType::iterator> &>(), "range");


      register_function(system, &Input_Range<ContainerType>::empty, "empty");
      register_function(system, &Input_Range<ContainerType>::pop_front, "pop_front");
      register_function(system, &Input_Range<ContainerType>::front, "front");
      system.register_function(build_constructor<Input_Range<ContainerType>, const Input_Range<ContainerType> &>(), "clone");
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
    add_basic_constructors<Assignable>(system, type);
    add_oper_assign<Assignable>(system);
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
    system.register_function(boost::function<void (SequenceType *,typename SequenceType::value_type)>(&SequenceType::push_back), "push_back_ref");
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

  template<typename PairType>
    void bootstrap_pair(Dispatch_Engine &system, const std::string &type)
    {
      register_member(system, &PairType::first, "first");
      register_member(system, &PairType::second, "second");

      system.register_function(build_constructor<PairType >(), type);
      system.register_function(build_constructor<PairType, const PairType &>(), type);
      system.register_function(build_constructor<PairType, const PairType &>(), "clone");
      system.register_function(build_constructor<PairType, const typename PairType::first_type &, const typename PairType::second_type &>(), type);
     }


  template<typename ContainerType>
    void bootstrap_pair_associative_container(Dispatch_Engine &system, const std::string &type)
    {
      bootstrap_associative_container<ContainerType>(system, type);
      bootstrap_pair<typename ContainerType::value_type>(system, type + "_Pair");
    }

  template<typename ContainerType>
    void bootstrap_unique_associative_container(Dispatch_Engine &system, const std::string &type)
    {
      bootstrap_associative_container<ContainerType>(system, type);
      register_function(system, &ContainerType::count, "count");
    }

  template<typename ContainerType>
    void bootstrap_sorted_associative_container(Dispatch_Engine &system, const std::string &type)
    {
      typedef std::pair<typename ContainerType::iterator, typename ContainerType::iterator> 
                        (ContainerType::*eq_range)(const typename ContainerType::key_type &);

      bootstrap_reversible_container<ContainerType>(system, type);
      bootstrap_associative_container<ContainerType>(system, type);
      register_function(system, eq_range(&ContainerType::equal_range), "equal_range");
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

  template<typename String>
    void bootstrap_string(Dispatch_Engine &system, const std::string &type)
    {
      system.register_type<String>(type);
      add_oper_add<String>(system);
      add_oper_add_equals<String>(system);
      add_opers_comparison<String>(system);
      bootstrap_random_access_container<String>(system, type);
      bootstrap_sequence<String>(system, type);
    }

}

#endif
