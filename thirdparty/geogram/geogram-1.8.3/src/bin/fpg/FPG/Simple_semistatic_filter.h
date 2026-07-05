#ifndef SIMPLE_SEMISTATIC_FILTER_H
#define SIMPLE_SEMISTATIC_FILTER_H

#include <FPG/Misc_visitors.h>
#include <FPG/Transformation.h>
#include <FPG/Abstract_interpretation_visitor.h>

#include <stack>

namespace Simple_semistatic_filter {


struct Add_bound_variables : public Add_bound_variables_base {
    Add_bound_variables( )
      : expression_depth( 0 ),
        compute_call_count( new Compute_call_count )
    {}

    virtual void visit( AST::IdentifierExpression* );
    virtual void visit( AST::FunctionDefinition* );
    virtual void visit( AST::VariableDeclaration* );
    virtual void visit( AST::TranslationUnit* );
    virtual void visit( AST::BinaryExpression* );
    virtual void visit( AST::UnaryFunction* );
    virtual void visit( AST::Return* );

    // all but the topmost function call in each expression need special treatment:
    unsigned int        expression_depth;
    Compute_call_count *compute_call_count;
};

typedef std::map< AST::Expression*, AST::Expression* > Bound_map;

// this visitor works destructively on the input!
struct Add_bound_computation : public Statement_addition_visitor {
    Add_bound_computation();

    virtual void visit( AST::LiteralExpression* );
    virtual void visit( AST::IdentifierExpression* );
    virtual void visit( AST::UnaryExpression* );
    virtual void visit( AST::BinaryExpression* );
    virtual void visit( AST::ConditionalExpression* );
    virtual void visit( AST::ConditionalStatement* );
    virtual void visit( AST::AssignmentExpression* );
    virtual void visit( AST::FunctionCall* );
    virtual void visit( AST::UnaryFunction* );
    virtual void visit( AST::ExpressionStatement* );
    virtual void visit( AST::Return* );
    virtual void visit( AST::FunctionDefinition* );
    virtual void visit( AST::TranslationUnit* );

    // needed for next phase "rewrite float comparisons"
    // for each interesting expression, store its bound expression:
    Bound_map *bound_map;
    Add_bound_variables             *add_bounds;
    Abstract_interpretation_visitor *absint;
protected:
    void             store_bound( AST::Expression *e, AST::Expression *bound );
    // regarding the currently visited fundef
    bool             has_result_bound_variable();
    Variable*        result_bound_variable();
    Variable*        bound_variable( Variable *var );
    Variable*        bound_variable( AST::FunctionCall *funcall );
    AST::Expression* bound_expression( AST::Expression *e );

    // for bound_expressions
    void             push_bound( AST::Expression *e );
    AST::Expression* pop_bound();

    std::stack< AST::FunctionDefinition* >         fundef_stack;
    std::stack< AST::Expression*         >         bound_expressions;

    Label_filtered_functions        *should_be_filtered;
};

struct Rewrite_float_comparisons : public Transformation_visitor {
    Rewrite_float_comparisons( Add_bound_computation *add_bounds )
       : add_bounds( add_bounds )
    {}

    virtual void visit( AST::AssignmentExpression * );
    virtual void visit( AST::BinaryExpression * );
    virtual void visit( AST::UnaryFunction *    );

    virtual void update( AST::Node *node_old, AST::Node *node_new );
protected:
    AST::Expression* bound_expression( AST::Expression *e );
    bool             has_bound( AST::Expression *e );

    Add_bound_computation *add_bounds;
};

}

#endif
