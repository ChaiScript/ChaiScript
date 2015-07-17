// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2015, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_PARSER_HPP_
#define CHAISCRIPT_PARSER_HPP_

#include <exception>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>
#include <cstring>


#include "../dispatchkit/boxed_value.hpp"
#include "chaiscript_common.hpp"

namespace chaiscript
{
  /// \brief Classes and functions used during the parsing process.
  namespace parser
  {
    /// \brief Classes and functions internal to the parsing process. Not supported for the end user.
    namespace detail 
    {
      enum Alphabet
      {   symbol_alphabet = 0
        ,   keyword_alphabet
          ,   int_alphabet
          ,   float_alphabet
          ,   x_alphabet
          ,   hex_alphabet
          ,   b_alphabet
          ,   bin_alphabet
          ,   id_alphabet
          ,   white_alphabet
          ,   int_suffix_alphabet
          ,   float_suffix_alphabet
          ,   max_alphabet
          ,   lengthof_alphabet = 256
      };
    }

    class ChaiScript_Parser {

      std::string::const_iterator m_input_pos, m_input_end;
      int m_line, m_col;
      std::string m_multiline_comment_begin;
      std::string m_multiline_comment_end;
      std::string m_singleline_comment;
      std::shared_ptr<std::string> m_filename;
      std::vector<AST_NodePtr> m_match_stack;
      bool m_alphabet[detail::max_alphabet][detail::lengthof_alphabet];

      std::vector<std::vector<std::string>> m_operator_matches;
      std::vector<AST_Node_Type::Type> m_operators;

      public:
      ChaiScript_Parser()
        : m_line(-1), m_col(-1),
          m_multiline_comment_begin("/*"),
          m_multiline_comment_end("*/"),
          m_singleline_comment("//")
      {
        m_match_stack.reserve(2);
        setup_operators();
      }

      ChaiScript_Parser(const ChaiScript_Parser &) = delete;
      ChaiScript_Parser &operator=(const ChaiScript_Parser &) = delete;

      void setup_operators()
      {
        m_operators.emplace_back(AST_Node_Type::Ternary_Cond);
        m_operator_matches.emplace_back(std::initializer_list<std::string>({"?"}));

        m_operators.emplace_back(AST_Node_Type::Logical_Or);
        m_operator_matches.emplace_back(std::initializer_list<std::string>({"||"}));

        m_operators.emplace_back(AST_Node_Type::Logical_And);
        m_operator_matches.emplace_back(std::initializer_list<std::string>({"&&"}));

        m_operators.emplace_back(AST_Node_Type::Bitwise_Or);
        m_operator_matches.emplace_back(std::initializer_list<std::string>({"|"}));

        m_operators.emplace_back(AST_Node_Type::Bitwise_Xor);
        m_operator_matches.emplace_back(std::initializer_list<std::string>({"^"}));

        m_operators.emplace_back(AST_Node_Type::Bitwise_And);
        m_operator_matches.emplace_back(std::initializer_list<std::string>({"&"}));

        m_operators.emplace_back(AST_Node_Type::Equality);
        m_operator_matches.emplace_back(std::initializer_list<std::string>({"==", "!="}));

        m_operators.emplace_back(AST_Node_Type::Comparison);
        m_operator_matches.emplace_back(std::initializer_list<std::string>({"<", "<=", ">", ">="}));

        m_operators.emplace_back(AST_Node_Type::Shift);
        m_operator_matches.emplace_back(std::initializer_list<std::string>({"<<", ">>"}));

        //We share precedence here but then separate them later
        m_operators.emplace_back(AST_Node_Type::Addition);
        m_operator_matches.emplace_back(std::initializer_list<std::string>({"+", "-"}));

        //We share precedence here but then separate them later
        m_operators.emplace_back(AST_Node_Type::Multiplication);
        m_operator_matches.emplace_back(std::initializer_list<std::string>({"*", "/", "%"}));

        for (auto & elem : m_alphabet) {
          std::fill(std::begin(elem), std::end(elem), false);
        }

        m_alphabet[detail::symbol_alphabet][static_cast<int>('?')]=true;
        m_alphabet[detail::symbol_alphabet][static_cast<int>('+')]=true;
        m_alphabet[detail::symbol_alphabet][static_cast<int>('-')]=true;
        m_alphabet[detail::symbol_alphabet][static_cast<int>('*')]=true;
        m_alphabet[detail::symbol_alphabet][static_cast<int>('/')]=true;
        m_alphabet[detail::symbol_alphabet][static_cast<int>('|')]=true;
        m_alphabet[detail::symbol_alphabet][static_cast<int>('&')]=true;
        m_alphabet[detail::symbol_alphabet][static_cast<int>('^')]=true;
        m_alphabet[detail::symbol_alphabet][static_cast<int>('=')]=true;
        m_alphabet[detail::symbol_alphabet][static_cast<int>('.')]=true;
        m_alphabet[detail::symbol_alphabet][static_cast<int>('<')]=true;
        m_alphabet[detail::symbol_alphabet][static_cast<int>('>')]=true;

        for ( int c = 'a' ; c <= 'z' ; ++c ) { m_alphabet[detail::keyword_alphabet][c]=true; }
        for ( int c = 'A' ; c <= 'Z' ; ++c ) { m_alphabet[detail::keyword_alphabet][c]=true; }
        for ( int c = '0' ; c <= '9' ; ++c ) { m_alphabet[detail::keyword_alphabet][c]=true; }
        m_alphabet[detail::keyword_alphabet][static_cast<int>('_')]=true;

        for ( int c = '0' ; c <= '9' ; ++c ) { m_alphabet[detail::int_alphabet][c]=true; }
        for ( int c = '0' ; c <= '9' ; ++c ) { m_alphabet[detail::float_alphabet][c]=true; }
        m_alphabet[detail::float_alphabet][static_cast<int>('.')]=true;

        for ( int c = '0' ; c <= '9' ; ++c ) { m_alphabet[detail::hex_alphabet][c]=true; }
        for ( int c = 'a' ; c <= 'f' ; ++c ) { m_alphabet[detail::hex_alphabet][c]=true; }
        for ( int c = 'A' ; c <= 'F' ; ++c ) { m_alphabet[detail::hex_alphabet][c]=true; }

        m_alphabet[detail::x_alphabet][static_cast<int>('x')]=true;
        m_alphabet[detail::x_alphabet][static_cast<int>('X')]=true;

        for ( int c = '0' ; c <= '1' ; ++c ) { m_alphabet[detail::bin_alphabet][c]=true; }
        m_alphabet[detail::b_alphabet][static_cast<int>('b')]=true;
        m_alphabet[detail::b_alphabet][static_cast<int>('B')]=true;

        for ( int c = 'a' ; c <= 'z' ; ++c ) { m_alphabet[detail::id_alphabet][c]=true; }
        for ( int c = 'A' ; c <= 'Z' ; ++c ) { m_alphabet[detail::id_alphabet][c]=true; }
        m_alphabet[detail::id_alphabet][static_cast<int>('_')] = true;

        m_alphabet[detail::white_alphabet][static_cast<int>(' ')]=true;
        m_alphabet[detail::white_alphabet][static_cast<int>('\t')]=true;

        m_alphabet[detail::int_suffix_alphabet][static_cast<int>('l')] = true;
        m_alphabet[detail::int_suffix_alphabet][static_cast<int>('L')] = true;
        m_alphabet[detail::int_suffix_alphabet][static_cast<int>('u')] = true;
        m_alphabet[detail::int_suffix_alphabet][static_cast<int>('U')] = true;

        m_alphabet[detail::float_suffix_alphabet][static_cast<int>('l')] = true;
        m_alphabet[detail::float_suffix_alphabet][static_cast<int>('L')] = true;
        m_alphabet[detail::float_suffix_alphabet][static_cast<int>('f')] = true;
        m_alphabet[detail::float_suffix_alphabet][static_cast<int>('F')] = true;
      }

      /// test a char in an m_alphabet
      bool char_in_alphabet(char c, detail::Alphabet a) const { return m_alphabet[a][static_cast<int>(c)]; }

      /// Prints the parsed ast_nodes as a tree
      /*
         void debug_print(AST_NodePtr t, std::string prepend = "") {
         std::cout << prepend << "(" << ast_node_type_to_string(t->identifier) << ") " << t->text << " : " << t->start.line << ", " << t->start.column << '\n';
         for (unsigned int j = 0; j < t->children.size(); ++j) {
         debug_print(t->children[j], prepend + "  ");
         }
         }
         */

      /// Shows the current stack of matched ast_nodes
      void show_match_stack() const {
        for (auto & elem : m_match_stack) {
          //debug_print(match_stack[i]);
          std::cout << elem->to_string();
        }
      }

      /// Clears the stack of matched ast_nodes
      void clear_match_stack() {
        m_match_stack.clear();
      }

      /// Returns the front-most AST node
      AST_NodePtr ast() const {
        if (m_match_stack.empty()) throw exception::eval_error("Attempted to access AST of failed parse.");
        return m_match_stack.front();
      }

      static std::map<std::string, int> count_fun_calls(const AST_NodePtr &p, bool in_loop) {
        if (p->identifier == AST_Node_Type::Fun_Call) {
          if (p->children[0]->identifier == AST_Node_Type::Id) {
            return std::map<std::string, int>{{p->children[0]->text, in_loop?99:1}};
          }
          return std::map<std::string, int>();
        } else {
          std::map<std::string, int> counts;
          for (const auto &child : p->children) {
            auto childcounts = count_fun_calls(child, in_loop || p->identifier == AST_Node_Type::For || p->identifier == AST_Node_Type::While);
            for (const auto &count : childcounts) {
              counts[count.first] += count.second;
            }
          }
          return counts;
        }

      }


      static void optimize_blocks(AST_NodePtr &p)
      {
        for (auto &c : p->children)
        {
          if (c->identifier == AST_Node_Type::Block) {
            if (c->children.size() == 1) {
              // std::cout << "swapping out block child for block\n";
              c = c->children[0];
            }
          }
          optimize_blocks(c);
        }
      }

      static void optimize_returns(AST_NodePtr &p)
      {
        for (auto &c : p->children)
        {
          if (c->identifier == AST_Node_Type::Def && c->children.size() > 0) {
            auto &last_child = c->children.back();
            if (last_child->identifier == AST_Node_Type::Block) {
              auto &block_last_child = last_child->children.back();
              if (block_last_child->identifier == AST_Node_Type::Return) {
                if (block_last_child->children.size() == 1) {
                  block_last_child = block_last_child->children[0];
                }
              }
            }
          }
          optimize_returns(c);
        }
      }


      static int count_nodes(const AST_NodePtr &p)
      {
        int count = 1;
        for (auto &c : p->children) {
          count += count_nodes(c);
        }
        return count;
      }

      AST_NodePtr optimized_ast(bool t_optimize_blocks = false, bool t_optimize_returns = true) {
        AST_NodePtr p = ast();
        //Note, optimize_blocks is currently broken; it breaks stack management
        if (t_optimize_blocks) { optimize_blocks(p); }
        if (t_optimize_returns) { optimize_returns(p); }
        return p;
      }


      /// Helper function that collects ast_nodes from a starting position to the top of the stack into a new AST node
      template<typename NodeType>
      void build_match(size_t t_match_start, std::string t_text = "") {
        bool is_deep = false;

        Parse_Location filepos = [&]()->Parse_Location{ 
          //so we want to take everything to the right of this and make them children
          if (t_match_start != m_match_stack.size()) {
            is_deep = true;
            return Parse_Location(
                m_filename,
                m_match_stack[t_match_start]->location.start.line,
                m_match_stack[t_match_start]->location.start.column,
                m_line,
                m_col
              );
          } else {
            return Parse_Location(
                m_filename,
                m_line,
                m_col,
                m_line,
                m_col
              );
          }
        }();

        std::vector<AST_NodePtr> new_children;

        if (is_deep) {
          new_children.assign(std::make_move_iterator(m_match_stack.begin() + static_cast<int>(t_match_start)), 
                              std::make_move_iterator(m_match_stack.end()));
          m_match_stack.erase(m_match_stack.begin() + static_cast<int>(t_match_start), m_match_stack.end());
        }

        /// \todo fix the fact that a successful match that captured no ast_nodes doesn't have any real start position
        m_match_stack.push_back(
            chaiscript::make_shared<chaiscript::AST_Node, NodeType>(
              std::move(t_text),
              std::move(filepos),
              std::move(new_children)));
      }

      /// Check to see if there is more text parse
      inline bool has_more_input() const {
        return (m_input_pos != m_input_end);
      }

      /// Skips any multi-line or single-line comment
      bool SkipComment() {
        if (Symbol_(m_multiline_comment_begin.c_str())) {
          while (m_input_pos != m_input_end) {
            if (Symbol_(m_multiline_comment_end.c_str())) {
              break;
            } else if (!Eol_()) {
              ++m_col;
              ++m_input_pos;
            }
          }
          return true;
        } else if (Symbol_(m_singleline_comment.c_str())) {
          while (m_input_pos != m_input_end) {
            if (Symbol_("\r\n")) {
              m_input_pos -= 2;
              break;
            } else if (Char_('\n')) {
              --m_input_pos;
              break;
            } else {
              ++m_col;
              ++m_input_pos;
            }
          }
          return true;
        }
        return false;
      }


      /// Skips ChaiScript whitespace, which means space and tab, but not cr/lf
      /// jespada: Modified SkipWS to skip optionally CR ('\n') and/or LF+CR ("\r\n")
      bool SkipWS(bool skip_cr=false) {
        bool retval = false;

        while (has_more_input()) {
          auto end_line = (*m_input_pos != 0) && ((*m_input_pos == '\n') || (*m_input_pos == '\r' && *(m_input_pos+1) == '\n'));

          if ( char_in_alphabet(*m_input_pos,detail::white_alphabet) || (skip_cr && end_line)) {

            if(end_line) {
              m_col = 1;
              ++m_line;

              if(*(m_input_pos) == '\r') {
                // discards lf
                ++m_input_pos;
              }
            }
            else {
              ++m_col;
            }
            ++m_input_pos;

            retval = true;
          }
          else if (SkipComment()) {
            retval = true;
          } else {
            break;
          }
        }
        return retval;
      }

      /// Reads the optional exponent (scientific notation) and suffix for a Float
      bool read_exponent_and_suffix() {
        // Support a form of scientific notation: 1e-5, 35.5E+8, 0.01e19
        if (has_more_input() && (std::tolower(*m_input_pos) == 'e')) {
          ++m_input_pos;
          ++m_col;
          if (has_more_input() && ((*m_input_pos == '-') || (*m_input_pos == '+'))) {
            ++m_input_pos;
            ++m_col;
          }
          auto exponent_pos = m_input_pos;
          while (has_more_input() && char_in_alphabet(*m_input_pos,detail::int_alphabet) ) {
            ++m_input_pos;
            ++m_col;
          }
          if (m_input_pos == exponent_pos) {
            // Require at least one digit after the exponent
            return false;
          }
        }

        // Parse optional float suffix
        while (has_more_input() && char_in_alphabet(*m_input_pos, detail::float_suffix_alphabet))
        {
          ++m_input_pos;
          ++m_col;
        }

        return true;
      }


      /// Reads a floating point value from input, without skipping initial whitespace
      bool Float_() {
        if (has_more_input() && char_in_alphabet(*m_input_pos,detail::float_alphabet) ) {
          while (has_more_input() && char_in_alphabet(*m_input_pos,detail::int_alphabet) ) {
            ++m_input_pos;
            ++m_col;
          }

          if (has_more_input() && (std::tolower(*m_input_pos) == 'e')) {
            // The exponent is valid even without any decimal in the Float (1e8, 3e-15)
            return read_exponent_and_suffix();
          }
          else if (has_more_input() && (*m_input_pos == '.')) {
            ++m_input_pos;
            ++m_col;
            if (has_more_input() && char_in_alphabet(*m_input_pos,detail::int_alphabet)) {
              while (has_more_input() && char_in_alphabet(*m_input_pos,detail::int_alphabet) ) {
                ++m_input_pos;
                ++m_col;
              }

              // After any decimal digits, support an optional exponent (3.7e3)
              return read_exponent_and_suffix();
            } else {
              --m_input_pos;
              --m_col;
            }
          }
        }
        return false;
      }

      /// Reads a hex value from input, without skipping initial whitespace
      bool Hex_() {
        if (has_more_input() && (*m_input_pos == '0')) {
          ++m_input_pos;
          ++m_col;

          if (has_more_input() && char_in_alphabet(*m_input_pos, detail::x_alphabet) ) {
            ++m_input_pos;
            ++m_col;
            if (has_more_input() && char_in_alphabet(*m_input_pos, detail::hex_alphabet)) {
              while (has_more_input() && char_in_alphabet(*m_input_pos, detail::hex_alphabet) ) {
                ++m_input_pos;
                ++m_col;
              }
              while (has_more_input() && char_in_alphabet(*m_input_pos, detail::int_suffix_alphabet))
              {
                ++m_input_pos;
                ++m_col;
              }

              return true;
            }
            else {
              --m_input_pos;
              --m_col;
            }
          }
          else {
            --m_input_pos;
            --m_col;
          }
        }

        return false;
      }

      /// Reads an integer suffix
      void IntSuffix_() {
        while (has_more_input() && char_in_alphabet(*m_input_pos, detail::int_suffix_alphabet))
        {
          ++m_input_pos;
          ++m_col;
        }
      }

      /// Reads a binary value from input, without skipping initial whitespace
      bool Binary_() {
        if (has_more_input() && (*m_input_pos == '0')) {
          ++m_input_pos;
          ++m_col;

          if (has_more_input() && char_in_alphabet(*m_input_pos, detail::b_alphabet) ) {
            ++m_input_pos;
            ++m_col;
            if (has_more_input() && char_in_alphabet(*m_input_pos, detail::bin_alphabet) ) {
              while (has_more_input() && char_in_alphabet(*m_input_pos, detail::bin_alphabet) ) {
                ++m_input_pos;
                ++m_col;
              }
              return true;
            } else {
              --m_input_pos;
              --m_col;
            }
          } else {
            --m_input_pos;
            --m_col;
          }
        }

        return false;
      }

      /// Parses a floating point value and returns a Boxed_Value representation of it
      static Boxed_Value buildFloat(const std::string &t_val)
      {
        bool float_ = false;
        bool long_ = false;

        auto i = t_val.size();

        for (; i > 0; --i)
        {
          char val = t_val[i-1];

          if (val == 'f' || val == 'F')
          {
            float_ = true;
          } else if (val == 'l' || val == 'L') {
            long_ = true;
          } else {
            break;
          }
        }

        if (float_)
        {
          return const_var(std::stof(t_val.substr(0,i)));
        } else if (long_) {
          return const_var(std::stold(t_val.substr(0,i)));
        } else {
          return const_var(std::stod(t_val.substr(0,i)));
        }
      }



      template<typename IntType>
      static Boxed_Value buildInt(const IntType &t_type, const std::string &t_val)
      {
        bool unsigned_ = false;
        bool long_ = false;
        bool longlong_ = false;

        auto i = t_val.size();

        for (; i > 0; --i)
        {
          const char val = t_val[i-1];

          if (val == 'u' || val == 'U')
          {
            unsigned_ = true;
          } else if (val == 'l' || val == 'L') {
            if (long_)
            {
              longlong_ = true;
            }

            long_ = true;
          } else {
            break;
          }
        }

        std::stringstream ss(t_val.substr(0, i));
        ss >> t_type;

        std::stringstream testu(t_val.substr(0, i));
        uint64_t u;
        testu >> t_type >> u;

        bool unsignedrequired = false;

        if ((u >> (sizeof(int) * 8)) > 0)
        {
          //requires something bigger than int
          long_ = true;
        }

        static_assert(sizeof(long) == sizeof(uint64_t) || sizeof(long) * 2 == sizeof(uint64_t), "Unexpected sizing of integer types");

        if ((sizeof(long) < sizeof(uint64_t)) 
            && (u >> ((sizeof(uint64_t) - sizeof(long)) * 8)) > 0)
        {
          //requires something bigger than long
          longlong_ = true;
        }


        const size_t size = [&]()->size_t{
          if (longlong_)
          {
            return sizeof(int64_t) * 8;
          } else if (long_) {
            return sizeof(long) * 8;
          } else {
            return sizeof(int) * 8;
          }
        }();

        if ( (u >> (size - 1)) > 0)
        {
          unsignedrequired = true;
        }

        if (unsignedrequired && !unsigned_)
        {
          if (t_type == &std::hex || t_type == &std::oct)
          {
            // with hex and octal we are happy to just make it unsigned
            unsigned_ = true;
          } else {
            // with decimal we must bump it up to the next size
            if (long_)
            {
              longlong_ = true;
            } else if (!long_ && !longlong_) {
              long_ = true;
            }
          }
        }

        if (unsigned_)
        {
          if (longlong_)
          {
            uint64_t val;
            ss >> val;
            return const_var(val);
          } else if (long_) {
            unsigned long val;
            ss >> val;
            return const_var(val);
          } else {
            unsigned int val;
            ss >> val;
            return const_var(val);
          }
        } else {
          if (longlong_)
          {
            int64_t val;
            ss >> val;
            return const_var(val);
          } else if (long_) {
            long val;
            ss >> val;
            return const_var(val);
          } else {
            int val;
            ss >> val;
            return const_var(val);
          }
        }
      }

      template<typename T, typename ... Param>
      std::shared_ptr<AST_Node> make_node(std::string t_match, const int t_prev_line, const int t_prev_col, Param && ...param)
      {
        return chaiscript::make_shared<AST_Node, T>(std::move(t_match), Parse_Location(m_filename, t_prev_line, t_prev_col, m_line, m_col), std::forward<Param>(param)...);
      }

      /// Reads a number from the input, detecting if it's an integer or floating point
      bool Num(const bool t_capture = false) {
        SkipWS();

        if (!t_capture) {
          return Hex_() || Float_();
        } else {
          const auto start = m_input_pos;
          const auto prev_col = m_col;
          const auto prev_line = m_line;
          if (has_more_input() && char_in_alphabet(*m_input_pos, detail::float_alphabet) ) {
            if (Hex_()) {
              std::string match(start, m_input_pos);
              auto bv = buildInt(std::hex, match);
              m_match_stack.emplace_back(make_node<eval::Int_AST_Node>(std::move(match), prev_line, prev_col, std::move(bv)));
              return true;
            }

            if (Binary_()) {
              std::string match(start, m_input_pos);
              int64_t temp_int = 0;
              size_t pos = 0;
              const auto end = match.length();

              while ((pos < end) && (pos < (2 + sizeof(int) * 8))) {
                temp_int <<= 1;
                if (match[pos] == '1') {
                  temp_int += 1;
                }
                ++pos;
              }

              Boxed_Value i = [&]()->Boxed_Value{
                if (match.length() <= sizeof(int) * 8)
                {
                  return const_var(static_cast<int>(temp_int));
                } else {
                  return const_var(temp_int);
                }
              }();

              m_match_stack.push_back(make_node<eval::Int_AST_Node>(std::move(match), prev_line, prev_col, std::move(i)));
              return true;
            }
            if (Float_()) {
              std::string match(start, m_input_pos);
              auto bv = buildFloat(match);
              m_match_stack.push_back(make_node<eval::Float_AST_Node>(std::move(match), prev_line, prev_col, std::move(bv)));
              return true;
            }
            else {
              IntSuffix_();
              std::string match(start, m_input_pos);
              if (!match.empty() && (match[0] == '0')) {
                auto bv = buildInt(std::oct, match);
                m_match_stack.push_back(make_node<eval::Int_AST_Node>(std::move(match), prev_line, prev_col, std::move(bv)));
              }
              else {
                auto bv = buildInt(std::dec, match);
                m_match_stack.push_back(make_node<eval::Int_AST_Node>(std::move(match), prev_line, prev_col, std::move(bv)));
              }
              return true;
            }
          }
          else {
            return false;
          }
        }
      }

      /// Reads an identifier from input which conforms to C's identifier naming conventions, without skipping initial whitespace
      bool Id_() {
        if (has_more_input() && char_in_alphabet(*m_input_pos, detail::id_alphabet)) {
          while (has_more_input() && char_in_alphabet(*m_input_pos, detail::keyword_alphabet) ) {
            ++m_input_pos;
            ++m_col;
          }

          return true;
        } else if (has_more_input() && (*m_input_pos == '`')) {
          ++m_col;
          ++m_input_pos;
          const auto start = m_input_pos;

          while (has_more_input() && (*m_input_pos != '`')) {
            if (Eol()) {
              throw exception::eval_error("Carriage return in identifier literal", File_Position(m_line, m_col), *m_filename);
            }
            else {
              ++m_input_pos;
              ++m_col;
            }
          }

          if (start == m_input_pos) {
            throw exception::eval_error("Missing contents of identifier literal", File_Position(m_line, m_col), *m_filename);
          }
          else if (m_input_pos == m_input_end) {
            throw exception::eval_error("Incomplete identifier literal", File_Position(m_line, m_col), *m_filename);
          }

          ++m_col;
          ++m_input_pos;

          return true;
        }
        return false;
      }

      /// Reads (and potentially captures) an identifier from input
      bool Id() {
        SkipWS();

        const auto start = m_input_pos;
        const auto prev_col = m_col;
        const auto prev_line = m_line;
        if (Id_()) {
          m_match_stack.push_back(make_node<eval::Id_AST_Node>(
                [&]()->std::string{
                  if (*start == '`') {
                    //Id Literal
                    return std::string(start+1, m_input_pos-1);
                  } else {
                    return std::string(start, m_input_pos);
                  }
                }(),
                prev_line, prev_col));
          return true;
        } else {
          return false;
        }
      }

      /// Reads an argument from input
      bool Arg(const bool t_type_allowed = true) {
        const auto prev_stack_top = m_match_stack.size();
        SkipWS();

        if (!Id()) {
          return false;
        }

        SkipWS();

        if (t_type_allowed) {
          Id();
        }

        build_match<eval::Arg_AST_Node>(prev_stack_top);

        return true;
      }



      /// Checks for a node annotation of the form "#<annotation>"
      bool Annotation() {
        SkipWS();
        const auto start = m_input_pos;
        const auto prev_col = m_col;
        const auto prev_line = m_line;
        if (Symbol_("#")) {
          do {
            while (m_input_pos != m_input_end) {
              if (Eol_()) {
                break;
              }
              else {
                ++m_col;
                ++m_input_pos;
              }
            }
          } while (Symbol("#"));

          std::string match(start, m_input_pos);
          m_match_stack.push_back(make_node<eval::Annotation_AST_Node>(std::move(match), prev_line, prev_col));
          return true;
        }
        else {
          return false;
        }
      }

      /// Reads a quoted string from input, without skipping initial whitespace
      bool Quoted_String_() {
        if (has_more_input() && (*m_input_pos == '\"')) {
          char prev_char = *m_input_pos;
          ++m_input_pos;
          ++m_col;

          while (has_more_input() && ((*m_input_pos != '\"') || ((*m_input_pos == '\"') && (prev_char == '\\')))) {
            if (!Eol_()) {
              if (prev_char == '\\') {
                prev_char = 0;
              } else {
                prev_char = *m_input_pos;
              }
              ++m_input_pos;
              ++m_col;
            }
          }

          if (has_more_input()) {
            ++m_input_pos;
            ++m_col;
          } else {
            throw exception::eval_error("Unclosed quoted string", File_Position(m_line, m_col), *m_filename);
          }

          return true;
        }
        return false;
      }

      /// Reads (and potentially captures) a quoted string from input.  Translates escaped sequences.
      bool Quoted_String(const bool t_capture = false) {
        SkipWS();

        if (!t_capture) {
          return Quoted_String_();
        } else {
          const auto start = m_input_pos;
          const auto prev_col = m_col;
          const auto prev_line = m_line;

          if (Quoted_String_()) {
            std::string match;
            bool is_escaped = false;
            bool is_interpolated = false;
            bool saw_interpolation_marker = false;
            const auto prev_stack_top = m_match_stack.size();

            std::string::const_iterator s = start + 1, end = m_input_pos - 1;

            while (s != end) {
              if (saw_interpolation_marker) {
                if (*s == '{') {
                  //We've found an interpolation point

                  if (is_interpolated) {
                    //If we've seen previous interpolation, add on instead of making a new one
                    m_match_stack.push_back(make_node<eval::Quoted_String_AST_Node>(match, prev_line, prev_col));

                    build_match<eval::Binary_Operator_AST_Node>(prev_stack_top, "+");
                  } else {
                    m_match_stack.push_back(make_node<eval::Quoted_String_AST_Node>(match, prev_line, prev_col));
                  }

                  //We've finished with the part of the string up to this point, so clear it
                  match.clear();

                  std::string eval_match;

                  ++s;
                  while ((s != end) && (*s != '}')) {
                    eval_match.push_back(*s);
                    ++s;
                  }

                  if (*s == '}') {
                    is_interpolated = true;
                    ++s;

                    const auto tostr_stack_top = m_match_stack.size();

                    m_match_stack.push_back(make_node<eval::Id_AST_Node>("to_string", prev_line, prev_col));

                    const auto ev_stack_top = m_match_stack.size();

                    /// \todo can we evaluate this in place and save the runtime cost of evaluating with each execution of the node?
                    m_match_stack.push_back(make_node<eval::Id_AST_Node>("eval", prev_line, prev_col));

                    const auto arg_stack_top = m_match_stack.size();

                    m_match_stack.push_back(make_node<eval::Quoted_String_AST_Node>(eval_match, prev_line, prev_col));

                    build_match<eval::Arg_List_AST_Node>(arg_stack_top);
                    build_match<eval::Inplace_Fun_Call_AST_Node>(ev_stack_top);
                    build_match<eval::Arg_List_AST_Node>(ev_stack_top);
                    build_match<eval::Fun_Call_AST_Node>(tostr_stack_top);
                    build_match<eval::Binary_Operator_AST_Node>(prev_stack_top, "+");
                  } else {
                    throw exception::eval_error("Unclosed in-string eval", File_Position(prev_line, prev_col), *m_filename);
                  }
                } else {
                  match.push_back('$');
                }
                saw_interpolation_marker = false;
              } else {
                if (*s == '\\') {
                  if (is_escaped) {
                    match.push_back('\\');
                    is_escaped = false;
                  } else {
                    is_escaped = true;
                  }
                } else {
                  if (is_escaped) {
                    switch (*s) {
                      case ('b') : match.push_back('\b'); break;
                      case ('f') : match.push_back('\f'); break;
                      case ('n') : match.push_back('\n'); break;
                      case ('r') : match.push_back('\r'); break;
                      case ('t') : match.push_back('\t'); break;
                      case ('\'') : match.push_back('\''); break;
                      case ('\"') : match.push_back('\"'); break;
                      case ('$') : match.push_back('$'); break;
                      default: throw exception::eval_error("Unknown escaped sequence in string", File_Position(prev_line, prev_col), *m_filename);
                    }
                  } else if (*s == '$') {
                    saw_interpolation_marker = true;
                  } else {
                    match.push_back(*s);
                  }
                  is_escaped = false;
                }
                ++s;
              }
            }

            if (is_interpolated) {
              m_match_stack.push_back(make_node<eval::Quoted_String_AST_Node>(match, prev_line, prev_col));

              build_match<eval::Binary_Operator_AST_Node>(prev_stack_top, "+");
            } else {
              m_match_stack.push_back(make_node<eval::Quoted_String_AST_Node>(match, prev_line, prev_col));
            }
            return true;
          } else {
            return false;
          }
        }
      }

      /// Reads a character group from input, without skipping initial whitespace
      bool Single_Quoted_String_() {
        bool retval = false;
        if (has_more_input() && (*m_input_pos == '\'')) {
          retval = true;
          char prev_char = *m_input_pos;
          ++m_input_pos;
          ++m_col;

          while (has_more_input() && ((*m_input_pos != '\'') || ((*m_input_pos == '\'') && (prev_char == '\\')))) {
            if (!Eol_()) {
              if (prev_char == '\\') {
                prev_char = 0;
              } else {
                prev_char = *m_input_pos;
              }
              ++m_input_pos;
              ++m_col;
            }
          }

          if (m_input_pos != m_input_end) {
            ++m_input_pos;
            ++m_col;
          } else {
            throw exception::eval_error("Unclosed single-quoted string", File_Position(m_line, m_col), *m_filename);
          }
        }
        return retval;
      }

      /// Reads (and potentially captures) a char group from input.  Translates escaped sequences.
      bool Single_Quoted_String(const bool t_capture = false) {
        SkipWS();

        if (!t_capture) {
          return Single_Quoted_String_();
        } else {
          const auto start = m_input_pos;
          const auto prev_col = m_col;
          const auto prev_line = m_line;
          if (Single_Quoted_String_()) {
            std::string match;
            bool is_escaped = false;
            for (auto s = start + 1, end = m_input_pos - 1; s != end; ++s) {
              if (*s == '\\') {
                if (is_escaped) {
                  match.push_back('\\');
                  is_escaped = false;
                } else {
                  is_escaped = true;
                }
              } else {
                if (is_escaped) {
                  switch (*s) {
                    case ('b') : match.push_back('\b'); break;
                    case ('f') : match.push_back('\f'); break;
                    case ('n') : match.push_back('\n'); break;
                    case ('r') : match.push_back('\r'); break;
                    case ('t') : match.push_back('\t'); break;
                    case ('\'') : match.push_back('\''); break;
                    case ('\"') : match.push_back('\"'); break;
                    default: throw exception::eval_error("Unknown escaped sequence in string", File_Position(prev_line, prev_col), *m_filename);
                  }
                } else {
                  match.push_back(*s);
                }
                is_escaped = false;
              }
            }
            m_match_stack.push_back(make_node<eval::Single_Quoted_String_AST_Node>(match, prev_line, prev_col));
            return true;
          }
          else {
            return false;
          }
        }
      }

      /// Reads a char from input if it matches the parameter, without skipping initial whitespace
      bool Char_(const char c) {
        if (has_more_input() && (*m_input_pos == c)) {
          ++m_input_pos;
          ++m_col;
          return true;
        } else {
          return false;
        }
      }

      /// Reads (and potentially captures) a char from input if it matches the parameter
      bool Char(const char t_c, bool t_capture = false) {
        SkipWS();

        if (!t_capture) {
          return Char_(t_c);
        } else {
          const auto start = m_input_pos;
          const auto prev_col = m_col;
          const auto prev_line = m_line;
          if (Char_(t_c)) {
            m_match_stack.push_back(make_node<eval::Char_AST_Node>(std::string(start, m_input_pos), prev_line, prev_col));
            return true;
          } else {
            return false;
          }
        }
      }

      /// Reads a string from input if it matches the parameter, without skipping initial whitespace
      bool Keyword_(const char *t_s) {
        const auto len = strlen(t_s);

        if ((m_input_end - m_input_pos) >= static_cast<std::make_signed<size_t>::type>(len)) {
          auto tmp = m_input_pos;
          for (size_t i = 0; i < len; ++i) {
            if (*tmp != t_s[i]) {
              return false;
            }
            ++tmp;
          }
          m_input_pos = tmp;
          m_col += static_cast<int>(len);
          return true;
        }

        return false;
      }

      /// Reads (and potentially captures) a string from input if it matches the parameter
      bool Keyword(const char *t_s, bool t_capture = false) {
        SkipWS();
        const auto start = m_input_pos;
        const auto prev_col = m_col;
        const auto prev_line = m_line;
        bool retval = Keyword_(t_s);
        // ignore substring matches
        if ( retval && has_more_input() && char_in_alphabet(*m_input_pos, detail::keyword_alphabet) ) {
          m_input_pos = start;
          m_col = prev_col;
          m_line = prev_line;
          retval = false;
        }

        if ( t_capture && retval ) {
          m_match_stack.push_back(make_node<eval::Str_AST_Node>(std::string(start, m_input_pos), prev_line, prev_col));
        }
        return retval;
      }

      /// Reads a symbol group from input if it matches the parameter, without skipping initial whitespace
      bool Symbol_(const char *t_s) {
        const auto len = strlen(t_s);

        if ((m_input_end - m_input_pos) >= static_cast<std::make_signed<decltype(len)>::type>(len)) {
          auto tmp = m_input_pos;
          for (size_t i = 0; i < len; ++i) {
            if (*tmp != t_s[i]) {
              return false;
            }
            ++tmp;
          }
          m_input_pos = tmp;
          m_col += static_cast<int>(len);
          return true;
        }

        return false;
      }

      /// Reads (and potentially captures) a symbol group from input if it matches the parameter
      bool Symbol(const char *t_s, const bool t_capture = false, const bool t_disallow_prevention=false) {
        SkipWS();
        const auto start = m_input_pos;
        const auto prev_col = m_col;
        const auto prev_line = m_line;
        bool retval = Symbol_(t_s);

        // ignore substring matches
        if (retval && has_more_input() && (t_disallow_prevention == false) && char_in_alphabet(*m_input_pos,detail::symbol_alphabet)) {
          m_input_pos = start;
          m_col = prev_col;
          m_line = prev_line;
          retval = false;
        }

        if ( t_capture && retval ) {
          m_match_stack.push_back(make_node<eval::Str_AST_Node>(std::string(start, m_input_pos), prev_line, prev_col));
        }

        return retval;
      }

      /// Reads an end-of-line group from input, without skipping initial whitespace
      bool Eol_(const bool t_eos = false) {
        bool retval = false;

        if (has_more_input() && (Symbol_("\r\n") || Char_('\n'))) {
          retval = true;
          ++m_line;
          m_col = 1;
        } else if (has_more_input() && !t_eos && Char_(';')) {
          retval = true;
        }

        return retval;
      }

      /// Reads until the end of the current statement
      bool Eos() {
        SkipWS();

        return Eol_(true);
      }

      /// Reads (and potentially captures) an end-of-line group from input
      bool Eol() {
        SkipWS();

        return Eol_();
      }

      /// Reads a comma-separated list of values from input. Id's only, no types allowed
      bool Id_Arg_List() {
        SkipWS(true);
        bool retval = false;

        const auto prev_stack_top = m_match_stack.size();

        if (Arg(false)) {
          retval = true;
          while (Eol()) {}
          if (Char(',')) {
            do {
              while (Eol()) {}
              if (!Arg(false)) {
                throw exception::eval_error("Unexpected value in parameter list", File_Position(m_line, m_col), *m_filename);
              }
            } while (Char(','));
          }
        }
        build_match<eval::Arg_List_AST_Node>(prev_stack_top);

        SkipWS(true);

        return retval;
      }

      /// Reads a comma-separated list of values from input, for function declarations
      bool Decl_Arg_List() {
        SkipWS(true);
        bool retval = false;

        const auto prev_stack_top = m_match_stack.size();

        if (Arg()) {
          retval = true;
          while (Eol()) {}
          if (Char(',')) {
            do {
              while (Eol()) {}
              if (!Arg()) {
                throw exception::eval_error("Unexpected value in parameter list", File_Position(m_line, m_col), *m_filename);
              }
            } while (Char(','));
          }
        }
        build_match<eval::Arg_List_AST_Node>(prev_stack_top);

        SkipWS(true);

        return retval;
      }


      /// Reads a comma-separated list of values from input
      bool Arg_List() {
        SkipWS(true);
        bool retval = false;

        const auto prev_stack_top = m_match_stack.size();

        if (Equation()) {
          retval = true;
          while (Eol()) {}
          if (Char(',')) {
            do {
              while (Eol()) {}
              if (!Equation()) {
                throw exception::eval_error("Unexpected value in parameter list", File_Position(m_line, m_col), *m_filename);
              }
            } while (Char(','));
          }
        }

        build_match<eval::Arg_List_AST_Node>(prev_stack_top);

        SkipWS(true);

        return retval;
      }

      /// Reads possible special container values, including ranges and map_pairs
      bool Container_Arg_List() {
        bool retval = false;
        SkipWS(true);

        const auto prev_stack_top = m_match_stack.size();

        if (Value_Range()) {
          retval = true;
          build_match<eval::Arg_List_AST_Node>(prev_stack_top);
        } else if (Map_Pair()) {
          retval = true;
          while (Eol()) {}
          if (Char(',')) {
            do {
              while (Eol()) {}
              if (!Map_Pair()) {
                throw exception::eval_error("Unexpected value in container", File_Position(m_line, m_col), *m_filename);
              }
            } while (Char(','));
          }
          build_match<eval::Arg_List_AST_Node>(prev_stack_top);
        } else if (Operator()) {
          retval = true;
          while (Eol()) {}
          if (Char(',')) {
            do {
              while (Eol()) {}
              if (!Operator()) {
                throw exception::eval_error("Unexpected value in container", File_Position(m_line, m_col), *m_filename);
              }
            } while (Char(','));
          }
          build_match<eval::Arg_List_AST_Node>(prev_stack_top);
        }

        SkipWS(true);

        return retval;
      }

      /// Reads a lambda (anonymous function) from input
      bool Lambda() {
        bool retval = false;

        const auto prev_stack_top = m_match_stack.size();

        if (Keyword("fun")) {
          retval = true;

          if (Char('[')) {
            Id_Arg_List();
            if (!Char(']')) {
              throw exception::eval_error("Incomplete anonymous function bind", File_Position(m_line, m_col), *m_filename);
            }
          } else {
            // make sure we always have the same number of nodes
            build_match<eval::Arg_List_AST_Node>(prev_stack_top);
          }

          if (Char('(')) {
            Decl_Arg_List();
            if (!Char(')')) {
              throw exception::eval_error("Incomplete anonymous function", File_Position(m_line, m_col), *m_filename);
            }
          } else {
            throw exception::eval_error("Incomplete anonymous function", File_Position(m_line, m_col), *m_filename);
          }


          while (Eol()) {}

          if (!Block()) {
            throw exception::eval_error("Incomplete anonymous function", File_Position(m_line, m_col), *m_filename);
          }

          build_match<eval::Lambda_AST_Node>(prev_stack_top);
        }

        return retval;
      }

      /// Reads a function definition from input
      bool Def(const bool t_class_context = false) {
        bool retval = false;
        AST_NodePtr annotation;

        if (Annotation()) {
          while (Eol_()) {}
          annotation = m_match_stack.back();
          m_match_stack.pop_back();
        }

        const auto prev_stack_top = m_match_stack.size();

        if (Keyword("def")) {
          retval = true;

          if (!Id()) {
            throw exception::eval_error("Missing function name in definition", File_Position(m_line, m_col), *m_filename);
          }

          bool is_method = false;

          if (Symbol("::", false)) {
            //We're now a method
            is_method = true;

            if (!Id()) {
              throw exception::eval_error("Missing method name in definition", File_Position(m_line, m_col), *m_filename);
            }
          }

          if (Char('(')) {
            Decl_Arg_List();
            if (!Char(')')) {
              throw exception::eval_error("Incomplete function definition", File_Position(m_line, m_col), *m_filename);
            }
          }

          while (Eos()) {}

          if (Char(':')) {
            if (!Operator()) {
              throw exception::eval_error("Missing guard expression for function", File_Position(m_line, m_col), *m_filename);
            }
          }

          while (Eol()) {}
          if (!Block()) {
            throw exception::eval_error("Incomplete function definition", File_Position(m_line, m_col), *m_filename);
          }

          if (is_method || t_class_context) {
            build_match<eval::Method_AST_Node>(prev_stack_top);
          } else {
            build_match<eval::Def_AST_Node>(prev_stack_top);
          }

          if (annotation) {
            m_match_stack.back()->annotation = std::move(annotation);
          }
        }

        return retval;
      }

      /// Reads a function definition from input
      bool Try() {
        bool retval = false;

        const auto prev_stack_top = m_match_stack.size();

        if (Keyword("try")) {
          retval = true;

          while (Eol()) {}

          if (!Block()) {
            throw exception::eval_error("Incomplete 'try' block", File_Position(m_line, m_col), *m_filename);
          }

          bool has_matches = true;
          while (has_matches) {
            while (Eol()) {}
            has_matches = false;
            if (Keyword("catch", false)) {
              const auto catch_stack_top = m_match_stack.size();
              if (Char('(')) {
                if (!(Arg() && Char(')'))) {
                  throw exception::eval_error("Incomplete 'catch' expression", File_Position(m_line, m_col), *m_filename);
                }
                if (Char(':')) {
                  if (!Operator()) {
                    throw exception::eval_error("Missing guard expression for catch", File_Position(m_line, m_col), *m_filename);
                  }
                }
              }

              while (Eol()) {}

              if (!Block()) {
                throw exception::eval_error("Incomplete 'catch' block", File_Position(m_line, m_col), *m_filename);
              }
              build_match<eval::Catch_AST_Node>(catch_stack_top);
              has_matches = true;
            }
          }
          while (Eol()) {}
          if (Keyword("finally", false)) {
            const auto finally_stack_top = m_match_stack.size();

            while (Eol()) {}

            if (!Block()) {
              throw exception::eval_error("Incomplete 'finally' block", File_Position(m_line, m_col), *m_filename);
            }
            build_match<eval::Finally_AST_Node>(finally_stack_top);
          }

          build_match<eval::Try_AST_Node>(prev_stack_top);
        }

        return retval;
      }

      /// Reads an if/else if/else block from input
      bool If() {
        bool retval = false;

        const auto prev_stack_top = m_match_stack.size();

        if (Keyword("if")) {
          retval = true;

          if (!Char('(')) {
            throw exception::eval_error("Incomplete 'if' expression", File_Position(m_line, m_col), *m_filename);
          }

          if (!(Operator() && Char(')'))) {
            throw exception::eval_error("Incomplete 'if' expression", File_Position(m_line, m_col), *m_filename);
          }

          while (Eol()) {}

          if (!Block()) {
            throw exception::eval_error("Incomplete 'if' block", File_Position(m_line, m_col), *m_filename);
          }

          bool has_matches = true;
          while (has_matches) {
            while (Eol()) {}
            has_matches = false;
            if (Keyword("else", true)) {
              if (Keyword("if")) {
                const AST_NodePtr back(m_match_stack.back());
                m_match_stack.back() = 
                  chaiscript::make_shared<AST_Node, eval::If_AST_Node>("else if", back->location, back->children);
                m_match_stack.back()->annotation = back->annotation;
                if (!Char('(')) {
                  throw exception::eval_error("Incomplete 'else if' expression", File_Position(m_line, m_col), *m_filename);
                }

                if (!(Operator() && Char(')'))) {
                  throw exception::eval_error("Incomplete 'else if' expression", File_Position(m_line, m_col), *m_filename);
                }

                while (Eol()) {}

                if (!Block()) {
                  throw exception::eval_error("Incomplete 'else if' block", File_Position(m_line, m_col), *m_filename);
                }
                has_matches = true;
              } else {
                while (Eol()) {}

                if (!Block()) {
                  throw exception::eval_error("Incomplete 'else' block", File_Position(m_line, m_col), *m_filename);
                }
                has_matches = true;
              }
            }
          }

          build_match<eval::If_AST_Node>(prev_stack_top);
        }

        return retval;
      }

      /// Reads a class block from input
      bool Class() {
        bool retval = false;

        size_t prev_stack_top = m_match_stack.size();

        if (Keyword("class")) {
          retval = true;

          if (!Id()) {
            throw exception::eval_error("Missing class name in definition", File_Position(m_line, m_col), *m_filename);
          }


          while (Eol()) {}

          if (!Class_Block()) {
            throw exception::eval_error("Incomplete 'class' block", File_Position(m_line, m_col), *m_filename);
          }

          build_match<eval::Class_AST_Node>(prev_stack_top);
        }

        return retval;
      }


      /// Reads a while block from input
      bool While() {
        bool retval = false;

        const auto prev_stack_top = m_match_stack.size();

        if (Keyword("while")) {
          retval = true;

          if (!Char('(')) {
            throw exception::eval_error("Incomplete 'while' expression", File_Position(m_line, m_col), *m_filename);
          }

          if (!(Operator() && Char(')'))) {
            throw exception::eval_error("Incomplete 'while' expression", File_Position(m_line, m_col), *m_filename);
          }

          while (Eol()) {}

          if (!Block()) {
            throw exception::eval_error("Incomplete 'while' block", File_Position(m_line, m_col), *m_filename);
          }

          build_match<eval::While_AST_Node>(prev_stack_top);
        }

        return retval;
      }


      /// Reads the C-style for conditions from input
      bool For_Guards() {
        if (!(Equation() && Eol()))
        {
          if (!Eol())
          {
            throw exception::eval_error("'for' loop initial statment missing", File_Position(m_line, m_col), *m_filename);
          } else {
            m_match_stack.push_back(chaiscript::make_shared<AST_Node, eval::Noop_AST_Node>());
          }
        }

        if (!(Equation() && Eol()))
        {
          if (!Eol())
          {
            throw exception::eval_error("'for' loop condition missing", File_Position(m_line, m_col), *m_filename);
          } else {
            m_match_stack.push_back(chaiscript::make_shared<AST_Node, eval::Noop_AST_Node>());
          }
        }

        if (!Equation())
        {
          m_match_stack.push_back(chaiscript::make_shared<AST_Node, eval::Noop_AST_Node>());
        }

        return true; 
      }

      /// Reads a for block from input
      bool For() {
        bool retval = false;

        const auto prev_stack_top = m_match_stack.size();

        if (Keyword("for")) {
          retval = true;

          if (!Char('(')) {
            throw exception::eval_error("Incomplete 'for' expression", File_Position(m_line, m_col), *m_filename);
          }

          if (!(For_Guards() && Char(')'))) {
            throw exception::eval_error("Incomplete 'for' expression", File_Position(m_line, m_col), *m_filename);
          }

          while (Eol()) {}

          if (!Block()) {
            throw exception::eval_error("Incomplete 'for' block", File_Position(m_line, m_col), *m_filename);
          }

          build_match<eval::For_AST_Node>(prev_stack_top);
        }

        return retval;
      }

      /// Reads a case block from input
      bool Case() {
        bool retval = false;

        const auto prev_stack_top = m_match_stack.size();

        if (Keyword("case")) {
          retval = true;

          if (!Char('(')) {
            throw exception::eval_error("Incomplete 'case' expression", File_Position(m_line, m_col), *m_filename);
          }

          if (!(Operator() && Char(')'))) {
            throw exception::eval_error("Incomplete 'case' expression", File_Position(m_line, m_col), *m_filename);
          }

          while (Eol()) {}

          if (!Block()) {
            throw exception::eval_error("Incomplete 'case' block", File_Position(m_line, m_col), *m_filename);
          }

          build_match<eval::Case_AST_Node>(prev_stack_top);
        } else if (Keyword("default")) {
          while (Eol()) {}

          if (!Block()) {
            throw exception::eval_error("Incomplete 'default' block", File_Position(m_line, m_col), *m_filename);
          }

          build_match<eval::Default_AST_Node>(prev_stack_top);
        }

        return retval;
      }


      /// Reads a switch statement from input
      bool Switch() {
        const auto prev_stack_top = m_match_stack.size();

        if (Keyword("switch")) {

          if (!Char('(')) {
            throw exception::eval_error("Incomplete 'switch' expression", File_Position(m_line, m_col), *m_filename);
          }

          if (!(Operator() && Char(')'))) {
            throw exception::eval_error("Incomplete 'switch' expression", File_Position(m_line, m_col), *m_filename);
          }

          while (Eol()) {}

          if (Char('{')) {
            while (Eol()) {}

            while (Case()) {
              while (Eol()) { } // eat
            }

            while (Eol()) { } // eat

            if (!Char('}')) {
              throw exception::eval_error("Incomplete block", File_Position(m_line, m_col), *m_filename);
            }
          }
          else {
            throw exception::eval_error("Incomplete block", File_Position(m_line, m_col), *m_filename);
          }

          build_match<eval::Switch_AST_Node>(prev_stack_top);
          return true;

        } else {
          return false;
        }

      }


      /// Reads a curly-brace C-style class block from input
      bool Class_Block() {
        bool retval = false;

        const auto prev_stack_top = m_match_stack.size();

        if (Char('{')) {
          retval = true;

          Class_Statements();
          if (!Char('}')) {
            throw exception::eval_error("Incomplete class block", File_Position(m_line, m_col), *m_filename);
          }

          if (m_match_stack.size() == prev_stack_top) {
            m_match_stack.push_back(chaiscript::make_shared<AST_Node, eval::Noop_AST_Node>());
          }

          build_match<eval::Block_AST_Node>(prev_stack_top);
        }

        return retval;
      }

      /// Reads a curly-brace C-style block from input
      bool Block() {
        bool retval = false;

        const auto prev_stack_top = m_match_stack.size();

        if (Char('{')) {
          retval = true;

          Statements();
          if (!Char('}')) {
            throw exception::eval_error("Incomplete block", File_Position(m_line, m_col), *m_filename);
          }

          if (m_match_stack.size() == prev_stack_top) {
            m_match_stack.push_back(chaiscript::make_shared<AST_Node, eval::Noop_AST_Node>());
          }

          build_match<eval::Block_AST_Node>(prev_stack_top);
        }

        return retval;
      }

      /// Reads a return statement from input
      bool Return() {
        const auto prev_stack_top = m_match_stack.size();

        if (Keyword("return")) {
          Operator();
          build_match<eval::Return_AST_Node>(prev_stack_top);
          return true;
        } else {
          return false;
        }
      }

      /// Reads a break statement from input
      bool Break() {
        const auto prev_stack_top = m_match_stack.size();

        if (Keyword("break")) {
          build_match<eval::Break_AST_Node>(prev_stack_top);
          return true;
        } else {
          return false;
        }
      }

      /// Reads a continue statement from input
      bool Continue() {
        const auto prev_stack_top = m_match_stack.size();

        if (Keyword("continue")) {
          build_match<eval::Continue_AST_Node>(prev_stack_top);
          return true;
        } else {
          return false;
        }
      }

      /// Reads a dot expression(member access), then proceeds to check if it's a function or array call
      bool Dot_Fun_Array() {
        bool retval = false;

        const auto prev_stack_top = m_match_stack.size();
        if (Lambda() || Num(true) || Quoted_String(true) || Single_Quoted_String(true) ||
            Paren_Expression() || Inline_Container() || Id())
        {
          retval = true;
          bool has_more = true;

          while (has_more) {
            has_more = false;

            if (Char('(')) {
              has_more = true;

              Arg_List();
              if (!Char(')')) {
                throw exception::eval_error("Incomplete function call", File_Position(m_line, m_col), *m_filename);
              }

              build_match<eval::Fun_Call_AST_Node>(prev_stack_top);
              /// \todo Work around for method calls until we have a better solution
              if (!m_match_stack.back()->children.empty()) {
                if (m_match_stack.back()->children[0]->identifier == AST_Node_Type::Dot_Access) {
                  AST_NodePtr dot_access = m_match_stack.back()->children[0];
                  AST_NodePtr func_call = m_match_stack.back();
                  m_match_stack.pop_back();
                  func_call->children.erase(func_call->children.begin());
                  func_call->children.insert(func_call->children.begin(), dot_access->children.back());
                  dot_access->children.pop_back();
                  dot_access->children.push_back(std::move(func_call));
                  m_match_stack.push_back(std::move(dot_access));
                }
              }
            } else if (Char('[')) {
              has_more = true;

              if (!(Operator() && Char(']'))) {
                throw exception::eval_error("Incomplete array access", File_Position(m_line, m_col), *m_filename);
              }

              build_match<eval::Array_Call_AST_Node>(prev_stack_top);
            }
            else if (Symbol(".", true)) {
              has_more = true;
              if (!(Id())) {
                throw exception::eval_error("Incomplete array access", File_Position(m_line, m_col), *m_filename);
              }

              build_match<eval::Dot_Access_AST_Node>(prev_stack_top);
            }
          }
        }

        return retval;
      }

      /// Reads a variable declaration from input
      bool Var_Decl(const bool t_class_context = false) {
        bool retval = false;

        const auto prev_stack_top = m_match_stack.size();

        if (t_class_context && (Keyword("attr") || Keyword("auto") || Keyword("var"))) {
          retval = true;

          if (!Id()) {
            throw exception::eval_error("Incomplete attribute declaration", File_Position(m_line, m_col), *m_filename);
          }

          build_match<eval::Attr_Decl_AST_Node>(prev_stack_top);
        } else if (Keyword("auto") || Keyword("var") ) {
          retval = true;

          if (!(Reference() || Id())) {
            throw exception::eval_error("Incomplete variable declaration", File_Position(m_line, m_col), *m_filename);
          }

          build_match<eval::Var_Decl_AST_Node>(prev_stack_top);
        } else if (Keyword("GLOBAL")) {
          retval = true;

          if (!(Reference() || Id())) {
            throw exception::eval_error("Incomplete global declaration", File_Position(m_line, m_col), *m_filename);
          }

          build_match<eval::Global_Decl_AST_Node>(prev_stack_top);
        } else if (Keyword("attr")) {
          retval = true;

          if (!Id()) {
            throw exception::eval_error("Incomplete attribute declaration", File_Position(m_line, m_col), *m_filename);
          }
          if (!Symbol("::", false)) {
            throw exception::eval_error("Incomplete attribute declaration", File_Position(m_line, m_col), *m_filename);
          }
          if (!Id()) {
            throw exception::eval_error("Missing attribute name in definition", File_Position(m_line, m_col), *m_filename);
          }


          build_match<eval::Attr_Decl_AST_Node>(prev_stack_top);
        }

        return retval;
      }

      /// Reads an expression surrounded by parentheses from input
      bool Paren_Expression() {
        if (Char('(')) {
          if (!Operator()) {
            throw exception::eval_error("Incomplete expression", File_Position(m_line, m_col), *m_filename);
          }
          if (!Char(')')) {
            throw exception::eval_error("Missing closing parenthesis ')'", File_Position(m_line, m_col), *m_filename);
          }
          return true;
        } else {
          return false;
        }
      }

      /// Reads, and identifies, a short-form container initialization from input
      bool Inline_Container() {
        const auto prev_stack_top = m_match_stack.size();

        if (Char('[')) {
          Container_Arg_List();

          if (!Char(']')) {
            throw exception::eval_error("Missing closing square bracket ']' in container initializer", File_Position(m_line, m_col), *m_filename);
          }
          if ((prev_stack_top != m_match_stack.size()) && (m_match_stack.back()->children.size() > 0)) {
            if (m_match_stack.back()->children[0]->identifier == AST_Node_Type::Value_Range) {
              build_match<eval::Inline_Range_AST_Node>(prev_stack_top);
            }
            else if (m_match_stack.back()->children[0]->identifier == AST_Node_Type::Map_Pair) {
              build_match<eval::Inline_Map_AST_Node>(prev_stack_top);
            }
            else {
              build_match<eval::Inline_Array_AST_Node>(prev_stack_top);
            }
          }
          else {
            build_match<eval::Inline_Array_AST_Node>(prev_stack_top);
          }

          return true;
        } else {
          return false;
        }
      }

      /// Parses a variable specified with a & aka reference
      bool Reference() {
        const auto prev_stack_top = m_match_stack.size();

        if (Symbol("&", false)) {
          if (!Id()) {
            throw exception::eval_error("Incomplete '&' expression", File_Position(m_line, m_col), *m_filename);
          }

          build_match<eval::Reference_AST_Node>(prev_stack_top);
          return true;
        } else {
          return false;
        }
      }

      /// Reads a unary prefixed expression from input
      bool Prefix() {
        const auto prev_stack_top = m_match_stack.size();
        const std::vector<std::string> prefix_opers{"++", "--", "-", "+", "!", "~", "&"};

        for (const auto &oper : prefix_opers)
        {
          bool is_char = oper.size() == 1;
          if ((is_char && Char(oper[0], true)) || (!is_char && Symbol(oper.c_str(), true)))
          {
            if (!Operator(m_operators.size()-1)) {
              throw exception::eval_error("Incomplete prefix '" + oper + "' expression", File_Position(m_line, m_col), *m_filename);
            }

            build_match<eval::Prefix_AST_Node>(prev_stack_top);
            return true;
          }
        }

        return false;
      }

      /// Parses any of a group of 'value' style ast_node groups from input
      bool Value() {
        return Var_Decl() || Dot_Fun_Array() || Prefix();
      }

      bool Operator_Helper(const size_t t_precedence) {
        for (auto & elem : m_operator_matches[t_precedence]) {
          if (Symbol(elem.c_str(), true)) {
            return true;
          }
        }
        return false;
      }

      bool Operator(const size_t t_precedence = 0) {
        bool retval = false;
        const auto prev_stack_top = m_match_stack.size();

        if (t_precedence < m_operators.size()) {
          if (Operator(t_precedence+1)) {
            retval = true;
            if (Operator_Helper(t_precedence)) {
              do {
                while (Eol()) {}
                if (!Operator(t_precedence+1)) {
                  throw exception::eval_error("Incomplete "
                      + std::string(ast_node_type_to_string(m_operators[t_precedence])) + " expression",
                      File_Position(m_line, m_col), *m_filename);
                }

                AST_NodePtr oper = m_match_stack.at(m_match_stack.size()-2);

                switch (m_operators[t_precedence]) {
                  case(AST_Node_Type::Ternary_Cond) :
                    m_match_stack.erase(m_match_stack.begin() + m_match_stack.size() - 2,
                        m_match_stack.begin() + m_match_stack.size() - 1);
                    if (Symbol(":")) {
                      if (!Operator(t_precedence+1)) {
                        throw exception::eval_error("Incomplete "
                            + std::string(ast_node_type_to_string(m_operators[t_precedence])) + " expression",
                            File_Position(m_line, m_col), *m_filename);
                      }
                      build_match<eval::Ternary_Cond_AST_Node>(prev_stack_top);
                    }
                    else {
                      throw exception::eval_error("Incomplete "
                          + std::string(ast_node_type_to_string(m_operators[t_precedence])) + " expression",
                          File_Position(m_line, m_col), *m_filename);
                    }
                    break;

                  case(AST_Node_Type::Addition) :
                  case(AST_Node_Type::Multiplication) :
                  case(AST_Node_Type::Shift) :
                  case(AST_Node_Type::Equality) :
                  case(AST_Node_Type::Bitwise_And) :
                  case(AST_Node_Type::Bitwise_Xor) :
                  case(AST_Node_Type::Bitwise_Or) :
                  case(AST_Node_Type::Comparison) :
                    assert(m_match_stack.size() > 1);
                    m_match_stack.erase(m_match_stack.begin() + m_match_stack.size() - 2, m_match_stack.begin() + m_match_stack.size() - 1);
                    build_match<eval::Binary_Operator_AST_Node>(prev_stack_top, oper->text);
                    break;

                  case(AST_Node_Type::Logical_And) :
                    build_match<eval::Logical_And_AST_Node>(prev_stack_top);
                    break;
                  case(AST_Node_Type::Logical_Or) :
                    build_match<eval::Logical_Or_AST_Node>(prev_stack_top);
                    break;

                  default:
                    throw exception::eval_error("Internal error: unhandled ast_node", File_Position(m_line, m_col), *m_filename);
                }
              } while (Operator_Helper(t_precedence));
            }
          }
        }
        else {
          return Value();
        }

        return retval;
      }

      /// Reads a pair of values used to create a map initialization from input
      bool Map_Pair() {
        bool retval = false;

        const auto prev_stack_top = m_match_stack.size();
        const auto prev_pos = m_input_pos;
        const auto prev_col = m_col;

        if (Operator()) {
          if (Symbol(":")) {
            retval = true;
            if (!Operator()) {
              throw exception::eval_error("Incomplete map pair", File_Position(m_line, m_col), *m_filename);
            }

            build_match<eval::Map_Pair_AST_Node>(prev_stack_top);
          }
          else {
            m_input_pos = prev_pos;
            m_col = prev_col;
            while (prev_stack_top != m_match_stack.size()) {
              m_match_stack.pop_back();
            }
          }
        }

        return retval;
      }

      /// Reads a pair of values used to create a range initialization from input
      bool Value_Range() {
        bool retval = false;

        const auto prev_stack_top = m_match_stack.size();
        const auto prev_pos = m_input_pos;
        const auto prev_col = m_col;

        if (Operator()) {
          if (Symbol("..")) {
            retval = true;
            if (!Operator()) {
              throw exception::eval_error("Incomplete value range", File_Position(m_line, m_col), *m_filename);
            }

            build_match<eval::Value_Range_AST_Node>(prev_stack_top);
          }
          else {
            m_input_pos = prev_pos;
            m_col = prev_col;
            while (prev_stack_top != m_match_stack.size()) {
              m_match_stack.pop_back();
            }
          }
        }

        return retval;
      }

      /// Parses a string of binary equation operators
      bool Equation() {
        bool retval = false;

        const auto prev_stack_top = m_match_stack.size();

        if (Operator()) {
          retval = true;
          if (Symbol("=", true, true) || Symbol(":=", true, true) || Symbol("+=", true, true) ||
              Symbol("-=", true, true) || Symbol("*=", true, true) || Symbol("/=", true, true) ||
              Symbol("%=", true, true) || Symbol("<<=", true, true) || Symbol(">>=", true, true) ||
              Symbol("&=", true, true) || Symbol("^=", true, true) || Symbol("|=", true, true)) {
            SkipWS(true);
            if (!Equation()) {
              throw exception::eval_error("Incomplete equation", File_Position(m_line, m_col), *m_filename);
            }

            build_match<eval::Equation_AST_Node>(prev_stack_top);
          }
        }

        return retval;
      }

      /// Parses statements allowed inside of a class block
      bool Class_Statements() {
        bool retval = false;

        bool has_more = true;
        bool saw_eol = true;

        while (has_more) {
          const auto prev_line = m_line;
          const auto prev_col = m_col;
          if (Def(true) || Var_Decl(true)) {
            if (!saw_eol) {
              throw exception::eval_error("Two function definitions missing line separator", File_Position(prev_line, prev_col), *m_filename);
            }
            has_more = true;
            retval = true;
            saw_eol = true;
          } else if (Eol()) {
            has_more = true;
            retval = true;
            saw_eol = true;
          } else {
            has_more = false;
          }
        }

        return retval;
      }

      /// Top level parser, starts parsing of all known parses
      bool Statements() {
        bool retval = false;

        bool has_more = true;
        bool saw_eol = true;

        while (has_more) {
          int prev_line = m_line;
          int prev_col = m_col;
          if (Def() || Try() || If() || While() || Class() || For() || Switch()) {
            if (!saw_eol) {
              throw exception::eval_error("Two function definitions missing line separator", File_Position(prev_line, prev_col), *m_filename);
            }
            has_more = true;
            retval = true;
            saw_eol = true;
          }
          else if (Return() || Break() || Continue() || Equation()) {
            if (!saw_eol) {
              throw exception::eval_error("Two expressions missing line separator", File_Position(prev_line, prev_col), *m_filename);
            }
            has_more = true;
            retval = true;
            saw_eol = false;
          }
          else if (Block() || Eol()) {
            has_more = true;
            retval = true;
            saw_eol = true;
          }
          else {
            has_more = false;
          }
        }

        return retval;
      }

      /// Parses the given input string, tagging parsed ast_nodes with the given m_filename.
      bool parse(const std::string &t_input, std::string t_fname) {
        m_input_pos = t_input.begin();
        m_input_end = t_input.end();
        m_line = 1;
        m_col = 1;
        m_filename = std::make_shared<std::string>(std::move(t_fname));

        if ((t_input.size() > 1) && (t_input[0] == '#') && (t_input[1] == '!')) {
          while ((m_input_pos != m_input_end) && (!Eol())) {
            ++m_input_pos;
          }
          /// \todo respect // -*- coding: utf-8 -*- on line 1 or 2 see: http://evanjones.ca/python-utf8.html)
        }

        if (Statements()) {
          if (m_input_pos != m_input_end) {
            throw exception::eval_error("Unparsed input", File_Position(m_line, m_col), t_fname);
          } else {
            build_match<eval::File_AST_Node>(0);
            return true;
          }
        } else {
          return false;
        }
      }
    };
  }
}

#endif /* CHAISCRIPT_PARSER_HPP_ */

