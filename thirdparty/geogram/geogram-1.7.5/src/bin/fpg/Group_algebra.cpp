#include <CGALmini/basic.h>
#include <FPG/Group_algebra.h>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <list>


//#define DEBUG 1
#include <FPG/MSG.h>

namespace Group_algebra {

namespace {
struct Group_item_is_equal {
    Group_item* a;
    bool consider_varset;
    Group_item_is_equal( Group_item* a, bool consider_varset = false )
      : a(a), consider_varset( consider_varset )
    {}

    bool operator()( Group_item* b ) const {
        return a->is_equal( b , consider_varset );
    }
};

struct Group_item_has_same_degree {
    Group_item* a;
    Group_item_has_same_degree( Group_item* a ) :a(a){}
    bool operator()( Group_item* b ) const {
        return a->degree() == b->degree();
    }
};
}

unsigned int Group_item::indent_level = 0;

// constructors
Leaf_item::Leaf_item( unsigned int degree, unsigned int index )
    : degree_(degree),
      index_ (index )
{}

Leaf_item::Leaf_item( unsigned int degree, unsigned int index, std::set<Variable*> &variables )
    : degree_(degree),
      index_ (index )
{
    this->variables = variables;
}

Product_item::Product_item( Group_item* a, Group_item* b ) {
    assert( a != nullptr );
    assert( b != nullptr );
    items.push_back( a );
    items.push_back( b );
}

Sum_item::Sum_item( Group_item* a, Group_item* b ) {
    assert( a != nullptr );
    assert( b != nullptr );
    assert( a->degree() == b->degree() );
    items.push_back( a );
    items.push_back( b );
}


// degree
unsigned int
Leaf_item::degree() {
    return degree_;
}

unsigned int
Product_item::degree() {
    std::vector< Group_item* >::iterator it;
    unsigned int result = 0;
    for( it = items.begin(); it != items.end(); ++it ) {
        assert( *it != nullptr );
        result += (*it)->degree();
    }
    return result;
}

unsigned int
Sum_item::degree() {
    std::vector< Group_item* >::iterator it;
    unsigned int result = 0;
    for( it = items.begin(); it != items.end(); ++it )
        result = std::max( result, (*it)->degree() );
    return result;
}

// is_equal =====================================================
bool
Leaf_item::is_equal( Group_item *other, bool consider_varset ) {
    assert( other != nullptr );
    Leaf_item* l = dynamic_cast<Leaf_item*>(other);
    if( l == nullptr )
        return false;
    else
        return degree() == l->degree() &&
               index_ == l->index_ &&
               (!consider_varset || variables == l->variables);
}

bool
Array_item::is_equal( Group_item *other, bool consider_varset ) {
    assert( other != nullptr );
    Array_item* p = dynamic_cast<Array_item*>(other);
    if( p == nullptr )
        return false;
    if( p->degree() != degree() || items.size() != p->items.size() ) {
        MSG("not equal: " << p->degree() << " != " << degree() << " || " << items.size() << " != " << p->items.size() )
        return false;
    }
    std::list< Group_item * > todo;
    std::copy( p->items.begin(), p->items.end(), std::back_inserter( todo ) );
    std::vector< Group_item* >::iterator it;
    for( it = items.begin(); it != items.end(); ++it ) {
        std::list< Group_item * >::iterator it2 = std::find_if( todo.begin(), todo.end(), Group_item_is_equal(*it, consider_varset) );
        if( it2 == todo.end() )
            return false;
        else
            todo.erase( it2 );
    }
    assert( todo.empty() );
    return true;
}

bool
Sum_item::is_equal( Group_item *other, bool consider_varset ) {
    assert( other != nullptr );
    Sum_item* p = dynamic_cast<Sum_item*>(other);
    if( p == nullptr )
        return false;
    return Array_item::is_equal( other, consider_varset );
}

bool
Product_item::is_equal( Group_item *other, bool consider_varset ) {
    assert( other != nullptr );
    Product_item* p = dynamic_cast<Product_item*>(other);
    if( p == nullptr )
        return false;
    return Array_item::is_equal( other, consider_varset );
}


// add ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Group_item*
Group_item::add( Group_item *other ) {
    return other;
}

Group_item*
Leaf_item::add( Group_item* other ) {
    MSG("")
    assert( other != nullptr );
    //dump();
    //other->dump();
    if( degree() != other->degree() ) {
        std::cerr << " !!!! warning: cannot directly add numbers of different degrees: " << degree() << " + " << other->degree() << "! possible loss of precision." << std::endl;
    }
    Leaf_item* l = dynamic_cast<Leaf_item*>(other);
    Sum_item * s = dynamic_cast< Sum_item*>(other);
    if( l != nullptr ) {
        std::set<Variable*> new_variables;
        std::copy(    variables.begin(),    variables.end(), std::inserter( new_variables, new_variables.begin() ) );
        std::copy( l->variables.begin(), l->variables.end(), std::inserter( new_variables, new_variables.begin() ) );
        if( index_ == l->index_ && l->degree() == degree() )
            return new Leaf_item( degree(), index_, new_variables );
        else if( l->degree() == 1 && degree() == 1 )
            return new Leaf_item( degree(), 0,      new_variables );
        else
            return new Leaf_item( std::max( degree(), l->degree() ), 0, new_variables );
    } else if( s != nullptr )
        return s->add( this );
    MSG("cannot directly add, creating new sum")
    return new Sum_item( this, other );
}

Sum_item*
Sum_item::add( Group_item* other ) {
    MSG("")
    assert( other != nullptr );
    //dump();
    //other->dump();
    if( degree() != other->degree() ) {
        std::cerr << " !!!! error: cannot directly add numbers of different degrees: " << degree() << " + " << other->degree() << "! possible loss of precision." << std::endl;
        //dump();
        //other->dump();
        std::exit(1); // TODO: implement method from Leaf_item also here
    }
    Sum_item* new_s = new Sum_item( *this );
    Sum_item* s = dynamic_cast<Sum_item*>(other);
    std::list< Group_item* > this_todo, other_todo;
    std::list< Group_item* >::iterator other_it;
    std::vector< Group_item* >::iterator this_it;
    if( s != nullptr )
        std::copy( s->items.begin(), s->items.end(), std::back_inserter( other_todo ) );
    else
        other_todo.push_back( other );

    //  in a first round, sort out the easy cases where items are equal:
    for( other_it = other_todo.begin(); other_it != other_todo.end();  ) {
        MSG( "looking for equal items to:" )
        //(*other_it)->dump();
        this_it = std::find_if( new_s->items.begin(), new_s->items.end(), Group_item_is_equal(*other_it, true) );
        if( this_it == new_s->items.end() )
            this_it = std::find_if( new_s->items.begin(), new_s->items.end(), Group_item_is_equal(*other_it, false) );
        if( this_it == new_s->items.end() )
            this_it = std::find_if( new_s->items.begin(), new_s->items.end(), Group_item_has_same_degree(*other_it) );
        if( this_it != new_s->items.end() ) {
            MSG("  found: " )
            //(*this_it)->dump();
            *this_it = (*this_it)->add( *other_it );
            MSG("  after adding: " )
            //(*this_it)->dump();
            other_it = other_todo.erase( other_it );
        } else
            ++other_it;
    }
    MSG("still todo after adding: " << other_todo.size() )
    // now, we have to deal with the non-equal items: assimilate
    std::copy( other_todo.begin(), other_todo.end(), std::back_inserter( new_s->items ) );
    return new_s;
}

Group_item*
Product_item::add( Group_item* other ) {
    MSG("")
    assert( other != nullptr );
    //dump();
    //other->dump();
    if( degree() != other->degree() ) {
        std::cerr << " !!!! warning: cannot directly add numbers of different degrees: " << degree() << " + " << other->degree() << "! possible loss of precision." << std::endl;
    }
    Sum_item  *s = dynamic_cast<Sum_item*>(other);
    Leaf_item *l = dynamic_cast<Leaf_item*>(other);
    if( s != nullptr )
        return s->add( this );
    else if( l != nullptr )
        return l->add( this );

    Product_item* new_p = new Product_item();
    Product_item* p = dynamic_cast<Product_item*>(other);
    std::list< Group_item* > this_todo, other_todo;
    std::list< Group_item* >::iterator this_it, other_it;
    std::copy( items.begin(), items.end(), std::back_inserter( this_todo ) );
    if( p != nullptr )
        std::copy( p->items.begin(), p->items.end(), std::back_inserter( other_todo ) );
    else
        other_todo.push_back( other );

    //  in a first round, sort out the easy cases where items are equal:
    for( this_it = this_todo.begin(); this_it != this_todo.end(); ) {
        MSG( "looking for equal items to:" )
        //(*this_it)->dump();
        other_it = std::find_if( other_todo.begin(), other_todo.end(), Group_item_is_equal(*this_it, true) );
        if( other_it == other_todo.end() )
            other_it = std::find_if( other_todo.begin(), other_todo.end(), Group_item_is_equal(*this_it, false ) );
        if( other_it == other_todo.end() )
            other_it = std::find_if( other_todo.begin(), other_todo.end(), Group_item_has_same_degree(*this_it) );
        if( other_it != other_todo.end() ) {
            MSG("  found: " )
            //(*other_it)->dump();
            Group_item *tmp_result = (*this_it)->add( *other_it );
            MSG("  new entry for result:" )
            //tmp_result->dump();
            new_p->items.push_back( tmp_result );
            other_todo.erase( other_it );
            this_it = this_todo.erase( this_it );
        } else
            ++this_it;
    }
    if( !this_todo.empty() || !other_todo.empty() ) {
        MSG("items left!")
        Group_item *i1 = nullptr;
        Group_item *i2 = nullptr;
        if( this_todo.size() > 1 ) {
            Product_item *p1 = new Product_item();
            std::copy(  this_todo.begin(),  this_todo.end(), std::back_inserter( p1->items ) );
            i1 = p1;
        } else if( this_todo.size() == 1 )
            i1 = this_todo.front();
        if( other_todo.size() > 1 ) {
            Product_item *p2 = new Product_item();
            std::copy(  other_todo.begin(),  other_todo.end(), std::back_inserter( p2->items ) );
            i2 = p2;
        } else if( other_todo.size() == 1 )
            i2 = other_todo.front();
        assert( i1 != nullptr || i2 != nullptr );
        if( i1 == nullptr ) {
            assert( i2 != nullptr );
            i1 = i2;
        } else if( i2 == nullptr ) {
            assert( i1 != nullptr );
            i2 = i1;
        }
        MSG("rest1: " )
        //i1->dump();
        MSG("rest2: " )
        //i2->dump();
        MSG("---")
        if( i1 == i2 )
            new_p->items.push_back( i1 );
        else if( i1->degree() == i2->degree() && i1->degree() >= 2 ) {
            MSG("trying to add ... ")
            new_p->items.push_back( i1->add( i2 ) );
        }
        else
            new_p->items.push_back( new Sum_item( i1, i2 ) );
    }
    if( new_p->items.size() == 1 )
        return new_p->items.front();
    else
        return new_p;
}


// mul **************************************************
Group_item*
Group_item::mul( Group_item *other ) {
    MSG("")
    assert( other != nullptr );
    //dump();
    //other->dump();
    Leaf_item* other_l = dynamic_cast<Leaf_item*>(other);
    // dont do anything for multiplications with constants:
    if( other_l != nullptr && other_l->variables.empty() )
        return this;
    Product_item *p = dynamic_cast< Product_item* >( other );
    if( p != nullptr )
        return p->mul( this );
    else
        return new Product_item( this, other );
}


Product_item*
Product_item::mul( Group_item* other ) {
    MSG("")
    assert( other != nullptr );
    //dump();
    //other->dump();
    Product_item *new_p = new Product_item( *this );
    Leaf_item* other_l = dynamic_cast<Leaf_item*>(other);
    // dont do anything for multiplications with constants:
    if( other_l != nullptr && other_l->variables.empty() )
        return this;

    Product_item* other_p = dynamic_cast<Product_item*>(other);
    if( other_p != nullptr )
        std::copy( other_p->items.begin(), other_p->items.end(), std::back_inserter( new_p->items ) );
    else
        new_p->items.push_back( other );
    return new_p;
}


// dump << << << << << << << << << << << << << << << << << << << << << << << << << <<
void
Group_item::print_indent() {
    for( unsigned int i = 0; i < indent_level; ++i )
        std::cout << "    ";
}

void
Leaf_item::dump() {
    print_indent();
    std::cout << "Leaf( degree: " << degree_ << " index: " << index_ << " vars: ";
    std::set<Variable*>::iterator it;
    for( it = variables.begin(); it != variables.end(); ++it )
        std::cout << (*it)->id << " ";
    std::cout << ")" << std::endl;
}

void
Array_item::dump() {
    ++indent_level;
    std::vector< Group_item* >::iterator it;
    for( it = items.begin(); it != items.end(); ++it ) {
        (*it)->dump();
    }
    --indent_level;
    std::cout << std::endl;
}

void
Product_item::dump() {
    print_indent();
    std::cout << "Product" << std::endl;
    Array_item::dump();
}

void
Sum_item::dump() {
    print_indent();
    std::cout << "Sum" << std::endl;
    Array_item::dump();
}

// simplify %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
/*void
Leaf_item::simplify {
}

void
Array_item::simplify {
    std::vector< Group_item* >::iterator it;
    for( it = items.begin(); it != items.end(); ++it ) {
        (*it)->simplify;
    }
}

void
Product_item::simplify {
    Array_item::simplify;
}

void
Sum_item::simplify {
    Array_item::simplify;
}
*/

void
Leaf_item::add_to_group( Variable *var ) {
    assert( var != nullptr );
    MSG( var->id )
    variables.insert( var );
}

void
Leaf_item::collect_groups( Group_varset &groups ) {
    groups.insert( std::make_pair( degree(), variables ) );
}

void
Array_item::add_to_group( Variable *var ) {
    argused(var);
    CGAL_error_msg("can only add to leaf_items!");
}

void
Array_item::collect_groups( Group_varset &groups ) {
    std::vector< Group_item* >::iterator it;
    for( it = items.begin(); it != items.end(); ++it )
        (*it)->collect_groups( groups );
}

}
