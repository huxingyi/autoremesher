#include <cassert>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <map>
#include <limits.h>

#include <FPG/AST.h>
#include <FPG/Error.h>
#include <FPG/Symbol.h>
#include <FPG/SymbolEnvironment.h>
#include <FPG/Abstract_interpretation_visitor.h>

//#define DEBUG
#include <FPG/MSG.h>

Abstract_interpretation_visitor::Abstract_interpretation_visitor( Abstract_value* initial_value )
 : expression_value( nullptr ), initial_value( initial_value )
{}

void
Abstract_interpretation_visitor::visit( AST::LiteralExpression* lexp ) {
    argused(lexp);
    //assert( lexp->is_float() );
    expression_value = new_abstract_value();
}

void
Abstract_interpretation_visitor::visit( AST::IdentifierExpression* iexp ) {
    if( is_float(iexp->var->type) ) {
        //if( !is_defined_in_env( iexp->var ) )
        //    std::cerr << iexp->var->id << std::endl;
        assert( is_defined_in_env( iexp->var ) );
        MSG( "var \"" << iexp->var->id << "\" has value \"" << value_env()[ iexp->var ] << "\"" )
        Abstract_value *v = value_env()[ iexp->var ];
        v->idexp( iexp );
        assert( v );
        expression_value = v;
    } else if( is_int(iexp->var->type) ) {
        expression_value = new_abstract_value();
    }
}

void
Abstract_interpretation_visitor::visit( AST::VariableDeclaration* vardecl ) {
    // this is needed for use_as_max computation to work:
    assert( value_env_stack.size() > 0 );
    value_env()[ vardecl->var ] = new_abstract_value();
}

void
Abstract_interpretation_visitor::visit( AST::UnaryExpression* uexp ) {
    MSG( "" )
    analyze( uexp->e );
    //expression_value = nullptr;
}

void
Abstract_interpretation_visitor::visit( AST::BinaryExpression* bexp) {
    MSG("")
    Abstract_value *value1 = analyze( bexp->e1 );
    Abstract_value *value2 = analyze( bexp->e2 );
    if( !is_float( bexp->e1->getType() ) && !is_float( bexp->e2->getType() ) ) {
        MSG("binexp of two non-floats. ignoring")
        expression_value = new_abstract_value();
        return;
    }
    assert( value1 != nullptr );
    assert( value2 != nullptr );
    switch( bexp->kind ) {
    case AST::BinaryExpression::ADD:
        expression_value = value1->add( value2, bexp );
        break;
    case AST::BinaryExpression::SUB:
        expression_value = value1->sub( value2, bexp );
        break;
    case AST::BinaryExpression::MUL:
        expression_value = value1->mul( value2, bexp );
        break;
    case AST::BinaryExpression::DIV:
        expression_value = value1->div( value2, bexp );
        break;
    case AST::BinaryExpression::EQ:
    case AST::BinaryExpression::GEQ:
    case AST::BinaryExpression::LEQ:
    case AST::BinaryExpression::NEQ:
    case AST::BinaryExpression::LE:
    case AST::BinaryExpression::GR:
        //expression_value->dump( std::cout );
        // result of comparison is an integer, so we start from scratch:
        expression_value = new_abstract_value();
        // the value probably wont be reused, anyway
        break;
    default:
        throw RuntimeError( "binary operation " + AST::BinaryExpression::operators[bexp->kind] + " unsupported", bexp->location );
    }
    //bexp->dump(0);
    MSG(" result: " << expression_value )
    MSG(" e1:     " << get_analysis_result( bexp->e1 ) )
    MSG(" e2:     " << get_analysis_result( bexp->e2 ) )
}

void
Abstract_interpretation_visitor::visit( AST::ConditionalExpression* cexp ) {
    analyze(cexp->cond);
    Abstract_value *value1 = analyze(cexp->e1);
    Abstract_value *value2 = analyze(cexp->e2);
    if( is_float(cexp->getType() ) ) {
        //std::cout << "join: ";
        //cexp->dump(0);
        value1->dump(std::cout);
        value2->dump(std::cout);
        //std::cout << cexp->getType()->id << std::endl;
        expression_value = value1->join( value2 );
    }
}

void
Abstract_interpretation_visitor::visit( AST::AssignmentExpression* aexp ) {
    // expression_value is passed upwards unchanged
    AST::IdentifierExpression *id_expr = dynamic_cast< AST::IdentifierExpression* >( aexp->e1 );
    if( id_expr != nullptr ) {
        Variable *var = id_expr->var;
        Abstract_value *val = analyze( aexp->e2 );
        if( is_float(var->type) ) {
            //if( is_defined_in_env( var ) )
            //    delete value_env()[ var ];
            value_env()[ var ] = val;
            MSG( "var \"" << var->id << "\" := \"" << value_env()[ var ] << "\"" )
            val->assign( aexp );
        }
    }
}

void
Abstract_interpretation_visitor::visit( AST::FunctionCall* funcall ) {
    MSG( "analyzing function call to  " << funcall->fun_type->id )
    std::vector< Abstract_value * > abstract_arguments;
    AST::ExpressionList::iterator arg_iter;
    FunctionType::ParameterList::iterator param_iter;
    // analyze actual arguments
    arg_iter   = funcall->exp_list->begin();
    param_iter = funcall->fun_type->parameters.begin();
    // NOTE: funcall arguments need to be evaluated inside the old/current
    // value environment!
    for( ; arg_iter != funcall->exp_list->end(); ++arg_iter, ++param_iter ) {
        AST::Expression *exp = *arg_iter;
        Variable *var = *param_iter;
        Abstract_value *av = analyze( exp );
        av->funcall( exp );
        if( is_float(var->type) ) {
            // if fundef is an external function
            if( funcall->fun_type->is_extern && is_float(var->type) && !av->is_fresh() ) {
                funcall->dump(0);
                throw RuntimeError("external functions cannot accept derived floating point values", funcall->location );
            }
            abstract_arguments.push_back( av );
        }
    }
    // for non-external functions:
    if( !funcall->fun_type->is_extern ) {
        AST::FunctionDefinition *fun_def = funcall->called_function;
        assert( fun_def != nullptr );
        // now it's safe to create the new environment:
        value_env_stack.push( Value_environment() );
        std::vector< Abstract_value * >::iterator abs_arg_iter = abstract_arguments.begin();
        param_iter = funcall->fun_type->parameters.begin();
        for( ; param_iter != funcall->fun_type->parameters.end(); ++param_iter ) {
            Variable *var = *param_iter;
            if( is_float(var->type) ) {
                assert( abs_arg_iter != abstract_arguments.end() );
                Abstract_value *a = *abs_arg_iter;
                assert( a );
                value_env()[ var ] = a;
                MSG( "  setting " << var->id << " := " << a )
                ++abs_arg_iter;
            }
        }
        return_values.push( nullptr );
        analyze( fun_def->body );
#if 0
        // NOTE: this is an ugly hack! in particular, if there was an assignment to one of the
        //       parameters, this should _not_ change the arguments outside the function call!
        //       with this approach, however, it does:
        // propagate back any changes made to the arguments' abstract values.
        // this is important for determining the max variables in Group_index_value!
        arg_iter   = funcall->exp_list->begin();
        param_iter = funcall->fun_type->parameters.begin();
        for( ; arg_iter != funcall->exp_list->end(); ++arg_iter, ++param_iter ) {
            AST::Expression *exp = *arg_iter;
            Variable *var = *param_iter;
            if( is_float(var->type) ) {
                analysis_result[ exp ] = value_env()[ var ];
                //exp->dump(0);
                MSG( "propagating back from " << var->id << " with value " << value_env()[ var ] )
            }
        }
#endif
        value_env_stack.pop();
        // keep abstract return value
        expression_value = return_values.top();
        return_values.pop();
        if( funcall->fun_type->getReturnType() != type_void ) {
            assert( expression_value != nullptr );
            MSG( "return value=" << expression_value )
        }
    } else
        // for external function, assume a fresh abstract value
        expression_value = new_abstract_value();
}

void
Abstract_interpretation_visitor::visit( AST::UnaryFunction* uf ) {
    analyze(uf->e);
    switch( uf->kind ) {
    case AST::UnaryFunction::XSIGN :
        /*std::cout << "xxxxxxxxxxxxxxxxx aftersign xxxxxxxxxxxxxxx "; expression_value->dump( std::cout ); */
        expression_value = new_abstract_value();
        break;
    case AST::UnaryFunction::XSQRT : expression_value = expression_value->sqrt(); break;
    case AST::UnaryFunction::XABS  : break; // leave result on stack
    default: ;
    }
}


void
Abstract_interpretation_visitor::visit( AST::EmptyStatement* ) {
    // an emptystatement does not depend on anything
    assert( expression_value == nullptr );
}

void
Abstract_interpretation_visitor::visit( AST::ExpressionStatement* expr_stmt ) {
    analyze( expr_stmt->e );
    expression_value = nullptr;
}

void
Abstract_interpretation_visitor::visit( AST::ConditionalStatement* stmt ) {
    analyze( stmt->cond );
    // store topmost value_env
    Value_environment other_env = value_env();
    analyze( stmt->then_branch );
    if( stmt->else_branch ) {
        value_env_stack.push( other_env );
        analyze( stmt->else_branch );
        merge_env();
    }
}

#if 0
int foo( float a, float b ) {
  if( cond ) {
      return bar( ... )
  } else {
      return ...
  }
}

int bar( float c, float d ) {
    if( cond2 ) {
        if( cond3 )
            return ..;
        else
            return ...;
    } else
        return ...;
}
#endif

void
Abstract_interpretation_visitor::visit( AST::Return* ret) {
    MSG("");
    if( ret->e ) {
        analyze(ret->e);
        assert( return_values.size() > 0 );
        if( return_values.top() == nullptr ) {
            MSG( "setting retval=" << expression_value )
            return_values.top() = expression_value;
        } else  if( is_float( ret->e->getType() ) && return_values.top() != expression_value ) {
            MSG( "joining " << return_values.top() << " and " << expression_value )
            return_values.top()->join( expression_value );
            MSG( "result: " << return_values.top() )
        }
    }
    expression_value = nullptr;
}

void
Abstract_interpretation_visitor::visit( AST::StatementList* slist ) {
    for( AST::StatementContainer::iterator it = slist->statements->begin(); it != slist->statements->end(); ++it ) {
        AST::Statement *stmt = *it;
        analyze(stmt);
        // statements have no expression value
        assert( expression_value == nullptr );
    }
}

void
Abstract_interpretation_visitor::visit( AST::CompoundStatement* compound ) {
    analyze(compound->statements);
}

void
Abstract_interpretation_visitor::visit( AST::FunctionDefinition* fundef ) {
    MSG( fundef->type->id )
    value_env_stack.push( Value_environment() );
    return_values.push( nullptr );
    FunctionType::ParameterList::iterator it;
    for( it = fundef->type->parameters.begin(); it != fundef->type->parameters.end(); ++it ) {
        Variable *var = *it;
        if( is_float(var->type) ) {
            Abstract_value *value;
            try {
                value = initial_value->get_initial_value( var );
            } catch( RuntimeError &e ) {
                throw RuntimeError( e.msg, fundef->location );
            }

            value_env()[ var ] = value;
            MSG( "init var \"" << var->id << "\"" )
        }
    }
    analyze( fundef->body );
    value_env_stack.pop();
    // toplevel return values are ignored, they are most probably of integer type anyway.
    return_values.pop();
    assert( return_values.size() == 0 );
    assert( expression_value == nullptr );
}

void
Abstract_interpretation_visitor::visit( AST::TranslationUnit* tu ) {
    assert( value_env_stack.size() == 0 );
    AST::ListOfFunctions::iterator it;
    for( it = tu->functions.begin(); it != tu->functions.end(); ++it )
        analyze( *it );
    assert( value_env_stack.size() == 0 );
}

Abstract_value*
Abstract_interpretation_visitor::get_analysis_result( AST::Expression *e ) {
    if( is_float(e->getType()) ) {
        /*if( analysis_result.find(e) == analysis_result.end() ) {
            e->dump(0);
            std::cout << e->location << std::endl;
            std::cout << "xxxxxx use " << (unsigned int)e << std::endl;
        }*/
        assert( analysis_result.find(e) != analysis_result.end() );
        Abstract_value *val = analysis_result[ e ];
        assert( val != nullptr );
        return val;
    } else if( is_int(e->getType()) )
        return new_abstract_value();
    else
        throw InternalError( "can only report abstract values for int and float" );
}


void
Abstract_interpretation_visitor::update( AST::Expression *old_e, AST::Expression *new_e ) {
    assert( old_e != nullptr );
    assert( new_e != nullptr );
    if( is_float(old_e->getType()) ) {
        assert( is_float(new_e->getType()) );
        //old_e->dump(0);
        //std::cout << old_e->location << std::endl;
        assert( analysis_result.find(old_e) != analysis_result.end() );
        assert( analysis_result.find(new_e) == analysis_result.end() );
        //std::cout << "xxxxxx create " << (unsigned int)new_e << std::endl;
        analysis_result[ new_e ] = analysis_result[ old_e ];
    }
}

Abstract_value*
Abstract_interpretation_visitor::new_abstract_value() {
    return initial_value->clone();
}


Abstract_interpretation_visitor::Value_environment&
Abstract_interpretation_visitor::value_env() {
    assert( value_env_stack.size() >= 1 );
    return value_env_stack.top();
}

void
Abstract_interpretation_visitor::merge_env() {
    // expect the input on top of stack
    assert( value_env_stack.size() >= 2 );
    Value_environment::iterator it, it2;
    // not terribly efficient, but who cares ..
    Value_environment other_env = value_env_stack.top();
    value_env_stack.pop();
    // for each variable in env ...
    for( it = value_env().begin(); it != value_env().end(); ++it ) {
        // .. look for corresponding entries in other environment
        it2 = other_env.find( it->first );
        // ignore variables that are unique to one of the environments
        if( it2 != other_env.end() ) {
            Variable *v1 = it->first;
            if( is_float(v1->type) ) {
                Abstract_value *a1 = it->second;
                Abstract_value *a2 = it2->second;
                // perform in-place join operation on abstract value
                if( a1 != a2 )
                    a1->join( a2 );
            }
        }
    }
    // now, the topmost environment stores the result of merge
}

bool
Abstract_interpretation_visitor::is_defined_in_env( Variable* var ) {
    return value_env().find( var ) != value_env().end();
}

Abstract_value*
Abstract_interpretation_visitor::analyze( AST::Node* n ) {
    expression_value = nullptr;
    //std::cout << "xxxxxx analyze " << (unsigned int)n << std::endl;
    if( !is_bound( n->getType() ) ) {
        //std::cout << "xxxxxx accept " << (unsigned int)n << std::endl;
        n->accept( this );
        if( is_float(n->getType()) && expression_value != nullptr ) {
            AST::Expression *e = dynamic_cast<AST::Expression*>(n);
            assert( e );
            //std::cout << "xxxxxx create " << (unsigned int)e << std::endl;
            analysis_result[ e ] = expression_value;
        }
    }
    return expression_value != nullptr ? expression_value
                                    : new_abstract_value();
}
