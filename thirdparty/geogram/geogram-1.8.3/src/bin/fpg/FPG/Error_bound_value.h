#ifndef ERROR_BOUND_VALUE_H_
#define ERROR_BOUND_VALUE_H_

#include <FPG/Abstract_value.h>

struct Error_bound_value : public Abstract_value {
    Error_bound_value();

    virtual Abstract_value *get_initial_value( Variable *var );

    Error_bound_value* downcast( Abstract_value* value );
    virtual Error_bound_value* add( Abstract_value* other );
    virtual Error_bound_value* sub( Abstract_value* other );
    virtual Error_bound_value* div( Abstract_value* other );
    virtual Error_bound_value* mul( Abstract_value* other );
    virtual Error_bound_value* sqrt();
    virtual Error_bound_value* join( Abstract_value* other );
    virtual Error_bound_value* clone();
    virtual bool               is_fresh();
    virtual std::ostream&      dump( std::ostream& out );

    unsigned int index;
};


#endif /*ERROR_BOUND_VALUE_H_*/
