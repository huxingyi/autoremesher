#ifndef FPG_GROUP_ALGEBRA_H
#define FPG_GROUP_ALGEBRA_H

#include <FPG/Symbol.h> // for Variable
#include <vector>
#include <set>

namespace Group_algebra {

typedef std::set< std::pair< unsigned int, std::set< Variable* > > > Group_varset;

struct Group_item {

    virtual ~Group_item() {}
    virtual Group_item*   add( Group_item* other );
    virtual Group_item*   mul( Group_item* other );
    virtual unsigned int  degree() = 0;
    virtual bool          is_equal( Group_item *other, bool consider_varset = false ) = 0;
    virtual void          dump() = 0;
    //virtual void          simplify() = 0;
    virtual void          add_to_group( Variable *var ) = 0;
    virtual void          collect_groups( Group_varset &groups ) = 0;
protected:
    static unsigned int indent_level;
    static void print_indent();
};


struct Leaf_item : public Group_item {
    unsigned int degree_;
    unsigned int index_;
    std::set< Variable* > variables;

    Leaf_item( unsigned int degree, unsigned int index );
    Leaf_item( unsigned int degree, unsigned int index, std::set<Variable*> &variables );
    virtual Group_item*  add( Group_item* other );
    virtual unsigned int degree();
    virtual bool         is_equal( Group_item *other, bool consider_varset = false );
    virtual void         dump();

    virtual void         add_to_group( Variable *var );
    virtual void         collect_groups( Group_varset &groups );
};

struct Array_item : public Group_item {
    std::vector< Group_item* > items;

    virtual bool          is_equal( Group_item *other, bool consider_varset = false );
    virtual void          dump();
    virtual void          add_to_group( Variable *var );
    virtual void          collect_groups( Group_varset &groups );
};

struct Product_item : public Array_item {
    Product_item() {}
    Product_item( Group_item* a, Group_item* b );
    virtual Group_item*   add( Group_item* other );
    virtual Product_item* mul( Group_item* other );
    virtual unsigned int  degree();
    virtual bool          is_equal( Group_item *other, bool consider_varset = false );
    virtual void          dump();
};

struct Sum_item : public Array_item {
    Sum_item() {}
    Sum_item( Group_item* a, Group_item* b );
    virtual Sum_item*     add( Group_item* other );
    virtual unsigned int  degree();
    virtual bool          is_equal( Group_item *other, bool consider_varset = false );
    virtual void          dump();
};

}

#endif
