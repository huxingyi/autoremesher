#ifndef ABSTRACT_VALUE_H_
#define ABSTRACT_VALUE_H_

#include <iostream>
#include <FPG/Error.h>

namespace AST {
    struct Expression;
    struct IdentifierExpression;
    struct BinaryExpression;
    struct AssignmentExpression;
}

struct Variable;

// we combine value and transfer function:
struct Abstract_value {
    virtual Abstract_value *get_initial_value( Variable *var ) = 0;
    virtual ~Abstract_value() {}
    virtual Abstract_value* add( Abstract_value* other ) { argused(other); return clone(); }
    virtual Abstract_value* sub( Abstract_value* other ) { argused(other); return clone(); }
    virtual Abstract_value* div( Abstract_value* other ) { argused(other); return clone(); }
    virtual Abstract_value* mul( Abstract_value* other ) { argused(other); return clone(); }
    virtual Abstract_value* sqrt() = 0;

    // callbacks, needed for Group_index_value:
    virtual void idexp( AST::IdentifierExpression* ) {}
    virtual void assign( AST::AssignmentExpression* ) {}
    // expression is used as argument in funcall
    virtual void funcall( AST::Expression* ) {}

    // by default, ignore hint:
    virtual Abstract_value* add( Abstract_value* other, AST::BinaryExpression *bexp ) {
        argused(bexp);
        return add(other);
    }
    virtual Abstract_value* sub( Abstract_value* other, AST::BinaryExpression *bexp ) {
        argused(bexp);
        return sub(other);
    }
    virtual Abstract_value* mul( Abstract_value* other, AST::BinaryExpression *bexp ) {
        argused(bexp);
        return mul(other);
    }
    virtual Abstract_value* div( Abstract_value* other, AST::BinaryExpression *bexp ) {
        argused(bexp);
        return div(other);
    }

    virtual Abstract_value* join( Abstract_value* other ) = 0;
    virtual Abstract_value* clone() = 0;
    // returns true if it is equivalent to a default constructed abstract value
    virtual bool            is_fresh() = 0;

    virtual std::ostream& dump( std::ostream& out ) = 0;
};

inline std::ostream&
operator<<( std::ostream& out, Abstract_value* val ) {
  return val->dump( out );
}

#endif /*ABSTRACT_VALUE_H_*/
