
void 
::visit( AST::LiteralExpression* lexp ) {

}

void
::visit( AST::IdentifierExpression* iexp ) {

}

void 
::visit( AST::UnaryExpression* uexp ) {

}

void 
::visit( AST::BinaryExpression* bexp ) {

}

void 
::visit( AST::ConditionalExpression* cexp ) {

}

void 
::visit( AST::AssignmentExpression* aexp ) {
    AST::IdentifierExpression *id_expr = dynamic_cast< AST::IdentifierExpression* >( aexp->e1 );
    assert( id_expr );
    Variable *var = id_expr->var;

}

void 
::visit( AST::FunctionCall* funcall ) {
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
::visit( AST::EmptyStatement* stmt ) {

}

void 
::visit( AST::ExpressionStatement* estmt ) {

}

void 
::visit( AST::ConditionalStatement* cstmt ) {
    if( cstmt->else_branch ) {

    }
}

void 
::visit( AST::Return* ret ) {

}

void 
::visit( AST::StatementList* slist ) {
    AST::StatementContainer::iterator it;
    for( it = slist->statements->begin(); it != slist->statements->end(); ++it ) {
        AST::Statement *stmt = *it;

    }
}

void 
::visit( AST::VariableDeclaration* vardecl ) {

}

void 
::visit( AST::CompoundStatement* compound ) {

}

void 
::visit( AST::FunctionDefinition* fundef ) {
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
::visit( AST::TranslationUnit* tu ) {
    std::vector< AST::FunctionDefinition* >::iterator it;
    for( it = tu->functions.begin(); it != tu->functions.end(); ++it ) {
      AST::FunctionDefinition *fundef = *it;

    }

}