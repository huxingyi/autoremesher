#include <FPG/Group_index_value.h>
#include <FPG/Symbol.h>
#include <FPG/AST.h>
#include <CGALmini/basic.h>
#include <cassert>
#include <iostream>
#include <algorithm>

//#define DEBUG 0
#include <FPG/MSG.h>


Group_index_value::Group_index_value( Expression_filter* filter, Group_algebra::Group_item *item )
   : filter(filter),
     group_item(item),
     var(nullptr)
{
    if( group_item == nullptr )
        group_item = new Group_algebra::Leaf_item( 1, 0 );
}

Abstract_value *
Group_index_value::get_initial_value( Variable *var ) {
    MSG( "group index: " << var->group_index )
    Group_index_value* giv = new Group_index_value( filter, new Group_algebra::Leaf_item( var->degree, var->group_index ) );
    giv->var = var;
    return giv;
}

Group_index_value*
Group_index_value::downcast( Abstract_value* value ) {
    Group_index_value *f = dynamic_cast<Group_index_value*>( value );
    assert( f != nullptr );
    return f;
}

Group_index_value*
Group_index_value::add( Abstract_value* other, AST::BinaryExpression *e ) {
    argused(e);
    MSG("")
    Group_index_value *g = clone();
    Group_index_value *h = downcast(other);
    assert( group_item != nullptr );
    g->group_item = group_item->add( h->group_item );
    return g;
}

Group_index_value*
Group_index_value::sub( Abstract_value* other, AST::BinaryExpression *e ) {
    return add( other, e );
}


Group_index_value*
Group_index_value::div( Abstract_value* other ) {
    argused(other);
    CGAL_error_msg( "division not supported" );
    return nullptr;
}

Group_index_value*
Group_index_value::mul( Abstract_value* other ) {
    MSG("")
    Group_index_value *g = clone();
    Group_index_value *h = downcast(other);
    assert( group_item != nullptr );
    g->group_item = group_item->mul( h->group_item );
    return g;
}

Group_index_value*
Group_index_value::sqrt() {
    CGAL_error_msg( "sqrt not supported" );
    return nullptr;
}

void
Group_index_value::idexp( AST::IdentifierExpression* idexp ) {
    MSG( idexp->var->id )
    if( (*filter)(idexp) && var != nullptr ) {
        MSG("idexp->var: " << idexp->var->id )
        //assert( rep.group_indices.size() == 1 );
        //unsigned int group_index = rep.group_indices[0];
        //rep.add_to_group( group_index, var );
        assert( group_item != nullptr );
        assert( dynamic_cast<Group_algebra::Leaf_item*>(group_item) != nullptr );
        group_item->add_to_group( var );
        var = nullptr;
    }
}

void
Group_index_value::assign( AST::AssignmentExpression* aexp ) {
    MSG("")
    AST::IdentifierExpression *id_expr = dynamic_cast< AST::IdentifierExpression* >( aexp->e1 );
    assert( id_expr != nullptr );
    if( (*filter)(aexp) && var != nullptr ) {
        assert( group_item != nullptr );
        assert( dynamic_cast<Group_algebra::Leaf_item*>(group_item) != nullptr );
        group_item->add_to_group( id_expr->var );
        var = nullptr;
    } else
        MSG("group_indices.size(): " << rep.group_indices.size() );
}

Group_index_value*
Group_index_value::join( Abstract_value* other ) {
    argused(other);
    CGAL_error_msg( "sqrt not supported" );
    return nullptr;
}

Group_index_value*
Group_index_value::clone() {
    return new Group_index_value(*this);
}

bool
Group_index_value::is_fresh() {
    return true; // not needed
}

std::ostream&
Group_index_value::dump( std::ostream& out ) {
    group_item->dump();
    return out;
}
