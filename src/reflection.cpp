
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
  std::shared_ptr<const chaiscript::dispatch::Dynamic_Proxy_Function> pf
    = std::dynamic_pointer_cast<const chaiscript::dispatch::Dynamic_Proxy_Function>(t_pf);
  if (pf)
  {
    return bool(pf->get_parse_tree());
  } else {
    return false;
  }
}

chaiscript::AST_NodePtr get_parse_tree(const chaiscript::Const_Proxy_Function &t_pf)
{
  std::shared_ptr<const chaiscript::dispatch::Dynamic_Proxy_Function> pf 
    = std::dynamic_pointer_cast<const chaiscript::dispatch::Dynamic_Proxy_Function>(t_pf);
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


  chaiscript::bootstrap::standard_library::vector_type<std::vector<std::shared_ptr<chaiscript::AST_Node> > >("AST_NodeVector", m);

  using namespace chaiscript;

  chaiscript::utility::add_class<chaiscript::exception::eval_error>(*m,
      "eval_error",
      { },
      { {fun(&chaiscript::exception::eval_error::reason), "reason"},
        {fun(&chaiscript::exception::eval_error::call_stack), "call_stack"} }
      );

  chaiscript::utility::add_class<chaiscript::File_Position>(*m,
      "File_Position",
      { constructor<File_Position()>(),
        constructor<File_Position(int, int)>() },
      { {fun(&File_Position::line), "line"},
        {fun(&File_Position::column), "column"} }
      );

  chaiscript::utility::add_class<AST_Node>(*m,
      "AST_Node",
      {  },
      { {fun(&AST_Node::text), "text"},
        {fun(&AST_Node::identifier), "identifier"},
        {fun(&AST_Node::filename), "filename"},
        {fun(&AST_Node::start), "start"},
        {fun(&AST_Node::end), "end"},
        {fun(&AST_Node::internal_to_string), "internal_to_string"},
        {fun(&AST_Node::children), "children"},
        {fun(&AST_Node::replace_child), "replace_child"}
      }
    );

  chaiscript::utility::add_class<parser::ChaiScript_Parser>(*m,
      "ChaiScript_Parser",
      { constructor<parser::ChaiScript_Parser ()>() },
      { {fun(&parser::ChaiScript_Parser::parse), "parse"},
        {fun(&parser::ChaiScript_Parser::ast), "ast"} }
      );



  return m;
}


#ifdef BOOST_MSVC
#pragma warning(pop)
#endif
