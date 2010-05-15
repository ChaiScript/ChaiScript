// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2010, Jonathan Turner (jturner@minnow-lang.org)
// and Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com

/**
* This file contains utility functions for registration of STL container
* classes. The methodology used is based on the SGI STL concepts.
* http://www.sgi.com/tech/stl/table_of_contents.html
*/

#ifndef __bootstrap_stl_hpp__
#define __bootstrap_stl_hpp__

#include "dispatchkit.hpp"
#include "register_function.hpp"


namespace chaiscript 
{
  namespace bootstrap
  {
    /**
    * Bidir_Range, based on the D concept of ranges.
    * \todo Update the Range code to base its capabilities on
    *       the user_typetraits of the iterator passed in
    */
    template<typename Container>
    struct Bidir_Range
    {
      typedef typename std::iterator_traits<typename Container::iterator>::reference reference_type;

      Bidir_Range(Container &c)
        : m_begin(c.begin()), m_end(c.end())
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

      void pop_back()
      {
        if (empty())
        {
          throw std::range_error("Range empty");
        }
        --m_end;
      }

      reference_type front() const
      {
        if (empty())
        {
          throw std::range_error("Range empty");
        }
        return *m_begin;
      }

      reference_type back() const
      {
        if (empty())
        {
          throw std::range_error("Range empty");
        }
        typename Container::iterator pos = m_end;
        --pos;
        return *(pos);
      }

      typename Container::iterator m_begin;
      typename Container::iterator m_end;
    };

    template<typename Range>
    struct Retro
    {
      Retro(const Range &r)
        : m_r(r)
      {}

      bool empty() { return m_r.empty(); }
      void pop_front() { m_r.pop_back(); }
      void pop_back() { m_r.pop_front(); }
      typename Range::reference_type front() { return m_r.back(); }
      typename Range::reference_type back() { return m_r.front(); }

    private:
      Range m_r;
    };


    /**
    * Add Bidir_Range support for the given ContainerType
    */
    template<typename ContainerType>
    ModulePtr input_range_type(const std::string &type, ModulePtr m = ModulePtr(new Module()))
    {
      m->add(user_type<Bidir_Range<ContainerType> >(), type + "_Range");

      copy_constructor<Bidir_Range<ContainerType> >(type + "_Range", m);

      m->add(constructor<Bidir_Range<ContainerType> (ContainerType &)>(), "range");

      m->add(fun(&Bidir_Range<ContainerType>::empty), "empty");
      m->add(fun(&Bidir_Range<ContainerType>::pop_front), "pop_front");
      m->add(fun(&Bidir_Range<ContainerType>::front), "front");
      m->add(fun(&Bidir_Range<ContainerType>::pop_back), "pop_back");
      m->add(fun(&Bidir_Range<ContainerType>::back), "back");

      return m;
    } 

    /**
    * Add random_access_container concept to the given ContainerType
    * http://www.sgi.com/tech/stl/RandomAccessContainer.html
    */
    template<typename ContainerType>
    ModulePtr random_access_container_type(const std::string &/*type*/, ModulePtr m = ModulePtr(new Module()))
    {
      typedef typename ContainerType::reference(ContainerType::*indexoper)(size_t);

      //In the interest of runtime safety for the m, we prefer the at() method for [] access,
      //to throw an exception in an out of bounds condition.
      m->add(
        fun(boost::function<typename ContainerType::reference (ContainerType *, int)>(boost::mem_fn(static_cast<indexoper>(&ContainerType::at)))), "[]");

      return m;
    }

    /**
    * Add assignable concept to the given ContainerType
    * http://www.sgi.com/tech/stl/Assignable.html
    */
    template<typename ContainerType>
    ModulePtr assignable_type(const std::string &type, ModulePtr m = ModulePtr(new Module()))
    {
      basic_constructors<ContainerType>(type, m);
      operators::assign<ContainerType>(m);
      return m;
    }

    /**
    * Add container concept to the given ContainerType
    * http://www.sgi.com/tech/stl/Container.html
    */
    template<typename ContainerType>
    ModulePtr container_type(const std::string &/*type*/, ModulePtr m = ModulePtr(new Module()))
    {
      m->add(fun(boost::function<int (const ContainerType *)>(boost::mem_fn(&ContainerType::size))), "size");
      m->add(fun<bool (ContainerType::*)() const>(&ContainerType::empty), "empty");
      m->add(fun<void (ContainerType::*)()>(&ContainerType::clear), "clear");

      return m;
    }

    /**
    * Add default constructable concept to the given Type
    * http://www.sgi.com/tech/stl/DefaultConstructible.html
    */
    template<typename Type>
    ModulePtr default_constructible_type(const std::string &type, ModulePtr m = ModulePtr(new Module()))
    {
      m->add(constructor<Type ()>(), type);
      return m;
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
    ModulePtr sequence_type(const std::string &/*type*/, ModulePtr m = ModulePtr(new Module()))
    {
      std::string insert_name;
      if (typeid(typename ContainerType::value_type) == typeid(Boxed_Value))
      {
        insert_name = "insert_ref_at";
      } else {
        insert_name = "insert_at";
      }

      m->add(fun(&insert_at<ContainerType>), insert_name);
      m->add(fun(&erase_at<ContainerType>), "erase_at");

      return m;
    }

    /**
    * Add back insertion sequence concept to the given ContainerType
    * http://www.sgi.com/tech/stl/BackInsertionSequence.html
    */
    template<typename ContainerType>
    ModulePtr back_insertion_sequence_type(const std::string &/*type*/, ModulePtr m = ModulePtr(new Module()))
    {
      typedef typename ContainerType::reference (ContainerType::*backptr)();

      m->add(fun(static_cast<backptr>(&ContainerType::back)), "back");

      std::string push_back_name;
      if (typeid(typename ContainerType::value_type) == typeid(Boxed_Value))
      {
        push_back_name = "push_back_ref";
      } else {
        push_back_name = "push_back";
      }

      typedef void (ContainerType::*pushback)(const typename ContainerType::value_type &);
      m->add(fun(static_cast<pushback>(&ContainerType::push_back)), push_back_name);
      m->add(fun(&ContainerType::pop_back), "pop_back");
      return m;
    }


    /**
     *Front insertion sequence
     *http://www.sgi.com/tech/stl/FrontInsertionSequence.html
     */
    template<typename ContainerType>
    ModulePtr front_insertion_sequence_type(const std::string &type, ModulePtr m = ModulePtr(new Module()))
    {
      typedef typename ContainerType::reference (ContainerType::*frontptr)();
      m->add(fun(static_cast<frontptr>(&ContainerType::front)), "front");

      std::string push_front_name;
      if (typeid(typename ContainerType::value_type) == typeid(Boxed_Value))
      {
        push_front_name = "push_front_ref";
      } else {
        push_front_name = "push_front";
      }
      m->add(fun(&ContainerType::push_front), push_front_name);
      m->add(fun(&ContainerType::pop_front), "pop_front");
      return m;
    }

    /**
    * bootstrap a given PairType
    * http://www.sgi.com/tech/stl/pair.html
    */
    template<typename PairType>
    ModulePtr pair_type(const std::string &type, ModulePtr m = ModulePtr(new Module()))
    {
      m->add(user_type<PairType>(), type);

      m->add(fun(&PairType::first), "first");
      m->add(fun(&PairType::second), "second");

      basic_constructors<PairType>(type, m);
      m->add(constructor<PairType (const typename PairType::first_type &, const typename PairType::second_type &)>(), type);

      return m;
    }


    /**
    * Add pair associative container concept to the given ContainerType
    * http://www.sgi.com/tech/stl/PairAssociativeContainer.html
    */
    template<typename ContainerType>
    ModulePtr pair_associative_container_type(const std::string &type, ModulePtr m = ModulePtr(new Module()))
    {
      pair_type<typename ContainerType::value_type>(type + "_Pair", m);

      return m;
    }

    /**
    * Add unique associative container concept to the given ContainerType
    * http://www.sgi.com/tech/stl/UniqueAssociativeContainer.html
    */
    template<typename ContainerType>
    ModulePtr unique_associative_container_type(const std::string &/*type*/, ModulePtr m = ModulePtr(new Module()))
    {
      m->add(fun(boost::function<int (const ContainerType *, const typename ContainerType::key_type &)>(boost::mem_fn(&ContainerType::count))), "count");

      return m;
    }

    /**
    * Add a MapType container
    * http://www.sgi.com/tech/stl/Map.html
    */
    template<typename MapType>
    ModulePtr map_type(const std::string &type, ModulePtr m = ModulePtr(new Module()))
    {
      m->add(user_type<MapType>(), type);

      typedef typename MapType::mapped_type &(MapType::*elemaccess)(const typename MapType::key_type &);
      m->add(fun(static_cast<elemaccess>(&MapType::operator[])), "[]");

      container_type<MapType>(type, m);
      assignable_type<MapType>(type, m);
      unique_associative_container_type<MapType>(type, m);
      pair_associative_container_type<MapType>(type, m);
      input_range_type<MapType>(type, m);

      return m;
    }

    /**
     * hopefully working List type
     * http://www.sgi.com/tech/stl/List.html
     */
    template<typename ListType>
    ModulePtr list_type(const std::string &type, ModulePtr m = ModulePtr(new Module()))
    {
      m->add(user_type<ListType>(), type);

      front_insertion_sequence_type<ListType>(type, m);
      back_insertion_sequence_type<ListType>(type, m);
      sequence_type<ListType>(type, m);
      container_type<ListType>(type, m);
      default_constructible_type<ListType>(type, m);
      assignable_type<ListType>(type, m);
      input_range_type<ListType>(type, m);

      return m;
    }

    /**
    * Create a vector type with associated concepts
    * http://www.sgi.com/tech/stl/Vector.html
    */
    template<typename VectorType>
    ModulePtr vector_type(const std::string &type, ModulePtr m = ModulePtr(new Module()))
    {
      m->add(user_type<VectorType>(), type);

      typedef typename VectorType::reference (VectorType::*frontptr)();
      m->add(fun(static_cast<frontptr>(&VectorType::front)), "front");


      back_insertion_sequence_type<VectorType>(type, m);
      sequence_type<VectorType>(type, m);
      random_access_container_type<VectorType>(type, m);
      container_type<VectorType>(type, m);
      default_constructible_type<VectorType>(type, m);
      assignable_type<VectorType>(type, m);
      input_range_type<VectorType>(type, m);


      m->eval("def Vector::`==`(rhs) : type_match(rhs, this) { \
               if ( rhs.size() != this.size() ) {    \
                 return false;  \
               } else {  \
                 var r1 = range(this); \
                 var r2 = range(rhs);  \
                 while (!r1.empty()) \
                 {  \
                   if (!eq(r1.front(), r2.front())) \
                   {  \
                     return false; \
                   } \
                   r1.pop_front(); \
                   r2.pop_front(); \
                 } \
                 return true; \
              } \
            }");


      return m;
    }

    /**
    * Add a String container
    * http://www.sgi.com/tech/stl/basic_string.html
    */
    template<typename String>
    ModulePtr string_type(const std::string &type, ModulePtr m = ModulePtr(new Module()))
    {
      m->add(user_type<String>(), type);
      operators::addition<String>(m);
      operators::assign_sum<String>(m);
      opers_comparison<String>(m);
      random_access_container_type<String>(type, m);
      sequence_type<String>(type, m);
      default_constructible_type<String>(type, m);
      container_type<String>(type, m);
      assignable_type<String>(type, m);
      input_range_type<String>(type, m);

      //Special case: add push_back to string (which doesn't support other back_insertion operations
      std::string push_back_name;
      if (typeid(typename String::value_type) == typeid(Boxed_Value))
      {
        push_back_name = "push_back_ref";
      } else {
        push_back_name = "push_back";
      }
      m->add(fun(&String::push_back), push_back_name);

      typedef typename String::size_type (String::*find_func_ptr)(const String &, typename String::size_type) const;

      typedef boost::function<int (const String *, const String &, int)> find_func;

      m->add(fun(find_func(boost::mem_fn(static_cast<find_func_ptr>(&String::find)))), "find");
      m->add(fun(find_func(boost::mem_fn(static_cast<find_func_ptr>(&String::rfind)))), "rfind");
      m->add(fun(find_func(boost::mem_fn(static_cast<find_func_ptr>(&String::find_first_of)))), "find_first_of");
      m->add(fun(find_func(boost::mem_fn(static_cast<find_func_ptr>(&String::find_last_of)))), "find_last_of");
      m->add(fun(find_func(boost::mem_fn(static_cast<find_func_ptr>(&String::find_first_not_of)))), "find_first_not_of");
      m->add(fun(find_func(boost::mem_fn(static_cast<find_func_ptr>(&String::find_last_not_of)))), "find_last_not_of");

      return m;
    }
  }
}

#endif
