#include <FPG/Error_bound_value.h>

#include <cassert>
#include <iostream>
#include <algorithm>

//#define DEBUG 0
#include <FPG/MSG.h>

Error_bound_value::Error_bound_value()
    : index(0)
{}

Abstract_value *
Error_bound_value::get_initial_value( Variable *var ) {
    argused(var);
    return new Error_bound_value;
}


Error_bound_value*
Error_bound_value::downcast( Abstract_value* value ) {
    Error_bound_value *bound = dynamic_cast<Error_bound_value*>( value );
    assert( bound != nullptr );
    return bound;
}

Error_bound_value*
Error_bound_value::add( Abstract_value* other ) {
    Error_bound_value* new_val = clone();
    new_val->index = 1 + std::max( index, downcast( other )->index );
    MSG( index );
    return new_val;
}

Error_bound_value*
Error_bound_value::sub( Abstract_value* other ) {
    return add( other );
}


// self / other
Error_bound_value*
Error_bound_value::div( Abstract_value* other ) {
    Error_bound_value* new_val = clone();
    new_val->index = 1 + std::max( index, downcast( other )->index + 1);
    return new_val;
}

Error_bound_value*
Error_bound_value::mul( Abstract_value* other ) {
    Error_bound_value* new_val = clone();
    new_val->index = 1 + index + downcast( other )->index;
    MSG( index );
    return new_val;
}

Error_bound_value*
Error_bound_value::sqrt() {
    Error_bound_value* new_val = clone();
    new_val->index = 1 + index;
    MSG( index );
    return new_val;
}

Error_bound_value*
Error_bound_value::join( Abstract_value* other ) {
    Error_bound_value* new_val = clone();
    MSG( "joining " << this << " and " << other )
    new_val->index = std::max( index, downcast( other )->index );
    MSG( "join result: " << this )
    return new_val;
}

Error_bound_value*
Error_bound_value::clone() {
    return new Error_bound_value(*this);
}

bool
Error_bound_value::is_fresh() {
    return index == 0;
}

std::ostream&
Error_bound_value::dump( std::ostream& out ) {
    out << "index: " << index;
    return out;
}
