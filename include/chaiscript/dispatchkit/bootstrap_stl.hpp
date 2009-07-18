// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009, Jonathan Turner (jturner@minnow-lang.org)
// and Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com

/**
 * This file contains utility functions for registration of STL container
 * classes. The methodology used is based on the SGI STL concepts.
 * http://www.sgi.com/tech/stl/table_of_contents.html
 */

#ifndef __bootstrap_stl_hpp
#define __bootstrap_stl_hpp__

#include "dispatchkit.hpp"
#include "register_function.hpp"


namespace chaiscript 
{
  /**
   * Input_Range, based on the D concept of ranges.
   * \todo Update the Range code to base its capabilities on
   *       the type_traits of the iterator passed in
   */
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

  
  /**
   * Add Input_Range support for the given ContainerType
   */
  template<typename ContainerType>
    void bootstrap_input_range(Dispatch_Engine &system, const std::string &type)
    {
      system.add(type_<Input_Range<ContainerType> >(), type + "_Range");
      system.add(type_<typename ContainerType::iterator>(), type+"_Iterator");

      system.add(constructor<Input_Range<ContainerType> (ContainerType &)>(), "range");
      system.add(constructor<Input_Range<ContainerType> (typename ContainerType::iterator)>(), "range");

      typedef std::pair<typename ContainerType::iterator, typename ContainerType::iterator> ItrPair;

      system.add(constructor<Input_Range<ContainerType> (const ItrPair &)>(), "range");

      system.add(type_<ItrPair>(), type+"_Iterator_Pair");

      system.add(fun(&Input_Range<ContainerType>::empty), "empty");
      system.add(fun(&Input_Range<ContainerType>::pop_front), "pop_front");
      system.add(fun(&Input_Range<ContainerType>::front), "front");
      system.add(constructor<Input_Range<ContainerType> (const Input_Range<ContainerType> &)>(), "clone");
    } 
  
  /**
   * Add reversible_container concept to the given ContainerType
   * http://www.sgi.com/tech/stl/ReversibleContainer.html
   */
  template<typename ContainerType>
  void bootstrap_reversible_container(Dispatch_Engine &/*system*/, const std::string &/*type*/)
  {
  }

  /**
   * Add random_access_container concept to the given ContainerType
   * http://www.sgi.com/tech/stl/RandomAccessContainer.html
   */
  template<typename ContainerType>
  void bootstrap_random_access_container(Dispatch_Engine &system, const std::string &type)
  {
    bootstrap_reversible_container<ContainerType>(system, type);
    typedef typename ContainerType::reference(ContainerType::*indexoper)(size_t);

    //In the interest of runtime safety for the system, we prefer the at() method for [] access,
    //to throw an exception in an out of bounds condition.
    system.add(
        fun(boost::function<typename ContainerType::reference (ContainerType *, int)>(indexoper(&ContainerType::at))), "[]");
    system.add(
        fun(boost::function<typename ContainerType::reference (ContainerType *, int)>(indexoper(&ContainerType::operator[]))), "at");

  }

  /**
   * Add assignable concept to the given ContainerType
   * http://www.sgi.com/tech/stl/Assignable.html
   */
  template<typename ContainerType>
  void bootstrap_assignable(Dispatch_Engine &system, const std::string &type)
  {
    add_basic_constructors<ContainerType>(system, type);
    add_oper_assign<ContainerType>(system);
  }

  /**
   * Add container concept to the given ContainerType
   * http://www.sgi.com/tech/stl/Container.html
   */
  template<typename ContainerType>
  void bootstrap_container(Dispatch_Engine &system, const std::string &type)
  {
    bootstrap_assignable<ContainerType>(system, type);

    system.add(fun(&ContainerType::size), "size");
    system.add(fun(&ContainerType::max_size), "max_size");
    system.add(fun(&ContainerType::empty), "empty");
  }

  /**
   * Add forward container concept to the given ContainerType
   * http://www.sgi.com/tech/stl/ForwardContainer.html
   */
  template<typename ContainerType>
  void bootstrap_forward_container(Dispatch_Engine &system, const std::string &type)
  {
    bootstrap_input_range<ContainerType>(system, type);
    bootstrap_container<ContainerType>(system, type);
  }

  /**
   * Add default constructable concept to the given Type
   * http://www.sgi.com/tech/stl/DefaultConstructible.html
   */
  template<typename Type>
  void bootstrap_default_constructible(Dispatch_Engine &system, const std::string &type)
  {
    system.add(constructor<Type ()>(), type);
  }

  /**
   * Algorithm for inserting at a specific position into a container
   */
  template<typename Type>
    void insert_at(Type &container, int pos, const typename Type::value_type &v)
    {
      typename Type::iterator itr = container.begin();
      typename Type::iterator end = container.end();
      
      if (pos < 0 || std::distance(itr, end) < pos)
      {
        throw std::range_error("Cannot insert past end of range");
      }

      std::advance(itr, pos);
      container.insert(itr, v);
    }

  /**
   * Algorithm for erasing a specific position from a container
   */
  template<typename Type>
    void erase_at(Type &container, int pos)
    {
      typename Type::iterator itr = container.begin();
      typename Type::iterator end = container.end();
      
      if (pos < 0 || std::distance(itr, end) < (pos-1))
      {
        throw std::range_error("Cannot erase past end of range");
      }

      std::advance(itr, pos);
      container.erase(itr);
    }

  /**
   * Add sequence concept to the given ContainerType
   * http://www.sgi.com/tech/stl/Sequence.html
   */
  template<typename ContainerType>
  void bootstrap_sequence(Dispatch_Engine &system, const std::string &type)
  {
    bootstrap_forward_container<ContainerType>(system, type);
    bootstrap_default_constructible<ContainerType>(system, type);

    std::string insert_name;
    if (typeid(typename ContainerType::value_type) == typeid(Boxed_Value))
    {
      insert_name = "insert_ref_at";
    } else {
      insert_name = "insert_at";
    }

    system.add(fun(&insert_at<ContainerType>), insert_name);
    system.add(fun(&erase_at<ContainerType>), "erase_at");
  }

  /**
   * Add back insertion sequence concept to the given ContainerType
   * http://www.sgi.com/tech/stl/BackInsertionSequence.html
   */
  template<typename ContainerType>
  void bootstrap_back_insertion_sequence(Dispatch_Engine &system, const std::string &type)
  {
    bootstrap_sequence<ContainerType>(system, type);


    typedef typename ContainerType::reference (ContainerType::*backptr)();

    system.add(fun(backptr(&ContainerType::back)), "back");

    std::string push_back_name;
    if (typeid(typename ContainerType::value_type) == typeid(Boxed_Value))
    {
      push_back_name = "push_back_ref";
    } else {
      push_back_name = "push_back";
    }

    system.add(fun(&ContainerType::push_back), push_back_name);
    system.add(fun(&ContainerType::pop_back), "pop_back");
  }

  /**
   * Create a vector type with associated concepts
   * http://www.sgi.com/tech/stl/Vector.html
   */
  template<typename VectorType>
  void bootstrap_vector(Dispatch_Engine &system, const std::string &type)
  {
    system.add(type_<VectorType>(), type);
    bootstrap_random_access_container<VectorType>(system, type);
    bootstrap_back_insertion_sequence<VectorType>(system, type);
  }

  /**
   * Create a vector type with associated concepts
   * http://www.sgi.com/tech/stl/Vector.html
   */
  template<typename ContainerType>
    void bootstrap_associative_container(Dispatch_Engine &system, const std::string &type)
    {
      bootstrap_forward_container<ContainerType>(system, type);
      bootstrap_default_constructible<ContainerType>(system, type);
    }

  /**
   * bootstrap a given PairType
   * http://www.sgi.com/tech/stl/pair.html
   */
  template<typename PairType>
    void bootstrap_pair(Dispatch_Engine &system, const std::string &type)
    {
      system.add(type_<PairType>(), type);

      system.add(fun(&PairType::first), "first");
      system.add(fun(&PairType::second), "second");

      system.add(constructor<PairType ()>(), type);
      system.add(constructor<PairType (const PairType &)>(), type);
      system.add(constructor<PairType (const PairType &)>(), "clone");
      system.add(constructor<PairType (const typename PairType::first_type &, const typename PairType::second_type &)>(), type);
     }


  /**
   * Add pair associative container concept to the given ContainerType
   * http://www.sgi.com/tech/stl/PairAssociativeContainer.html
   */
  template<typename ContainerType>
    void bootstrap_pair_associative_container(Dispatch_Engine &system, const std::string &type)
    {
      bootstrap_associative_container<ContainerType>(system, type);
      bootstrap_pair<typename ContainerType::value_type>(system, type + "_Pair");
    }

  /**
   * Add unique associative container concept to the given ContainerType
   * http://www.sgi.com/tech/stl/UniqueAssociativeContainer.html
   */
  template<typename ContainerType>
    void bootstrap_unique_associative_container(Dispatch_Engine &system, const std::string &type)
    {
      bootstrap_associative_container<ContainerType>(system, type);
      system.add(fun(&ContainerType::count), "count");
    }

  /**
   * Add sorted associative container concept to the given ContainerType
   * http://www.sgi.com/tech/stl/SortedAssociativeContainer.html
   */
  template<typename ContainerType>
    void bootstrap_sorted_associative_container(Dispatch_Engine &system, const std::string &type)
    {
      typedef std::pair<typename ContainerType::iterator, typename ContainerType::iterator> 
                        (ContainerType::*eq_range)(const typename ContainerType::key_type &);

      bootstrap_reversible_container<ContainerType>(system, type);
      bootstrap_associative_container<ContainerType>(system, type);
      system.add(fun(eq_range(&ContainerType::equal_range)), "equal_range");
     }

  /**
   * Add unique sorted associative container concept to the given ContainerType
   * http://www.sgi.com/tech/stl/UniqueSortedAssociativeContainer.html
   */
  template<typename ContainerType>
    void bootstrap_unique_sorted_associative_container(Dispatch_Engine &system, const std::string &type)
    {
      bootstrap_sorted_associative_container<ContainerType>(system, type);
      bootstrap_unique_associative_container<ContainerType>(system, type);
    }

  /**
   * Add a MapType container
   * http://www.sgi.com/tech/stl/Map.html
   */
  template<typename MapType>
    void bootstrap_map(Dispatch_Engine &system, const std::string &type)
    {
      system.add(type_<MapType>(), type);
      system.add(fun(&MapType::operator[]), "[]");
      bootstrap_unique_sorted_associative_container<MapType>(system, type);
      bootstrap_pair_associative_container<MapType>(system, type);
    }

  /**
   * Add a String container
   * http://www.sgi.com/tech/stl/basic_string.html
   */
  template<typename String>
    void bootstrap_string(Dispatch_Engine &system, const std::string &type)
    {
      system.add(type_<String>(), type);
      add_oper_add<String>(system);
      add_oper_add_equals<String>(system);
      add_opers_comparison<String>(system);
      bootstrap_random_access_container<String>(system, type);
      bootstrap_sequence<String>(system, type);
      typedef typename String::size_type (String::*find_func)(const String &, typename String::size_type) const;
      system.add(fun(find_func(&String::find)), "find");
      system.add(fun(find_func(&String::rfind)), "rfind");
      system.add(fun(find_func(&String::find_first_of)), "find_first_of");
      system.add(fun(find_func(&String::find_last_of)), "find_last_of");
      system.add(fun(find_func(&String::find_first_not_of)), "find_first_not_of");
      system.add(fun(find_func(&String::find_last_not_of)), "find_last_not_of");
    }
}

#endif
