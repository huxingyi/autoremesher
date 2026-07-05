#ifndef GENERIC_VISITOR_H
#define GENERIC_VISITOR_H

#include <stack>
#include <set>

#include <FPG/Visitor.h>
#include <FPG/AST.h>
#include <FPG/SymbolEnvironment.h>

// also checks for callgraph cycles. if there is one, an exception is thrown
struct Generic_visitor : public Visitor {
    // straight forward implementation: whenever we see a function call
    // twice, it's a callgraph cycle. keep track of the function calls:
    std::set< AST::FunctionDefinition* > callgraph_cycle_detection;
    bool                                 encountered_recursion;
    bool                                 interprocedural;

    static bool has_recursion( AST::Node *n );

    Generic_visitor( bool interprocedural = true )
      : encountered_recursion(false),
        interprocedural(interprocedural)
    {}

    virtual void handle( AST::Node *n );

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
};


#endif
