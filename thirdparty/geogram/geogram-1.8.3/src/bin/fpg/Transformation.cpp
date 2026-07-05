#include <CGALmini/basic.h>
#include <FPG/Transformation.h>

#include <algorithm>
#include <cassert>
#include <FPG/Prettyprint_visitor.h>
//#define DEBUG 1
#include <FPG/MSG.h>

using namespace AST;

Make_unique_funcalls::Make_unique_funcalls()
    : should_be_filtered( nullptr ),
      compute_call_count( new Compute_call_count(1)             ),
      // false == intraproc. we are only interested in _direct_ function calls
      collect_funcalls  ( new Collect_function_calls( false )   ),
      tu                ( nullptr )
{}

void
Make_unique_funcalls::init( TranslationUnit *tu, Label_filtered_functions *should_be_filtered ) {
    this->tu = tu;
    this->should_be_filtered = should_be_filtered;
    compute_call_count->callcount.clear();
    tu->accept( compute_call_count );
}

void
Make_unique_funcalls::visit( FunctionCall *funcall ) {
    AST::FunctionDefinition *fundef = funcall->called_function;
    if( should_be_filtered->contains( fundef ) &&
        compute_call_count->callcount[ fundef ] > 1 )
    {
        FunctionDefinition *new_fundef =
            dynamic_cast<FunctionDefinition*>(::AST::clone( fundef ));
        assert( new_fundef );
        // give cloned function a new, unique name:
        std::string fresh_fun_name =
                symbol_env.make_fresh_function_name( new_fundef->type->id );

        new_fundef->type->id = fresh_fun_name;
        // NOTE: this is a dark corner of the project.
        // maybe it's time to add a pointer to the current symbol_env
        // to every instance of AST::Node ? what about translation_unit?
        // currently, we only have one valid AST in memory, so there is no conflict.
        symbol_env.add( new_fundef->type );
        symbol_env.add_fun_def( new_fundef );

        // add the cloned function right before the old function,
        // to prevent "undefined function" errors
        AST::ListOfFunctions::iterator it = std::find( tu->functions.begin(), tu->functions.end(), funcall->called_function );
        tu->functions.insert( it, new_fundef );
        // at least, this functionCall does not use it anymore:
        compute_call_count->callcount[ fundef ]--;

        // it's fresh, so we only have one function calling it:
        compute_call_count->callcount[ new_fundef ] = 1;
        // ^^^ actually, this visitor will never again see this
        // funcall/fundef, so ... no use setting callcount

        // functions, that are called from within the cloned function, now have
        // a higher callcount. first, collect (intraproc.) funcalls from within newfundef:
        collect_funcalls->clear();
        new_fundef->accept( collect_funcalls );
        for( std::list< AST::FunctionCall* >::iterator it =
                    collect_funcalls->funcalls.begin();
                it != collect_funcalls->funcalls.end(); ++it )
        {
            FunctionDefinition *f = (*it)->called_function;
            compute_call_count->callcount[ f ]++;
        }
        // finally, store fundef clone as new funcall target:
        // NOTE: the old fundef is still reachable, by at least one function, since
        // callcount was > 1
        funcall->called_function = new_fundef;
        funcall->fun_type = new_fundef->type;

        // this fundef had to be filtered. dont forget to filter its clone, too:
        should_be_filtered->add( new_fundef );
    }
    Generic_visitor::visit( funcall );
}

// --------------------------------------------------------------

bool
has_unique_funcalls( AST::TranslationUnit *tu, Label_filtered_functions *should_be_filtered ) {
    Check_unique_funcalls *check = new Check_unique_funcalls( should_be_filtered );
    tu->accept( check->compute_call_count );
    tu->accept( check );
    return check->result_value;
}

void
make_unique_funcalls( AST::TranslationUnit *tu, Label_filtered_functions *should_be_filtered ) {
    Make_unique_funcalls *mkunif = new Make_unique_funcalls();
    mkunif->init( tu, should_be_filtered );
    tu->accept( mkunif );
    delete mkunif;
}


// --------------------------------------------------------------

Check_unique_funcalls::Check_unique_funcalls( Label_filtered_functions *should_be_filtered )
    : should_be_filtered( should_be_filtered     ),
      compute_call_count( new Compute_call_count )
{
    set_default_value( true );
    set_shortcut_value( false );
}

bool
Check_unique_funcalls::combine( bool a, bool b ) { return a && b; }

void
Check_unique_funcalls::visit( FunctionCall *funcall ) {
    AST::FunctionDefinition *fundef = funcall->called_function;
    if( should_be_filtered->contains( fundef ) &&
        compute_call_count->callcount[ fundef ] > 1 )
    {
        //std::cout << "callcount of " << funcall->called_function->type->id << " == "
        //          << compute_call_count->callcount[ funcall->called_function ] << std::endl;
        result_value = false;
    }
    Attribute_visitor<bool>::visit( funcall );
}

// --------------------------------------------------------------


Statement_addition_visitor::Statement_addition_visitor()
  : Generic_visitor( false ),
    toplevel_position( nullptr ),
    fundef_toplevel( false )
{}

void
Statement_addition_visitor::visit( AST::StatementList * l ) {
    AST::StatementContainer::iterator it;
    positions.push_back( Position( l ) );
    if( fundef_toplevel && toplevel_position == nullptr )
        toplevel_position = &(positions.back());
    for( it = l->statements->begin(); it != l->statements->end(); ++it ) {
        AST::Statement *stmt = *it;
        positions.back().current_stmt = it;
        handle( stmt );
    }
    positions.pop_back();
}

void
Statement_addition_visitor::visit( AST::FunctionDefinition *fundef ) {
    fundef_toplevel = true;
    assert( toplevel_position == nullptr );
    Generic_visitor::visit( fundef );
    fundef_toplevel = false;
    toplevel_position = nullptr;
}


// new statement will be added before the last visited statement
void
Statement_addition_visitor::add_stmt( AST::Statement* stmt, bool before ) {
    assert( positions.size() > 0 );
    Position &p = positions.back();
    if( before )
        p.stmt_list->add( stmt, p.current_stmt );
    else {
        AST::StatementContainer::iterator it = p.current_stmt;
        ++it;
        p.stmt_list->add( stmt, it );
    }
}

// new statement will be added before the last visited statement
void
Statement_addition_visitor::add_stmt_toplevel( AST::Statement* stmt, bool before ) {
    assert( positions.size() > 0 );
    assert( toplevel_position != nullptr );
    if( before )
        toplevel_position->stmt_list->add( stmt, toplevel_position->current_stmt );
    else {
        AST::StatementContainer::iterator it = toplevel_position->current_stmt;
        ++it;
        toplevel_position->stmt_list->add( stmt, it );
    }
}

struct Collect_variable_declarations : public Generic_visitor {
    std::set<Variable *> &vars;
    Collect_variable_declarations( std::set<Variable*> &vars )
        : Generic_visitor(false),
          vars(vars)
    {}
    // stop evaluation at compound statements:
    virtual void visit( AST::CompoundStatement* ) {}

    virtual void visit( AST::VariableDeclaration *vardecl ) {
        //std::cout << "  candidate: " << vardecl->var->id << std::endl;
        std::set< Variable *>::iterator v_it = vars.find( vardecl->var );
        if( v_it != vars.end() )
            vars.erase( v_it );
    }
};



Statement_addition_visitor::Position
Statement_addition_visitor::get_scope( std::set< Variable *> &vars ) {
    std::list< Position >::iterator p_it;
    if( vars.size() == 0 )
        return positions.front();
#ifdef DEBUG
    std::cout << "getting scope for variables: " << std::endl;
    for( std::set< Variable *>::iterator v_it = vars.begin(); v_it != vars.end(); ++v_it )
        std::cout << (*v_it)->id << " ";
    std::cout << std::endl;
#endif
    for( p_it = positions.begin(); p_it != positions.end(); ++p_it ) {
#ifdef DEBUG
        Prettyprint_visitor pretty(std::cout);
        std::cout << "====================" << std::endl;
        std::cout << "iteration: " << std::endl;
        std::cout << "====================" << std::endl;
        p_it->stmt_list->accept( &pretty );
#endif
        Collect_variable_declarations collect_vardecl( vars );
        p_it->stmt_list->accept( &collect_vardecl );
        if( vars.size() == 0 ) {
            // found scope!
            return *p_it;
        }
    }
    std::cout << "could not find scope for remaining variables: " << std::endl;
    for( std::set< Variable *>::iterator v_it = vars.begin(); v_it != vars.end(); ++v_it )
        std::cout << (*v_it)->id << " ";
    std::cout << std::endl;
    throw 0;
}

void
Statement_addition_visitor::add_stmt_with_scope( AST::Statement* stmt, Position p ) {
    p.stmt_list->add( stmt, p.current_stmt );
}

bool
Statement_addition_visitor::is_last_stmt() {
    Position &p = positions.back();
    AST::StatementContainer::iterator it = p.current_stmt;
    if( it == p.stmt_list->statements->end() )
        return false;
    ++it;
    return it == p.stmt_list->statements->end();
}

// ------------------------------------------------------------


Transformation_visitor::Transformation_visitor()
{}

void
Transformation_visitor::visit( AST::LiteralExpression *lexp ) {
    push( lexp );
}

void
Transformation_visitor::visit( AST::IdentifierExpression* iexp ) {
    push( iexp );
}

void
Transformation_visitor::visit( AST::UnaryExpression* e ) {
    transform<AST::Expression>( e->e );
    push( e );
}

void
Transformation_visitor::visit( AST::BinaryExpression* e ) {
    transform<AST::Expression>( e->e1 );
    transform<AST::Expression>( e->e2 );
    push( e );
}

void
Transformation_visitor::visit( AST::ConditionalExpression* c ) {
    transform<AST::Expression>( c->cond );
    transform<AST::Expression>( c->e1 );
    if( c->e2 != nullptr )
        transform<AST::Expression>( c->e2 );
    push( c );
}

void
Transformation_visitor::visit( AST::AssignmentExpression* e ) {
    transform<AST::Expression>( e->e2 );
    push( e );
}

void
Transformation_visitor::visit( AST::FunctionCall* fun_call ) {
    AST::ExpressionList::iterator            arg_iter = fun_call->exp_list->begin();

    for( ; arg_iter != fun_call->exp_list->end(); ++arg_iter )
        transform<AST::Expression>( *arg_iter );
    push( fun_call );
}

void
Transformation_visitor::visit( AST::UnaryFunction* f ) {
    transform<AST::Expression>( f->e );
    push( f );
}

void Transformation_visitor::visit( AST::EmptyStatement* s) {
    push(s);
}

void Transformation_visitor::visit( AST::PlainText* t) {
    push(t);
}

void Transformation_visitor::visit( AST::PlainTextExpression* e) {
    push(e);
}

void
Transformation_visitor::visit( AST::ExpressionStatement* e ) {
    transform<AST::Expression>( e->e );
    push( e );
}

void
Transformation_visitor::visit( AST::ConditionalStatement* s ) {
    transform<AST::Expression>( s->cond );
    transform<AST::Statement>( s->then_branch );
    if( s->else_branch )
        transform<AST::Statement>( s->else_branch );
    push( s );
}

void
Transformation_visitor::visit( AST::Return* r ) {
    transform<AST::Expression>( r->e );
    push( r );
}

void
Transformation_visitor::visit( AST::StatementList* l ) {
    // NOTE: keep in sync with Base::visit!!
    AST::StatementContainer::iterator it;
    positions.push_back( Position( l ) );
    if( fundef_toplevel && toplevel_position == nullptr )
        toplevel_position = &(positions.back());
    for( it = l->statements->begin(); it != l->statements->end(); ++it ) {
        positions.back().current_stmt = it;
        transform<AST::Statement>( *it );
    }
    positions.pop_back();
    push( l );
}

void
Transformation_visitor::visit( AST::VariableDeclaration* decl ) {
    push( decl );
}

void
Transformation_visitor::visit( AST::CompoundStatement* cs ) {
    transform<AST::StatementList>( cs->statements );
    push( cs );
}

void
Transformation_visitor::visit( AST::FunctionDefinition* fd ) {
    Statement_addition_visitor::visit( fd );
    pop<AST::Statement>();
    // the translation unit does not care, so dont push
}

void
Transformation_visitor::update( AST::Node *node_old, AST::Node *node_new ) {
    argused(node_old);
    argused(node_new);
    MSG("transforming: old" )
    //node_old->dump(0);
    MSG("transforming: new" )
    //node_new->dump(0);
}

template< class T >
void
Transformation_visitor::transform( T *&t ) {
    MSG("")
    unsigned int size = (unsigned int)(node_stack.size());
    argused(size);
    handle( t );
    assert( node_stack.size() == size + 1 );
    T *new_t = pop<T>();
    if( t != new_t ) {
        MSG("update!")
        update( t, new_t );
        t = new_t;
    }
}

template< class T >
T*
Transformation_visitor::pop() {
    assert( node_stack.size() > 0 );
    T *t = dynamic_cast<T*>(node_stack.top());
    node_stack.pop();
    assert( t != nullptr );
    return t;
}

void
Transformation_visitor::push( AST::Node *n ) {
    assert( n != nullptr );
    node_stack.push( n );
}

// ------------------------------------

Add_bound_variables_base::Add_bound_variables_base()
{}

Add_bound_variables_base::~Add_bound_variables_base() {
    delete collect_variables;
}



Variable*
Add_bound_variables_base::add_bound( std::string id ) {
    return add_variable( id + "_bound", type_float_bound );
}

Variable*
Add_bound_variables_base::add_bound( Variable *var ) {
    assert( var != nullptr );
    Variable *new_var = add_bound( var->id );
    assert( ! has_bound_variable( var ) );
    bound_map[ var ] = new_var;
    return new_var;
}

Variable*
Add_bound_variables_base::add_bound( AST::FunctionCall *funcall ) {
    assert( funcall != nullptr );
    Variable *new_var = add_bound( funcall->called_function->type->id + "_result" );
    result_bound_storage_map[ funcall ] = new_var;
    return new_var;
}

Variable *
Add_bound_variables_base::bound_variable( Variable *float_var ) {
    assert( float_var != nullptr );
    assert( has_bound_variable( float_var ) );
    return bound_map[ float_var ];
}

Variable *
Add_bound_variables_base::bound_variable( AST::FunctionDefinition *fundef ) {
    assert( fundef != nullptr );
    assert( has_bound_variable( fundef ) );
    return result_bound_map[ fundef ];
}

Variable*
Add_bound_variables_base::bound_variable( AST::FunctionCall *funcall ) {
    assert( funcall != nullptr );
    assert( has_bound_variable( funcall ) );
    return result_bound_storage_map[ funcall ];
}

bool
Add_bound_variables_base::has_bound_variable( Variable *float_var ) {
    return bound_map.find( float_var ) != bound_map.end();
}

bool
Add_bound_variables_base::has_bound_variable( AST::FunctionDefinition *fundef ) {
    return result_bound_map.find( fundef ) != result_bound_map.end();
}

bool
Add_bound_variables_base::has_bound_variable( AST::FunctionCall *funcall ) {
    return result_bound_storage_map.find( funcall ) != result_bound_storage_map.end();
}

// -----------------

void
Common_subexpression_elimination::visit( AST::BinaryExpression* bexp ) {
    Transformation_visitor::visit( bexp );
    Transformation_visitor::pop<AST::BinaryExpression>();
    AST::Expression *result_value = bexp;
    if( filter( bexp ) ) {
        std::vector< AST::Expression* >::iterator
            reuse_exp_it = std::find_if( subexpressions.begin(),
                                         subexpressions.end(),
                                         AST::Is_equal_to_expression(bexp) );
        Variable *var = nullptr;
        if( reuse_exp_it == subexpressions.end() ) {
            AST::IdentifierExpression *idexp1 = dynamic_cast<AST::IdentifierExpression*>(bexp->e1),
                                      *idexp2 = dynamic_cast<AST::IdentifierExpression*>(bexp->e2);
            if( idexp1 != nullptr && idexp2 != nullptr ) {
                std::string id1 = idexp1->var->id;
                std::string id2 = idexp2->var->id;
                var = add_tmp_result( bexp, id1 + "_" + id2 );
            } else
                var = add_tmp_result( bexp );
            subexpressions.push_back( bexp );
            add_stmt_toplevel( new AST::VariableDeclaration( var ) );
            add_stmt_toplevel( make_assign_stmt( var, bexp ) );
        } else {
            assert( has_tmp_result_variable( *reuse_exp_it ) );
            var = tmp_result_variable( *reuse_exp_it );
            assert( var != nullptr );
        }
        assert( var != nullptr );
        result_value = new AST::IdentifierExpression( var );
    }
    Transformation_visitor::push( result_value );
}

// ----------------------------

struct Replace_return_stmt_with_assignment
  : public Transformation_visitor
{
    Variable    *var;
    std::string  label;
    bool         used_goto;

    Replace_return_stmt_with_assignment( Variable *var, std::string label )
      : var(var), label(label), used_goto(false)
    {
        // only return statements within function scope
        // shall be replaced!
        interprocedural = false;
    }

    virtual void visit( AST::Return *ret ) {
        MSG("")
        Transformation_visitor::visit( ret );
        Transformation_visitor::pop<AST::Statement>();
        AST::Statement *s = make_assign_stmt( var, ret->e );
        if( !is_last_stmt() ) {
            used_goto = true;
            add_stmt( s );
            Transformation_visitor::push( new AST::PlainText( std::string("goto ") + label + ";" ) );
        } else {
            //std::cerr << "is last statement!" << std::endl;
            Transformation_visitor::push( s );
       }
    }
};

struct Replace_variable_with_expression
  : public Transformation_visitor
{
    std::map< Variable*, AST::Expression* > var_to_exp;

    virtual void visit( AST::IdentifierExpression *iexp ) {
        std::map< Variable*, AST::Expression* >::iterator it = var_to_exp.find( iexp->var );
        if( it != var_to_exp.end() ) {
            //std::cout << "!! replacing var " << iexp->var->id << " with " << std::endl;
            AST::Expression *new_exp = it->second;
            //new_exp->dump(0);
            Transformation_visitor::push( new_exp );
        } else
            Transformation_visitor::push( iexp );
    }

};

struct Collect_conflict_variables : public Generic_visitor {
    Substitute_funcalls *subst_funcalls;
    AST::Clone_context  *clone_context;

    Collect_conflict_variables( Substitute_funcalls *subst_funcalls,  AST::Clone_context *clone_context )
        : Generic_visitor( false ),
          subst_funcalls( subst_funcalls ),
          clone_context( clone_context )
    {
    }

    virtual void visit( AST::VariableDeclaration *vardecl ) {
        if( subst_funcalls->collect_variables->contains( vardecl->var->id ) ) {
            //std::cout << "!! variable " << vardecl->var->id << " already declared in target scope" << std::endl;
            Variable *new_var = subst_funcalls->add_variable( vardecl->var->id + "_prime", vardecl->var->type );
            //std::cout << "  new variable: " << new_var->id << std::endl;
            clone_context->var_map[ vardecl->var ] = new_var;
        }
    }
};

// ------------------------------

void
Substitute_funcalls::visit( AST::FunctionCall *funcall ) {
    Transformation_visitor::visit( funcall );
    Transformation_visitor::pop<AST::Expression>();
    MSG( "checking " << funcall->fun_type->id )
    if( (*do_substitute)( funcall ) && !funcall->fun_type->is_extern ) {
        MSG( "do subst: " << funcall->fun_type->id )
        AST::FunctionDefinition *fundef = funcall->called_function;
        AST::Clone_context *clone_context = new AST::Clone_context;

        AST::ExpressionList::iterator           arg_iter = funcall->exp_list->begin();
        FunctionType::ParameterList::iterator param_iter = funcall->fun_type->parameters.begin();
        Collect_conflict_variables collect_conflict_vars( this, clone_context );
        fundef->body->accept( &collect_conflict_vars );
        Replace_variable_with_expression replace_var_with_exp;
        for( ; arg_iter != funcall->exp_list->end(); ++arg_iter, ++param_iter ) {
            AST::Expression *exp = *arg_iter;
            Variable        *var = *param_iter;
            MSG( var->id );
            if( var->type != nullptr ) {
                MSG( var->type->id );
            } else {
                MSG( " null type!");
            }
            assert( clone_context->var_map.find( var ) == clone_context->var_map.end() );
            AST::IdentifierExpression *idexp = dynamic_cast<AST::IdentifierExpression*>( exp );
            if( idexp == nullptr ) {
                replace_var_with_exp.var_to_exp[ var ] = exp;
                clone_context->var_map[ var ] = var;
            } else
                clone_context->var_map[ var ] = idexp->var;
        }
        // variables in new_body have been replaced by their new_var counterparts, as indicated
        // in "clone_context"
        AST::CompoundStatement *new_body = AST::clone( fundef->body, clone_context );
        new_body->accept( &replace_var_with_exp );
        //std::cout << "new body: " << std::endl;
        //new_body->dump(0);
        //Substitute_variables subst_var( var_map );
        //new_body->accept( &subst_var );
        Variable *return_var = add_variable( funcall->fun_type->id + std::string("_return_value"), funcall->getType() );
        add_stmt( new AST::VariableDeclaration( return_var ) );
        static unsigned int label_counter = 0;
        std::stringstream label_name;
        label_name << "label_" << ++label_counter;
        Replace_return_stmt_with_assignment replace( return_var, label_name.str() );
        new_body->accept( &replace );
        new_body->accept( collect_variables );
        AST::StatementContainer* stmts = new_body->statements->statements;
        for( StatementContainer::iterator it = stmts->begin(); it != stmts->end(); ++it ) {
            add_stmt( *it );
        }
        //add_stmt( new_body );
        if( replace.used_goto )
            add_stmt( new AST::PlainText( label_name.str() + ":" ) );
        Transformation_visitor::push( new AST::IdentifierExpression( return_var ) );
    } else
        Transformation_visitor::push( funcall );
}

void
Beautify_visitor::visit( AST::StatementList* stmt_list ) {
    AST::StatementContainer* stmts = stmt_list->statements;
    StatementContainer::iterator
        it      = stmts->begin(),
        it_succ = stmts->begin();
    if( it_succ != stmts->end() )
        ++it_succ;
    for( ; it != stmts->end() && it_succ != stmts->end(); ) {
        // language level datatype pattern matching would be very helpful here:
        AST::VariableDeclaration *vardecl = dynamic_cast< AST::VariableDeclaration * >( *it );
        AST::ExpressionStatement *expr_stmt = dynamic_cast< AST::ExpressionStatement * >( *it_succ );
        if( expr_stmt && vardecl ) {
            AST::AssignmentExpression *assign_expr = dynamic_cast< AST::AssignmentExpression* >( expr_stmt->e );
            if( assign_expr ) {
                AST::IdentifierExpression *id_expr = dynamic_cast< AST::IdentifierExpression* >( assign_expr->e1 );
                if( id_expr && id_expr->var == vardecl->var ) {
                    vardecl->initializer = assign_expr->e2;
                    it = stmts->erase( it_succ );
                    it_succ = it;
                    if( it_succ != stmts->end() )
                        ++it_succ;
                    continue;
                }
            }
        }
        ++it;
        ++it_succ;
    }
}
