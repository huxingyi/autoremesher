#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

#include <FPG/AST.h>
#include <FPG/Generic_visitor.h>
#include <FPG/Attribute_visitor.h>
#include <FPG/Misc_visitors.h>
#include <map>
#include <cassert>

// those functions which both
//   * have more than one calling site
//   * which contain (in)directly a float comparison/sign computation
// might have different abstract values at comparison site. we are
// conservative here and just clone each function, once per function_call > 1
// intuitively, after cloning, we have several call trees. inside each of
// those trees, each function with (in)direct float comp., has callcount 1
struct Make_unique_funcalls : public Generic_visitor {
    Label_filtered_functions *should_be_filtered;
    Compute_call_count       *compute_call_count;
    Collect_function_calls   *collect_funcalls;
    AST::TranslationUnit     *tu;

    Make_unique_funcalls();
    void init( AST::TranslationUnit *tu, Label_filtered_functions *should_be_filtered );
    virtual void visit( AST::FunctionCall *funcall );
};

// -----------------------------------
struct Check_unique_funcalls : public Attribute_visitor<bool> {
    Label_filtered_functions *should_be_filtered;
    Compute_call_count       *compute_call_count;

    Check_unique_funcalls( Label_filtered_functions *should_be_filtered );
    virtual bool combine( bool a, bool b );
    virtual void visit( AST::FunctionCall *funcall );
};

// while traversing the AST, keep track of
//   * the latest statement list
//   * the current position in that list
// so that sub-expressions or statements can extend the statement list
// right before/after that statement
struct Statement_addition_visitor : public Generic_visitor {
    struct Position {
        Position( AST::StatementList *stmt_list )
            : stmt_list(stmt_list),
              current_stmt( stmt_list->statements->begin() )
        {}

        AST::StatementList               *stmt_list;
        AST::StatementContainer::iterator current_stmt;
    };

    // intraprocedural
    Statement_addition_visitor();
    virtual void visit( AST::StatementList * );
    virtual void visit( AST::FunctionDefinition * );

    // new statement will be added before the last visited statement, by default
    // otherwise, it is added just afterwards
    // NOTE: be careful not to add statements after the current one, that are
    //       similar to the current one (obviously, then you would have a loop
    //       when using the Transformation_visitor)
    void add_stmt( AST::Statement*, bool before = true );
    void add_stmt_toplevel( AST::Statement*, bool before = true );
    // add stmt at the first scope (viewed from outside) where all vars are declared
    // NOTE: vars must not contain function parameters!
    Position get_scope( std::set< Variable *>& vars );
    void add_stmt_with_scope( AST::Statement*, Position p );
    bool is_last_stmt();
protected:
    std::list< Position > positions;
    Position *toplevel_position;
    bool fundef_toplevel;
};

struct Transformation_visitor : public Statement_addition_visitor {
    Transformation_visitor();

    virtual void visit( AST::LiteralExpression*     );
    virtual void visit( AST::IdentifierExpression*  );
    virtual void visit( AST::UnaryExpression*       );
    virtual void visit( AST::BinaryExpression*      );
    virtual void visit( AST::ConditionalExpression* );
    virtual void visit( AST::AssignmentExpression*  );
    virtual void visit( AST::FunctionCall*          );
    virtual void visit( AST::UnaryFunction*         );
    virtual void visit( AST::EmptyStatement*        );
    virtual void visit( AST::ExpressionStatement*   );
    virtual void visit( AST::ConditionalStatement*  );
    virtual void visit( AST::Return*                );
    virtual void visit( AST::PlainText*             );
    virtual void visit( AST::PlainTextExpression*   );
    virtual void visit( AST::StatementList*         );
    virtual void visit( AST::VariableDeclaration*   );
    virtual void visit( AST::CompoundStatement*     );
    virtual void visit( AST::FunctionDefinition*    );

    virtual void update( AST::Node *node_old, AST::Node *node_new );

    std::stack< AST::Node* > node_stack;

    template< class T >  void transform( T *&e   );
    template< class T >  T*   pop();
                         void push( AST::Node *n );

};

template< class Base >
struct Add_variable_visitor
    : public Base
{
     Add_variable_visitor()
        : collect_variables( new Collect_variables )
    {}

    Variable*
    add_variable( std::string id, Type *type ) {
        assert( type != nullptr );
        assert( id.length() > 0 );
        assert( collect_variables != nullptr );
        std::string fresh_name = collect_variables->make_fresh_variable_name( id );
        // reuse same type
        Variable *new_var = new Variable( fresh_name, type );
        collect_variables->add( new_var );
        assert( new_var != nullptr );
        return new_var;
    }

    Collect_variables *collect_variables;
};

template< class Base >
struct Add_tmp_variable_visitor
    : public Add_variable_visitor<Base>
{
    using Add_variable_visitor<Base>::add_variable;

    Variable*
    add_tmp_result( AST::Expression *e, std::string hint = std::string() ) {
        assert( e != nullptr );
        // return type can be either int or double:
        if( hint == std::string() )
            hint = e->getType()->id + "_tmp_result";
        Variable *new_var = add_variable( hint, e->getType() );
        tmp_result_storage_map[ e ] = new_var;
        return new_var;
    }

    Variable*
    tmp_result_variable( AST::Expression *e ) {
        assert( e != nullptr );
        assert( has_tmp_result_variable( e ) );
        return tmp_result_storage_map[ e ];
    }

    bool
    has_tmp_result_variable( AST::Expression *e ) {
        return tmp_result_storage_map.find( e ) != tmp_result_storage_map.end();
    }

    // this is used outside the called function, for storing temporarily the result value
    // (binaryExpression and signfunction are treated as functions, here)
    std::map< AST::Expression*,  Variable* > tmp_result_storage_map;
};

struct Add_bound_variables_base
    : public Add_tmp_variable_visitor<Statement_addition_visitor>
{
    // same as transformationvisitor: intraprocedural
    Add_bound_variables_base();
    virtual ~Add_bound_variables_base();

    Variable *bound_variable( Variable                *float_var );
    Variable *bound_variable( AST::FunctionDefinition *fundef    );
    Variable *bound_variable ( AST::FunctionCall      *funcall   );

    bool      has_bound_variable( Variable                *float_var );
    bool      has_bound_variable( AST::FunctionDefinition *fundef    );
    bool      has_bound_variable ( AST::FunctionCall      *funcall   );
    // used for binary comparisons/sign functions/function calls:

protected:
    Variable* add_bound( std::string id );
    Variable* add_bound( Variable *var );
    Variable* add_bound     ( AST::FunctionCall *funcall );


    std::map< Variable*,                Variable* > bound_map;

    // this is used inside the called function, for returning the bound
    std::map< AST::FunctionDefinition*, Variable* > result_bound_map;

    // this is used outside the called function, for receiving the bound
    std::map< AST::FunctionCall*,       Variable* > result_bound_storage_map;
};

// actually, it's not really *common* subexp elimin., because
// every occurrence is replaced, even if it occurs only once
struct Common_subexpression_elimination
    : public Add_tmp_variable_visitor<Transformation_visitor>
{
    // predicate to test if expression e is subject to CSE:
    virtual bool filter(  AST::BinaryExpression* ) = 0;

    virtual void visit( AST::BinaryExpression* );
    void clear() { subexpressions.clear(); }

    // here are the canonical expressions. once a to-be-replaced candidate
    // is encountered, we have to look here, if a similar expression
    // was already used/replaced with a tmp variable
    std::vector< AST::Expression* > subexpressions;
    // tmp variables are stored in the parent class
};

struct Substitute_funcalls :
    public Add_variable_visitor<Transformation_visitor>
{
    Substitute_funcalls( Expression_filter *f )
      : do_substitute(f), is_dirty(false)
    {
    }

    virtual void visit( AST::FunctionCall* );
    virtual void update( AST::Node *node_old, AST::Node *node_new ) {
        Transformation_visitor::update(node_old, node_new);
        is_dirty = true;
    }


    Expression_filter *do_substitute;
    bool is_dirty;
};

struct Beautify_visitor : public Generic_visitor {
    Beautify_visitor()
        : Generic_visitor( false )
    {}

    virtual void visit( AST::StatementList* stmt_list );
};


// -----------------------------------
// some convenience functions.
// calling only sensible with a whole translation unit
extern void make_unique_funcalls( AST::TranslationUnit *tu, Label_filtered_functions *should_be_filtered );
extern bool has_unique_funcalls( AST::TranslationUnit *tu, Label_filtered_functions *should_be_filtered );

#endif
