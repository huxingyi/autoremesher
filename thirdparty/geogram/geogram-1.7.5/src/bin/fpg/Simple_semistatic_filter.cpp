#include <FPG/Simple_semistatic_filter.h>
#include <FPG/Misc_visitors.h>
#include <FPG/Abstract_interpretation_visitor.h>
#include <FPG/Error_bound_value.h>
#include <FPG/Transformation.h>
#include <FPG/Prettyprint_visitor.h>

#include <cassert>
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>

//#define DEBUG 0
#include <FPG/MSG.h>


/*struct Prune_fabs_subtrees : public Abstract_value {
    Prune_fabs_subtrees() : phase(0) {}

    virtual Abstract_value* add( Abstract_value* other )
    {
        Prune_fabs_subtrees *o = downcast( other );
        Prune_fabs_subtrees *result = new Prune_fabs_subtrees();
        if( o->phase == 0 && this->phase == 0 )
            result->phase = 1;
        else
            result->phase = 2;
    }

    virtual Abstract_value* sub( Abstract_value* other )
    { return add( other ); }

    virtual Abstract_value* mul( Abstract_value* other ) {
        Prune_fabs_subtrees *o = downcast( other );
        Prune_fabs_subtrees *result = new Prune_fabs_subtrees();
        if( o->phase <= 1 && this->phase <= 1 )
            result->phase = 2;
        else
            result->phase = std::max( o->phase, this->phase );
    }

    // callbacks, needed for Group_index_value:
    virtual void idexp( AST::IdentifierExpression* ) {}
    virtual void assign( AST::AssignmentExpression* ) {}
    // expression is used as argument in funcall
    virtual void funcall( AST::Expression* ) {}

    virtual Abstract_value* join( Abstract_value* other ) = 0;

    virtual Abstract_value* clone()
    { return new Prune_fabs_subtrees( *this ); }

    virtual bool is_fresh() {
        return phase == 0;;
    }

    virtual std::ostream& dump( std::ostream& out ) {
        out << "phase: " << phase;
    }

    int phase;
};*/



using namespace Simple_semistatic_filter;

// --------------------------------------------------------------

void
Add_bound_variables::visit( AST::IdentifierExpression* idexp ) {
    if( expression_depth > 0 && !has_bound_variable( idexp->var ) && is_float(idexp->var->type) ) {
        Variable *new_var = add_bound( idexp->var );
        MSG("added bound: " << idexp->var->id << " " << new_var->id )
        add_stmt_toplevel( new AST::VariableDeclaration( new_var ) );
        add_stmt_toplevel( make_assign_stmt( new_var, new AST::UnaryFunction( idexp, AST::UnaryFunction::XABS ) ) );
    }
}

void
Add_bound_variables::visit( AST::BinaryExpression* bexp ) {
    if( is_float( bexp->e1->getType() ) ||
        is_float( bexp->e2->getType() )    )
    {
        switch(bexp->kind) {
        case AST::BinaryExpression::EQ:
        case AST::BinaryExpression::GEQ:
        case AST::BinaryExpression::LEQ:
        case AST::BinaryExpression::NEQ:
        case AST::BinaryExpression::LE:
        case AST::BinaryExpression::GR:
        case AST::BinaryExpression::AND:
        case AST::BinaryExpression::OR:
            Generic_visitor::visit( bexp );
            add_tmp_result( bexp );
            break;
        default:
            ++expression_depth;
            Generic_visitor::visit( bexp );
            --expression_depth;
        }
    } else
        Generic_visitor::visit( bexp );
}

void
Add_bound_variables::visit( AST::UnaryFunction* uf ) {
    ++expression_depth;
    Generic_visitor::visit( uf );
    --expression_depth;
    // SQRT and SIGN need special treatment in Rewrite_visitor
    if( uf->kind == AST::UnaryFunction::XSIGN ||
        uf->kind == AST::UnaryFunction::XSQRT    )
    {
        add_tmp_result( uf );
    }
}

void
Add_bound_variables::visit( AST::FunctionDefinition* fundef ) {
    MSG( fundef->type->id )
    assert( fundef != nullptr );
    assert( compute_call_count != nullptr );
    // those functions that are not called are regarded as "interface" functions.
    // consider float values as exact at function entry, so we dont need a bound here.
    // also, return_value is not used.

    assert( compute_call_count->callcount[ fundef ] == 0 );
    if( is_float(fundef->type->return_type) ) {
        std::cerr << "ignoring function "
                    << fundef->type->id << std::endl;
    } else
        Statement_addition_visitor::visit( fundef );
}

void
Add_bound_variables::visit( AST::VariableDeclaration* vardecl ) {
    assert( vardecl != nullptr );
    if( is_float(vardecl->var->type) ) {
        Variable *new_var = add_bound( vardecl->var );
        Statement_addition_visitor::add_stmt( new AST::VariableDeclaration( new_var ) );
    }
}

void
Add_bound_variables::visit( AST::Return *ret ) {
    Generic_visitor::visit( ret );
}

struct Always_true_expression_filter : public Expression_filter {
    virtual bool operator()( AST::Expression *e ) { argused(e); return true; }
};

void
Add_bound_variables::visit( AST::TranslationUnit* tu ) {
    Generic_visitor::visit( tu );
}



// ---------------------------------------------

Add_bound_computation::Add_bound_computation()
  : bound_map ( new Bound_map ),
    add_bounds( new Add_bound_variables      ),
    absint    ( new Abstract_interpretation_visitor( new Error_bound_value ) ),
    should_be_filtered( new Label_filtered_functions )
{}

void
Add_bound_computation::visit( AST::LiteralExpression* lexp ) {
    if( is_float(lexp->getType()) ) {
        lexp = dynamic_cast<AST::LiteralExpression*>( AST::clone( lexp ) );
        lexp->val.f = std::fabs( lexp->val.f );
        push_bound( lexp );
    } else if( is_int(lexp->getType()) ) {
        lexp = dynamic_cast<AST::LiteralExpression*>( AST::clone( lexp ) );
        lexp->val.i = std::abs( lexp->val.i );
        push_bound( lexp );
    } else {
        MSG("no push")
    }
}

void
Add_bound_computation::visit( AST::IdentifierExpression* idexp ) {
    if( is_float(idexp->getType()) ) {
        if( add_bounds->has_bound_variable( idexp->var ) )
            push_bound( new AST::IdentifierExpression( add_bounds->bound_variable( idexp->var ) ) );
        else
            push_bound( new AST::PlainTextExpression("NO_BOUND_NEEDED", type_float) );
    } else if( is_int(idexp->getType()) ) {
        push_bound( new AST::UnaryFunction( new AST::IdentifierExpression( idexp->var ), AST::UnaryFunction::XABS ) );
    }
}

void
Add_bound_computation::visit( AST::UnaryExpression* uexp ) {
    MSG("")
    Generic_visitor::visit( uexp );
    if( !is_float_or_int( uexp->e->getType() ) )
        return;
    assert( bound_expressions.size() > 0 );
    AST::Expression *bound = bound_expression( uexp->e );
    store_bound( uexp->e, bound );
    // leave bound unchanged (changing the sign of "e" does not change its bound)
    push_bound( bound );
}

static AST::Expression* add_min_double( AST::Expression *e ) {
    return e;
    /*return
        new AST::BinaryExpression(
            e,
            new AST::LiteralExpression( std::numeric_limits<double>::min() ),
            AST::BinaryExpression::ADD
        );*/
}

void
Add_bound_computation::visit( AST::BinaryExpression* bexp ) {
    MSG("")
    Generic_visitor::visit( bexp );
    if( !is_float_or_int( bexp->e1->getType() ) &&
        !is_float_or_int( bexp->e2->getType() )   )
        return;

    assert( bound_expressions.size() > 1 );
    // order is important here:
    AST::Expression *bound_2 = bound_expression( bexp->e2 );
    AST::Expression *bound_1 = bound_expression( bexp->e1 );
    AST::Expression *result_bound = nullptr;
    store_bound( bexp->e1, bound_1 );
    store_bound( bexp->e2, bound_2 );
    switch( bexp->kind ) {
    case AST::BinaryExpression::ADD:
    case AST::BinaryExpression::SUB:
        result_bound =
            new AST::BinaryExpression( bound_1, bound_2,
                                       AST::BinaryExpression::ADD );
        break;
    case AST::BinaryExpression::MUL:
        result_bound =
            add_min_double(
                new AST::BinaryExpression( bound_1, bound_2,
                                           AST::BinaryExpression::MUL )
            );
        break;
    case AST::BinaryExpression::DIV: {
        AST::Expression *num =
            new AST::BinaryExpression(
                new AST::BinaryExpression(
                    new AST::UnaryFunction( bexp->e1, AST::UnaryFunction::XABS ),
                    new AST::UnaryFunction( bexp->e2, AST::UnaryFunction::XABS ),
                    AST::BinaryExpression::DIV
                ),
                new AST::BinaryExpression(
                    bound_1,
                    bound_2,
                    AST::BinaryExpression::DIV
                ),
                AST::BinaryExpression::ADD
            );
        Error_bound_value *err_val = dynamic_cast<Error_bound_value*>(absint->get_analysis_result( bexp->e2 ) );
        assert( err_val != nullptr );
        AST::Expression *denom =
            new AST::BinaryExpression(
                new AST::BinaryExpression(
                    new AST::UnaryFunction( bexp->e2, AST::UnaryFunction::XABS ),
                    bound_2,
                    AST::BinaryExpression::DIV
                ),
                new AST::LiteralExpression( 67108864.0*( err_val->index + 1 ) ),
                AST::BinaryExpression::SUB
            );
        result_bound =
            add_min_double(
                new AST::BinaryExpression( num, denom, AST::BinaryExpression::MUL )
            );
        }
        break;
    case AST::BinaryExpression::EQ:
    case AST::BinaryExpression::GEQ:
    case AST::BinaryExpression::LEQ:
    case AST::BinaryExpression::NEQ:
    case AST::BinaryExpression::LE:
    case AST::BinaryExpression::GR:
    case AST::BinaryExpression::AND:
    case AST::BinaryExpression::OR:
        result_bound = new AST::LiteralExpression(1);
        break;
    default:
        throw InternalError( "case not handled", bexp->location );
        ;
    }
    assert( result_bound != nullptr );
    push_bound( result_bound );
}

void
Add_bound_computation::visit( AST::ConditionalExpression* cexp ) {
    MSG("")
    Generic_visitor::visit( cexp );
    AST::Expression *bound_e2 = bound_expression( cexp->e2 );
    AST::Expression *bound_e1 = bound_expression( cexp->e1 );
    pop_bound(); // ignore condition
    push_bound( new AST::ConditionalExpression( cexp->cond, bound_e1, bound_e2 ) );
}


void
Add_bound_computation::visit( AST::ConditionalStatement* cstmt ) {
    MSG("")
    handle( cstmt->cond );
    // ignore bound:
    pop_bound();
    assert( bound_expressions.size() == 0 );
    handle( cstmt->then_branch );
    if( cstmt->else_branch != nullptr )
        handle( cstmt->else_branch );
}

void
Add_bound_computation::visit( AST::AssignmentExpression* aexp ) {
    MSG("")
    Generic_visitor::visit( aexp );
    AST::IdentifierExpression *id_expr = dynamic_cast< AST::IdentifierExpression* >( aexp->e1 );
    assert( id_expr );
    if( is_float(aexp->getType()) ) {
        //id_expr->dump(0);
        // at this level, funcall_depth == 0, so we dont need to store_bound
        // add bound stmt _after_ the current expression, to allow function calls
        // enough time to set their bound. otherwise, we would get uninitialized variables
        bool add_before = ! contains_function_call( aexp->e2 );
        AST::Expression *bound_expr = bound_expression( aexp->e2 );
        add_stmt( make_assign_stmt( bound_variable( id_expr->var ),
                                    bound_expr ), add_before );
        push_bound( new AST::LiteralExpression(0) ); // just to conforn to our invariant
    } else if( is_int( aexp->getType() ) ) {
        //std::cout << "is int: " << id_expr->var->id << std::endl;
        store_bound( aexp->e2, nullptr );
    } else if( !is_float_or_int(aexp->getType()) && is_float_or_int( aexp->e2->getType() ) )
        pop_bound();
}


void
Add_bound_computation::visit( AST::UnaryFunction *uf ) {
    MSG("")
    Generic_visitor::visit( uf );
    AST::Expression *bound_e = bound_expression( uf->e );
    AST::Expression *result_bound = bound_e;
    // both SQRT and SIGN need bounds for e, so we need to split
   // the expression tree here, using
    switch( uf->kind ) {
    case AST::UnaryFunction::XSIGN:
    case AST::UnaryFunction::XSQRT: MSG("xxx"); store_bound( uf->e, bound_e ); break;
    default: ;
    }
    AST::Expression *gtzero = nullptr, *eqzero = nullptr;
    switch( uf->kind ) {
    case AST::UnaryFunction::XSIGN:
        result_bound = new AST::LiteralExpression(1); break;
    case AST::UnaryFunction::XSQRT:
        gtzero =
            new AST::BinaryExpression(
                new AST::BinaryExpression(bound_e,
                                          uf->e,
                                          AST::BinaryExpression::DIV
                ),
                new AST::UnaryFunction( uf->e, AST::UnaryFunction::XSQRT ),
                AST::BinaryExpression::MUL
            );
        eqzero =
            new AST::BinaryExpression(
                new AST::UnaryFunction( bound_e, AST::UnaryFunction::XSQRT ),
                new AST::LiteralExpression( 67108864.0 ), /* 2^(52/2) */
                AST::BinaryExpression::MUL
            );
        // bound = (uf->e > 0.0) ? (bound_e/uf->e)*sqrt(uf->e)
        //                       : sqrt(bound_e)*2^(p/2)
        result_bound =
            new AST::ConditionalExpression(
                new AST::BinaryExpression(
                    uf->e,
                    new AST::LiteralExpression( 0.0 ),
                    AST::BinaryExpression::GR
                ),
                gtzero,
                eqzero
            );
        break;
    default: ;
    }
    push_bound( result_bound );
}

void
Add_bound_computation::visit( AST::FunctionCall* funcall ) {
    // only extern funcalls should remain after funcall substitution:
    assert( funcall->fun_type->is_extern );
    if( is_float_or_int( funcall->getType() ) ) {
        // just to have something.
        push_bound( new AST::LiteralExpression(1) );
    }
}

void
Add_bound_computation::visit( AST::ExpressionStatement* estmt ) {
    // at this level, funcall_depth == 0, so we dont need to store_bound
    Generic_visitor::visit( estmt );
    if( is_float_or_int( estmt->e->getType() ) ) {
        MSG("popping")
        /* AST::Expression *e = */ pop_bound();
        // TODO: memory management
        //delete e;
    } else {
        MSG( estmt->e->getType()->id )
    }
    if( bound_expressions.size() != 0 ) {
        estmt->dump(0);
    }
    assert( bound_expressions.size() == 0 );
}

void
Add_bound_computation::visit( AST::Return* ret ) {
    MSG("")
    Generic_visitor::visit( ret );
    if( has_result_bound_variable() ) {
        AST::Expression *bound_expr = bound_expression( ret->e );
        store_bound( ret->e, bound_expr );
        Variable *bound_var = result_bound_variable();
        add_stmt( make_assign_stmt( bound_var, bound_expr ) );
        // at this level, funcall_count == 0, so we dont need to store_bound
    } else if( is_float_or_int( ret->getType() ) ) {
        pop_bound();
        MSG( "size : " << bound_expressions.size() )
    } else
        MSG( "do nothing" )

    // FIXME: side_of_bounded_sphere triggered this assertion:
    assert( bound_expressions.size() == 0 );
}

void
Add_bound_computation::visit( AST::FunctionDefinition* fundef ) {
    if( /* TODO: is called by a function which contains floatcomp */true  ) {
        MSG( fundef->type->id )
        fundef_stack.push( fundef );
        Generic_visitor::visit( fundef );
        fundef_stack.pop();
    } else {
        MSG( "skipping " << fundef->type->id )
    }
}

void
Add_bound_computation::visit( AST::TranslationUnit* tu ) {
    should_be_filtered->clear();
    tu->accept( should_be_filtered );

    Compute_call_count        compute_call_count;
    tu->accept( &compute_call_count );
    AST::ListOfFunctions::iterator it;
    AST::ListOfFunctions::reverse_iterator   r_it;
    Rewrite_float_comparisons rewrite( this );
    //translation_unit->accept( add_bound_computation.absint );

    Always_true_expression_filter  filter;
    Substitute_funcalls            subst_funcalls( &filter );
    subst_funcalls.collect_variables = add_bounds->collect_variables;

    for( r_it = tu->functions.rbegin(); r_it != tu->functions.rend(); ++r_it ) {
        AST::FunctionDefinition *fundef = *r_it;
        if( should_be_filtered->contains( fundef ) &&
            !fundef->type->is_extern )
        {
            //std::cerr << "substituting function calls in " << fundef->type->id << std::endl;
            // subst'ion works one funcall at a time, starting from the "outside":
            // function calls that are made in the called function have to be replaced
            // subsequently. repeat until no change has been made
            subst_funcalls.collect_variables->clear();
            fundef->accept( subst_funcalls.collect_variables );
            do {
                subst_funcalls.is_dirty = false;
                fundef->accept( &subst_funcalls );
            } while( subst_funcalls.is_dirty );
        }
    }
    for( it = tu->functions.begin(); it != tu->functions.end(); ++it ) {
        AST::FunctionDefinition *fundef = *it;
        //std::cerr << "checking " << fundef->type->id << std::endl;
        if( should_be_filtered->contains( fundef ) &&
            !fundef->type->is_extern )
        {
            std::cerr << "analyzing " << fundef->type->id << std::endl;
            //Prettyprint_visitor pretty( std::cout );
            //fundef->accept( &pretty );
            fundef->accept( absint );
            fundef->accept( add_bounds );
            handle( fundef );
            fundef->accept( &rewrite );
        }
    }

}

void
Add_bound_computation::store_bound( AST::Expression *e, AST::Expression *bound ) {
    assert( e != nullptr );
    if( bound != nullptr ) {
        (*bound_map)[ e ] = bound;
    }
}

Variable*
Add_bound_computation::result_bound_variable() {
    assert( fundef_stack.size() > 0 );
    Variable *var = add_bounds->bound_variable( fundef_stack.top() );
    assert( var != nullptr );
    return var;
}

bool
Add_bound_computation::has_result_bound_variable() {
    return add_bounds->has_bound_variable( fundef_stack.top() );
}

Variable*
Add_bound_computation::bound_variable( Variable *var ) {
    assert( add_bounds != nullptr );
    assert( var != nullptr );
    return add_bounds->bound_variable( var );
}

Variable*
Add_bound_computation::bound_variable( AST::FunctionCall *funcall ) {
    assert( add_bounds != nullptr );
    assert( funcall != nullptr );
    return add_bounds->bound_variable( funcall );
}

void
Add_bound_computation::push_bound( AST::Expression *e ) {
    assert( e != nullptr );
    bound_expressions.push( e );
    MSG( "size: " << bound_expressions.size() )
    //e->dump(10);
}

AST::Expression*
Add_bound_computation::pop_bound() {
    assert( bound_expressions.size() > 0 );
    AST::Expression *e = bound_expressions.top();
    bound_expressions.pop();
    assert( e != nullptr );
    MSG( "size: " << bound_expressions.size() )
    //e->dump(10);
    return e;
}

AST::Expression*
Add_bound_computation::bound_expression( AST::Expression *e ) {
    if( is_float_or_int( e->getType() ) ) {
        return pop_bound();
    } else
        throw RuntimeError( "cannot get bound for type " + e->getType()->name() );
    // for the compiler:
    return nullptr;
}

// ------------------

void
Rewrite_float_comparisons::visit( AST::BinaryExpression *bexp ) {
    MSG("")
    Transformation_visitor::transform<AST::Expression>( bexp->e1 );
    Transformation_visitor::transform<AST::Expression>( bexp->e2 );
    AST::Expression *result_expression = bexp;
    switch( bexp->kind ) {
    case AST::BinaryExpression::LE:
    case AST::BinaryExpression::GR:
        // we have a floating point comparison, if at least one of
        // the expressions has type "float" ( which is equal to
        // is_float(bexp-getType()) )
        MSG(" rewriting ! " )
        //bexp->dump(0);
        if( is_float(bexp->e1->getType()) || is_float(bexp->e2->getType()) ) {
            Abstract_value *absval1 = add_bounds->absint->get_analysis_result( bexp->e1 );
            Abstract_value *absval2 = add_bounds->absint->get_analysis_result( bexp->e2 );
            Error_bound_value *err1 = dynamic_cast< Error_bound_value* >(absval1);
            Error_bound_value *err2 = dynamic_cast< Error_bound_value* >(absval2);
            // floating point comparison on input values is exact, no need for
            // filters:
            if( err1->index == 0 && err2->index == 0 )
                break;
            if( bexp->kind == AST::BinaryExpression::GR ) {
                std::swap( bexp->e1, bexp->e2 );
                std::swap( absval1, absval2 );
                bexp->kind = AST::BinaryExpression::LE;
            }
            AST::Expression *e1 = bexp->e1;
            AST::Expression *e2 = bexp->e2;
            AST::Expression *bound_1 = bound_expression( e1 );
            AST::Expression *bound_2 = bound_expression( e2 );

            /*Variable *cse_var;
            if( add_bounds->has_tmp_result_variable( e1 ) )
                cse_var = add_bounds->tmp_result_variable( e1 );
            else
                cse_var = add_bounds->tmp_result_variable( e2 );
            add_stmt(
                make_assign_stmt( cse_var, bexp );*/

            AST::Expression *abs_e =
                new AST::UnaryFunction(
                    new AST::BinaryExpression( e1, e2, AST::BinaryExpression::SUB ),
                    AST::UnaryFunction::XABS
                );
            AST::Expression *bound_e = new AST::BinaryExpression( bound_1, bound_2, AST::BinaryExpression::ADD );
            Error_bound_value *err_result = dynamic_cast< Error_bound_value* >( absval1->sub( absval2 ) );
            assert( err_result != nullptr );
            Variable *tmp_var = add_bounds->add_bounds->tmp_result_variable( bexp );
            add_stmt( new AST::VariableDeclaration( tmp_var ) );
            bound_e =
                new AST::BinaryExpression(
                    bound_e,
                    new AST::LiteralExpression(
                        std::numeric_limits<double>::epsilon()*err_result->index ),
                    AST::BinaryExpression::MUL
                );
            add_stmt(
                new AST::ConditionalStatement(
                    new AST::BinaryExpression(
                        new AST::BinaryExpression( abs_e, bound_e, AST::BinaryExpression::GR ),
                        make_fe_condition(),
                        AST::BinaryExpression::AND
                    ),
                    make_assign_stmt(tmp_var,bexp),
                    make_fe_clear_and_fail()
                    //new AST::Return( new AST::LiteralExpression(-999) )
                )
            );
            result_expression = new AST::IdentifierExpression( tmp_var );
        }
        break;
    case AST::BinaryExpression::EQ:
    case AST::BinaryExpression::GEQ:
    case AST::BinaryExpression::LEQ:
    case AST::BinaryExpression::NEQ:
        if( is_float( bexp->e1->getType() ) ||
            is_float( bexp->e2->getType() )   )
        {
            //bexp->dump(0);
            Error_bound_value *err1 = dynamic_cast< Error_bound_value* >(add_bounds->absint->get_analysis_result( bexp->e1 ));
            Error_bound_value *err2 = dynamic_cast< Error_bound_value* >(add_bounds->absint->get_analysis_result( bexp->e2 ));
            if( err1->index > 0 || err2->index > 0 )
                throw RuntimeError( "no tests for equality allowed, "
                                    "between derived floating point values!", bexp->location );
        }
        break;
    default:
        ;
    }
    Transformation_visitor::push( result_expression );
}


void
Rewrite_float_comparisons::visit( AST::UnaryFunction *uf ) {
    Transformation_visitor::transform<AST::Expression>( uf->e );

    if( uf->kind == AST::UnaryFunction::XSIGN && is_float(uf->e->getType()) ) {
        AST::Expression *abs_e = new AST::UnaryFunction( uf->e, AST::UnaryFunction::XABS );
        AST::Expression *bound_e = bound_expression( uf->e );
        Abstract_value   *absval = add_bounds->absint->get_analysis_result( uf->e );
        Error_bound_value *err_result = dynamic_cast< Error_bound_value* >( absval );
        assert( err_result != nullptr );
        if( err_result->index == 0 ) {
            Transformation_visitor::push( uf );
            return;
        }
        Variable *tmp_var = add_bounds->add_bounds->tmp_result_variable( uf );
        add_stmt( new AST::VariableDeclaration( tmp_var ) );
        AST::Expression *cond_e =
            new AST::BinaryExpression(
                uf->e,
                new AST::LiteralExpression( 0.0 ),
                AST::BinaryExpression::LE
            );

        bound_e =
            new AST::BinaryExpression(
                bound_e,
                new AST::LiteralExpression(
                    std::numeric_limits<double>::epsilon()*err_result->index ),
                AST::BinaryExpression::MUL
            );
        add_stmt(
            new AST::ConditionalStatement(
                new AST::BinaryExpression(
                    new AST::BinaryExpression( abs_e, bound_e, AST::BinaryExpression::GR ),
                    make_fe_condition(),
                    AST::BinaryExpression::AND
                ),
                make_assign_stmt(
                    tmp_var,
                    new AST::ConditionalExpression(
                        cond_e,
                        new AST::LiteralExpression( -1 ),
                        new AST::LiteralExpression( +1 )
                    )
                ),
                make_fe_clear_and_fail()
                //new AST::Return( new AST::LiteralExpression(-999) )
            )
        );
        Transformation_visitor::push( new AST::IdentifierExpression( tmp_var ) );
    } else
        Transformation_visitor::push( uf );
}

void
Rewrite_float_comparisons::visit( AST::AssignmentExpression *ae ) {
    // do not rewrite bound expressions, as they may contain sqrt/comparisons
    // themselves:
    if( ae->e1->getType()->isEqual( type_float_bound     ) ||
        ae->e1->getType()->isEqual( type_float_bound_ptr )    )
        push( ae );
    else
        Transformation_visitor::visit( ae );
}


void
Rewrite_float_comparisons::update( AST::Node *node_old, AST::Node *node_new ) {
    Transformation_visitor::update( node_old, node_new );
    AST::Expression *e_old = dynamic_cast< AST::Expression* >(node_old);
    AST::Expression *e_new = dynamic_cast< AST::Expression* >(node_new);
    if( e_old != nullptr )
        assert( e_new != nullptr );
    if( has_bound( e_old ) ) {
        AST::Expression *bound_old = bound_expression( e_old );
        (*add_bounds->bound_map)[ e_new ] = bound_old;
    }
}

bool
Rewrite_float_comparisons::has_bound( AST::Expression *e ) {
    assert( add_bounds->bound_map != nullptr );
    return add_bounds->bound_map->find( e ) != add_bounds->bound_map->end();
}

AST::Expression*
Rewrite_float_comparisons::bound_expression( AST::Expression *e ) {
    assert( has_bound( e ) );
    return (*add_bounds->bound_map)[ e ];
}

