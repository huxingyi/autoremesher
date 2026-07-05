#include <cassert>
#include <iostream>
#include <sstream>

#include <FPG/AST.h>
#include <FPG/Visitor.h>
#include <FPG/Generic_visitor.h>
#include <FPG/Attribute_visitor.h>
#include <FPG/Misc_visitors.h>
#include <FPG/Error.h>
#include <FPG/SymbolEnvironment.h>

//#define DEBUG
#include <FPG/MSG.h>


namespace AST {


// be careful with order of global initialization, AST::Node depends on current_location!
Location current_location;
std::ostream*     Node::os = nullptr;
std::string       Node::context;
//SymbolEnvironment Node::symbol_env;

Type *return_type = nullptr;

LiteralExpression *literal_one  = new LiteralExpression( 1 );
LiteralExpression *literal_zero = new LiteralExpression( 0 );

namespace {
struct Clone_postprocessing : public Generic_visitor {
    Clone_context *context;

    Clone_postprocessing( Clone_context *context ) : context(context) {}


    virtual void visit( FunctionCall *funcall ) {
        // f not in m <==> there was no substitution
        //             ==> nothing to do
        if( context->fundef_map.find( funcall->called_function ) != context->fundef_map.end() ) {
            funcall->called_function = context->fundef_map[ funcall->called_function ];
            funcall->fun_type = funcall->called_function->type;
        }
        Generic_visitor::visit( funcall );
    }
};

}

Node*
clone_prime( Node* n, Clone_context *context ) {
    if( context == nullptr )
        context = new Clone_context;
    // store n ... we need to do postprocessing on it
    n = n->clone( context );
    // maintain proper links to function definitions
    Visitor *v = new Clone_postprocessing( context );
    n->accept( v );
    delete v;
    delete context;
    return n;
}

Expression*
makeBinaryExpression( Expression *e1, Expression *e2, BinaryExpression::Kind kind ) {
    //if( kind == BinaryExpression::AND )
    //    return new ConditionalExpression( e1, e2, literal_zero );
    //else if( kind == BinaryExpression::OR )
    //    return new ConditionalExpression( e1, literal_one, e2 );
    //else
        return new BinaryExpression(e1, e2, kind );
}

Statement*
make_assign_stmt( Variable *var, Expression *e ) {
    AST::Expression *a = new IdentifierExpression( var );
    if( var->type == type_float_bound_ptr )
        a = new AST::UnaryExpression( a, AST::UnaryExpression::DEREF );
    return new ExpressionStatement( new AssignmentExpression( a, e ) );
}


AST::FunctionCall*
make_funcall( FunctionType *funtype, AST::Expression *a1 ) {
    AST::ExpressionList *explist = new AST::ExpressionList;
    explist->push_back( a1 );
    return new AST::FunctionCall( funtype, explist );
}

AST::FunctionCall*
make_funcall( FunctionType *funtype, AST::Expression *a1, AST::Expression *a2 ) {
    AST::ExpressionList *explist = new AST::ExpressionList;
    explist->push_back( a1 );
    explist->push_back( a2 );
    return new AST::FunctionCall( funtype, explist );
}


void
Node::setOutputStream( std::ostream &os ) {
    Node::os = &os;
}

std::ostream&
Node::out() { return *os; }

std::string
Node::getNewLabel( const std::string &prefix ) {
    static unsigned int number = 0;
    std::stringstream label;
    /*label << "." << context << prefix << (++number) << std::ends;
    return label.str().substr( 0, label.str().length() - 1);*/
    label << "." << context << prefix << (++number);
    return label.str();
}

const std::string
UnaryExpression::operators[] = { "+", "-","!","*", "&" };

const std::string
BinaryExpression::operators[] = { "+","-","*","/","%","&&","||","^",
                                  "==",">=","<=","<",">","!=","|", "," };


ConditionalExpression::ConditionalExpression( Expression* cond, Expression *e1, Expression *e2 )
 : cond(cond), e1(e1), e2(e2)
{
    //if( is_float(cond->getType()) )
    //    this->cond = new BinaryExpression( cond, new LiteralExpression(0.0f), BinaryExpression::NEQ );
}


AssignmentExpression::AssignmentExpression( Expression* e1, Expression *e2, Kind kind )
{
#define CASE(KIND) case KIND : this->e2 = makeBinaryExpression( e1, e2, BinaryExpression:: KIND); break;
    this->e1 = e1;
    switch( kind ) {
        case PLAIN: this->e2 = e2; break;
        CASE(ADD) CASE(SUB) CASE(MUL) CASE(DIV)
        CASE(MOD) CASE(AND) CASE(OR)  CASE(XOR)
    }
#undef CASE
}

ConditionalStatement::ConditionalStatement(
    Expression *cond,
    Statement *then_branch,
    Statement *else_branch
)   : cond(cond), then_branch(then_branch), else_branch(else_branch)
{
    CompoundStatement *compound = dynamic_cast<CompoundStatement*>( then_branch );
    if( compound == nullptr ) {
        StatementList *slist = new StatementList();
        slist->add( then_branch );
        this->then_branch = new CompoundStatement( slist );
    } else
        this->then_branch = compound;

    if( else_branch != nullptr ) {
        compound = dynamic_cast<CompoundStatement*>( else_branch );
        if( compound == nullptr )
        {
            StatementList *slist = new StatementList();
            slist->add( else_branch );
            this->else_branch = new CompoundStatement( slist );
        } else
            this->else_branch = compound;
    }
}


bool
StatementList::hasToplevelVariableDeclaration() {
    FOREACH_STATEMENT
        if( (*it)->hasToplevelVariableDeclaration() )
            return true;
    return false;
}


bool
ConditionalStatement::hasBreakOrReturnOnExit() {
    if( else_branch )
        return then_branch->hasBreakOrReturnOnExit() &&
               else_branch->hasBreakOrReturnOnExit();
    else
        return then_branch->hasBreakOrReturnOnExit();
}

bool
StatementList::hasBreakOrReturnOnExit() {
    FOREACH_STATEMENT
        if( (*it)->hasBreakOrReturnOnExit() )
            return true;
    return false;
}


TranslationUnit::TranslationUnit()
    : global_statements( new StatementList() ), has_main( false )
{}


void
TranslationUnit::add( Statement *stm ) {
    global_statements->add( stm );
}

void
TranslationUnit::add( FunctionDefinition *fun_def ) {
    functions.push_back( fun_def );
    FunctionType *type = fun_def->type;
    if( type->id == "main" ) {
        has_main = true;
        if( type->getReturnType() != type_int)
            throw TypeError( "function main required return-type int", location );
        if( type->numberOfParameters() != 0 )
            throw TypeError( "function main must not have any arguments", location ); // TODO: special check
    }
}

// visitor

#define ACCEPT(CLASSNAME) \
void CLASSNAME ::accept( Visitor *visitor ) { assert( visitor != nullptr ); visitor->visit( this ); }

//ACCEPT(CastExpression)
ACCEPT(LiteralExpression)
ACCEPT(IdentifierExpression)
ACCEPT(UnaryExpression)
ACCEPT(BinaryExpression)
ACCEPT(ConditionalExpression)
//ACCEPT(ArrayExpression)
ACCEPT(AssignmentExpression)
ACCEPT(FunctionCall)
ACCEPT(UnaryFunction)
ACCEPT(EmptyStatement)
ACCEPT(ExpressionStatement)
ACCEPT(ConditionalStatement)
ACCEPT(Return)
ACCEPT(StatementList)
ACCEPT(VariableDeclaration)
ACCEPT(CompoundStatement)
ACCEPT(FunctionDefinition)
ACCEPT(PlainText)
ACCEPT(PlainTextExpression)
ACCEPT(TranslationUnit)

#undef ACCEPT

template< class T >
static T*
update_location( T *node_old, T *node_new ) {
    node_new->location = node_old->location;
    return node_new;
}

LiteralExpression*
LiteralExpression::clone( Clone_context *context ) {
    argused(context);
    return update_location( this, new LiteralExpression( *this ) );
}

IdentifierExpression*
IdentifierExpression::clone( Clone_context *context ) {
    argused(context);
    
    // at this point, the old variables have already been associated to the new ones.
    // so, why not map them right here:
    MSG( "looking for var " << var->id )

    // if is_global( var->id )
    if( symbol_env.findVariable(var->id) ) {
        return update_location( this, new IdentifierExpression( var ) );
    } else {
        Variable *new_var = var;
        std::map< Variable*, Variable* >::iterator it = context->var_map.find( var );
        if( it != context->var_map.end() )
            new_var = it->second;
        return update_location( this, new IdentifierExpression( new_var ) );
    }
}

UnaryExpression*
UnaryExpression::clone( Clone_context *context ) {
    // selector not used currently
    return update_location( this, new UnaryExpression( e->clone( context ), kind ) );
}

BinaryExpression*
BinaryExpression::clone( Clone_context *context ) {
    return update_location( this, new BinaryExpression( e1->clone( context ), e2->clone( context ), kind ) );
}

ConditionalExpression*
ConditionalExpression::clone( Clone_context *context ) {
    return update_location(
        this,
        new ConditionalExpression( cond->clone( context ), e1->clone( context ), e2->clone( context ) )
    );
}

AssignmentExpression*
AssignmentExpression::clone( Clone_context *context ) {
    return update_location( this, new AssignmentExpression( e1->clone( context ), e2->clone( context ) ) );
}

UnaryFunction*
UnaryFunction::clone( Clone_context *context ) {
    return update_location( this, new UnaryFunction( e->clone( context ), kind ) );
}

FunctionCall*
FunctionCall::clone( Clone_context *context ) {
    ExpressionList *new_exp_list = new ExpressionList;
    for( ExpressionList::iterator it = exp_list->begin();
         it != exp_list->end(); ++it ) {
         Expression *e = *it;
         new_exp_list->push_back( e->clone( context ) );
    }
    // the called_function still points to an old, non-cloned fundef. this
    // is resolved by AST::clone( Node* )
    return update_location( this, new FunctionCall( fun_type, new_exp_list, called_function ) );
}

EmptyStatement*
EmptyStatement::clone( Clone_context *context ) {
    argused(context);
    return update_location( this, new EmptyStatement() );
}

ExpressionStatement*
ExpressionStatement::clone( Clone_context *context ) {
    return update_location( this, new ExpressionStatement( e->clone( context ) ) );
}

ConditionalStatement*
ConditionalStatement::clone( Clone_context *context ) {
    Statement *new_else_branch = nullptr;
    if( else_branch != nullptr )
        new_else_branch = else_branch->clone( context );
    return update_location( this, new ConditionalStatement( cond->clone( context ), then_branch->clone( context ), new_else_branch ) );
}

Return*
Return::clone( Clone_context *context ) {
    return update_location( this, new Return( e->clone( context ) ) );
}

StatementList*
StatementList::clone( Clone_context *context ) {
    StatementList *new_statementlist = update_location( this, new StatementList );
    for( StatementContainer::iterator it = statements->begin();
         it != statements->end(); ++it ) {
        Statement *stmt = *it;
        new_statementlist->add( stmt->clone( context ) );
    }
    return new_statementlist;
}

VariableDeclaration*
VariableDeclaration::clone( Clone_context *context ) {
    MSG("mapping var " << (unsigned int)var<< " to var " << (unsigned int)new_var )
    Variable *new_var = nullptr;
    if( context->var_map.find( var ) == context->var_map.end() ) {
        new_var = new Variable( var->id, var->type );
        context->var_map [ var ] = new_var;
    } else {
        //there is already a mapping, reuse it
        new_var = context->var_map[ var ];
    }
    return update_location( this, new VariableDeclaration( new_var ) );
}

CompoundStatement*
CompoundStatement::clone( Clone_context *context ) {
    return update_location( this, new CompoundStatement( statements->clone( context ) ) );
}

// a function def cannot contain another fundef, so each call to fundef::clone
// only creates _one_ single new fundef
FunctionDefinition*
FunctionDefinition::clone( Clone_context *context ) {
    assert( context != nullptr );
    // NOTE: not yet inserted in symbol_env!
    FunctionType *new_type = new FunctionType( *type );

    FunctionType::ParameterList::iterator
        param_iter     = type->parameters.begin(),
        param_iter_new = new_type->parameters.begin();
    for( ; param_iter != type->parameters.end(); ++param_iter, ++param_iter_new ) {
        assert( param_iter_new  != new_type->parameters.end() );
        Variable *var     = *param_iter;
        Variable *new_var = *param_iter_new;
        MSG("mapping var " << (unsigned int)var<< " to var " << (unsigned int)new_var )
        context->var_map[ var ] = new_var;
    }
    FunctionDefinition *new_fundef = update_location( this, new FunctionDefinition( new_type, body->clone( context ) ) );
    context->fundef_map[ this ] = new_fundef;

    return new_fundef;
}

PlainText*
PlainText::clone( Clone_context *context ) {
    argused(context);
    return update_location( this, new PlainText( text ) );
}

PlainTextExpression*
PlainTextExpression::clone( Clone_context *context ) {
    argused(context);
    return update_location( this, new PlainTextExpression( text, type ) );
}

TranslationUnit*
TranslationUnit::clone( Clone_context *context ) {
    TranslationUnit *new_tu = update_location( this, new TranslationUnit() );
    for( ListOfFunctions::iterator it = functions.begin(); it != functions.end(); ++it )
         new_tu->add( (*it)->clone( context ) );
    return new_tu;
}


bool is_equal( const Expression* e1, const Expression* e2 ) {
    const BinaryExpression *bexp1 = dynamic_cast< const BinaryExpression* >(e1);
    const BinaryExpression *bexp2 = dynamic_cast< const BinaryExpression* >(e2);
    if( bexp1 && bexp2 ) {
        BinaryExpression::Kind k1 = bexp1->kind;
        BinaryExpression::Kind k2 = bexp2->kind;
        if( k1 != k2 )
            return false;
        const bool e1e1 = is_equal( bexp1->e1, bexp2->e1 );
        const bool e2e2 = is_equal( bexp1->e2, bexp2->e2 );
        const bool e1e2 = is_equal( bexp1->e1, bexp2->e2 );
        const bool e2e1 = is_equal( bexp1->e2, bexp2->e1 );
        if( e1e1 && e2e2 )
            return true;
        if( k1 == BinaryExpression::ADD || k1 == BinaryExpression::MUL )
            return e1e2 && e2e1;
    }
    const IdentifierExpression *idexp1 = dynamic_cast< const IdentifierExpression* >(e1);
    const IdentifierExpression *idexp2 = dynamic_cast< const IdentifierExpression* >(e2);
    if( idexp1 && idexp2 )
        return idexp1->var == idexp2->var;

    return false;
}

Expression *uncertain_return_value = new PlainTextExpression("FPG_UNCERTAIN_VALUE", type_int);

} // end namespace AST

AST::Expression*
make_fe_condition() {
    return
        new AST::BinaryExpression(
            new AST::PlainTextExpression(
                "::fetestexcept( FE_DIVBYZERO | FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID )", type_int
            ),
            new AST::LiteralExpression( 0 ),
            AST::BinaryExpression::EQ
        );
}

AST::Statement*
make_fe_clear_and_fail() {
    AST::StatementList *l = new AST::StatementList();
    l->add( new AST::ExpressionStatement(
                new AST::PlainTextExpression( "::feclearexcept( FE_DIVBYZERO | FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID )", type_int )
            ));
    l->add( new AST::Return( AST::uncertain_return_value  ) );
    return l;
}

