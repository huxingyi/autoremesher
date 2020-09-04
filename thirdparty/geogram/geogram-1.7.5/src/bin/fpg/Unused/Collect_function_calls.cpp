#include <FPG/Collect_function_calls.h>

void 
Collect_function_calls::visit( AST::LiteralExpression* lexp ) {

}

void
Collect_function_calls::visit( AST::IdentifierExpression* iexp ) {

}

void 
Collect_function_calls::visit( AST::UnaryExpression* uexp ) {

}

void 
Collect_function_calls::visit( AST::BinaryExpression* bexp ) {

}

void 
Collect_function_calls::visit( AST::ConditionalExpression* cexp ) {

}

void 
Collect_function_calls::visit( AST::AssignmentExpression* aexp ) {
    AST::IdentifierExpression *id_expr = dynamic_cast< AST::IdentifierExpression* >( aexp->e1 );
    assert( id_expr );
    Variable *var = id_expr->var;

}

void 
Collect_function_calls::visit( AST::FunctionCall* funcall ) {
    AST::ExpressionList::iterator           arg_iter;
    FunctionType::ParameterList::iterator param_iter;
    // do something with actual arguments
    arg_iter   = funcall->exp_list->begin();
    param_iter = funcall->fun_type->parameters.begin();
    for( ; arg_iter != funcall->exp_list->end(); ++arg_iter, ++param_iter ) {
        Variable        *var = *param_iter;
        AST::Expression *exp = *arg_iter;

    }


    //AST::FunctionDefinition *fun_def = symbol_env.findFunctionDef( funcall->fun_type->id );
    //assert( fun_def != nullptr );

    // postprocessing
    param_iter = funcall->fun_type->parameters.begin();
    for( ; param_iter != funcall->fun_type->parameters.end(); ++param_iter ) {
        Variable        *var = *param_iter;

    }

}

void 
Collect_function_calls::visit( AST::EmptyStatement* stmt ) {

}

void 
Collect_function_calls::visit( AST::ExpressionStatement* estmt ) {

}

void 
Collect_function_calls::visit( AST::ConditionalStatement* cstmt ) {
    if( cstmt->else_branch ) {

    }
}

void 
Collect_function_calls::visit( AST::Return* ret ) {

}

void 
Collect_function_calls::visit( AST::StatementList* slist ) {
    AST::StatementContainer::iterator it;
    for( it = slist->statements->begin(); it != slist->statements->end(); ++it ) {
        AST::Statement *stmt = *it;

    }
}

void 
Collect_function_calls::visit( AST::VariableDeclaration* vardecl ) {

}

void 
Collect_function_calls::visit( AST::CompoundStatement* compound ) {

}

void 
Collect_function_calls::visit( AST::FunctionDefinition* fundef ) {
    FunctionType::ParameterList::iterator it;
    for( it = fundef->type->parameters.begin(); it != fundef->type->parameters.end(); ++it ) {
        Variable *var = *it;

    }
    // do something with fundef->body

    // postprocessing
    for( it = fundef->type->parameters.begin(); it != fundef->type->parameters.end(); ++it ) {
      Variable *var = *it;

    }
}

void 
Collect_function_calls::visit( AST::TranslationUnit* tu ) {
    std::vector< AST::FunctionDefinition* >::iterator it;
    for( it = tu->functions.begin(); it != tu->functions.end(); ++it ) {
      AST::FunctionDefinition *fundef = *it;

    }

}