#include <cassert>

#include <FPG/AST.h>
#include <FPG/Error.h>

#include <FPG/Generic_visitor.h>


//#define DEBUG 0
#include <FPG/MSG.h>

bool
Generic_visitor::has_recursion( AST::Node *n ) {
    Generic_visitor v;
    n->accept( &v );
    return v.encountered_recursion;
}

void
Generic_visitor::handle( AST::Node *n ) {
    n->accept( this );
}

void
Generic_visitor::visit( AST::LiteralExpression* ) {
    MSG("")
}

void
Generic_visitor::visit( AST::IdentifierExpression* ) {
    MSG("")
}

void
Generic_visitor::visit( AST::UnaryExpression* e ) {
    MSG("")
    handle( e->e );
}

void
Generic_visitor::visit( AST::BinaryExpression* e ) {
    MSG("")
    handle( e->e1 );
    handle( e->e2 );
}

void
Generic_visitor::visit( AST::ConditionalExpression* c ) {
    MSG("")
    handle( c->cond );
    handle( c->e1 );
    assert( c->e2 != nullptr );
    handle( c->e2 );
}

void
Generic_visitor::visit( AST::AssignmentExpression* e ) {
    MSG("")
    handle( e->e2 );
}

void
Generic_visitor::visit( AST::FunctionCall* fun_call ) {
    MSG("")
    AST::ExpressionList::iterator            arg_iter = fun_call->exp_list->begin();

    for( ; arg_iter != fun_call->exp_list->end(); ++arg_iter ) {
        AST::Expression *exp = *arg_iter;
        handle( exp );
    }

    AST::FunctionDefinition *fun_def = fun_call->called_function;
    if( interprocedural && fun_def != nullptr ) {
        if( callgraph_cycle_detection.find( fun_def ) != callgraph_cycle_detection.end() ) {
            encountered_recursion = true;
        } else {
            callgraph_cycle_detection.insert( fun_def );
            fun_def->body->accept( this );
            callgraph_cycle_detection.erase( fun_def );
        }
    }
}

void
Generic_visitor::visit( AST::UnaryFunction* uf ) {
    MSG("")
    handle( uf->e );
}

void Generic_visitor::visit( AST::EmptyStatement* ) {}

void
Generic_visitor::visit( AST::ExpressionStatement* e ) {
    MSG("")
    handle( e->e );
}

void
Generic_visitor::visit( AST::ConditionalStatement* s ) {
    MSG(s->location)
    handle( s->cond );
    handle( s->then_branch );
    if( s->else_branch != nullptr )
        handle( s->else_branch );
}

void
Generic_visitor::visit( AST::Return* r ) {
    MSG("")
    if( r->e != nullptr )
        handle( r->e );
}

void
Generic_visitor::visit( AST::StatementList* l ) {
    MSG("")
    for( AST::StatementContainer::iterator it = l->statements->begin(); it != l->statements->end(); ++it )
        handle(*it);
}

void
Generic_visitor::visit( AST::VariableDeclaration* ) {
    MSG("")
}

void
Generic_visitor::visit( AST::CompoundStatement* cs ) {
    MSG("")
    handle( cs->statements );
}

void
Generic_visitor::visit( AST::FunctionDefinition* fd ) {
    MSG("")
    handle( fd->body );
}

void
Generic_visitor::visit( AST::TranslationUnit* tu) {
    MSG("")
    AST::ListOfFunctions::iterator it;
    for( it = tu->functions.begin(); it != tu->functions.end(); ++it ) {
        //std::cout << "handling " << (*it)->type->id << std::endl;
        handle(*it);
    }
}

