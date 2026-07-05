#ifndef ABSTRACT_INTERPRETATION_VISITOR_H
#define ABSTRACT_INTERPRETATION_VISITOR_H

#include <map>
#include <stack>

#include <FPG/AST.h>
#include <FPG/Abstract_value.h>
#include <FPG/Symbol.h>
#include <FPG/Visitor.h>

struct Abstract_interpretation_visitor : public Visitor {
    typedef std::map< AST::Expression*, Abstract_value* > Analysis_result;
    Analysis_result  analysis_result;

    Abstract_interpretation_visitor( Abstract_value* initial_value );

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
    virtual void visit( AST::StatementList*         );
    virtual void visit( AST::VariableDeclaration*   );
    virtual void visit( AST::CompoundStatement*     );
    virtual void visit( AST::FunctionDefinition*    );
    virtual void visit( AST::TranslationUnit*       );

    Abstract_value* get_analysis_result( AST::Expression *e );
    void clear() { analysis_result.clear(); }
    void clear(Abstract_value* initial_valuex ) {
        analysis_result.clear();
        this->initial_value = initial_valuex;
    }
    // transfer Abstract_value* from old_e to new_e
    void update( AST::Expression *old_e, AST::Expression *new_e );
protected:
    typedef std::map< Variable*, Abstract_value* > Value_environment;

    // why a stack? "Variable*"'s are unique.
    // but: for branches, we need to have multiple environments,
    // which will be merged at the end of the branch. branches can be nested
    std::stack< Value_environment >       value_env_stack;

    // size of stack corresponds to current length of call path in call "tree"
    // comining/joining the abstract values is done using the last entry
    // of the stack. rationale: although the return statements can be anywhere
    // in the block hierarchy, there is only one exit point per function.
    std::stack< Abstract_value* >         return_values;

    // used to pass expression values to the caller
    Abstract_value*                       expression_value;

    // this is what makes abs.-int. visitor generic: interpretation starts
    // with clones of this initial_value
    Abstract_value*                       initial_value;


    // returns a clone of initial_value
    Abstract_value*    new_abstract_value();

    Value_environment& value_env();
    // merge two topmost value env and store result in one topmost entry
    void               merge_env();
    bool               is_defined_in_env( Variable *var );

    // NOTE: for subclasses for Expression:
    // visitor should not call accept directly! use this instead
    // needed for storing the intermediate results
    // has no effect when called with AST::Node
    Abstract_value*    analyze( AST::Node *expr );

};

#endif
