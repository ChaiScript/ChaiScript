#include <chaiscript/utility/utility.hpp>


using namespace chaiscript;


template<typename T>
void use(T){}

template<typename To>
bool run_test_type_conversion(const Boxed_Value &bv, bool expectedpass)
{
  try {
    To ret = chaiscript::boxed_cast<To>(bv);
    use(ret);
  } catch (const chaiscript::exception::bad_boxed_cast &e) {
    if (expectedpass) {
      std::cerr << "Failure in run_test_type_conversion: " << e.what() << '\n';
      return false;
    } else {
      return true;
    }
  } catch (const std::exception &e) {
    std::cerr << "Unexpected standard exception when attempting cast_conversion: " << e.what() << '\n';
    return false;
  } catch (...) {
    std::cerr << "Unexpected unknown exception when attempting cast_conversion.\n";
    return false;
  }
 
  if (expectedpass)
  {
    return true;
  } else {
    return false;
  }
}

template<typename To>
bool test_type_conversion(const Boxed_Value &bv, bool expectedpass)
{
  bool ret = run_test_type_conversion<To>(bv, expectedpass);

  if (!ret)
  {
    std::cerr << "Error with type conversion test. From: " 
      << (bv.is_const()?(std::string("const ")):(std::string())) << bv.get_type_info().name() 
      << " To: "  
      << (std::is_const<To>::value?(std::string("const ")):(std::string())) << typeid(To).name() 
      << " test was expected to " << ((expectedpass)?(std::string("succeed")):(std::string("fail"))) << " but did not\n";
  }

  return ret;
}

template<typename Type>
bool do_test(const Boxed_Value &bv, bool T, bool ConstT, bool TRef, bool ConstTRef, bool TPtr, bool ConstTPtr, bool TPtrConst,
    bool ConstTPtrConst, bool SharedPtrT, bool SharedConstPtrT,
    bool ConstSharedPtrT, bool ConstSharedConstPtrT, bool ConstSharedPtrTRef, bool ConstSharedPtrTConstRef,
    bool WrappedRef, bool WrappedConstRef, bool ConstWrappedRef, bool ConstWrappedConstRef,
    bool ConstWrappedRefRef, bool ConstWrappedConstRefRef, bool Number,
    bool ConstNumber, bool ConstNumberRef, bool TPtrConstRef, bool ConstTPtrConstRef)
{
  bool passed = true;
  passed &= test_type_conversion<Type>(bv, T);
  passed &= test_type_conversion<const Type>(bv, ConstT);
  passed &= test_type_conversion<Type &>(bv, TRef);
  passed &= test_type_conversion<const Type &>(bv, ConstTRef);
  passed &= test_type_conversion<Type *>(bv, TPtr);
  passed &= test_type_conversion<const Type *>(bv, ConstTPtr);
  passed &= test_type_conversion<Type * const>(bv, TPtrConst);
  passed &= test_type_conversion<const Type * const>(bv, ConstTPtrConst);
  passed &= test_type_conversion<std::shared_ptr<Type> >(bv, SharedPtrT);
  passed &= test_type_conversion<std::shared_ptr<const Type> >(bv, SharedConstPtrT);
  passed &= test_type_conversion<std::shared_ptr<Type> &>(bv, false);
  passed &= test_type_conversion<std::shared_ptr<const Type> &>(bv, false);
  passed &= test_type_conversion<const std::shared_ptr<Type> >(bv, ConstSharedPtrT);
  passed &= test_type_conversion<const std::shared_ptr<const Type> >(bv, ConstSharedConstPtrT);
  passed &= test_type_conversion<const std::shared_ptr<Type> &>(bv, ConstSharedPtrTRef);
  passed &= test_type_conversion<const std::shared_ptr<const Type> &>(bv, ConstSharedPtrTConstRef);
  passed &= test_type_conversion<std::reference_wrapper<Type> >(bv, WrappedRef);
  passed &= test_type_conversion<std::reference_wrapper<const Type> >(bv, WrappedConstRef);
  passed &= test_type_conversion<std::reference_wrapper<Type> &>(bv, false);
  passed &= test_type_conversion<std::reference_wrapper<const Type> &>(bv, false);
  passed &= test_type_conversion<const std::reference_wrapper<Type> >(bv, ConstWrappedRef);
  passed &= test_type_conversion<const std::reference_wrapper<const Type> >(bv, ConstWrappedConstRef);
  passed &= test_type_conversion<const std::reference_wrapper<Type> &>(bv, ConstWrappedRefRef);
  passed &= test_type_conversion<const std::reference_wrapper<const Type> &>(bv, ConstWrappedConstRefRef);
  passed &= test_type_conversion<Boxed_Number>(bv, Number);
  passed &= test_type_conversion<const Boxed_Number>(bv, ConstNumber);
  passed &= test_type_conversion<Boxed_Number &>(bv, false);
  passed &= test_type_conversion<const Boxed_Number &>(bv, ConstNumberRef);
  passed &= test_type_conversion<Boxed_Number *>(bv, false);
  passed &= test_type_conversion<const Boxed_Number *>(bv, false);
  passed &= test_type_conversion<Boxed_Number * const>(bv, false);
  passed &= test_type_conversion<const Boxed_Number *const>(bv, false);
  passed &= test_type_conversion<Type *&>(bv, false);
  passed &= test_type_conversion<const Type *&>(bv, false);
  passed &= test_type_conversion<Type * const&>(bv, TPtrConstRef);
  passed &= test_type_conversion<const Type * const&>(bv, ConstTPtrConstRef);
  passed &= test_type_conversion<Boxed_Value>(bv, true);
  passed &= test_type_conversion<const Boxed_Value>(bv, true);
  passed &= test_type_conversion<const Boxed_Value &>(bv, true);

  return passed;
}

/** Tests intended for built int types **/
template<typename T>
bool built_in_type_test(const T &initial, bool ispod)
{
  bool passed = true;

  /** value tests **/
  T i = T(initial);
  passed &= do_test<T>(var(i), true, true, true, true, true, 
                                 true, true, true, true, true,
                                 true, true, true, true, true,
                                 true, true, true, true, true,
                                 ispod, ispod, ispod, true, true);

  passed &= do_test<T>(const_var(i), true, true, false, true, false, 
                                       true, false, true, false, true,
                                       false, true, false, true, false,
                                       true, false, true, false, true,
                                       ispod, ispod, ispod, false, true);

  passed &= do_test<T>(var(&i), true, true, true, true, true, 
                                 true, true, true, false, false,
                                 false, false, false, false, true,
                                 true, true, true, true, true,
                                 ispod, ispod, ispod, true, true);

  passed &= do_test<T>(const_var(&i), true, true, false, true, false, 
                                 true, false, true, false, false,
                                 false, false, false, false, false,
                                 true, false, true, false, true,
                                 ispod, ispod, ispod, false, true);

  passed &= do_test<T>(var(std::ref(i)), true, true, true, true, true, 
                                 true, true, true, false, false,
                                 false, false, false, false, true,
                                 true, true, true, true, true,
                                 ispod, ispod, ispod, true, true);

  passed &= do_test<T>(var(std::cref(i)), true, true, false, true, false, 
                                 true, false, true, false, false,
                                 false, false, false, false, false,
                                 true, false, true, false, true,
                                 ispod, ispod, ispod, false, true);

  /** Const Reference Variable tests */

  // This reference will be copied on input, which is expected
  const T &ir = i;

  passed &= do_test<T>(var(i), true, true, true, true, true, 
                                 true, true, true, true, true,
                                 true, true, true, true, true,
                                 true, true, true, true, true,
                                 ispod, ispod, ispod, true, true);

  // But a pointer or reference to it should be necessarily const
  passed &= do_test<T>(var(&ir), true, true, false, true, false, 
                                 true, false, true, false, false,
                                 false, false, false, false, false,
                                 true, false, true, false, true,
                                 ispod, ispod, ispod, false, true);

  passed &= do_test<T>(var(std::ref(ir)), true, true, false, true, false, 
                                 true, false, true, false, false,
                                 false, false, false, false, false,
                                 true, false, true, false, true,
                                 ispod, ispod, ispod, false, true);

  // Make sure const of const works too
  passed &= do_test<T>(const_var(&ir), true, true, false, true, false, 
                                 true, false, true, false, false,
                                 false, false, false, false, false,
                                 true, false, true, false, true,
                                 ispod, ispod, ispod, false, true);

  passed &= do_test<T>(const_var(std::ref(ir)), true, true, false, true, false, 
                                 true, false, true, false, false,
                                 false, false, false, false, false,
                                 true, false, true, false, true,
                                 ispod, ispod, ispod, false, true);

  /** Const Reference Variable tests */

  // This will always be seen as a const
  const T*cip = &i;
  passed &= do_test<T>(var(cip), true, true, false, true, false, 
                                 true, false, true, false, false,
                                 false, false, false, false, false,
                                 true, false, true, false, true,
                                 ispod, ispod, ispod, false, true);

  // make sure const of const works
  passed &= do_test<T>(const_var(cip), true, true, false, true, false, 
                                 true, false, true, false, false,
                                 false, false, false, false, false,
                                 true, false, true, false, true,
                                 ispod, ispod, ispod, false, true);

  /** shared_ptr tests **/

  std::shared_ptr<T> ip(new T(initial));

  passed &= do_test<T>(var(ip), true, true, true, true, true, 
                                 true, true, true, true, true,
                                 true, true, true, true, true,
                                 true, true, true, true, true,
                                 ispod, ispod, ispod, true, true);

  passed &= do_test<T>(const_var(ip), true, true, false, true, false, 
                                       true, false, true, false, true,
                                       false, true, false, true, false,
                                       true, false, true, false, true,
                                       ispod, ispod, ispod, false, true);

  /** const shared_ptr tests **/
  std::shared_ptr<const T> ipc(new T(initial));

  passed &= do_test<T>(var(ipc), true, true, false, true, false, 
                                       true, false, true, false, true,
                                       false, true, false, true, false,
                                       true, false, true, false, true,
                                       ispod, ispod, ispod, false, true);

  // const of this should be the same, making sure it compiles
  passed &= do_test<T>(const_var(ipc), true, true, false, true, false, 
                                       true, false, true, false, true,
                                       false, true, false, true, false,
                                       true, false, true, false, true,
                                       ispod, ispod, ispod, false, true);


  /** Double ptr tests **/

  /*
  T **doublep;

  passed &= do_test<T*>(var(doublep), true, true, false, true, false, 
                                       true, false, true, false, true,
                                       false, true, false, true, false,
                                       true, false, true, false, true,
                                       ispod, ispod, ispod, false, true);
*/

  return passed;
}


template<typename T>
bool pointer_test(const T& default_value, const T& new_value)
{
  T *p = new T(default_value);

  // we store a pointer to a pointer, so we can get a pointer to a pointer
  try {
    T **result = boxed_cast<T **>(var(&p));
    *(*result) = new_value;


    if (p != (*result) ) { 
      std::cerr << "Pointer passed in different than one returned\n";
      delete p;
      return false;
    }

    if (*p != *(*result) ) {
      std::cerr << "Somehow dereferenced pointer values are not the same?\n";
      delete p;
      return false;
    }

    delete p;
    return true;
  } catch (const exception::bad_boxed_cast &) {
    std::cerr << "Bad boxed cast performing ** to ** test\n";
    delete p;
    return false;
  } catch (...) {
    std::cerr << "Unknown exception performing ** to ** test\n";
    delete p;
    return false;
  }


}


int main()
{
  bool passed = true;

  /*
  bool T, bool ConstT, bool TRef, bool ConstTRef, bool TPtr, 
  bool ConstTPtr, bool TPtrConst, bool ConstTPtrConst, bool SharedPtrT, bool SharedConstPtrT,
    bool ConstSharedPtrT, bool ConstSharedConstPtrT, bool ConstSharedPtrTRef, bool ConstSharedPtrTConstRef, bool WrappedRef, 
    bool WrappedConstRef, bool ConstWrappedRef, bool ConstWrappedConstRef, bool ConstWrappedRefRef, bool ConstWrappedConstRefRef, 
    bool Number, bool ConstNumber, bool ConstNumberRef
    */

  passed &= built_in_type_test<int>(5, true);
  passed &= built_in_type_test<double>(1.1, true);
  passed &= built_in_type_test<char>('a', true);
  passed &= built_in_type_test<uint8_t>('a', true);
  passed &= built_in_type_test<int64_t>('a', true);
  passed &= built_in_type_test<bool>(false, false);
  passed &= built_in_type_test<std::string>("Hello World", false);
  
  // storing a pointer 
  passed &= pointer_test<int>(1, 0);

  if (passed)
  {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }

}
