#include <chaiscript/chaiscript.hpp>
#include <chaiscript/chaiscript_stdlib.hpp>
#include <chaiscript/dispatchkit/bootstrap_stl.hpp>
#include <chaiscript/dispatchkit/function_call.hpp>

int main( int /*argc*/ , char * /*argv*/[] )
{
  chaiscript::ChaiScript ch( chaiscript::Std_Lib::library( ) );


  try
  {
    static const char script[ ] =
      R""(

      class Rectangle
      {
        def Rectangle() { }
      }

    var rect = Rectangle( );

    )"";


    ch.eval( script );
  }
  catch ( const std::exception &e )
  {
    printf( " >>> Exception thrown: %s \n" , e.what( ) );
  }

  return 1;
}
