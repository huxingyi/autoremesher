#ifndef VISITOR_H
#define VISITOR_H

#include <iostream>
#include <FPG/AST.h>

struct Visitor {
    virtual ~Visitor() {}
    virtual void visit( AST::LiteralExpression*     ) = 0;
    virtual void visit( AST::IdentifierExpression*  ) = 0;
    virtual void visit( AST::UnaryExpression*       ) = 0;
    virtual void visit( AST::BinaryExpression*      ) = 0;
    virtual void visit( AST::ConditionalExpression* ) = 0;
    virtual void visit( AST::AssignmentExpression*  ) = 0;
    virtual void visit( AST::FunctionCall*          ) = 0;
    virtual void visit( AST::UnaryFunction*         ) = 0;
    virtual void visit( AST::EmptyStatement*        ) = 0;
    virtual void visit( AST::ExpressionStatement*   ) = 0;
    virtual void visit( AST::ConditionalStatement*  ) = 0;
    virtual void visit( AST::Return*                ) = 0;
    virtual void visit( AST::StatementList*         ) = 0;
    virtual void visit( AST::VariableDeclaration*   ) = 0;
    virtual void visit( AST::CompoundStatement*     ) = 0;
    virtual void visit( AST::FunctionDefinition*    ) = 0;
    virtual void visit( AST::PlainText*             ) {}
    virtual void visit( AST::PlainTextExpression*   ) {}
    virtual void visit( AST::TranslationUnit*       ) = 0;
};

#endif
