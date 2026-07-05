#ifndef FPG_STATIC_FILTER_ERROR_H_
#define FPG_STATIC_FILTER_ERROR_H_

#include <string>
#include <FPG/Abstract_value.h>
#include <CGALmini/Static_filter_error.h> 

struct Static_filter_error : public Abstract_value {
    explicit Static_filter_error( Expression_filter* filter )
        : e(1), filter(filter)
    {
    }

    explicit Static_filter_error( Expression_filter* filter,
                                  const CGAL::Static_filter_error& e )
        : e(e),
          filter(filter)
    {}

    virtual Abstract_value *get_initial_value( Variable *var ) {
        CGAL::FPU_CW_t backup = CGAL::FPU_get_and_set_cw(CGAL_FE_UPWARD);
        Static_filter_error *e = clone();
        double b = std::pow( e->e.bound(), (int)var->degree );
        e->e = CGAL::Static_filter_error( b, e->e.error(), e->e.degree() );
        CGAL::FPU_set_cw(backup);
        return e;
    }

    const CGAL::Static_filter_error& error()  const { return e; }

    Static_filter_error* downcast( Abstract_value* value ) {
        Static_filter_error *f = dynamic_cast<Static_filter_error*>( value );
        assert( f != nullptr );
        return f;
    }

    virtual Static_filter_error*
    add( Abstract_value* other, AST::BinaryExpression *e ) {
        if( (*filter)(e) ) {
            // error for first translation: bound stays the same, only error increases
            double bound = error().bound();
            CGAL::FPU_CW_t backup = CGAL::FPU_get_and_set_cw(CGAL_FE_UPWARD);
            double error = CGAL::Static_filter_error::ulp(bound)/2;
            CGAL::FPU_set_cw(backup);
            return new Static_filter_error( filter, CGAL::Static_filter_error(bound,error) );
        } else
            return new Static_filter_error( filter, error() + downcast(other)->error() );
    }

    virtual Static_filter_error*
    sub( Abstract_value* other, AST::BinaryExpression *e ) {
        return add( other, e );
    }

    virtual Static_filter_error* div( Abstract_value* other ) {
        argused(other);
        throw RuntimeError( "Static_filter_error: division not supported" );
    }

    virtual Static_filter_error* mul( Abstract_value* other ) {
        argused(other);        
        return new Static_filter_error( filter, error() * downcast(other)->error() );
    }

    virtual Static_filter_error* sqrt() {
        if( error().degree() % 2 != 0 )
            throw RuntimeError( "Static_filter_error: non-integral degree" );
        return new Static_filter_error( filter, CGAL::sqrt( error() ) );
    }

    virtual Static_filter_error* join( Abstract_value* other ) {
        argused(other);        
        throw RuntimeError( "Static_filter_error: join op not supported" );
    }

    virtual Static_filter_error* clone() {
        return new Static_filter_error(*this);
    }

    virtual bool
    is_fresh() { return error().error() == 0.0; }

    virtual std::ostream&
    dump( std::ostream& out ) {
        out << "error: " << error().error() << std::endl
            << "bound: " << error().bound() << std::endl
            << "degree: " << error().degree() << std::endl;
        return out;
    }

private:
    CGAL::Static_filter_error e;
    Expression_filter* filter; // true == first translation
};

#endif /* FPG_STATIC_ERROR_BOUND_VALUE_H_ */
