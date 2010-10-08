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
  } catch (const chaiscript::bad_boxed_cast &e) {
    if (expectedpass) {
      std::cerr << "Failure in run_test_type_conversion: " << e.what() << std::endl;
      return false;
    } else {
      return true;
    }
  } catch (const std::exception &e) {
    std::cerr << "Unexpected standard exception when attempting cast_conversion: " << e.what() << std::endl;
    return false;
  } catch (...) {
    std::cerr << "Unexpected unknown exception when attempting cast_conversion." << std::endl;
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
    std::cerr << "Error with type conversion test. From: " << bv.get_type_info().name() << " To: " << typeid(To).name() 
      << " test was expected to " << ((expectedpass)?(std::string("succeed")):(std::string("fail"))) << " but did not" << std::endl;
  }

  return ret;
}

template<typename Type>
bool do_test(const Boxed_Value &bv, bool T, bool ConstT, bool TRef, bool ConstTRef, bool TPtr, bool ConstTPtr, bool TPtrConst,
    bool ConstTPtrConst, bool SharedPtrT, bool SharedConstPtrT,
    bool ConstSharedPtrT, bool ConstSharedConstPtrT, bool ConstSharedPtrTRef, bool ConstSharedPtrTConstRef,
    bool BoostRef, bool BoostConstRef, bool ConstBoostRef, bool ConstBoostConstRef,
    bool ConstBoostRefRef, bool ConstBoostConstRefRef, bool PODValue,
    bool ConstPODValue, bool PODValueRef, bool ConstPODValueRef)
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
  passed &= test_type_conversion<boost::shared_ptr<Type> >(bv, SharedPtrT);
  passed &= test_type_conversion<boost::shared_ptr<const Type> >(bv, SharedConstPtrT);
  passed &= test_type_conversion<boost::shared_ptr<Type> &>(bv, false);
  passed &= test_type_conversion<boost::shared_ptr<const Type> &>(bv, false);
  passed &= test_type_conversion<const boost::shared_ptr<Type> >(bv, ConstSharedPtrT);
  passed &= test_type_conversion<const boost::shared_ptr<const Type> >(bv, ConstSharedConstPtrT);
  passed &= test_type_conversion<const boost::shared_ptr<Type> &>(bv, ConstSharedPtrTRef);
  passed &= test_type_conversion<const boost::shared_ptr<const Type> &>(bv, ConstSharedPtrTConstRef);
//  passed &= test_type_conversion<boost::reference_wrapper<Type> >(bv, BoostRef);
//  passed &= test_type_conversion<boost::reference_wrapper<const Type> >(bv, BoostConstRef);
  passed &= test_type_conversion<boost::reference_wrapper<Type> &>(bv, false);
  passed &= test_type_conversion<boost::reference_wrapper<const Type> &>(bv, false);
  passed &= test_type_conversion<const boost::reference_wrapper<Type> >(bv, ConstBoostRef);
  passed &= test_type_conversion<const boost::reference_wrapper<const Type> >(bv, ConstBoostConstRef);
  passed &= test_type_conversion<const boost::reference_wrapper<Type> &>(bv, ConstBoostRefRef);
  passed &= test_type_conversion<const boost::reference_wrapper<const Type> &>(bv, ConstBoostConstRefRef);
  passed &= test_type_conversion<Boxed_POD_Value>(bv, PODValue);
  passed &= test_type_conversion<const Boxed_POD_Value>(bv, ConstPODValue);
  passed &= test_type_conversion<Boxed_POD_Value &>(bv, PODValueRef);
  passed &= test_type_conversion<const Boxed_POD_Value &>(bv, ConstPODValueRef);
  passed &= test_type_conversion<Boxed_POD_Value *>(bv, false);
  passed &= test_type_conversion<const Boxed_POD_Value *>(bv, false);
  passed &= test_type_conversion<Boxed_POD_Value * const>(bv, false);
  passed &= test_type_conversion<const Boxed_POD_Value *const>(bv, false);
  passed &= test_type_conversion<Type *&>(bv, false);
  passed &= test_type_conversion<const Type *&>(bv, false);
  passed &= test_type_conversion<Type * const&>(bv, false);
  passed &= test_type_conversion<const Type * const&>(bv, false);

  return passed;
}



int main()
{
  bool passed = true;

  int i;
  /*
  bool T, bool ConstT, bool TRef, bool ConstTRef, bool TPtr, 
  bool ConstTPtr, bool TPtrConst, bool ConstTPtrConst, bool SharedPtrT, bool SharedConstPtrT,
    bool ConstSharedPtrT, bool ConstSharedConstPtrT, bool ConstSharedPtrTRef, bool ConstSharedPtrTConstRef, bool BoostRef, 
    bool BoostConstRef, bool ConstBoostRef, bool ConstBoostConstRef, bool ConstBoostRefRef, bool ConstBoostConstRefRef, 
    bool PODValue, bool ConstPODValue, bool PODValueRef, bool ConstPODValueRef
    */

  passed &= do_test<int>(var(i), true, true, true, true, true, 
                                 true, true, true, true, true,
                                 true, true, true, true, true,
                                 true, true, true, true, true,
                                 true, true, true, true);


  if (passed)
  {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }

}
