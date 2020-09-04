#ifndef AST_H
#define AST_H

#include <map>
#include <list>
#include <string>
#include <iostream>
#include <cassert>

#include <FPG/Value.h>
#include <FPG/Symbol.h>
#include <FPG/Location.h>

struct Visitor;

namespace AST {

extern Type *return_type;

/* the parser updates the current location */
extern Location current_location;

struct FunctionDefinition;

struct Clone_context {
    typedef std::map< FunctionDefinition*, FunctionDefinition* > Fundef_map;
    typedef std::map< Variable*, Variable* >                     Var_map;

    Fundef_map fundef_map;
    Var_map    var_map;
};



struct Node {
    friend struct ::Visitor;
    Node() : location(current_location), type(nullptr) {}
    virtual ~Node() {}
    static std::string       context;

    static void setOutputStream( std::ostream &os );
    static std::ostream& out();
    static std::string getNewLabel( const std::string &prefix = "" );

    // infers a type for this AST node. also, if a node is supposed not to
    // have any type (statements), type check of its components is performed.
    // calls computeType and caches result.
    Type* getType();

    // this method actually implements the type check.
    virtual Type* computeType() = 0;

    // print out a tree representation of the AST
    virtual void dump( int level = 0 ) { argused(level); }
    void dumpPrefix( int level );

    virtual bool hasToplevelVariableDeclaration() { return false; }
    virtual bool isBlock() { return false; }
    virtual void accept( Visitor *visitor ) = 0;

    // this is not meant to be called directly. use AST::clone( Node *n ) instead
    virtual Node* clone( Clone_context *context ) = 0;

    Location location;
    Type *type;
private:
    static std::ostream *os;
};

struct Expression : public Node {
    //virtual bool containsReference() { return false; }
    virtual Expression* clone( Clone_context *context ) = 0;
};

/*
struct CastExpression : public Expression {
    CastExpression( Expression *e, Type *target_type ) : e(e), target_type(target_type) {}
    virtual void accept( Visitor *visitor );
protected:
    Expression *e;
    Type *target_type;
};*/

struct LiteralExpression : public Expression {
    LiteralExpression( int i ) : val(i) {}
    LiteralExpression( double f ) : val(f) {}
    void dump( int level );

    virtual Type* computeType();
    bool is_float() { return val.type == Concrete_value::FLOATCONST; }
    virtual void accept( Visitor *visitor );
    virtual LiteralExpression* clone( Clone_context *context );

    Concrete_value val;
};

struct IdentifierExpression: public Expression {
    IdentifierExpression( Variable *var ) : var(var) {}
    void dump( int level );
    virtual Type* computeType();
    //bool containsReference() { return var->is_call_by_ref; }
    virtual void accept( Visitor *visitor );
    virtual IdentifierExpression* clone( Clone_context *context );

    Variable *var;
};


struct UnaryExpression : public Expression {
    enum Kind {
        PLUS,       // +e
        NEG,        // -e
        NOT,        // !e
        DEREF,      // *e
        REF         // &e
    };

    UnaryExpression( Expression *e, Kind kind )
      : e(e), kind( kind ) {}

    UnaryExpression( Expression *e, std::string selector, Kind kind )
      : e(e), kind( kind ), selector(selector)
    {}

    void dump( int level );
    virtual Type* computeType();

    virtual void accept( Visitor *visitor );
    virtual UnaryExpression* clone( Clone_context *context );

    Expression *e;
    Kind kind;
    std::string selector; // selector in a struct access
    static const std::string operators[];
};


struct BinaryExpression : public Expression {
    enum Kind { ADD=0, SUB, MUL, DIV, MOD, AND, OR, XOR,
                EQ, GEQ, LEQ, LE, GR, NEQ, BITWISE_OR
                /*COMMA*/ };

    BinaryExpression( Expression *e1, Expression *e2, Kind kind )
        : e1(e1), e2(e2), kind(kind),
          pointer_arith1(false), pointer_arith2(false) {
    }
    void dump( int level );
    Type* computeType();

    //bool containsReference() { return e1->containsReference() || e2->containsReference(); }
    virtual void accept( Visitor *visitor );
    virtual BinaryExpression* clone( Clone_context *context );

    Expression *e1, *e2;
    Kind kind;
    static const std::string operators[];
    static const std::string cma_operators[];
    std::string after_e1, after_e2;
    bool pointer_arith1, pointer_arith2;
};

struct ConditionalExpression : public Expression {
    ConditionalExpression( Expression* cond, Expression *e1, Expression *e2);

    void dump( int level );
    Type* computeType();

    virtual void accept( Visitor *visitor );
    virtual ConditionalExpression* clone( Clone_context *context );

    Expression *cond, *e1, *e2;
};


/*

struct ArrayExpression : public Expression {
    ArrayExpression( Expression *a, Expression *index )
        : a(a), index(index) {}

    void dump( int level );
    //bool containsReference() { return a->containsReference(); }
    virtual void accept( Visitor *visitor );
protected:
    Expression *a, *index;
    std::string pointer_arith;
};*/


struct AssignmentExpression : public Expression {
    enum Kind { PLAIN,  // e = e
                ADD,    // e += e
                SUB,    // e -= e
                MUL,    // e *= e
                DIV,    // e /= e
                MOD,    // e %= e
                AND,    // e &= e
                OR ,    // e |= e
                XOR     // e ^= e
              };
    AssignmentExpression( Expression *e1, Expression *e2, Kind kind = PLAIN );
    void dump( int level );
    virtual Type* computeType();
    virtual void accept( Visitor *visitor );
    virtual AssignmentExpression* clone( Clone_context *context );

    // internally, only plain = is considered. all other cases are
    // normalized to   =
    //               |   |
    //             e1   e1 op e2
    Expression *e1, *e2;
    std::string convert;
};

typedef std::list< Expression* > ExpressionList;

struct FunctionCall : public Expression {
    FunctionCall( FunctionType *fun_type, ExpressionList *exp_list, FunctionDefinition *called_function = nullptr )
       : fun_type( fun_type ),
         exp_list( exp_list ),
         called_function( called_function )
    {}

    virtual Type* computeType();

    void dump( int level );
    virtual void accept( Visitor *visitor );
    virtual FunctionCall* clone( Clone_context *context );

    FunctionType       *fun_type;
    ExpressionList     *exp_list;
    FunctionDefinition *called_function;
};

struct UnaryFunction : public Expression {
    enum Kind { XSIGN, XSQRT, XABS };

    UnaryFunction( Expression *e, Kind kind )
       : e(e), kind(kind)
    {}

    virtual Type* computeType();

    void dump( int level );
    virtual void accept( Visitor *visitor );
    virtual UnaryFunction* clone( Clone_context * );

    Expression *e;
    Kind kind;
};



// abstract class
struct Statement : public Node  {
    // has break or return on exit
    virtual bool hasBreakOrReturnOnExit() { return false; }
    virtual void accept( Visitor *visitor ) = 0;
    virtual Statement* clone( Clone_context *context ) = 0;
};

struct EmptyStatement: public Statement {
    Type* computeType() { return type_void; }
    virtual void accept( Visitor *visitor );
    virtual EmptyStatement* clone( Clone_context *context );
};

struct ExpressionStatement : public Statement {
    ExpressionStatement( Expression *e ) : e(e) {}
    void dump( int level );
    virtual Type* computeType() { return e ? e->getType() : type_void; }
    Expression* getExpression() { return e;}
    virtual void accept( Visitor *visitor );
    virtual ExpressionStatement* clone( Clone_context *context );

    Expression *e; // can be nullptr
};

typedef std::list< Statement*> StatementContainer;

struct StatementList : public Statement {
    StatementList() : statements( new StatementContainer() ) {}
    void dump( int level );
    virtual Type* computeType();
    void add( Statement *stmt ) { statements->push_back( stmt ); }
    void add( Statement *stmt, StatementContainer::iterator insert_before )
    { statements->insert( insert_before, stmt ); }
    bool hasToplevelVariableDeclaration();
    bool hasBreakOrReturnOnExit();
    virtual void accept( Visitor *visitor );
    virtual StatementList* clone( Clone_context *context );

    StatementContainer* statements;
};


struct CompoundStatement : public Statement {
    CompoundStatement( StatementList *statements )
            : statements( statements ) {}
    void dump( int level );
    virtual Type* computeType() { return statements->getType(); }
    bool hasBreakOrReturnOnExit() { return statements->hasBreakOrReturnOnExit(); }
    bool isBlock() { return true; }
    virtual void accept( Visitor *visitor );
    virtual CompoundStatement* clone( Clone_context *context );

    StatementList* statements;
};


struct ConditionalStatement : public Statement {
    ConditionalStatement( Expression *cond,
                          Statement *then_branch,
                          Statement *else_branch = nullptr );
    void dump( int level );
    virtual Type* computeType();
    bool hasBreakOrReturnOnExit();
    virtual void accept( Visitor *visitor );
    virtual ConditionalStatement* clone( Clone_context *context );

    Expression *cond;
    // to allow easy addition of statements, later
    Statement *then_branch, *else_branch;
};

struct Return : public Statement {
    Return(Expression* e) : e(e) {}
    void dump( int level );
    virtual Type* computeType();
    bool hasBreakOrReturnOnExit() { return true; }
    virtual void accept( Visitor *visitor );
    virtual Return* clone( Clone_context *context );

    Expression* e;
};


struct Declaration : public Statement {
  virtual Type* computeType() { return type_void; }
  virtual void accept( Visitor *visitor ) = 0;
  virtual Declaration* clone( Clone_context *context ) = 0;
};

struct VariableDeclaration : public Declaration {
    VariableDeclaration( Variable *var ) : var( var ), initializer(nullptr) {}
    void dump( int level );
    bool hasToplevelVariableDeclaration() { return true; }
    virtual void accept( Visitor *visitor );
    virtual VariableDeclaration* clone( Clone_context *context );

    Variable *var;
    Expression *initializer; // only non-nullptr after beautifying!
};



struct FunctionDefinition : public Node {
    FunctionDefinition( FunctionType *type,
                        CompoundStatement *body )
       : type(type), body(body) {}
    //void dump( int level );
    virtual Type* computeType();
    void dump( int level );

    virtual void accept( Visitor *visitor );
    virtual FunctionDefinition* clone( Clone_context *context );

    FunctionType *type;
    CompoundStatement *body;
};

struct PlainText : public Statement {
    PlainText( std::string text ) : text(text) {}
    virtual Type* computeType() { return type_void; }
    virtual void dump( int level );
    virtual void accept( Visitor *visitor );
    virtual PlainText* clone( Clone_context *context );
    std::string text;
};

struct PlainTextExpression : public Expression {
    PlainTextExpression( std::string text, Type *t )
      : text(text),type(t)
    {}
    virtual Type* computeType() { return type; }
    virtual void dump( int level );
    virtual void accept( Visitor *visitor );
    virtual PlainTextExpression* clone( Clone_context *context );
    std::string  text;
    Type       * type;
};



typedef std::list< FunctionDefinition* > ListOfFunctions;

struct TranslationUnit : public Node {
    TranslationUnit();

    void add( Statement *stm );
    void add( FunctionDefinition *fun_def );
    virtual Type* computeType();
    virtual void dump( int level = 0 );
    virtual void accept( Visitor *visitor );
    virtual TranslationUnit* clone( Clone_context *context );

    // global intializations and declarations
    // ignored
    StatementList* global_statements;

    // function definitions
    ListOfFunctions functions;
    bool has_main;
};

// this function also performs postprocessing to re-validate function calls
// (otherwise, called_function would point to the original AST, not the cloned one
extern Node* clone_prime( Node*, Clone_context *context );

template< class ASTElement >
ASTElement*
clone( ASTElement *e, Clone_context *context = nullptr ) {
    AST::Node *n = clone_prime( e, context );
    e = dynamic_cast<ASTElement*>(n);
    assert(e != nullptr);
    return e;
}

extern Expression* makeBinaryExpression( Expression *e1, Expression *e2, BinaryExpression::Kind kind);
extern void        requireConvertibleTypes( Type *target, Type *source, const Location& location = current_location );
extern Statement*  make_assign_stmt( Variable *var, Expression *e );
extern FunctionCall*
make_funcall( FunctionType *funtype, Expression *a1 );

extern FunctionCall*
make_funcall( FunctionType *funtype, Expression *a1, Expression *a2 );

// NOTE: only binexp and idexp are considered.
extern bool is_equal( const Expression* e1, const Expression* e2 );

struct Is_equal_to_expression {
    Is_equal_to_expression( const Expression *e ) : e(e) {}
    bool operator()( const Expression *e2 ) const {
        return is_equal( e, e2 );
    }
    const Expression* e;
};

extern Expression *uncertain_return_value;


} // end namespace

struct Expression_filter {
    virtual ~Expression_filter() {}
    virtual bool operator()( AST::Expression *e ) { argused(e); return false; }
};

AST::Expression* make_fe_condition();
AST::Statement*  make_fe_clear_and_fail();


#define FOREACH_ARG for( ExpressionList::iterator it = exp_list->begin(); it != exp_list->end(); ++it )

#define FOREACH_DECL for( Declarations::iterator it = declarations->begin(); it != declarations->end(); ++it )

//#define FOREACH_GLOBALSTATEMENT for( StatementContainer::iterator it = global_statements.begin(); it != global_statements.end(); ++it )

#define FOREACH_STATEMENT for( StatementContainer::iterator it = statements->begin(); it != statements->end(); ++it )

#define FOREACH_PARAMETER(T) for( FunctionType::ParameterList::iterator it \
      = T->parameters.begin(); it != T->parameters.end(); ++it )

#define FOREACH_DECLARATION for( std::vector< Declaration* >::iterator it = declarations.begin(); it != declarations.end(); ++it )

#define FOREACH_FUNCTION for( ListOfFunctions::iterator it = functions.begin(); it != functions.end(); ++it )

#define FOREACH_PARAMETER_ARGUMENT_PAIR \
    ExpressionList::iterator arg_iter = exp_list->begin(); \
    FunctionType::ParameterList::iterator param_iter = fun_type->parameters.begin(); \
    for( ; arg_iter != exp_list->end(); ++arg_iter, ++param_iter )

#define FOREACH_SWITCH_CASE \
    for( SwitchStatement::CaseContainer::iterator it = cases->begin(); it != cases->end(); ++it )



#endif
