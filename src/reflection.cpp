
#include <chaiscript/chaiscript.hpp>
#include <chaiscript/utility/utility.hpp>
#include <string>


// MSVC doesn't like that we are using C++ return types from our C declared module
// but this is the best way to do it for cross platform compatibility
#ifdef BOOST_MSVC
#pragma warning(push)
#pragma warning(disable : 4190)
#endif


bool has_parse_tree(const chaiscript::Const_Proxy_Function &t_pf)
{
  boost::shared_ptr<const chaiscript::dispatch::Dynamic_Proxy_Function> pf
    = boost::dynamic_pointer_cast<const chaiscript::dispatch::Dynamic_Proxy_Function>(t_pf);
  if (pf)
  {
    return pf->get_parse_tree();
  } else {
    return false;
  }
}

chaiscript::AST_NodePtr get_parse_tree(const chaiscript::Const_Proxy_Function &t_pf)
{
  boost::shared_ptr<const chaiscript::dispatch::Dynamic_Proxy_Function> pf 
    = boost::dynamic_pointer_cast<const chaiscript::dispatch::Dynamic_Proxy_Function>(t_pf);
  if (pf)
  {
    if (pf->get_parse_tree())
    {
      return pf->get_parse_tree();
    } else {
      throw std::runtime_error("Function does not have a parse tree");
    }
  } else {
    throw std::runtime_error("Function does not have a parse tree");
  }
}


CHAISCRIPT_MODULE_EXPORT  chaiscript::ModulePtr create_chaiscript_module_reflection()
{
  chaiscript::ModulePtr m(new chaiscript::Module());

  m->add(chaiscript::fun(&has_parse_tree), "has_parse_tree");
  m->add(chaiscript::fun(&get_parse_tree), "get_parse_tree");

  m->add(chaiscript::base_class<std::exception, chaiscript::exception::eval_error>());

  chaiscript::bootstrap::standard_library::vector_type<std::vector<boost::shared_ptr<chaiscript::AST_Node> > >("AST_NodeVector", m);

  CHAISCRIPT_CLASS_NO_CONSTRUCTOR( m,
      chaiscript::exception::eval_error,
      ((reason))
      ((call_stack))
    );

  CHAISCRIPT_CLASS( m,
      chaiscript::File_Position,
      (chaiscript::File_Position())
      (chaiscript::File_Position(int,int)),
      ((line))
      ((column))
    );

  CHAISCRIPT_CLASS_NO_CONSTRUCTOR( m, 
      chaiscript::AST_Node,
      ((text))
      ((identifier))
      ((filename))
      ((start))
      ((end))
      ((internal_to_string))
      ((children))
      ((replace_child))
    );

  CHAISCRIPT_CLASS( m, 
      chaiscript::parser::ChaiScript_Parser,
      (chaiscript::parser::ChaiScript_Parser ()),
      ((parse))
      ((ast))
    );


  return m;
}


#ifdef BOOST_MSVC
#pragma warning(pop)
#endif
