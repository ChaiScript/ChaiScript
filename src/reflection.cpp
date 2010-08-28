
#include <chaiscript/chaiscript.hpp>
#include <chaiscript/utility/utility.hpp>
#include <string>

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
      ((children))
    );

  CHAISCRIPT_CLASS( m, 
      chaiscript::ChaiScript_Parser,
      (chaiscript::ChaiScript_Parser ()),
      ((parse))
      ((ast))
      ((show_match_stack))
    );


  return m;
}
