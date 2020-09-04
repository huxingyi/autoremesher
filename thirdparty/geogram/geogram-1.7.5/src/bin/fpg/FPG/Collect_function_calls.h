#ifndef COLLECT_FUNCTION_CALLS_H
#define COLLECT_FUNCTION_CALLS_H

#include <FPG/Visitor.h>
#include <vector>

struct Collect_function_calls : public Visitor {
    virtual void visit( AST::LiteralExpression* );
    virtual void visit( AST::IdentifierExpression* );
    virtual void visit( AST::UnaryExpression* );
    virtual void visit( AST::BinaryExpression* );
    virtual void visit( AST::ConditionalExpression* );
    virtual void visit( AST::AssignmentExpression* );
    virtual void visit( AST::FunctionCall* );
    virtual void visit( AST::EmptyStatement* );
    virtual void visit( AST::ExpressionStatement* );
    virtual void visit( AST::ConditionalStatement* );
    virtual void visit( AST::Return* );
    virtual void visit( AST::StatementList* );
    virtual void visit( AST::VariableDeclaration* );
    virtual void visit( AST::CompoundStatement* );
    virtual void visit( AST::FunctionDefinition* );
    virtual void visit( AST::TranslationUnit* );

    std::vector< AST::FunctionCall* > result;
};

#endif