#ifndef ATTRIBUTE_VISITOR_H
#define ATTRIBUTE_VISITOR_H

#include <FPG/Generic_visitor.h>

// ResultType needs to be CopyConstructible and
// DefaultAssignable
template< typename ResultType >
struct Attribute_visitor : public Generic_visitor {
    ResultType result_value, default_value, shortcut_value;
    bool       use_shortcut_value;

    Attribute_visitor( bool interprocedural = true )
        : Generic_visitor(interprocedural),
          use_shortcut_value(false)
    {}

    void set_default_value( ResultType value ) {
        default_value = value;
        reset();
    }

    void set_shortcut_value( ResultType value ) {
        shortcut_value = value;
        use_shortcut_value = true;
    }

    void reset() {
        result_value = default_value;
    }

    virtual ResultType combine( ResultType a, ResultType b ) = 0;

    virtual bool is_certain() {
        return use_shortcut_value && result_value == shortcut_value;
    }

    // overrides function from base class
    virtual void handle( AST::Node *n ) {
        // do nothing, if result value is already certain
        if( is_certain() )
            return;
        ResultType v = result_value; // save
        // in case the visitor does not set the value, provide a sensible default:
        result_value = default_value;
        Generic_visitor::handle( n );
        result_value = combine( result_value, v );

    }
};

#endif
