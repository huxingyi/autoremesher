#ifndef FPG_GROUP_INDEX_VALUE_H_
#define FPG_GROUP_INDEX_VALUE_H_

#include <string>
#include <vector>
#include <set>
#include <FPG/Abstract_value.h>
#include <FPG/Group_algebra.h>

struct Expression_filter;

#if 0
struct Max_variables {
    typedef std::set< Variable* > Group;

    std::ostream&             dump( std::ostream&);
    void                      clear() { group_variables.clear(); }
    void                      add_to_group( unsigned int index, Variable *var );
    // var means result here: var = max(px,qx,rx ...)
    Group &                   get_group( unsigned int index );
    const Group &             get_group( unsigned int index ) const;
    unsigned int              degree() const { return group_indices.size(); }
    unsigned int              size() const { return group_variables.size(); }
    unsigned int              number_of_different_groups() const;

    void                      add( const Max_variables &a, const Max_variables &b);
    void                      mul( const Max_variables &a, const Max_variables &b );

    // one element represents a single group,
    // more than one elements represents multiplication, for example
    // 0 * 0 * 1 * 2 * 3
    // indices are sorted and may occur more than once.
    // 0 means "maximum of all variables that occur
    // in that formula"
    // 1,2,3... means respective group of variables
    std::vector< unsigned int > group_indices;
    // ...
    std::vector< Group        > group_variables;
protected:
    void                      merge( const Max_variables &other, unsigned int source_index, unsigned int target_index );
    void                      ensure_size( unsigned int new_size );
};
#endif

struct Group_index_value : public Abstract_value {
    Group_index_value( Expression_filter* filter,
                       Group_algebra::Group_item *item = NULL );
    virtual Abstract_value *get_initial_value( Variable *var );

            Group_index_value* downcast( Abstract_value* value );
    virtual Group_index_value* add( Abstract_value* other, AST::BinaryExpression *e );
    virtual Group_index_value* sub( Abstract_value* other, AST::BinaryExpression *e );
    virtual Group_index_value* div( Abstract_value* other );
    virtual Group_index_value* mul( Abstract_value* other );
    virtual Group_index_value* sqrt();
    virtual Group_index_value* join( Abstract_value* other );
    virtual Group_index_value* clone();
    virtual void idexp ( AST::IdentifierExpression* idexp );
    virtual void assign( AST::AssignmentExpression* aexp  );

    virtual bool                      is_fresh();
    virtual std::ostream&             dump( std::ostream& out );

    Expression_filter* filter;

    Group_algebra::Group_item *group_item;

    // as long as it is a fresh value, store var:
    Variable *var;
};


#endif /* FPG_GROUP_INDEX_VALUE_H_ */
