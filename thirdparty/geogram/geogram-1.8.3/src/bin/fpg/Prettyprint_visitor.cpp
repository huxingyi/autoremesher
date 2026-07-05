#include <FPG/Prettyprint_visitor.h>
#include <cassert>
#include <ostream>
#include <iomanip>

static bool do_indent = true;


static std::ostream&
operator<< (std::ostream& o, Prettyprint_visitor::Output_rep orep ) {
    orep();
    return o;
}


Prettyprint_visitor::Prettyprint_visitor(std::ostream& out)
  : out(out), indent_count(0)
{}


Prettyprint_visitor::Prettyprint_visitor(std::ostream& out, const Annotation_map& annotation_map)
  : out(out), annotation_map(annotation_map), indent_count(0)
{}

void
Prettyprint_visitor::dumpIndent() {
    if( do_indent == false )
        return;
    unsigned int bla = indent_count;
    while( bla-- ) out << " ";
}

void
Prettyprint_visitor::indent() {
    indent_count += 4;
}

void
Prettyprint_visitor::unindent() {
    assert( indent_count >= 4 );
    indent_count -= 4;
}

void
Prettyprint_visitor::printIndented( AST::Node *node  ) {
    if( node->isBlock () )
        out << oformat(node);
    else {
        indent();
        out << oformat(node);
        unindent();
    }
}

Prettyprint_visitor::Output_rep
Prettyprint_visitor::oformat( AST::Node *node ) {
    return Output_rep( node, this );
}
/*void
Prettyprint_visitor::visit( AST::CastExpression *node ) {
    o << "(" << target_type->name() << ")(" << e << ")";
}*/

void
Prettyprint_visitor::visit( AST::LiteralExpression *node ) {
    const Concrete_value& val = node->val;
    if( val.type == Concrete_value::INTCONST )
        out << std::setprecision(0) << std::fixed << val.i;
    else {
        out << std::setprecision(20) << std::scientific << val.f;
    }
}

void
Prettyprint_visitor::visit( AST::IdentifierExpression *node ) {
    out << node->var->id;
}


void
Prettyprint_visitor::visit( AST::UnaryExpression *node ) {
    switch( node->kind ) {
        default:
            out << AST::UnaryExpression::operators[ node->kind ] << oformat(node->e); break;
    }
}

void
Prettyprint_visitor::visit( AST::BinaryExpression *node ) {
    out << "(" << oformat(node->e1) << " " << AST::BinaryExpression::operators[node->kind] << " "
               << oformat(node->e2) << ")";
}

/*
void
Prettyprint_visitor::visit( AST::ArrayExpression *node ) {
    out << a << "[" << index << "]";
}*/


void
Prettyprint_visitor::visit( AST::AssignmentExpression *node ) {
    out << oformat(node->e1) << " = " << oformat(node->e2);
}

void
Prettyprint_visitor::visit( AST::UnaryFunction *node ) {
    AST::Expression *e = node->e;
    argused(e);
    assert( e != nullptr );
    switch( node->kind ) {
    case AST::UnaryFunction::XSIGN : out << "sign"; break;
    case AST::UnaryFunction::XSQRT : out << "sqrt"; break;
    case AST::UnaryFunction::XABS  : out << "fabs"; break;
    default: ;
    }
    out << "(" << oformat(node->e) << ")";
}

void
Prettyprint_visitor::visit( AST::FunctionCall *node ) {
    out << node->fun_type->id << "( ";
    AST::ExpressionList::iterator arg_iter; //->begin();
    //for( ; arg_iter != node->exp_list->end(); ++arg_iter )
    for( arg_iter = node->exp_list->begin(); arg_iter != node->exp_list->end(); ++arg_iter ) {
        AST::Expression *exp = *arg_iter;
        if( arg_iter != node->exp_list->begin() )
            out << ", ";
        out << oformat(exp);
    }
    out << " )";
}

void
Prettyprint_visitor::visit( AST::EmptyStatement *node ) {
    argused(node);
    dumpIndent();
    out << ";" << std::endl;
}

void
Prettyprint_visitor::visit( AST::ExpressionStatement *node ) {
    dumpIndent();

    if( node->e )
        out << oformat(node->e);
    out << ";" << std::endl;
}

void
Prettyprint_visitor::visit( AST::ConditionalExpression *node ) {
    out << "(" << oformat(node->cond) << " ? "
               << oformat(node->e1) << " : "
               << oformat(node->e2) << ")";
}

void
Prettyprint_visitor::visit( AST::VariableDeclaration *node ) {
    dumpIndent();
    out << node->var->type->name() << " " << node->var->id;
    if( node->initializer )
        out << " = " << oformat(node->initializer);
    out << ";" << std::endl;
}


void
Prettyprint_visitor::visit( AST::ConditionalStatement *node ) {
    dumpIndent();
    out << "if( " << oformat(node->cond) << " )" << std::endl;
    printIndented( node->then_branch );
    if( node->else_branch ) {
        dumpIndent();
        out << "else " << std::endl;
        printIndented( node->else_branch );
    }
}


void
Prettyprint_visitor::visit( AST::Return *node ) {
    dumpIndent();
    out << "return";
    if( node->e )
        out << " " << oformat(node->e);
    out << ";" << std::endl;
}

void
Prettyprint_visitor::visit( AST::StatementList *node ) {
    for( AST::StatementContainer::iterator it = node->statements->begin(); it != node->statements->end(); ++it ) {
        AST::CompoundStatement *c = dynamic_cast<AST::CompoundStatement*>(*it);
        if( c != nullptr && c->statements->statements->size() == 1 )
            out << oformat(c->statements);
        else
            out << oformat(*it);
    }
}

void
Prettyprint_visitor::visit( AST::CompoundStatement *node ) {
    /*if( node->statements->statements->size() == 1 &&
        (dynamic_cast<AST::ExpressionStatement*>(node->statements->statements->front()) != nullptr ||
         dynamic_cast<AST::Return*>(node->statements->statements->front()) != nullptr    ) )
    {
        indent();
        out << oformat(node->statements);
        unindent();
    } else {*/
        dumpIndent();
        out << "{" << std::endl;
        indent();
        out << oformat(node->statements);
        unindent();
        dumpIndent();
        out << "} " << std::endl;
    //}
}

/*
void
Prettyprint_visitor::visit( AST::ForLoop *node ) {
    dumpIndent( o );
    out << "for( ";
    do_indent = false;
    if( init )
        out << init;
    else
        out << ";";

    if ( cond )
        out << cond;
    else
        out << ";";

    if( iter )
        out << iter;

    out << " )" << std::endl;
    do_indent = true;


    if( body )
        printIndented( body, o );
    else
        out << ";";
    out << std::endl;
}

void
Prettyprint_visitor::visit( AST::DoLoop *node ) {
    dumpIndent( o );
    out << "do " << std::endl;
    printIndented( body, o );
    out << "while( " << cond << " );" << std::endl;
}

void
Prettyprint_visitor::visit( AST::WhileLoop *node ) {
    dumpIndent( o );
    out << "while( " << cond << " )" << std::endl;
    printIndented( body, o );
    out << std::endl;
}

void
Prettyprint_visitor::visit( AST::SwitchStatement *node ) {
    dumpIndent( o );
    out << "switch( " << val << " )" << std::endl;
    dumpIndent( o );
    out << "{" << std::endl;
    indent();
    FOREACH_SWITCH_CASE {
        Expression *exp  = (*it)->first;
        Statement  *stmt = (*it)->second;
        dumpIndent( o );
        out << "case " << exp->evaluate().i << ": " << std::endl;
        printIndented( stmt, o );
    }
    dumpIndent( o );
    out << "default: " << std::endl;
    printIndented( default_stmt, o );

    unindent();
    dumpIndent( o );
    out << "}" << std::endl;
}*/

void
Prettyprint_visitor::visit( AST::FunctionDefinition *fundef ) {
    out << std::endl << fundef->type->name() << "( ";
    // FunctionType::ParameterList::iterator it = fundef->type->parameters.begin();
    FOREACH_PARAMETER(fundef->type) {
        std::string comma = (it == fundef->type->parameters.begin()) ? "" : ", ";
        std::cout << comma << (*it)->type->name() << " " << (*it)->id;
    }
    out << ") " << oformat(fundef->body) << std::endl;
}

void
Prettyprint_visitor::visit( AST::PlainText *text) {
    dumpIndent();
    out << text->text << std::endl;
}

void
Prettyprint_visitor::visit( AST::PlainTextExpression *eplain) {
    out << eplain->text;
}

void
Prettyprint_visitor::visit( AST::TranslationUnit *node ) {
    // TODO: add a switch to ignore/not ignore global statements.
    //out << oformat(node->global_statements);
    AST::ListOfFunctions::iterator it;
    for( it = node->functions.begin(); it != node->functions.end(); ++it )
        out << oformat(*it);
}



