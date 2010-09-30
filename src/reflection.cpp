
#include <chaiscript/chaiscript.hpp>
#include <chaiscript/utility/utility.hpp>
#include <string>


// MSVC doesn't like that we are using C++ return types from our C declared module
// but this is the best way to do it for cross platform compatibility
#ifdef BOOST_MSVC
#pragma warning(push)
#pragma warning(disable : 4190)
#endif

CHAISCRIPT_MODULE_EXPORT  chaiscript::ModulePtr create_chaiscript_module_reflection()
{
  chaiscript::ModulePtr m(new chaiscript::Module());

  chaiscript::bootstrap::vector_type<std::vector<boost::shared_ptr<chaiscript::AST_Node> > >("AST_NodeVector", m);

  CHAISCRIPT_CLASS( m,
      chaiscript::File_Position,
      (chaiscript::File_Position())
      (chaiscript::File_Position(int,int)),
      ((line))
      ((column))
    );

  CHAISCRIPT_CLASS( m, 
      chaiscript::AST_Node,
      (chaiscript::AST_Node (const std::string &, int, char *)),
      ((text))
      ((identifier))
      ((filename))
      ((start))
      ((end))
      ((internal_to_string))
      ((children))
    );

  CHAISCRIPT_CLASS( m, 
      chaiscript::ChaiScript_Parser,
      (chaiscript::ChaiScript_Parser ()),
      ((parse))
      ((ast))
    );


  return m;
}


#ifdef BOOST_MSVC
#pragma warning(pop)
#endif
