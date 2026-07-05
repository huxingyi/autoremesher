#ifndef CONCRETE_VALUE_H
#define CONCRETE_VALUE_H

#include <FPG/Error.h>
#include <iostream>

#define VALUE_OP(OP) \
    Value operator OP (Value &other ) { \
        if( type == INTCONST && other.type == INTCONST ) \
            return Value( i OP other.i ); \
        else if( type == INTCONST && other.type == FLOATCONST ) \
            return Value( i OP other.f ); \
        else if( type == FLOATCONST && other.type == INTCONST ) \
            return Value( f OP other.i ); \
        else \
            return Value( f OP other.f ); \
    }

#define VALUE_OP_INT(OP) \
    Value operator OP (Value &other ) { \
        if( type == INTCONST && other.type == INTCONST ) \
            return Value( i OP other.i ); \
        else \
            throw ParseError("operators %,^ only defined for integers"); \
    }


struct Concrete_value  {
    enum Type { INTCONST, FLOATCONST };
    Type type;
    union {
        int i;
        double f;
    };

    Concrete_value( int i   ) : type( INTCONST   ), i(i) {}
    Concrete_value( double f ) : type( FLOATCONST ), f(f) {}

    /*VALUE_OP(+)  VALUE_OP(-)  VALUE_OP(*) VALUE_OP(/)
    VALUE_OP(&&) VALUE_OP(||)
    VALUE_OP(<)  VALUE_OP(<=) VALUE_OP(>) VALUE_OP(>=)
    VALUE_OP(==)  VALUE_OP(!=)
    VALUE_OP_INT(^) VALUE_OP_INT(%)*/
};


#undef VALUE_OP

#endif

