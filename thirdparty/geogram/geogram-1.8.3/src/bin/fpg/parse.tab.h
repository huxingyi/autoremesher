/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_PARSE_TAB_H_INCLUDED
# define YY_YY_PARSE_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    PTR_OP = 258,
    INC_OP = 259,
    DEC_OP = 260,
    LEFT_OP = 261,
    RIGHT_OP = 262,
    LE_OP = 263,
    GE_OP = 264,
    EQ_OP = 265,
    NE_OP = 266,
    AND_OP = 267,
    OR_OP = 268,
    MUL_ASSIGN = 269,
    DIV_ASSIGN = 270,
    MOD_ASSIGN = 271,
    ADD_ASSIGN = 272,
    SUB_ASSIGN = 273,
    LEFT_ASSIGN = 274,
    RIGHT_ASSIGN = 275,
    AND_ASSIGN = 276,
    XOR_ASSIGN = 277,
    OR_ASSIGN = 278,
    INT = 279,
    FLOAT = 280,
    DOUBLE = 281,
    VOID = 282,
    TYPEDEF = 283,
    INLINE = 284,
    BOOL = 285,
    IF = 286,
    ELSE = 287,
    RETURN = 288,
    SIGN = 289,
    ABS = 290,
    SQRT = 291,
    COMPARE = 292,
    ENUM = 293,
    EXTERN = 294,
    EXACT = 295,
    GROUP = 296,
    DEGREE = 297,
    INT_CONSTANT = 298,
    FLOAT_CONSTANT = 299,
    IDENTIFIER = 300,
    STRING_LITERAL = 301,
    USER_TYPE = 302
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 56 "parse.y" /* yacc.c:1909  */

  AST::Expression* exp;
  AST::Statement* stm;
  AST::CompoundStatement* compound_stm;
  AST::ExpressionStatement* exp_stm;
  AST::StatementList* statements;
  AST::VariableDeclaration *var_decl;
  AST::StatementList* stm_list;


  //AST::SwitchStatement* switch_stm;
  //AST::WhileLoop* while_loop;
  //AST::DoLoop* do_loop;
  //AST::ForLoop* for_loop;
  //AST::LabelContext* label_context;

  Declarator *declarator;
  FunctionDeclarator::ParameterDeclList *param_decl_list;
  FunctionDeclarator::ParameterDecl *param_decl;
  std::list< Declarator* > *decl_list;
  AST::ExpressionList *exp_list;

  AST::BinaryExpression::Kind binexp_kind;
  AST::UnaryExpression::Kind unexp_kind;
  AST::AssignmentExpression::Kind ass_exp_kind;
  AST::FunctionDefinition *fun_def;
  //AST::SwitchStatement::SwitchCase *switch_case;
  //AST::SwitchStatement::CaseContainer *case_container;
  FunctionType *fun_type;
  //StructType *struct_type;

  Type* type;
  int int_const;
  double float_const;
  char* string_const;

  Group_rep* single_group;
  std::list< Group_rep* > *group_list;

#line 142 "parse.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSE_TAB_H_INCLUDED  */
