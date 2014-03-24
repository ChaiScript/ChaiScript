// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2014, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_PARSER_HPP_
#define CHAISCRIPT_PARSER_HPP_

#include <exception>
#include <fstream>
#include <sstream>
#include <cstring>

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

      std::vector<std::vector<std::string> > m_operator_matches;
      std::vector<AST_Node_Type::Type> m_operators;

      public:
      ChaiScript_Parser()
        : m_multiline_comment_begin("/*"),
          m_multiline_comment_end("*/"),
          m_singleline_comment("//")
      {
        setup_operators();
      }

      ChaiScript_Parser(const ChaiScript_Parser &); // explicitly unimplemented copy constructor
      ChaiScript_Parser &operator=(const ChaiScript_Parser &); // explicitly unimplemented assignment operator

      void setup_operators()
      {
        m_operators.push_back(AST_Node_Type::Ternary_Cond);
        std::vector<std::string> ternary_cond;
        ternary_cond.push_back("?");
        m_operator_matches.push_back(ternary_cond);

        m_operators.push_back(AST_Node_Type::Logical_Or);
        std::vector<std::string> logical_or;
        logical_or.push_back("||");
        m_operator_matches.push_back(logical_or);

        m_operators.push_back(AST_Node_Type::Logical_And);
        std::vector<std::string> logical_and;
        logical_and.push_back("&&");
        m_operator_matches.push_back(logical_and);

        m_operators.push_back(AST_Node_Type::Bitwise_Or);
        std::vector<std::string> bitwise_or;
        bitwise_or.push_back("|");
        m_operator_matches.push_back(bitwise_or);

        m_operators.push_back(AST_Node_Type::Bitwise_Xor);
        std::vector<std::string> bitwise_xor;
        bitwise_xor.push_back("^");
        m_operator_matches.push_back(bitwise_xor);

        m_operators.push_back(AST_Node_Type::Bitwise_And);
        std::vector<std::string> bitwise_and;
        bitwise_and.push_back("&");
        m_operator_matches.push_back(bitwise_and);

        m_operators.push_back(AST_Node_Type::Equality);
        std::vector<std::string> equality;
        equality.push_back("==");
        equality.push_back("!=");
        m_operator_matches.push_back(equality);

        m_operators.push_back(AST_Node_Type::Comparison);
        std::vector<std::string> comparison;
        comparison.push_back("<");
        comparison.push_back("<=");
        comparison.push_back(">");
        comparison.push_back(">=");
        m_operator_matches.push_back(comparison);

        m_operators.push_back(AST_Node_Type::Shift);
        std::vector<std::string> shift;
        shift.push_back("<<");
        shift.push_back(">>");
        m_operator_matches.push_back(shift);

        //We share precedence here but then separate them later
        m_operators.push_back(AST_Node_Type::Addition);
        std::vector<std::string> addition;
        addition.push_back("+");
        addition.push_back("-");
        m_operator_matches.push_back(addition);

        //We share precedence here but then separate them later
        m_operators.push_back(AST_Node_Type::Multiplication);
        std::vector<std::string> multiplication;
        multiplication.push_back("*");
        multiplication.push_back("/");
        multiplication.push_back("%");
        m_operator_matches.push_back(multiplication);

        for ( int c = 0 ; c < detail::lengthof_alphabet ; ++c ) {
          for ( int a = 0 ; a < detail::max_alphabet ; a ++ ) {
            m_alphabet[a][c]=false;
          }
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

      /**
       * test a char in an m_alphabet
       */
      bool char_in_alphabet(unsigned char c, detail::Alphabet a) { return m_alphabet[a][c]; }

      /**
       * Prints the parsed ast_nodes as a tree
       */
      /*
         void debug_print(AST_NodePtr t, std::string prepend = "") {
         std::cout << prepend << "(" << ast_node_type_to_string(t->identifier) << ") " << t->text << " : " << t->start.line << ", " << t->start.column << std::endl;
         for (unsigned int j = 0; j < t->children.size(); ++j) {
         debug_print(t->children[j], prepend + "  ");
         }
         }
         */

      /**
       * Shows the current stack of matched ast_nodes
       */
      void show_match_stack() {
        for (unsigned int i = 0; i < m_match_stack.size(); ++i) {
          //debug_print(match_stack[i]);
          std::cout << m_match_stack[i]->to_string();
        }
      }

      /**
       * Clears the stack of matched ast_nodes
       */
      void clear_match_stack() {
        m_match_stack.clear();
      }

      /**
       * Returns the front-most AST node
       */
      AST_NodePtr ast() {
        return m_match_stack.front();
      }

      /**
       * Helper function that collects ast_nodes from a starting position to the top of the stack into a new AST node
       */
      void build_match(AST_NodePtr t_t, size_t t_match_start) {
        int pos_line_start, pos_col_start, pos_line_stop, pos_col_stop;
        int is_deep = false;

        //so we want to take everything to the right of this and make them children
        if (t_match_start != m_match_stack.size()) {
          pos_line_start = m_match_stack[t_match_start]->start.line;
          pos_col_start = m_match_stack[t_match_start]->start.column;
          pos_line_stop = m_line;
          pos_col_stop = m_col;
          is_deep = true;
        }
        else {
          pos_line_start = m_line;
          pos_col_start = m_col;
          pos_line_stop = m_line;
          pos_col_stop = m_col;
        }

        t_t->filename = m_filename;
        t_t->start.line = pos_line_start;
        t_t->start.column = pos_col_start;
        t_t->end.line = pos_line_stop;
        t_t->end.column = pos_col_stop;

        if (is_deep) {
          t_t->children.assign(m_match_stack.begin() + t_match_start, m_match_stack.end());
          m_match_stack.erase(m_match_stack.begin() + t_match_start, m_match_stack.end());
          m_match_stack.push_back(t_t);
        }
        else {
          /// \todo fix the fact that a successful match that captured no ast_nodes doesn't have any real start position
          m_match_stack.push_back(t_t);
        }
      }

      /**
       * Check to see if there is more text parse
       */
      inline bool has_more_input() {
        return (m_input_pos != m_input_end);
      }

      /**
       * Skips any multi-line or single-line comment
       */
      bool SkipComment() {
        bool retval = false;

        if (Symbol_(m_multiline_comment_begin.c_str())) {
          while (m_input_pos != m_input_end) {
            if (Symbol_(m_multiline_comment_end.c_str())) {
              break;
            }
            else if (!Eol_()) {
              ++m_col;
              ++m_input_pos;
            }
          }
          retval = true;
        }
        else if (Symbol_(m_singleline_comment.c_str())) {
          while (m_input_pos != m_input_end) {
            if (Symbol_("\r\n")) {
              m_input_pos -= 2;
              break;
            }
            else if (Char_('\n')) {
              --m_input_pos;
              break;
            }
            else {
              ++m_col;
              ++m_input_pos;
            }
          }
          retval = true;
        }
        return retval;
      }

      /**
       * Skips ChaiScript whitespace, which means space and tab, but not cr/lf
       * jespada: Modified SkipWS to skip optionally CR ('\n')
       */
      bool SkipWS(bool skip_cr=false) {
        bool retval = false;
        while (has_more_input()) {
          if ( char_in_alphabet(*m_input_pos,detail::white_alphabet) || (skip_cr && (*m_input_pos == '\n'))) {
            if(*m_input_pos == '\n') {
              m_col = 1;
              ++m_line;
            }
            else {
              ++m_col;
            }
            ++m_input_pos;

            retval = true;
          }
          else if (SkipComment()) {
            retval = true;
          }
          else {
            break;
          }
        }
        return retval;
      }

      /**
       * Reads a floating point value from input, without skipping initial whitespace
       */
      bool Float_() {
        bool retval = false;

        if (has_more_input() && char_in_alphabet(*m_input_pos,detail::float_alphabet) ) {
          while (has_more_input() && char_in_alphabet(*m_input_pos,detail::int_alphabet) ) {
            ++m_input_pos;
            ++m_col;
          }
          if (has_more_input() && (*m_input_pos == '.')) {
            ++m_input_pos;
            ++m_col;
            if (has_more_input() && char_in_alphabet(*m_input_pos,detail::int_alphabet)) {
              retval = true;
              while (has_more_input() && char_in_alphabet(*m_input_pos,detail::int_alphabet) ) {
                ++m_input_pos;
                ++m_col;
              }

              while (has_more_input() && char_in_alphabet(*m_input_pos, detail::float_suffix_alphabet))
              {
                ++m_input_pos;
                ++m_col;
              }
            }
            else {
              --m_input_pos;
              --m_col;
            }
          }
        }
        return retval;
      }

      /**
       * Reads a hex value from input, without skipping initial whitespace
       */
      bool Hex_() {
        bool retval = false;
        if (has_more_input() && (*m_input_pos == '0')) {
          ++m_input_pos;
          ++m_col;

          if (has_more_input() && char_in_alphabet(*m_input_pos, detail::x_alphabet) ) {
            ++m_input_pos;
            ++m_col;
            if (has_more_input() && char_in_alphabet(*m_input_pos, detail::hex_alphabet)) {
              retval = true;
              while (has_more_input() && char_in_alphabet(*m_input_pos, detail::hex_alphabet) ) {
                ++m_input_pos;
                ++m_col;
              }
              while (has_more_input() && char_in_alphabet(*m_input_pos, detail::int_suffix_alphabet))
              {
                ++m_input_pos;
                ++m_col;
              }
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

        return retval;
      }

      void IntSuffix_() {
        while (has_more_input() && char_in_alphabet(*m_input_pos, detail::int_suffix_alphabet))
        {
          ++m_input_pos;
          ++m_col;
        }
      }

      /**
       * Reads a binary value from input, without skipping initial whitespace
       */
      bool Binary_() {
        bool retval = false;
        if (has_more_input() && (*m_input_pos == '0')) {
          ++m_input_pos;
          ++m_col;

          if (has_more_input() && char_in_alphabet(*m_input_pos, detail::b_alphabet) ) {
            ++m_input_pos;
            ++m_col;
            if (has_more_input() && char_in_alphabet(*m_input_pos, detail::bin_alphabet) ) {
              retval = true;
              while (has_more_input() && char_in_alphabet(*m_input_pos, detail::bin_alphabet) ) {
                ++m_input_pos;
                ++m_col;
              }
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

        return retval;
      }

      Boxed_Value buildFloat(const std::string &t_val)
      {
        bool float_ = false;
        bool long_ = false;

        size_t i = t_val.size();

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

        std::stringstream ss(t_val.substr(0, i));

        if (float_)
        {
          float f;
          ss >> f;   
          return Boxed_Value(const_var(f));
        } else if (long_) {
          long double f;
          ss >> f;   
          return Boxed_Value(const_var(f));
        } else {
          double f;
          ss >> f;   
          return Boxed_Value(const_var(f));
        }
      }



      template<typename IntType>
      Boxed_Value buildInt(const IntType &t_type, const std::string &t_val)
      {
        bool unsigned_ = false;
        bool long_ = false;
        bool longlong_ = false;

        size_t i = t_val.size();

        for (; i > 0; --i)
        {
          char val = t_val[i-1];

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
        size_t size = sizeof(int) * 8;

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


        if (longlong_)
        {
          size = sizeof(int64_t) * 8;
        } else if (long_) {
          size = sizeof(long) * 8;
        } 

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
            return Boxed_Value(const_var(val));
          } else if (long_) {
            unsigned long val;
            ss >> val;
            return Boxed_Value(const_var(val));
          } else {
            unsigned int val;
            ss >> val;
            return Boxed_Value(const_var(val));
          }
        } else {
          if (longlong_)
          {
            int64_t val;
            ss >> val;   
            return Boxed_Value(const_var(val));
          } else if (long_) {
            long val;
            ss >> val;
            return Boxed_Value(const_var(val));
          } else {
            int val;
            ss >> val;
            return Boxed_Value(const_var(val));
          }
        }
      }

      /**
       * Reads a number from the input, detecting if it's an integer or floating point
       */
      bool Num(bool t_capture = false) {
        SkipWS();

        if (!t_capture) {
          return Hex_() || Float_();
        }
        else {
          std::string::const_iterator start = m_input_pos;
          int prev_col = m_col;
          int prev_line = m_line;
          if (has_more_input() && char_in_alphabet(*m_input_pos, detail::float_alphabet) ) {
            if (Hex_()) {
              std::string match(start, m_input_pos);
              Boxed_Value i = buildInt(std::hex, match);
              AST_NodePtr t(new eval::Int_AST_Node(match, i, m_filename, prev_line, prev_col, m_line, m_col));
              m_match_stack.push_back(t);
              return true;
            }
            if (Binary_()) {
              std::string match(start, m_input_pos);
              int64_t temp_int = 0;
              size_t pos = 0, end = match.length();

              while ((pos < end) && (pos < (2 + sizeof(int) * 8))) {
                temp_int <<= 1;
                if (match[pos] == '1') {
                  temp_int += 1;
                }
                ++pos;
              }

              Boxed_Value i;
              if (match.length() <= sizeof(int) * 8)
              {
                i = Boxed_Value(const_var(int(temp_int)));
              } else {
                i = Boxed_Value(const_var(temp_int));
              }

              AST_NodePtr t(new eval::Int_AST_Node(match, i, m_filename, prev_line, prev_col, m_line, m_col));
              m_match_stack.push_back(t);
              return true;
            }
            if (Float_()) {
              std::string match(start, m_input_pos);
              Boxed_Value f = buildFloat(match);
              AST_NodePtr t(new eval::Float_AST_Node(match, f, m_filename, prev_line, prev_col, m_line, m_col));
              m_match_stack.push_back(t);
              return true;
            }
            else {
              IntSuffix_();
              std::string match(start, m_input_pos);
              if ((match.size() > 0) && (match[0] == '0')) {
                Boxed_Value i = buildInt(std::oct, match);
                AST_NodePtr t(new eval::Int_AST_Node(match, i, m_filename, prev_line, prev_col, m_line, m_col));
                m_match_stack.push_back(t);
              }
              else {
                Boxed_Value i = buildInt(std::dec, match);
                AST_NodePtr t(new eval::Int_AST_Node(match, i, m_filename, prev_line, prev_col, m_line, m_col));
                m_match_stack.push_back(t);
              }
              return true;
            }
          }
          else {
            return false;
          }
        }
      }

      /**
       * Reads an identifier from input which conforms to C's identifier naming conventions, without skipping initial whitespace
       */
      bool Id_() {
        bool retval = false;
        if (has_more_input() && char_in_alphabet(*m_input_pos, detail::id_alphabet)) {
          retval = true;
          while (has_more_input() && char_in_alphabet(*m_input_pos, detail::keyword_alphabet) ) {
            ++m_input_pos;
            ++m_col;
          }
        }
        else if (has_more_input() && (*m_input_pos == '`')) {
          retval = true;
          ++m_col;
          ++m_input_pos;
          std::string::const_iterator start = m_input_pos;

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
        }
        return retval;
      }

      /**
       * Reads (and potentially captures) an identifier from input
       */
      bool Id(bool t_capture = false) {
        SkipWS();

        if (!t_capture) {
          return Id_();
        }
        else {
          std::string::const_iterator start = m_input_pos;
          int prev_col = m_col;
          int prev_line = m_line;
          if (Id_()) {
            if (*start == '`') {
              //Id Literal
              std::string match(start+1, m_input_pos-1);
              AST_NodePtr t(new eval::Id_AST_Node(match, m_filename, prev_line, prev_col, m_line, m_col));
              m_match_stack.push_back(t);
              return true;
            }
            else {
              std::string match(start, m_input_pos);
              AST_NodePtr t(new eval::Id_AST_Node(match, m_filename, prev_line, prev_col, m_line, m_col));
              m_match_stack.push_back(t);
              return true;
            }
          }
          else {
            return false;
          }
        }
      }

      /**
       * Checks for a node annotation of the form "#<annotation>"
       */
      bool Annotation() {
        SkipWS();
        std::string::const_iterator start = m_input_pos;
        int prev_col = m_col;
        int prev_line = m_line;
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
          AST_NodePtr t(new eval::Annotation_AST_Node(match, m_filename, prev_line, prev_col, m_line, m_col));
          m_match_stack.push_back(t);
          return true;
        }
        else {
          return false;
        }
      }

      /**
       * Reads a quoted string from input, without skipping initial whitespace
       */
      bool Quoted_String_() {
        bool retval = false;
        char prev_char = 0;
        if (has_more_input() && (*m_input_pos == '\"')) {
          retval = true;
          prev_char = *m_input_pos;
          ++m_input_pos;
          ++m_col;

          while (has_more_input() && ((*m_input_pos != '\"') || ((*m_input_pos == '\"') && (prev_char == '\\')))) {
            if (!Eol_()) {
              if (prev_char == '\\') {
                prev_char = 0;
              }
              else {
                prev_char = *m_input_pos;
              }
              ++m_input_pos;
              ++m_col;
            }
          }

          if (has_more_input()) {
            ++m_input_pos;
            ++m_col;
          }
          else {
            throw exception::eval_error("Unclosed quoted string", File_Position(m_line, m_col), *m_filename);
          }
        }
        return retval;
      }

      /**
       * Reads (and potentially captures) a quoted string from input.  Translates escaped sequences.
       */
      bool Quoted_String(bool t_capture = false) {
        SkipWS();

        if (!t_capture) {
          return Quoted_String_();
        }
        else {
          std::string::const_iterator start = m_input_pos;
          int prev_col = m_col;
          int prev_line = m_line;
          if (Quoted_String_()) {
            std::string match;
            bool is_escaped = false;
            bool is_interpolated = false;
            bool saw_interpolation_marker = false;
            size_t prev_stack_top = m_match_stack.size();

            std::string::const_iterator s = start + 1, end = m_input_pos - 1;

            while (s != end) {
              if (saw_interpolation_marker) {
                if (*s == '{') {
                  //We've found an interpolation point

                  if (is_interpolated) {
                    //If we've seen previous interpolation, add on instead of making a new one

                    AST_NodePtr t(new eval::Quoted_String_AST_Node(match, m_filename, prev_line, prev_col, m_line, m_col));
                    m_match_stack.push_back(t);

                    build_match(AST_NodePtr(new eval::Addition_AST_Node()), prev_stack_top);
                  }
                  else {
                    AST_NodePtr t(new eval::Quoted_String_AST_Node(match, m_filename, prev_line, prev_col, m_line, m_col));
                    m_match_stack.push_back(t);
                  }

                  //We've finished with the part of the string up to this point, so clear it
                  match = "";

                  std::string eval_match;

                  ++s;
                  while ((*s != '}') && (s != end)) {
                    eval_match.push_back(*s);
                    ++s;
                  }
                  if (*s == '}') {
                    is_interpolated = true;
                    ++s;

                    size_t tostr_stack_top = m_match_stack.size();

                    AST_NodePtr tostr(new eval::Id_AST_Node("to_string", m_filename, prev_line, prev_col, m_line, m_col));
                    m_match_stack.push_back(tostr);

                    size_t ev_stack_top = m_match_stack.size();

                    AST_NodePtr ev(new eval::Id_AST_Node("eval", m_filename, prev_line, prev_col, m_line, m_col));
                    m_match_stack.push_back(ev);

                    size_t arg_stack_top = m_match_stack.size();

                    AST_NodePtr t(new eval::Quoted_String_AST_Node(eval_match, m_filename, prev_line, prev_col, m_line, m_col));
                    m_match_stack.push_back(t);

                    build_match(AST_NodePtr(new eval::Arg_List_AST_Node()), arg_stack_top);

                    build_match(AST_NodePtr(new eval::Inplace_Fun_Call_AST_Node()), ev_stack_top);

                    build_match(AST_NodePtr(new eval::Arg_List_AST_Node()), ev_stack_top);

                    build_match(AST_NodePtr(new eval::Fun_Call_AST_Node()), tostr_stack_top);

                    build_match(AST_NodePtr(new eval::Addition_AST_Node()), prev_stack_top);
                  }
                  else {
                    throw exception::eval_error("Unclosed in-string eval", File_Position(prev_line, prev_col), *m_filename);
                  }
                }
                else {
                  match.push_back('$');
                }
                saw_interpolation_marker = false;
              }
              else {
                if (*s == '\\') {
                  if (is_escaped) {
                    match.push_back('\\');
                    is_escaped = false;
                  }
                  else {
                    is_escaped = true;
                  }
                }
                else {
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
                  }
                  else if (*s == '$') {
                    saw_interpolation_marker = true;
                  }
                  else {
                    match.push_back(*s);
                  }
                  is_escaped = false;
                }
                ++s;
              }
            }
            if (is_interpolated) {
              AST_NodePtr t(new eval::Quoted_String_AST_Node(match, m_filename, prev_line, prev_col, m_line, m_col));
              m_match_stack.push_back(t);

              build_match(AST_NodePtr(new eval::Addition_AST_Node()), prev_stack_top);
            }
            else {
              AST_NodePtr t(new eval::Quoted_String_AST_Node(match, m_filename, prev_line, prev_col, m_line, m_col));
              m_match_stack.push_back(t);
            }
            return true;
          }
          else {
            return false;
          }
          }
        }

        /**
         * Reads a character group from input, without skipping initial whitespace
         */
        bool Single_Quoted_String_() {
          bool retval = false;
          char prev_char = 0;
          if (has_more_input() && (*m_input_pos == '\'')) {
            retval = true;
            prev_char = *m_input_pos;
            ++m_input_pos;
            ++m_col;

            while (has_more_input() && ((*m_input_pos != '\'') || ((*m_input_pos == '\'') && (prev_char == '\\')))) {
              if (!Eol_()) {
                if (prev_char == '\\') {
                  prev_char = 0;
                }
                else {
                  prev_char = *m_input_pos;
                }
                ++m_input_pos;
                ++m_col;
              }
            }

            if (m_input_pos != m_input_end) {
              ++m_input_pos;
              ++m_col;
            }
            else {
              throw exception::eval_error("Unclosed single-quoted string", File_Position(m_line, m_col), *m_filename);
            }
          }
          return retval;
        }

        /**
         * Reads (and potentially captures) a char group from input.  Translates escaped sequences.
         */
        bool Single_Quoted_String(bool t_capture = false) {
          SkipWS();

          if (!t_capture) {
            return Single_Quoted_String_();
          }
          else {
            std::string::const_iterator start = m_input_pos;
            int prev_col = m_col;
            int prev_line = m_line;
            if (Single_Quoted_String_()) {
              std::string match;
              bool is_escaped = false;
              for (std::string::const_iterator s = start + 1, end = m_input_pos - 1; s != end; ++s) {
                if (*s == '\\') {
                  if (is_escaped) {
                    match.push_back('\\');
                    is_escaped = false;
                  }
                  else {
                    is_escaped = true;
                  }
                }
                else {
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
                  }
                  else {
                    match.push_back(*s);
                  }
                  is_escaped = false;
                }
              }
              AST_NodePtr t(new eval::Single_Quoted_String_AST_Node(match, m_filename, prev_line, prev_col, m_line, m_col));
              m_match_stack.push_back(t);
              return true;
            }
            else {
              return false;
            }
          }
        }

        /**
         * Reads a char from input if it matches the parameter, without skipping initial whitespace
         */
        bool Char_(char c) {
          bool retval = false;
          if (has_more_input() && (*m_input_pos == c)) {
            ++m_input_pos;
            ++m_col;
            retval = true;
          }

          return retval;
        }

        /**
         * Reads (and potentially captures) a char from input if it matches the parameter
         */
        bool Char(char t_c, bool t_capture = false) {
          SkipWS();

          if (!t_capture) {
            return Char_(t_c);
          }
          else {
            std::string::const_iterator start = m_input_pos;
            int prev_col = m_col;
            int prev_line = m_line;
            if (Char_(t_c)) {
              std::string match(start, m_input_pos);
              AST_NodePtr t(new eval::Char_AST_Node(match, m_filename, prev_line, prev_col, m_line, m_col));
              m_match_stack.push_back(t);
              return true;
            }
            else {
              return false;
            }
          }
        }

        /**
         * Reads a string from input if it matches the parameter, without skipping initial whitespace
         */
        bool Keyword_(const char *t_s) {
          bool retval = false;
          int len = static_cast<int>(strlen(t_s));

          if ((m_input_end - m_input_pos) >= len) {
            std::string::const_iterator tmp = m_input_pos;
            for (int i = 0; i < len; ++i) {
              if (*tmp != t_s[i]) {
                return false;
              }
              ++tmp;
            }
            retval = true;
            m_input_pos = tmp;
            m_col += len;
          }

          return retval;
        }

        /**
         * Reads (and potentially captures) a string from input if it matches the parameter
         */
        bool Keyword(const char *t_s, bool t_capture = false) {
          SkipWS();
          std::string::const_iterator start = m_input_pos;
          int prev_col = m_col;
          int prev_line = m_line;
          bool retval = Keyword_(t_s);
          // ignore substring matches
          if ( retval && has_more_input() && char_in_alphabet(*m_input_pos, detail::keyword_alphabet) ) {
            m_input_pos = start;
            m_col = prev_col;
            m_line = prev_line;
            retval = false;
          }

          if ( t_capture && retval ) {
            std::string match(start, m_input_pos);
            AST_NodePtr t(new eval::Str_AST_Node(match, m_filename, prev_line, prev_col, m_line, m_col));
            m_match_stack.push_back(t);
          }
          return retval;
        }

        /**
         * Reads a symbol group from input if it matches the parameter, without skipping initial whitespace
         */
        bool Symbol_(const char *t_s) {
          bool retval = false;
          int len = static_cast<int>(strlen(t_s));

          if ((m_input_end - m_input_pos) >= len) {
            std::string::const_iterator tmp = m_input_pos;
            for (int i = 0; i < len; ++i) {
              if (*tmp != t_s[i]) {
                return false;
              }
              ++tmp;
            }
            retval = true;
            m_input_pos = tmp;
            m_col += len;
          }

          return retval;
        }

        /**
         * Reads (and potentially captures) a symbol group from input if it matches the parameter
         */
        bool Symbol(const char *t_s, bool t_capture = false, bool t_disallow_prevention=false) {
          SkipWS();
          std::string::const_iterator start = m_input_pos;
          int prev_col = m_col;
          int prev_line = m_line;
          bool retval = Symbol_(t_s);
          // ignore substring matches
          if (retval && has_more_input() && (t_disallow_prevention == false) && char_in_alphabet(*m_input_pos,detail::symbol_alphabet)) {
            m_input_pos = start;
            m_col = prev_col;
            m_line = prev_line;
            retval = false;
          }

          if ( t_capture && retval ) {
            std::string match(start, m_input_pos);
            AST_NodePtr t(new eval::Str_AST_Node(match, m_filename, prev_line, prev_col, m_line, m_col));
            m_match_stack.push_back(t);
          }

          return retval;
        }

        /**
         * Reads an end-of-line group from input, without skipping initial whitespace
         */
        bool Eol_() {
          bool retval = false;

          if (has_more_input() && (Symbol_("\r\n") || Char_('\n'))) {
            retval = true;
            ++m_line;
            m_col = 1;
          }
          else if (has_more_input() && Char_(';')) {
            retval = true;
          }

          return retval;
        }

        /**
         * Reads (and potentially captures) an end-of-line group from input
         */
        bool Eol(bool t_capture = false) {
          SkipWS();

          if (!t_capture) {
            return Eol_();
          }
          else {
            std::string::const_iterator start = m_input_pos;
            int prev_col = m_col;
            int prev_line = m_line;
            if (Eol_()) {
              std::string match(start, m_input_pos);
              AST_NodePtr t(new eval::Eol_AST_Node(match, m_filename, prev_line, prev_col, m_line, m_col));
              m_match_stack.push_back(t);
              return true;
            }
            else {
              return false;
            }
          }
        }

        /**
         * Reads a comma-separated list of values from input
         */
        bool Arg_List() {

          SkipWS(true);
          bool retval = false;

          size_t prev_stack_top = m_match_stack.size();

          if (Equation()) {
            retval = true;
            while (Eol()) {}
            if (Char(',')) {
              do {
                while (Eol()) {}
                if (!Equation()) {
                  throw exception::eval_error("Unexpected value in parameter list", File_Position(m_line, m_col), *m_filename);
                }
              } while (retval && Char(','));
            }
            build_match(AST_NodePtr(new eval::Arg_List_AST_Node()), prev_stack_top);
          }

          SkipWS(true);

          return retval;
        }

        /**
         * Reads possible special container values, including ranges and map_pairs
         */
        bool Container_Arg_List() {
          bool retval = false;
          SkipWS(true);

          size_t prev_stack_top = m_match_stack.size();

          if (Value_Range()) {
            retval = true;
            build_match(AST_NodePtr(new eval::Arg_List_AST_Node()), prev_stack_top);
          }
          else if (Map_Pair()) {
            retval = true;
            while (Eol()) {}
            if (Char(',')) {
              do {
                while (Eol()) {}
                if (!Map_Pair()) {
                  throw exception::eval_error("Unexpected value in container", File_Position(m_line, m_col), *m_filename);
                }
              } while (retval && Char(','));
            }
            build_match(AST_NodePtr(new eval::Arg_List_AST_Node()), prev_stack_top);
          }
          else if (Operator()) {
            retval = true;
            while (Eol()) {}
            if (Char(',')) {
              do {
                while (Eol()) {}
                if (!Operator()) {
                  throw exception::eval_error("Unexpected value in container", File_Position(m_line, m_col), *m_filename);
                }
              } while (retval && Char(','));
            }
            build_match(AST_NodePtr(new eval::Arg_List_AST_Node()), prev_stack_top);
          }

          SkipWS(true);

          return retval;

        }

        /**
         * Reads a lambda (anonymous function) from input
         */
        bool Lambda() {
          bool retval = false;

          size_t prev_stack_top = m_match_stack.size();

          if (Keyword("fun")) {
            retval = true;

            if (Char('(')) {
              Arg_List();
              if (!Char(')')) {
                throw exception::eval_error("Incomplete anonymous function", File_Position(m_line, m_col), *m_filename);
              }
            }

            while (Eol()) {}

            if (!Block()) {
              throw exception::eval_error("Incomplete anonymous function", File_Position(m_line, m_col), *m_filename);
            }

            build_match(AST_NodePtr(new eval::Lambda_AST_Node()), prev_stack_top);
          }

          return retval;
        }

        /**
         * Reads a function definition from input
         */
        bool Def() {
          bool retval = false;
          bool is_annotated = false;
          bool is_method = false;
          AST_NodePtr annotation;

          if (Annotation()) {
            while (Eol_()) {}
            annotation = m_match_stack.back();
            m_match_stack.pop_back();
            is_annotated = true;
          }

          size_t prev_stack_top = m_match_stack.size();

          if (Keyword("def")) {
            retval = true;

            if (!Id(true)) {
              throw exception::eval_error("Missing function name in definition", File_Position(m_line, m_col), *m_filename);
            }

            if (Symbol("::", false)) {
              //We're now a method
              is_method = true;

              if (!Id(true)) {
                throw exception::eval_error("Missing method name in definition", File_Position(m_line, m_col), *m_filename);
              }
            }

            if (Char('(')) {
              Arg_List();
              if (!Char(')')) {
                throw exception::eval_error("Incomplete function definition", File_Position(m_line, m_col), *m_filename);
              }
            }

            while (Eol()) {}

            if (Char(':')) {
              if (!Operator()) {
                throw exception::eval_error("Missing guard expression for function", File_Position(m_line, m_col), *m_filename);
              }
            }

            while (Eol()) {}
            if (!Block()) {
              throw exception::eval_error("Incomplete function definition", File_Position(m_line, m_col), *m_filename);
            }

            if (is_method) {
              build_match(AST_NodePtr(new eval::Method_AST_Node()), prev_stack_top);
            }
            else {
              build_match(AST_NodePtr(new eval::Def_AST_Node()), prev_stack_top);
            }

            if (is_annotated) {
              m_match_stack.back()->annotation = annotation;
            }
          }

          return retval;
        }

        /**
         * Reads a function definition from input
         */
        bool Try() {
          bool retval = false;

          size_t prev_stack_top = m_match_stack.size();

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
                size_t catch_stack_top = m_match_stack.size();
                if (Char('(')) {
                  if (!(Id(true) && Char(')'))) {
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
                build_match(AST_NodePtr(new eval::Catch_AST_Node()), catch_stack_top);
                has_matches = true;
              }
            }
            while (Eol()) {}
            if (Keyword("finally", false)) {
              size_t finally_stack_top = m_match_stack.size();

              while (Eol()) {}

              if (!Block()) {
                throw exception::eval_error("Incomplete 'finally' block", File_Position(m_line, m_col), *m_filename);
              }
              build_match(AST_NodePtr(new eval::Finally_AST_Node()), finally_stack_top);
            }

            build_match(AST_NodePtr(new eval::Try_AST_Node()), prev_stack_top);
          }

          return retval;
        }

        /**
         * Reads an if/elseif/else block from input
         */
        bool If() {
          bool retval = false;

          size_t prev_stack_top = m_match_stack.size();

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
                  AST_NodePtr back(m_match_stack.back());
                  m_match_stack.back() = AST_NodePtr(new eval::If_AST_Node("else if"));
                  m_match_stack.back()->start = back->start;
                  m_match_stack.back()->end = back->end;
                  m_match_stack.back()->children = back->children;
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
                }
                else {
                  while (Eol()) {}

                  if (!Block()) {
                    throw exception::eval_error("Incomplete 'else' block", File_Position(m_line, m_col), *m_filename);
                  }
                  has_matches = true;
                }
              }
            }

            build_match(AST_NodePtr(new eval::If_AST_Node()), prev_stack_top);
          }

          return retval;
        }

        /**
         * Reads a while block from input
         */
        bool While() {
          bool retval = false;

          size_t prev_stack_top = m_match_stack.size();

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

            build_match(AST_NodePtr(new eval::While_AST_Node()), prev_stack_top);
          }

          return retval;
        }


        /**
         * Reads the C-style for conditions from input
         */
        bool For_Guards() {
          if (!(Equation() && Eol()))
          {
            if (!Eol())
            {
              throw exception::eval_error("'for' loop initial statment missing", File_Position(m_line, m_col), *m_filename);          
            } else {
              AST_NodePtr t(new eval::Noop_AST_Node());
              m_match_stack.push_back(t);
            }
          }

          if (!(Equation() && Eol()))
          {
            if (!Eol())
            {
              throw exception::eval_error("'for' loop condition missing", File_Position(m_line, m_col), *m_filename);          
            } else {
              AST_NodePtr t(new eval::Noop_AST_Node());
              m_match_stack.push_back(t);
            }
          }

          if (!Equation())
          {
            AST_NodePtr t(new eval::Noop_AST_Node());
            m_match_stack.push_back(t);
          }

          return true; 
        }

        /**
         * Reads a for block from input
         */
        bool For() {
          bool retval = false;

          size_t prev_stack_top = m_match_stack.size();

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

            build_match(AST_NodePtr(new eval::For_AST_Node()), prev_stack_top);
          }

          return retval;
        }

        /**
         * Reads a case block from input
         */
        bool Case() {
          bool retval = false;

          size_t prev_stack_top = m_match_stack.size();

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
            
            build_match(AST_NodePtr(new eval::Case_AST_Node()), prev_stack_top);
          }
          else if (Keyword("default")) {
            while (Eol()) {}

            if (!Block()) {
              throw exception::eval_error("Incomplete 'default' block", File_Position(m_line, m_col), *m_filename);
            }
            
            build_match(AST_NodePtr(new eval::Default_AST_Node()), prev_stack_top);
          }            
          
          return retval;
        }
      
        /**
         * Reads a switch statement from input
         */
        bool Switch() {
          size_t prev_stack_top = m_match_stack.size();

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
            
            build_match(AST_NodePtr(new eval::Switch_AST_Node()), prev_stack_top);
            return true;

          } else {
            return false;
          }

        }

        /**
         * Reads a curly-brace C-style block from input
         */
        bool Block() {
          bool retval = false;

          size_t prev_stack_top = m_match_stack.size();

          if (Char('{')) {
            retval = true;

            Statements();
            if (!Char('}')) {
              throw exception::eval_error("Incomplete block", File_Position(m_line, m_col), *m_filename);
            }

            build_match(AST_NodePtr(new eval::Block_AST_Node()), prev_stack_top);
          }

          return retval;
        }

        /**
         * Reads a return statement from input
         */
        bool Return() {
          bool retval = false;

          size_t prev_stack_top = m_match_stack.size();

          if (Keyword("return")) {
            retval = true;

            Operator();
            build_match(AST_NodePtr(new eval::Return_AST_Node()), prev_stack_top);
          }

          return retval;
        }

        /**
         * Reads a break statement from input
         */
        bool Break() {
          bool retval = false;

          size_t prev_stack_top = m_match_stack.size();

          if (Keyword("break")) {
            retval = true;

            build_match(AST_NodePtr(new eval::Break_AST_Node()), prev_stack_top);
          }

          return retval;
        }

        /**
         * Reads a continue statement from input
         */
        bool Continue() {
          bool retval = false;

          size_t prev_stack_top = m_match_stack.size();

          if (Keyword("continue")) {
            retval = true;

            build_match(AST_NodePtr(new eval::Continue_AST_Node()), prev_stack_top);
          }

          return retval;
        }

        /**
         * Reads a dot expression(member access), then proceeds to check if it's a function or array call
         */
        bool Dot_Fun_Array() {
          bool retval = false;

          size_t prev_stack_top = m_match_stack.size();
          if (Lambda() || Num(true) || Quoted_String(true) || Single_Quoted_String(true) ||
              Paren_Expression() || Inline_Container() || Id(true)) {
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

                build_match(AST_NodePtr(new eval::Fun_Call_AST_Node()), prev_stack_top);
                /// \todo Work around for method calls until we have a better solution
                if (!m_match_stack.back()->children.empty()) {
                  if (m_match_stack.back()->children[0]->identifier == AST_Node_Type::Dot_Access) {
                    AST_NodePtr dot_access = m_match_stack.back()->children[0];
                    AST_NodePtr func_call = m_match_stack.back();
                    m_match_stack.pop_back();
                    func_call->children.erase(func_call->children.begin());
                    func_call->children.insert(func_call->children.begin(), dot_access->children.back());
                    dot_access->children.pop_back();
                    dot_access->children.push_back(func_call);
                    m_match_stack.push_back(dot_access);
                  }
                }
              }
              else if (Char('[')) {
                has_more = true;

                if (!(Operator() && Char(']'))) {
                  throw exception::eval_error("Incomplete array access", File_Position(m_line, m_col), *m_filename);
                }

                build_match(AST_NodePtr(new eval::Array_Call_AST_Node()), prev_stack_top);
              }
              else if (Symbol(".", true)) {
                has_more = true;
                if (!(Id(true))) {
                  throw exception::eval_error("Incomplete array access", File_Position(m_line, m_col), *m_filename);
                }

                build_match(AST_NodePtr(new eval::Dot_Access_AST_Node()), prev_stack_top);
              }
            }
          }

          return retval;
        }

        /**
         * Reads a variable declaration from input
         */
        bool Var_Decl() {
          bool retval = false;

          size_t prev_stack_top = m_match_stack.size();

          if (Keyword("auto") || Keyword("var")) {
            retval = true;

            if (!(Reference() || Id(true))) {
              throw exception::eval_error("Incomplete variable declaration", File_Position(m_line, m_col), *m_filename);
            }

            build_match(AST_NodePtr(new eval::Var_Decl_AST_Node()), prev_stack_top);
          }
          else if (Keyword("attr")) {
            retval = true;

            if (!Id(true)) {
              throw exception::eval_error("Incomplete attribute declaration", File_Position(m_line, m_col), *m_filename);
            }
            if (!Symbol("::", false)) {
              throw exception::eval_error("Incomplete attribute declaration", File_Position(m_line, m_col), *m_filename);
            }
            if (!Id(true)) {
              throw exception::eval_error("Missing attribute name in definition", File_Position(m_line, m_col), *m_filename);
            }


            build_match(AST_NodePtr(new eval::Attr_Decl_AST_Node()), prev_stack_top);
          }

          return retval;
        }

        /**
         * Reads an expression surrounded by parentheses from input
         */
        bool Paren_Expression() {
          bool retval = false;

          if (Char('(')) {
            retval = true;
            if (!Operator()) {
              throw exception::eval_error("Incomplete expression", File_Position(m_line, m_col), *m_filename);
            }
            if (!Char(')')) {
              throw exception::eval_error("Missing closing parenthesis ')'", File_Position(m_line, m_col), *m_filename);
            }
          }
          return retval;
        }

        /**
         * Reads, and identifies, a short-form container initialization from input
         */
        bool Inline_Container() {
          bool retval = false;

          size_t prev_stack_top = m_match_stack.size();

          if (Char('[')) {
            retval = true;
            Container_Arg_List();

            if (!Char(']')) {
              throw exception::eval_error("Missing closing square bracket ']' in container initializer", File_Position(m_line, m_col), *m_filename);
            }
            if ((prev_stack_top != m_match_stack.size()) && (m_match_stack.back()->children.size() > 0)) {
              if (m_match_stack.back()->children[0]->identifier == AST_Node_Type::Value_Range) {
                build_match(AST_NodePtr(new eval::Inline_Range_AST_Node()), prev_stack_top);
              }
              else if (m_match_stack.back()->children[0]->identifier == AST_Node_Type::Map_Pair) {
                build_match(AST_NodePtr(new eval::Inline_Map_AST_Node()), prev_stack_top);
              }
              else {
                build_match(AST_NodePtr(new eval::Inline_Array_AST_Node()), prev_stack_top);
              }
            }
            else {
              build_match(AST_NodePtr(new eval::Inline_Array_AST_Node()), prev_stack_top);
            }
          }

          return retval;
        }

        bool Reference() {
          bool retval = false;

          size_t prev_stack_top = m_match_stack.size();

          if (Symbol("&", false)) {
            retval = true;

            if (!Id(true)) {
              throw exception::eval_error("Incomplete '&' expression", File_Position(m_line, m_col), *m_filename);
            }

            build_match(AST_NodePtr(
                  new eval::Reference_AST_Node()), prev_stack_top);
          }

          return retval;
        }

        /**
         * Reads a unary prefixed expression from input
         */
        bool Prefix() {
          bool retval = false;

          size_t prev_stack_top = m_match_stack.size();

          if (Symbol("++", true)) {
            retval = true;

            if (!Operator(m_operators.size()-1)) {
              throw exception::eval_error("Incomplete '++' expression", File_Position(m_line, m_col), *m_filename);
            }

            build_match(AST_NodePtr(new eval::Prefix_AST_Node()), prev_stack_top);
          }
          else if (Symbol("--", true)) {
            retval = true;

            if (!Operator(m_operators.size()-1)) {
              throw exception::eval_error("Incomplete '--' expression", File_Position(m_line, m_col), *m_filename);
            }

            build_match(AST_NodePtr(new eval::Prefix_AST_Node()), prev_stack_top);
          }
          else if (Char('-', true)) {
            retval = true;

            if (!Operator(m_operators.size()-1)) {
              throw exception::eval_error("Incomplete unary '-' expression", File_Position(m_line, m_col), *m_filename);
            }

            build_match(AST_NodePtr(new eval::Prefix_AST_Node()), prev_stack_top);
          }
          else if (Char('+', true)) {
            retval = true;

            if (!Operator(m_operators.size()-1)) {
              throw exception::eval_error("Incomplete unary '+' expression", File_Position(m_line, m_col), *m_filename);
            }

            build_match(AST_NodePtr(new eval::Prefix_AST_Node()), prev_stack_top);
          }
          else if (Char('!', true)) {
            retval = true;

            if (!Operator(m_operators.size()-1)) {
              throw exception::eval_error("Incomplete '!' expression", File_Position(m_line, m_col), *m_filename);
            }

            build_match(AST_NodePtr(new eval::Prefix_AST_Node()), prev_stack_top);
          }
          else if (Char('~', true)) {
            retval = true;

            if (!Operator(m_operators.size()-1)) {
              throw exception::eval_error("Incomplete '~' expression", File_Position(m_line, m_col), *m_filename);
            }

            build_match(AST_NodePtr(new eval::Prefix_AST_Node()), prev_stack_top);
          }
          else if (Char('&', true)) {
            retval = true;

            if (!Operator(m_operators.size()-1)) {
              throw exception::eval_error("Incomplete '~' expression", File_Position(m_line, m_col), *m_filename);
            }

            build_match(AST_NodePtr(new eval::Prefix_AST_Node()), prev_stack_top);
          }

          return retval;
        }

        /**
         * Parses any of a group of 'value' style ast_node groups from input
         */
        bool Value() {
          if (Var_Decl() || Dot_Fun_Array() || Prefix()) {
            return true;
          }
          else {
            return false;
          }
        }

        bool Operator_Helper(size_t t_precedence) {
          for (size_t i = 0; i < m_operator_matches[t_precedence].size(); ++i) {
            if (Symbol(m_operator_matches[t_precedence][i].c_str(), true)) {
              return true;
            }
          }
          return false;
        }

        bool Operator(size_t t_precedence = 0) {
          bool retval = false;
          AST_NodePtr oper;
          size_t prev_stack_top = m_match_stack.size();

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

                  switch (m_operators[t_precedence]) {
                    case(AST_Node_Type::Comparison) :
                      build_match(AST_NodePtr(new eval::Comparison_AST_Node()), prev_stack_top);
                      break;
                    case(AST_Node_Type::Ternary_Cond) :
                      m_match_stack.erase(m_match_stack.begin() + m_match_stack.size() - 2,
                          m_match_stack.begin() + m_match_stack.size() - 1);
                      if (Symbol(":")) {
                        if (!Operator(t_precedence+1)) {
                          throw exception::eval_error("Incomplete "
                              + std::string(ast_node_type_to_string(m_operators[t_precedence])) + " expression",
                              File_Position(m_line, m_col), *m_filename);
                        }
                        build_match(AST_NodePtr(new eval::Ternary_Cond_AST_Node()), prev_stack_top);
                      }
                      else {
                        throw exception::eval_error("Incomplete "
                            + std::string(ast_node_type_to_string(m_operators[t_precedence])) + " expression",
                            File_Position(m_line, m_col), *m_filename);
                      }
                      break;
                    case(AST_Node_Type::Addition) :
                      oper = m_match_stack.at(m_match_stack.size()-2);
                      m_match_stack.erase(m_match_stack.begin() + m_match_stack.size() - 2,
                          m_match_stack.begin() + m_match_stack.size() - 1);
                      if (oper->text == "+") {
                        build_match(AST_NodePtr(new eval::Addition_AST_Node()), prev_stack_top);
                      }
                      else if (oper->text == "-") {
                        build_match(AST_NodePtr(new eval::Subtraction_AST_Node()), prev_stack_top);
                      }
                      break;
                    case(AST_Node_Type::Multiplication) :
                      oper = m_match_stack.at(m_match_stack.size()-2);
                      m_match_stack.erase(m_match_stack.begin() + m_match_stack.size() - 2,
                          m_match_stack.begin() + m_match_stack.size() - 1);
                      if (oper->text == "*") {
                        build_match(AST_NodePtr(new eval::Multiplication_AST_Node()), prev_stack_top);
                      }
                      else if (oper->text == "/") {
                        build_match(AST_NodePtr(new eval::Division_AST_Node()), prev_stack_top);
                      }
                      else if (oper->text == "%") {
                        build_match(AST_NodePtr(new eval::Modulus_AST_Node()), prev_stack_top);
                      }
                      break;
                    case(AST_Node_Type::Shift) :
                      build_match(AST_NodePtr(new eval::Shift_AST_Node()), prev_stack_top);
                      break;
                    case(AST_Node_Type::Equality) :
                      build_match(AST_NodePtr(new eval::Equality_AST_Node()), prev_stack_top);
                      break;
                    case(AST_Node_Type::Bitwise_And) :
                      build_match(AST_NodePtr(new eval::Bitwise_And_AST_Node()), prev_stack_top);
                      break;
                    case(AST_Node_Type::Bitwise_Xor) :
                      build_match(AST_NodePtr(new eval::Bitwise_Xor_AST_Node()), prev_stack_top);
                      break;
                    case(AST_Node_Type::Bitwise_Or) :
                      build_match(AST_NodePtr(new eval::Bitwise_Or_AST_Node()), prev_stack_top);
                      break;
                    case(AST_Node_Type::Logical_And) :
                      build_match(AST_NodePtr(new eval::Logical_And_AST_Node()), prev_stack_top);
                      break;
                    case(AST_Node_Type::Logical_Or) :
                      build_match(AST_NodePtr(new eval::Logical_Or_AST_Node()), prev_stack_top);
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

        /**
         * Reads a pair of values used to create a map initialization from input
         */
        bool Map_Pair() {
          bool retval = false;

          size_t prev_stack_top = m_match_stack.size();
          std::string::const_iterator prev_pos = m_input_pos;
          int prev_col = m_col;

          if (Operator()) {
            if (Symbol(":")) {
              retval = true;
              if (!Operator()) {
                throw exception::eval_error("Incomplete map pair", File_Position(m_line, m_col), *m_filename);
              }

              build_match(AST_NodePtr(new eval::Map_Pair_AST_Node()), prev_stack_top);
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

        /**
         * Reads a pair of values used to create a range initialization from input
         */
        bool Value_Range() {
          bool retval = false;

          size_t prev_stack_top = m_match_stack.size();
          std::string::const_iterator prev_pos = m_input_pos;
          int prev_col = m_col;

          if (Operator()) {
            if (Symbol("..")) {
              retval = true;
              if (!Operator()) {
                throw exception::eval_error("Incomplete value range", File_Position(m_line, m_col), *m_filename);
              }

              build_match(AST_NodePtr(new eval::Value_Range_AST_Node()), prev_stack_top);
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

        /**
         * Parses a string of binary equation operators
         */
        bool Equation() {
          bool retval = false;

          size_t prev_stack_top = m_match_stack.size();

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

              build_match(AST_NodePtr(new eval::Equation_AST_Node()), prev_stack_top);
            }
          }

          return retval;
        }

        /**
         * Top level parser, starts parsing of all known parses
         */
        bool Statements() {
          bool retval = false;

          bool has_more = true;
          bool saw_eol = true;

          while (has_more) {
            int prev_line = m_line;
            int prev_col = m_col;
            if (Def()) {
              if (!saw_eol) {
                throw exception::eval_error("Two function definitions missing line separator", File_Position(prev_line, prev_col), *m_filename);
              }
              has_more = true;
              retval = true;
              saw_eol = true;
            }
            else if (Try()) {
              if (!saw_eol) {
                throw exception::eval_error("Two function definitions missing line separator", File_Position(prev_line, prev_col), *m_filename);
              }
              has_more = true;
              retval = true;
              saw_eol = true;
            }
            else if (If()) {
              if (!saw_eol) {
                throw exception::eval_error("Two function definitions missing line separator", File_Position(prev_line, prev_col), *m_filename);
              }
              has_more = true;
              retval = true;
              saw_eol = true;
            }
            else if (While()) {
              if (!saw_eol) {
                throw exception::eval_error("Two function definitions missing line separator", File_Position(prev_line, prev_col), *m_filename);
              }
              has_more = true;
              retval = true;
              saw_eol = true;
            }
            else if (For()) {
              if (!saw_eol) {
                throw exception::eval_error("Two function definitions missing line separator", File_Position(prev_line, prev_col), *m_filename);
              }
              has_more = true;
              retval = true;
              saw_eol = true;
            }
            else if (Switch()) {
              if (!saw_eol) {
                throw exception::eval_error("Two function definitions missing line separator", File_Position(prev_line, prev_col), *m_filename);
              }
              has_more = true;
              retval = true;
              saw_eol = true;
            }
            else if (Return()) {
              if (!saw_eol) {
                throw exception::eval_error("Two expressions missing line separator", File_Position(prev_line, prev_col), *m_filename);
              }
              has_more = true;
              retval = true;
              saw_eol = false;
            }
            else if (Break()) {
              if (!saw_eol) {
                throw exception::eval_error("Two expressions missing line separator", File_Position(prev_line, prev_col), *m_filename);
              }
              has_more = true;
              retval = true;
              saw_eol = false;
            }
            else if (Continue()) {
              if (!saw_eol) {
                throw exception::eval_error("Two expressions missing line separator", File_Position(prev_line, prev_col), *m_filename);
              }
              has_more = true;
              retval = true;
              saw_eol = false;
            }
            else if (Block()) {
              has_more = true;
              retval = true;
              saw_eol = true;
            }
            else if (Equation()) {
              if (!saw_eol) {
                throw exception::eval_error("Two expressions missing line separator", File_Position(prev_line, prev_col), *m_filename);
              }
              has_more = true;
              retval = true;
              saw_eol = false;
            }
            else if (Eol()) {
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

        /**
         * Parses the given input string, tagging parsed ast_nodes with the given m_filename.
         */
        bool parse(const std::string &t_input, const std::string &t_fname) {
          m_input_pos = t_input.begin();
          m_input_end = t_input.end();
          m_line = 1;
          m_col = 1;
          m_filename = std::shared_ptr<std::string>(new std::string(t_fname));

          if ((t_input.size() > 1) && (t_input[0] == '#') && (t_input[1] == '!')) {
            while ((m_input_pos != m_input_end) && (!Eol())) {
              ++m_input_pos;
            }
            /// \todo respect // -*- coding: utf-8 -*- on line 1 or 2 see: http://evanjones.ca/python-utf8.html)
          }

          if (Statements()) {
            if (m_input_pos != m_input_end) {
              throw exception::eval_error("Unparsed input", File_Position(m_line, m_col), t_fname);
            }
            else {
              build_match(AST_NodePtr(new eval::File_AST_Node()), 0);
              return true;
            }
          }
          else {
            return false;
          }
        }
      };
  }
}

#endif /* CHAISCRIPT_PARSER_HPP_ */
