#include <FPG/Misc_visitors.h>

#include <cassert>

Contains_function_call::Contains_function_call()
   : Attribute_visitor<bool>( false ) // not used, anyway, because the method is overridden
{
    set_default_value( false );
    set_shortcut_value( true );
}

bool
Contains_function_call::combine(bool a, bool b) {
    return a || b;
}

void
Contains_function_call::visit( AST::FunctionCall * funcall ) {
    argused(funcall);
    result_value = true; // no need for further recursion
}

bool
contains_function_call( AST::Node *n ) {
    Contains_function_call c;
    n->accept( &c );
    return c.result_value;
}


// ------------------------------------

Contains_floatingpoint_comparison::Contains_floatingpoint_comparison( bool interprocedural )
  : Attribute_visitor<bool>( interprocedural )
{
    set_default_value( false );
    set_shortcut_value( true );
}

bool
Contains_floatingpoint_comparison::combine( bool a, bool b ) {
    return a || b;
}

void
Contains_floatingpoint_comparison::visit( AST::BinaryExpression *bexp ) {
    switch( bexp->kind ) {
    case AST::BinaryExpression::LE:
    case AST::BinaryExpression::GR:
    // first, assume we can also compare for equality. check later, if it is relly safe
    // (ie., the values are plain input values)
    case AST::BinaryExpression::EQ:
    case AST::BinaryExpression::NEQ:
    case AST::BinaryExpression::LEQ:
    case AST::BinaryExpression::GEQ:
        // we have a floating point comparison, if at least one of
        // the expressions has type "float"
        if( is_float(bexp->e1->getType()) || is_float(bexp->e2->getType()) )
            result_value = true;
        else {
            handle( bexp->e1 );
            handle( bexp->e2);
        }
        break;
    default:
        // TODO: how to transform comparisons, nested in expressions?
        handle( bexp->e1 ); handle( bexp->e2);
    }
}

void
Contains_floatingpoint_comparison::visit( AST::UnaryFunction* f ) {
    if( f->kind == AST::UnaryFunction::XSIGN && is_float(f->e->getType()) )
        result_value = true;
    else
        handle( f->e );
}

bool
contains_floatingpoint_comparison( AST::Node *n, bool interprocedural ) {
    Contains_floatingpoint_comparison fcomp( interprocedural );
    n->accept( &fcomp );
    return fcomp.result_value;
}


// --------------------------

// label those functions that need to be filtered
void
Label_filtered_functions::visit( AST::FunctionDefinition* fundef ) {
    if( contains_floatingpoint_comparison( fundef ) && is_int( fundef->type->return_type ) )
        add( fundef );
    // other functions are reached via translation_unit. no need to traverse AST any further
}

void
Label_filtered_functions::add( AST::FunctionDefinition* fundef ) {
    filtered.insert( fundef );
}

bool
Label_filtered_functions::contains( AST::FunctionDefinition* fundef ) {
    return filtered.find( fundef ) != filtered.end();
}


// --------------------------

Collect_function_calls::Collect_function_calls( bool interprocedural )
  : Generic_visitor(interprocedural)
{}

void
Collect_function_calls::visit( AST::FunctionCall *funcall ) {
    // do not forget the function arguments! handled by parent
    Generic_visitor::visit( funcall );

    if( set_of_funcalls.find( funcall ) == set_of_funcalls.end() ) {
        funcalls.push_back( funcall );
        set_of_funcalls.insert( funcall );
    }
}

void
Collect_function_calls::clear() {
    funcalls.clear();
    set_of_funcalls.clear();
}


// ---------------------------

// interprocedural does not make sense here
Collect_variables::Collect_variables() : Generic_visitor( false ) {}

//    std::set< Variable*   > variables;
//    std::set< std::string > variable_names;

void
Collect_variables::visit( AST::VariableDeclaration* vardecl ) {
    assert( vardecl != nullptr );
    add( vardecl->var );
    Generic_visitor::visit( vardecl );
}

void
Collect_variables::visit( AST::FunctionDefinition* fundef ) {
    assert( fundef != nullptr );
    FunctionType::ParameterList::iterator it;
    for( it = fundef->type->parameters.begin(); it != fundef->type->parameters.end(); ++it ) {
        Variable *var = *it;
        add( var );
    }
    // traverse function body, too:
    Generic_visitor::visit( fundef );
}

void
Collect_variables::add( Variable *var ) {
    assert( var != nullptr );
    variables.insert( var );
    variable_names.insert( var->id );
}

bool
Collect_variables::contains( Variable *var ) {
    assert( var != nullptr );
    return variables.find( var ) != variables.end();
}

bool
Collect_variables::contains( const std::string& varname ) {
    return variable_names.find( varname ) != variable_names.end();
}

unsigned int
Collect_variables::number_of_variable_names() {
    return (unsigned int)(variable_names.size());
}

void
Collect_variables::clear() {
    variable_names.clear();
    variables.clear();
}

std::string
Collect_variables::make_fresh_variable_name( const std::string& prefix ) {
    std::string suffix;
    // NOTE: this should quickly terminate ... 60^7 is a fairly large number!
    // use a while loop: maybe, we dont even need the suffix in some cases
    while( contains( prefix + suffix ) || symbol_env.hasFunction( prefix + suffix) ) {
        suffix = "_" + random_identifier();
    }
    return prefix + suffix;
}


// ---------------------------

Compute_call_count::Compute_call_count( unsigned int initial_value )
   : Generic_visitor( false ), // intra-procedural! we are not interested in indirect callcount
     initial_value(initial_value)
{}

void
Compute_call_count::visit( AST::FunctionCall* funcall ) {
    Generic_visitor::visit( funcall );
    if( funcall->called_function != nullptr ) {
        if( callcount.find( funcall->called_function ) == callcount.end() )
            callcount[ funcall->called_function ] = initial_value + 1;
        else
            callcount[ funcall->called_function ] ++ ;
    } // else we have an external function
}
