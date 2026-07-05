/* parse.y

   Bison reports a shift/reduce conflict because of if/else.
   However, the correct parser is produced, and that is what
   counts.
*/

/* some C necessities */
%{
#define YYERROR_VERBOSE

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <sstream>
#include <FPG/AST.h>
#include <FPG/MSG.h>
#include <FPG/Declarator.h>
#include <FPG/SymbolEnvironment.h>

extern int lineno;
extern AST::TranslationUnit *translation_unit;

typedef std::list< std::string > StringContainer;
typedef std::list< Declarator* > DeclaratorList;

// function forward declarations
extern int yylex();

void yyerror (char *s);

void yyerror (const std::string& s);
void yywarning( const std::string& s );


AST::Expression*
makeFunctionCall( const std::string &id, AST::ExpressionList *l = nullptr );

static Type *base_type = nullptr;

static Type            *init_type   = nullptr;
static AST::Expression *init_target = nullptr;
static EnumType* current_enum;
static bool is_inline = false;
static bool is_extern = false;
static bool is_exact  = false;

struct Group_rep {
    unsigned int           degree;
    std::list<std::string> variables;
};
%}


%union {
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
};

/* correspondence between lexer tokens and parser entities */
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN
%token INT FLOAT DOUBLE VOID TYPEDEF INLINE BOOL
%token IF ELSE RETURN SIGN ABS SQRT COMPARE
%token ENUM EXTERN EXACT GROUP DEGREE
%token <int_const>    INT_CONSTANT
%token <float_const>  FLOAT_CONSTANT
%token <string_const> IDENTIFIER STRING_LITERAL
%token <type>         USER_TYPE

%type <binexp_kind>  binary_op_kind_mul binary_op_kind_add
%type <binexp_kind>  binary_op_kind_rel
%type <unexp_kind>   unary_operator
%type <ass_exp_kind> assignment_operator
%type <exp> identifier_expression primary_expression postfix_expression unary_expression
%type <exp> cast_expression multiplicative_expression additive_expression
%type <exp> relational_expression equality_expression logical_and_expression
%type <exp> logical_or_expression assignment_expression expression
%type <exp> conditional_expression
%type <exp_stm> expression_statement
%type <stm> declaration
%type <stm> initializer declarator init_declarator
%type <stm> statement
%type <compound_stm> compound_statement
%type <stm> conditional_statement jump_statement
%type <statements> statement_list init_declarator_list
%type <type> type_specifier declaration_specifiers
%type <declarator> declarator2 direct_declarator
%type <param_decl_list> parameter_list
%type <param_decl> parameter_declaration
%type <fun_def> function_definition
%type <exp_list> argument_expression_list
%type <single_group> group_var_list single_group
%type <group_list>   group_definition group_definition2
%type <int_const> group_options

%start program
%%

identifier_expression
      :  IDENTIFIER
        {
          Variable *var = symbol_env.findVariable( $1 );
          if( var == nullptr )
              yyerror( "variable " + std::string($1) + " not declared" );

          $$ = new AST::IdentifierExpression( var );
        }
      ;

primary_expression
      : identifier_expression
      | INT_CONSTANT
        { $$ = new AST::LiteralExpression($1); }
      | FLOAT_CONSTANT
        { $$ = new AST::LiteralExpression($1); }
      | '(' expression ')' { $$ = $2; }
      ;

postfix_expression
      : primary_expression
      /* special treatment for abs,sign,sqrt: */
      | ABS '(' expression ')'
        { $$ = new AST::UnaryFunction( $3, AST::UnaryFunction::XABS ); }
      | SIGN '(' expression ')'
        { $$ = new AST::UnaryFunction( $3, AST::UnaryFunction::XSIGN ); }
      | SQRT '(' expression ')'
        { $$ = new AST::UnaryFunction( $3, AST::UnaryFunction::XSQRT ); }
      | IDENTIFIER '(' ')'
        { $$ = makeFunctionCall( $1 ); }
      | IDENTIFIER '(' argument_expression_list ')'
        { $$ = makeFunctionCall( $1, $3 ); }
      | COMPARE '(' expression ',' expression ')'
        { $$ = new AST::UnaryFunction(
                  new AST::BinaryExpression( $3, $5, AST::BinaryExpression::SUB ),
                  AST::UnaryFunction::XSIGN
                );
        }
/*      | postfix_expression INC_OP
        { $$ = new AST::UnaryExpression($1,AST::UnaryExpression::INC_POST); }
      | postfix_expression DEC_OP
        { $$ = new AST::UnaryExpression($1,AST::UnaryExpression::DEC_POST); }*/
      ;

argument_expression_list
      : expression
        {   AST::ExpressionList *l = new AST::ExpressionList();
            l->push_back( $1 );
            $$ = l;
        }
      | argument_expression_list ',' expression
        {   $1->push_back( $3 );
            $$ = $1;
        }
      ;


unary_expression
      : postfix_expression
      | unary_operator postfix_expression
        { $$ = new AST::UnaryExpression($2,$1); }
      ;
      /*| INC_OP unary_expression
        {$$ = new AST::UnaryExpression($2,AST::UnaryExpression::INC_PRE); }
      | DEC_OP unary_expression
        {$$ = new AST::UnaryExpression($2,AST::UnaryExpression::DEC_PRE); }
      | SIZEOF unary_expression
        { // size of an expression can be determined statically
          Type *type = $2->getType();
          $$ = new AST::LiteralExpression( (int)type->sizeOf() );
        }
      | SIZEOF '(' type_name ')'
        { $$ = new AST::LiteralExpression ((int) $3->sizeOf() ); }
      ;*/


unary_operator
      : /*'&' { $$= AST::UnaryExpression::REF;  }
      | '*' { $$= AST::UnaryExpression::DEREF;}
      | */'+' { $$= AST::UnaryExpression::PLUS; }
      | '-' { $$= AST::UnaryExpression::NEG;  }
      | '!' { $$ = AST::UnaryExpression::NOT; }
      ;


cast_expression
      : unary_expression
/*      | '(' type_name ')' cast_expression
        { $$ = new AST::CastExpression( $4, $2 ); }*/
      ;


multiplicative_expression
      : cast_expression
      | multiplicative_expression binary_op_kind_mul cast_expression
        { $$ = new AST::BinaryExpression($1,$3,$2); }
      ;

binary_op_kind_mul
      : '*' { $$ = AST::BinaryExpression::MUL; }
      | '/' { $$ = AST::BinaryExpression::DIV; }
/*      | '%' { $$ = AST::BinaryExpression::MOD; }*/
      ;

additive_expression
      : multiplicative_expression
      | additive_expression binary_op_kind_add multiplicative_expression
        { $$ = new AST::BinaryExpression($1,$3,$2); }
      ;

binary_op_kind_add
      : '+' { $$ = AST::BinaryExpression::ADD; }
      | '-' { $$ = AST::BinaryExpression::SUB; }
      ;

relational_expression
      : additive_expression
      | relational_expression binary_op_kind_rel additive_expression
        { $$ = new AST::BinaryExpression($1,$3,$2); }
      ;

binary_op_kind_rel
      : '<'   { $$ = AST::BinaryExpression::LE;  }
      | '>'   { $$ = AST::BinaryExpression::GR;  }
      | LE_OP { $$ = AST::BinaryExpression::LEQ; }
      | GE_OP { $$ = AST::BinaryExpression::GEQ; }
      ;

equality_expression
      : relational_expression
      | equality_expression EQ_OP relational_expression
        { $$ = new AST::BinaryExpression($1,$3,AST::BinaryExpression::EQ); }
      | equality_expression NE_OP relational_expression
        { $$ = new AST::BinaryExpression($1,$3,AST::BinaryExpression::NEQ); }
      ;

logical_and_expression
      : equality_expression
      | logical_and_expression AND_OP equality_expression
        { $$ = AST::makeBinaryExpression( $1, $3, AST::BinaryExpression::AND ); }
      ;

logical_or_expression
      : logical_and_expression
      | logical_or_expression OR_OP logical_and_expression
        { $$ = AST::makeBinaryExpression( $1, $3, AST::BinaryExpression::OR ); }
      ;


conditional_expression
      : logical_or_expression
      | logical_or_expression '?' expression ':' conditional_expression
        { $$ = new AST::ConditionalExpression( $1, $3, $5 ); }
      ;

assignment_expression
      : conditional_expression
      | unary_expression assignment_operator assignment_expression
        { $$ = new AST::AssignmentExpression($1,$3,$2); }
      ;

assignment_operator
      : '='         { $$ = AST::AssignmentExpression::PLAIN; }
      | MUL_ASSIGN  { $$ = AST::AssignmentExpression::MUL;   }
      | DIV_ASSIGN  { $$ = AST::AssignmentExpression::DIV;   }
      | MOD_ASSIGN  { $$ = AST::AssignmentExpression::MOD;   }
      | ADD_ASSIGN  { $$ = AST::AssignmentExpression::ADD;   }
      | SUB_ASSIGN  { $$ = AST::AssignmentExpression::SUB;   }
      | AND_ASSIGN  { $$ = AST::AssignmentExpression::AND;   }
      | XOR_ASSIGN  { $$ = AST::AssignmentExpression::XOR;   }
      | OR_ASSIGN   { $$ = AST::AssignmentExpression::OR;    }
      ;

expression
      : conditional_expression
        { $$ = $1; }
      ;

statement
      : compound_statement { $$ = $1; }
      | declaration
      | conditional_statement
      | expression_statement { $$=dynamic_cast<AST::Statement*>($1); }
      | jump_statement
      ;

statement_list
      : statement
        { $$ = new AST::StatementList(); $$->add($1); }
      | statement_list statement
        { $1->add($2); $$ = $1; }
      ;

expression_statement
      : assignment_expression ';'
        { $$ = new AST::ExpressionStatement($1); }
      ;

compound_statement
      : '{' '}'
        { $$ = new AST::CompoundStatement(new AST::StatementList()); }
      | '{'
        {   symbol_env.enterBlock();  }
        statement_list '}'
        {
            symbol_env.leaveBlock();
            $$ = new AST::CompoundStatement($3);
        }
      ;


conditional_statement
      : IF '(' expression ')' statement
        { $$ = new AST::ConditionalStatement($3,$5,nullptr); }
      | IF '(' expression ')' statement ELSE statement
        { $$ = new AST::ConditionalStatement($3,$5,$7); }
      ;

jump_statement
      : RETURN ';'            { $$ = new AST::Return(nullptr); }
      | RETURN expression ';' { $$ = new AST::Return($2);   }
      ;

declaration
      : declaration_specifiers init_declarator_list ';'
        { $$ = $2;  }
      | TYPEDEF type_specifier IDENTIFIER ';'
        {
            Type *t = $2;
            AliasType *at = dynamic_cast<AliasType*>(t);
            if( at != nullptr )
                t = at->base_type;
            symbol_env.add( new AliasType( $3, t ) );
            //$$ = new AST::PlainText( std::string("typedef ") + $2->name() + " " + $3 + ";" );
            $$ = new AST::EmptyStatement();
        }
      | enum_specifier ';'
        { $$ = new AST::EmptyStatement(); }
      ;

declaration_specifiers
      : type_specifier { base_type = $1; }
      ;

init_declarator_list
      : init_declarator
        { $$ = new AST::StatementList(); $$->add( $1 ); }
      | init_declarator_list ',' init_declarator
        { $1->add( $3 ); $$ = $1; }
      ;

init_declarator
      : declarator
      | declarator '='
        {
            Variable* var = dynamic_cast<AST::VariableDeclaration*>($1)->var;
            init_type   = var->type;
            init_target = new AST::IdentifierExpression(var);
        }
        initializer
        {
            AST::StatementList *l = new AST::StatementList();
            l->add( $1 );
            l->add( $4 );
            $$ = l;
        }
      ;

//struct { int a, float b } x = { 3, 4.0 };
//int a[][] = { { 123,123}, {123} };

initializer
      : assignment_expression
        {
            AST::AssignmentExpression *e = new AST::AssignmentExpression(init_target,$1);
            $$ = new AST::ExpressionStatement(e);
            init_type = nullptr;
            init_target = nullptr;
        }
      ;

type_specifier
      : VOID   { $$ = type_void;  }
      | INT    { $$ = type_int;   }
      | FLOAT  { $$ = type_float; }
      | DOUBLE { $$ = type_float; }
      | BOOL   { $$ = type_bool;  }
      | USER_TYPE { $$ = $1;      }
      ;

enum_specifier
      : ENUM IDENTIFIER
        {
           current_enum = new EnumType($2);
        }
        '{' enumerator_list '}'
        {
            symbol_env.add( new AliasType( current_enum->id, type_int ) );
        }
      ;

enumerator_list
        : enumerator
        | enumerator_list ',' enumerator
        ;

enumerator
      : IDENTIFIER
        {
            Variable *v = new Variable( $1, type_int );
            v->make_constant( current_enum->add_value($1) );
            symbol_env.add( v );
        }
      | IDENTIFIER '=' INT_CONSTANT
        {
            Variable *v = new Variable( $1, type_int );
            v->make_constant( current_enum->add_value($1,$3) );
            symbol_env.add( v );
        }
      ;


declarator
      : declarator2
        {
            std::string id;
            Type *type = $1->declare( base_type, id );
            FunctionType* fun_type = dynamic_cast<FunctionType*>(type);
            if( fun_type != nullptr ) {
                if( symbol_env.findFunction( type->id, (unsigned int)(fun_type->parameters.size()) ) != nullptr )
                    yyerror( std::string( "function " ) + type->id + std::string( " already declared" ) );
                else {
                    if( is_extern && is_float(fun_type->return_type) && !is_exact )
                        yyerror( std::string("external functions with floating point result type need to be marked \"exact\"" ) );
                    fun_type->is_inline = is_inline;
                    fun_type->is_extern = is_extern;
                    is_inline = false;
                    is_extern = false;
                    is_exact  = false;
                    symbol_env.add( fun_type );
                }
                $$ = new AST::EmptyStatement;
            } else {
                Variable *var = new Variable( id, type );
                symbol_env.add( var );
                $$ = new AST::VariableDeclaration( var );
            }
        }
      ;

declarator2
      : direct_declarator
      ;

direct_declarator
      : IDENTIFIER { $$ = new IdentifierDeclarator( $1 ); }
      | '(' declarator2 ')' { $$ = $2; }
      | IDENTIFIER  '(' parameter_list ')'
        { $$ = new FunctionDeclarator( new IdentifierDeclarator( $1 ), $3 ); }
      | IDENTIFIER '(' ')'
        { $$ = new FunctionDeclarator( new IdentifierDeclarator( $1 ) ); }
      ;


parameter_list
      : parameter_declaration
        {
            $$ = new FunctionDeclarator::ParameterDeclList();
            $$->push_back( $1 );
        }
      | parameter_list ',' parameter_declaration { $1->push_back( $3 ); $$ = $1; }
      ;

parameter_declaration
      : type_specifier declarator2
        { $$ = new FunctionDeclarator::ParameterDecl( $1, $2 ); }
        // no abstract declarators needed
        // (would only be required for old-style function declarations)
      ;


group_options
    : /* maybe empty. default degree=1 */
      { $$ = 1; }
    | '[' DEGREE '=' INT_CONSTANT ']'
      { $$ = $4; }
    ;

group_var_list
    : IDENTIFIER {
          $$ = new Group_rep;
          $$->variables.push_back( $1 );
      }
    | group_var_list IDENTIFIER {
          $1->variables.push_back( $2 );
          $$ = $1;
      }
    ;

single_group
    : GROUP group_options group_var_list ';' {
        $$ = $3;
        $$->degree = (unsigned int)($2);
      }
    ;

group_definition2
    : single_group {
          $$ = new std::list< Group_rep* >;
          $$->push_back( $1 );
      }
    | group_definition single_group {
          $1->push_back( $2 );
          $$ = $1;
      }
    ;

group_definition
    :                   { $$ = nullptr; }
    | group_definition2 { $$ = $1;   }
    ;

function_definition
      : declaration_specifiers declarator2 group_definition
        {
            std::string id;
            Type *type = $2->declare( $1, id );
            FunctionType *fun_type = dynamic_cast<FunctionType*>(type);
            if( fun_type ) {
                // "replace_function_type" is
                // similar to "add", but doesnt care if fun_type was already declared.
                // important: previous declarations might have different variable names!
                // which do not match the function definition body. so, we prefer
                // the most recent type, which has the correct variable names
                symbol_env.replace_function_type( fun_type );
            } else
                yyerror( std::string("internal error: invalid function definition!" ) );
            fun_type->is_inline = is_inline;
            is_inline = false;
            if( is_extern )
                yyerror( std::string("external function definition does not make sense" ) );
            $<fun_type>$ = fun_type;

            symbol_env.enterBlock();
            FOREACH_PARAMETER(fun_type) {
                Variable *var = *it;
                // array arguments are supplied by reference.

                symbol_env.add( var );
            }
            if( $3 != nullptr ) {
                unsigned int group_index = 1;
                std::list< Group_rep* >::iterator it;
                for( it  = $3->begin(); it != $3->end(); ++it, ++group_index )
                {
                    unsigned int degree = (*it)->degree;
                    for( std::list<std::string>::iterator
                        it2  = (*it)->variables.begin();
                        it2 != (*it)->variables.end();
                        ++it2)
                    {
                        //std::cout << *it2 << " ";
                        Variable *var = symbol_env.findVariable( *it2 );
                        if( var == nullptr )
                            yyerror( std::string("undefined variable ") + *it2 );
                        var->group_index = group_index;
                        var->degree      = degree;
                    }
                    //std::cout << std::endl;
                }
            }

        }
        compound_statement
        {
            symbol_env.leaveBlock();
            AST::FunctionDefinition *fun_def = new AST::FunctionDefinition( $<fun_type>4, $5 );
            symbol_env.add_fun_def( fun_def );
            $$ = fun_def;
        }
      ;

translation_unit
      : external_declaration
      | translation_unit external_declaration
      ;

external_declaration
      : function_definition { translation_unit->add( $1 ); }
      | declaration         { translation_unit->add( $1 ); }
      | INLINE              { is_inline = true; }
      | EXTERN              { is_extern = true; }
      | EXACT               { is_exact  = true; }
      ;


program
      : translation_unit ;


%%

extern char yytext[];
extern int column;


void yyerror(char *s)
{
  throw ParseError (s, AST::current_location);
}

void yyerror(const std::string& s) {
  throw ParseError (s, AST::current_location);
}

void yywarning( const std::string& s ) {
  std::cerr << "warning: " << AST::current_location.toString() << " " << s << std::endl;
}

inline
AST::Expression*
makeFunctionCall( const std::string &id, AST::ExpressionList *l ) {
    if( l == nullptr )
        l = new AST::ExpressionList();
    FunctionType *fun_type = symbol_env.findFunction( id, (unsigned int)(l->size()) );
    if( fun_type == nullptr ) {
        yyerror( std::string("function ") + id + " not declared!" );
    }
    return new AST::FunctionCall( fun_type, l );
}


