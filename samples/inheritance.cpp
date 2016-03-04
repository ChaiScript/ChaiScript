#include <chaiscript/chaiscript.hpp>
#include <chaiscript/chaiscript_stdlib.hpp>

class BaseClass
{
  public:
    BaseClass()
    {
    }

    BaseClass(const BaseClass &) = default;

    virtual ~BaseClass() {}

    virtual std::string doSomething(float, double) const = 0;


    void setValue(const std::string &t_val) {
      if (validateValue(t_val))
      {
        m_value = t_val;
      }
    }

    std::string getValue() const {
      return m_value;
    }

  protected:
    virtual bool validateValue(const std::string &t_val) = 0;

  private:
    std::string m_value;
};

class ChaiScriptDerived : public BaseClass
{
  public:
    ChaiScriptDerived(const std::vector<chaiscript::Boxed_Value> &t_funcs)
    {
      // using the range-checked .at() methods to give us an exception
      // instead of a crash if the user passed in too-few params
      tie(t_funcs.at(0), m_doSomethingImpl);
      tie(t_funcs.at(1), m_validateValueImpl);
    }

    std::string doSomething(float f, double d) const override
    {
      assert(m_doSomethingImpl);
      return m_doSomethingImpl(*this, f, d);
    }

  protected:
    bool validateValue(const std::string &t_val) override
    {
      assert(m_validateValueImpl);
      return m_validateValueImpl(*this, t_val);
    }

  private:
    template<typename Param>
    void tie(const chaiscript::Boxed_Value &t_func, Param &t_param)
    {
      t_param = chaiscript::boxed_cast<Param>(t_func);
    }

    std::function<std::string (const ChaiScriptDerived&, float, double)> m_doSomethingImpl;
    std::function<bool (ChaiScriptDerived&, const std::string &t_val)> m_validateValueImpl;
};

int main()
{
  chaiscript::ChaiScript chai(chaiscript::Std_Lib::library());
  chai.add(chaiscript::fun(&BaseClass::doSomething), "doSomething");
  chai.add(chaiscript::fun(&BaseClass::setValue), "setValue");
  chai.add(chaiscript::fun(&BaseClass::getValue), "getValue");
  chai.add(chaiscript::constructor<ChaiScriptDerived (const std::vector<chaiscript::Boxed_Value> &)>(), "ChaiScriptDerived");
  chai.add(chaiscript::base_class<BaseClass, ChaiScriptDerived>());
  chai.add(chaiscript::user_type<BaseClass>(), "BaseClass");
  chai.add(chaiscript::user_type<ChaiScriptDerived>(), "ChaiScriptDerived");

  std::string script = R""(
    def MakeDerived() {
      return ChaiScriptDerived(
          // create a dynamically created array and pass it in to the constructor
          [
          fun(this, f, d) {
            // see here that we are calling back into the 'this' pointer
            return "${this.getValue()}${f * d}";
          },

          fun(this, new_val) {
            if (new_val.size() < 5) {
              true;
            } else {
              print("String ${new_val} is too long");
              false;
            }
          }
          ]
          );
    }

    var myderived := MakeDerived(); // avoid a copy by using reference assignment :=

    )"";

  chai.eval(script);
  
  BaseClass &myderived = chai.eval<ChaiScriptDerived&>("myderived");

  // at this point in the code myderived is both a ChaiScript variable and a C++ variable. In both cases
  // it is a derivation of BaseClass, and the implementation is provided via ChaiScript functors
  // assigned in the MakeDerived() factory function
  //
  // Notice that our validateValue() function has a requirement that the new string be < 5 characters long

  myderived.setValue("1234");
  assert(myderived.getValue() == "1234");

  // chaiscript defined function will print out an error message and refuse to allow the setting
  myderived.setValue("12345");
  assert(myderived.getValue() == "1234");


  chai.eval("myderived.setValue(\"new\")"); // set the value via chaiscript
  assert(myderived.getValue() == "new");

  // call the other derived method via chaiscript and return the value to c++ land:
  std::string retval = chai.eval<std::string>("myderived.doSomething(2,4.3)");
  assert(retval == "new8.6");

  // The whole process is fully orthogonal
}


