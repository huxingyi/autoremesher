#include <cassert>
#include <iostream>
#include <sstream>

#include <FPG/AST.h>
#include <FPG/Error.h>


namespace AST {

void
requireConvertibleTypes( Type *target, Type *source, const Location& location ) {
    bool b = true;
    try {
        b = source->isConvertible( target );
    } catch( Warning &w ) {
        std::cerr << "warning: " << location.toString() << " " << w.msg << std::endl;
    }
    if( !b )
        throw TypeError( "type `" + source->name() + "` is not convertible to type `" + target->name() + "`", location );
}

Type*
Node::getType() {
    if( type )
        return type;
    else
        return type = computeType();
}

Type*
LiteralExpression::computeType() {
  return ( val.type == Concrete_value::INTCONST ) ? type_int : type_float;
}

Type*
IdentifierExpression::computeType() {
    return var->type;
}

Type*
UnaryExpression::computeType() {
    Type *base_type = e->getType();
    switch( kind ) {
    case REF:
        if( base_type == type_float_bound )
            return type_float_bound_ptr;
        else
            throw TypeError("ref operation only applicable to float bounds");
    case DEREF:
        if( base_type == type_float_bound_ptr )
            return type_float_bound;
        else
            throw TypeError("deref operation only applicable to float bound ptr");
    default:
        return base_type;
    }
}

Type*
BinaryExpression::computeType() {
#define TYPE_ERROR       throw TypeError(error_prefix + operators[kind]        , location);
#define TYPE_ERROR2(msg) throw TypeError(error_prefix + operators[kind]  + " : " + msg , location);
    Type *t1 = e1->getType();
    Type *t2 = e2->getType();
    const std::string error_prefix("invalid operands of types `" + t1->name() + "` and `" + t2->name() + "` to binary operator ");

    bool is_int1 =   is_int(t1),
         is_int2 =   is_int(t2);

    switch( kind ) {
        case ADD:
        case SUB:
        case MUL:
        case DIV:
            if( is_int1 && is_int2 )
                return type_int;
            else
                return type_float;
        case MOD:
        case XOR:
            if( is_int1 && is_int2 )
                return type_int;
            else
                TYPE_ERROR
        case AND:
        case OR :
            if( is_int1 && is_int2 )
                return type_int;
            else
                TYPE_ERROR
        case EQ :
        case NEQ:
        case GEQ:
        case LEQ:
        case LE :
        case GR :
            // keep in sync with cases ADD...DIV
            return type_int;
        default:
            throw InternalError("bla",location);
    }
#undef TYPE_ERROR
#undef TYPE_ERROR2
}

Type*
ConditionalExpression::computeType() {
    Type* t0 = cond->getType();
    Type* t1 = e1->getType();
    Type* t2 = e2->getType();
    requireConvertibleTypes( type_int, t0, location );

    if( t1 == type_void || t2 == type_void )
        throw TypeError("cond-exp cannot return values of type void", location);

    if( is_int(t1) && is_int(t2) )
        return type_int;
    else
        return type_float;
}

Type*
AssignmentExpression::computeType() {
    Type *lhs = e1->getType();
    Type *rhs = e2->getType();

    requireConvertibleTypes( lhs, rhs, location );
    return lhs;
}

Type*
UnaryFunction::computeType() {
    requireConvertibleTypes( type_float, e->getType() );
    switch( kind ) {
    case XABS:
    case XSQRT: return type_float;
    case XSIGN: return type_int;
    default: ;
    }
    return type_void;
}

// TODO: implement a seperate type check. while transforming the AST, type correctness is violated --> checking disabled temporarily
Type*
FunctionCall::computeType() {
#if 0
    const int arg_count = exp_list->size();
    const int param_count = fun_type->parameters.size();

    // check if number of formal parameters matches number of actual parameters
    if( arg_count != param_count ) {
        std::stringstream s;
        s << "function `" << fun_type->id << "` requires " << param_count << " arguments. however, only " << arg_count << "supplied" << std::endl;
        throw TypeError( s.str(), location );
    }

    FOREACH_PARAMETER_ARGUMENT_PAIR {
        Variable *v = *param_iter;
        Type *lhs = v->type;
        Type *rhs = (*arg_iter)->getType();
        requireConvertibleTypes( lhs, rhs, location );
    }
#endif
    return fun_type->getReturnType();
}

Type*
ConditionalStatement::computeType() {
    Type *cond_type = cond->getType();
    requireConvertibleTypes( type_int, cond_type, location );
    then_branch->getType();
    if( else_branch )
        else_branch->getType();
    return type_void;
}

Type*
Return::computeType() {
    Type *t;
    if( e )
        t = e->getType();
    else
        t = type_void;
    if( return_type )
        requireConvertibleTypes( return_type, t, location );
    return t;
}


Type*
StatementList::computeType() {
    FOREACH_STATEMENT {
        Statement *stmt = *it;
        assert( stmt != nullptr );
        stmt->getType();
    }
    return type_void;
}


Type*
FunctionDefinition::computeType() {
    return_type = type->return_type;
    body->getType();
    return_type = nullptr;
    return type_void;
}


Type*
TranslationUnit::computeType() {
    global_statements->getType();
    FOREACH_FUNCTION
        (*it)->getType();
    return type_void;
}

}
