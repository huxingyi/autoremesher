#include <cassert>
#include <iostream>
#include <sstream>

#include <FPG/AST.h>
#include <FPG/Error.h>

namespace AST {

void
Node::dumpPrefix( int level )
{
    //int bla = level;
    /*if( getType() != type_void ) {
        while( bla-- ) std::cout << " ";
        std::cout << "type: " << getType()->name() << std::endl;
        bla = level;
    }*/
    while( level-- ) std::cout << " ";
}

void
LiteralExpression::dump( int level ) {
    dumpPrefix( level );
    switch(val.type) {
        case Concrete_value::INTCONST  : std::cout << "int("   << val.i << ")" << std::endl; break;
        case Concrete_value::FLOATCONST: std::cout << "float(" << val.f << ")" << std::endl; break;
    }
}

/*
void
StringLiteralExpression::dump( int level ) {
    dumpPrefix( level );
    std::cout << "string literal at position " << offset << std::endl;
}*/

void
IdentifierExpression::dump( int level ) {
    dumpPrefix( level );
    std::cout << "var(" << var->type->name() << ":" << var->id << ")"<< std::endl;
}

void
UnaryExpression::dump( int level ) {
    dumpPrefix( level );
    std::cout << "uexp " << operators[kind] << std::endl;
    e->dump( level + 3 );
}

void
BinaryExpression::dump( int level ) {
    dumpPrefix( level );
    std::cout << "bexp " << operators[kind] << std::endl;
    e1->dump( level + 3 );
    e2->dump( level + 3 );
}

void
ConditionalExpression::dump( int level ) {
  dumpPrefix( level );
  std::cout << "condexp";
  cond->dump ( level + 3);
  e1->dump( level + 3 );
  e2->dump( level + 3 );
}

/*
void
ArrayExpression::dump( int level ) {
   dumpPrefix( level );
    std::cout << "array access" << std::endl;
    a->dump( level + 3 );
    index->dump( level + 3 );
}*/

void
AssignmentExpression::dump( int level ) {
    dumpPrefix( level );
    std::cout << " = " << std::endl;
    e1->dump( level + 3 );
    e2->dump( level + 3 );
}

void
FunctionCall::dump( int level ) {
    dumpPrefix( level );
    assert( fun_type != nullptr );
    std::cout << "funcall " << fun_type->id << std::endl;
    FOREACH_ARG
       (*it)->dump( level + 3 );
}

void
UnaryFunction::dump( int level ) {
    dumpPrefix( level );
    switch( kind ) {
    case XSIGN: std::cout << "sign" << std::endl; break;
    case XSQRT: std::cout << "sqrt" << std::endl; break;
    case XABS : std::cout << "fabs" << std::endl; break;
    }
    e->dump( level + 3 );
}

void
ConditionalStatement::dump( int level ) {
    dumpPrefix( level );
    std::cout << "if " << std::endl;
    cond->dump( level + 3 );
    then_branch->dump( level + 3 );
    dumpPrefix( level + 3);
    std::cout << "else" << std::endl;
    cond->dump( level + 3 );
    if( else_branch != nullptr )
        else_branch->dump( level +3 );
}

/*
void
SwitchStatement::dump( int level ) {
    dumpPrefix( level );
    std::cout << "switch " << std::endl;
    FOREACH_SWITCH_CASE {
        dumpPrefix( level );
        std::cout << "case " << std::endl;
        (*it)->first->dump( level + 3);
        (*it)->second->dump( level + 3);

    }
}*/

void
Return::dump( int level ) {
    dumpPrefix( level );
    std::cout << "return " << std::endl;
    if(e)
        e->dump(level+3);
}

void
PlainText::dump( int level ) {
    dumpPrefix( level );
    std::cout << "plaintext: " << text << std::endl;
}

void
PlainTextExpression::dump( int level ) {
    dumpPrefix( level );
    std::cout << "plaintextexpr " << text << std::endl;
}

/*
void
WhileLoop::dump( int level ) {
    dumpPrefix( level );
    std::cout << "while (" <<std::endl;
    if( cond ) cond->dump( level ); std::cout<<")"<<std::endl;
    if( body ) body->dump( level + 3 );
}

void
DoLoop::dump( int level ) {
  dumpPrefix( level );
  std::cout << "do" <<std::endl;
  if( body ) body->dump( level + 3 );
  std::cout << "while (" <<std::endl;
  if( cond ) cond->dump( level ); std::cout<<")"<<std::endl;
}

void
ForLoop::dump( int level ) {
  dumpPrefix( level );
  std::cout<< " for ( "<< std::endl;
  if(init) init-> dump( level );
  if(cond) cond-> dump( level );
  if(iter) iter-> dump( level );
  dumpPrefix( level );
  std::cout<<")"<<std::endl;
  if(body) body-> dump( level + 3 );
}

void
Jump::dump( int level ) {
    dumpPrefix( level );
    std::cout<< " jump "<<target<<std::endl;
}*/

void
VariableDeclaration::dump( int level ) {
    dumpPrefix( level );
    std::cout << "var declaration: " << var->type->name() << " " << var->id << std::endl;
}


void
ExpressionStatement::dump( int level ) {
    dumpPrefix( level );
    std::cout << "expression statement " << std::endl;
    if(e)
	e->dump( level + 3 );
    else std::cout<< ";"<<std::endl;
}

void
CompoundStatement::dump( int level ) {
    dumpPrefix( level );
    std::cout << "compound statement " << std::endl;
    statements->dump( level + 3 );
}

void
StatementList::dump( int level ) {
    dumpPrefix( level );
    std::cout << "statement list" << std::endl;
    FOREACH_STATEMENT
    (*it)->dump( level + 3 );
}

void
FunctionDefinition::dump( int level ) {
    dumpPrefix( level );
    std::cout << "function definition: " << type->return_type->name() << " " << type->id << "(";
    FOREACH_PARAMETER(type)
        std::cout << (it == type->parameters.begin() ? "" : ", ") << (*it)->type->name() << " " << (*it)->id;

    std::cout << ")" << std::endl;
    body->dump( level + 3);
}

void
TranslationUnit::dump(int level ) {
    global_statements->dump( level );
    FOREACH_FUNCTION
    (*it)->dump( level );
}

}
