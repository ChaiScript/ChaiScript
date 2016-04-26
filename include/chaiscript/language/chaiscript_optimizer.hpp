// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2016, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_OPTIMIZER_HPP_
#define CHAISCRIPT_OPTIMIZER_HPP_

#include "chaiscript_eval.hpp"


namespace chaiscript {
  namespace optimizer {

    template<typename ... T>
      struct Optimizer : T...
    {
      Optimizer() = default;
      Optimizer(T ... t)
        : T(std::move(t))...
      {
      }

      AST_NodePtr optimize(AST_NodePtr p) {
        (void)std::initializer_list<int>{ (p = T::optimize(p), 0)... };
        return p;
      }
    };

    template<typename T>
      auto child_at(const T &node, const size_t offset) {
        if (node->identifier == AST_Node_Type::Compiled) {
          return dynamic_cast<const eval::Compiled_AST_Node&>(*node).m_original_node->children[offset];
        } else {
          return node->children[offset];
        }
      }

    template<typename T>
      auto child_count(const T &node) {
        if (node->identifier == AST_Node_Type::Compiled) {
          return dynamic_cast<const eval::Compiled_AST_Node&>(*node).m_original_node->children.size();
        } else {
          return node->children.size();
        }
      }

    template<typename T>
      AST_NodePtr make_compiled_node(const AST_NodePtr &original_node, std::vector<AST_NodePtr> children, T callable)
      {
        return chaiscript::make_shared<AST_Node, eval::Compiled_AST_Node>(original_node, std::move(children), std::move(callable));
      }


    struct Return {
      AST_NodePtr optimize(const AST_NodePtr &p)
      {
        if (p->identifier == AST_Node_Type::Def
            && !p->children.empty())
        {
          auto &last_child = p->children.back();
          if (last_child->identifier == AST_Node_Type::Block) {
            auto &block_last_child = last_child->children.back();
            if (block_last_child->identifier == AST_Node_Type::Return) {
              if (block_last_child->children.size() == 1) {
                last_child->children.back() = block_last_child->children[0];
              }
            }
          }
        }

        return p;
      }
    };

    template<typename T>
    bool contains_var_decl_in_scope(const T &node)
    {
      if (node->identifier == AST_Node_Type::Var_Decl) {
        return true;
      }

      const auto num = child_count(node);

      for (size_t i = 0; i < num; ++i) {
        const auto &child = child_at(node, i);
        if (child->identifier != AST_Node_Type::Block
            && contains_var_decl_in_scope(child)) {
          return true;
        }
      }

      return false;
    }

    struct Block {
      AST_NodePtr optimize(const AST_NodePtr &node) {
        if (node->identifier == AST_Node_Type::Block
            && node->children.size() == 1
            && !contains_var_decl_in_scope(node))
        {
          return node->children[0];
        } else {
          return node;
        }
      }
    };

    struct If {
      AST_NodePtr optimize(const AST_NodePtr &node) {
        if ((node->identifier == AST_Node_Type::If || node->identifier == AST_Node_Type::Ternary_Cond)
             && node->children.size() >= 2
             && node->children[0]->identifier == AST_Node_Type::Constant)
        {
          const auto condition = std::dynamic_pointer_cast<eval::Constant_AST_Node>(node->children[0])->m_value;
          if (condition.get_type_info().bare_equal_type_info(typeid(bool))) {
            if (boxed_cast<bool>(condition)) {
              return node->children[1];
            } else if (node->children.size() == 3) {
              return node->children[2];
            }
          }
        }

        return node;
      }
    };

    struct Constant_Fold {
      AST_NodePtr optimize(const AST_NodePtr &node) {

        if (node->identifier == AST_Node_Type::Binary
            && node->children.size() == 2
            && node->children[0]->identifier == AST_Node_Type::Constant
            && node->children[1]->identifier == AST_Node_Type::Constant)
        {
          try {
            const auto oper = node->text;
            const auto parsed = Operators::to_operator(oper);
            if (parsed != Operators::Opers::invalid) {
              const auto lhs = std::dynamic_pointer_cast<eval::Constant_AST_Node>(node->children[0])->m_value;
              const auto rhs = std::dynamic_pointer_cast<eval::Constant_AST_Node>(node->children[1])->m_value;
              if (lhs.get_type_info().is_arithmetic() && rhs.get_type_info().is_arithmetic()) {
                const auto val  = Boxed_Number::do_oper(parsed, lhs, rhs);
                const auto match = node->children[0]->text + " " + oper + " " + node->children[1]->text;
                return chaiscript::make_shared<AST_Node, eval::Constant_AST_Node>(std::move(match), node->location, std::move(val));
              }
            }
          } catch (const std::exception &) {
            //failure to fold, that's OK
          }
        }

        return node;
      }
    };

    struct For_Loop {
      AST_NodePtr optimize(const AST_NodePtr &for_node) {

        if (for_node->identifier != AST_Node_Type::For) {
          return for_node;
        }

        const auto eq_node = child_at(for_node, 0);
        const auto binary_node = child_at(for_node, 1);
        const auto prefix_node = child_at(for_node, 2);

        if (eq_node->identifier == AST_Node_Type::Equation
            && child_count(eq_node) == 2
            && child_at(eq_node, 0)->identifier == AST_Node_Type::Var_Decl
            && child_at(eq_node, 1)->identifier == AST_Node_Type::Constant
            && binary_node->identifier == AST_Node_Type::Binary
            && binary_node->text == "<"
            && child_count(binary_node) == 2
            && child_at(binary_node, 0)->identifier == AST_Node_Type::Id
            && child_at(binary_node, 0)->text == child_at(child_at(eq_node,0), 0)->text
            && child_at(binary_node, 1)->identifier == AST_Node_Type::Constant
            && prefix_node->identifier == AST_Node_Type::Prefix
            && prefix_node->text == "++"
            && child_count(prefix_node) == 1
            && child_at(prefix_node, 0)->identifier == AST_Node_Type::Id
            && child_at(prefix_node, 0)->text == child_at(child_at(eq_node,0), 0)->text)
        {
          const Boxed_Value &begin = std::dynamic_pointer_cast<const eval::Constant_AST_Node>(child_at(eq_node, 1))->m_value;
          const Boxed_Value &end = std::dynamic_pointer_cast<const eval::Constant_AST_Node>(child_at(binary_node, 1))->m_value;
          const std::string &id = child_at(prefix_node, 0)->text;

          if (begin.get_type_info().bare_equal(user_type<int>()) 
              && end.get_type_info().bare_equal(user_type<int>())) {

            const auto start_int = boxed_cast<int>(begin);
            const auto end_int = boxed_cast<int>(end);

            const auto body = child_at(for_node, 3);

            return make_compiled_node(for_node, {body}, 
                [id, start_int, end_int](const std::vector<AST_NodePtr> &children, const chaiscript::detail::Dispatch_State &t_ss) {
                  assert(children.size() == 1);
                  chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

                  int i = start_int;
                  t_ss.add_object(id, var(&i));

                  try {
                    for (; i < end_int; ++i) {
                      try {
                        // Body of Loop
                        children[0]->eval(t_ss);
                      } catch (eval::detail::Continue_Loop &) {
                        // we got a continue exception, which means all of the remaining 
                        // loop implementation is skipped and we just need to continue to
                        // the next iteration step
                      }
                    }
                  } catch (eval::detail::Break_Loop &) {
                    // loop broken
                  }

                  return void_var();
                }
            );
          } else {
            return for_node;
          }
        } else {
          return for_node;
        }
      }
    };


  }
}


#endif
