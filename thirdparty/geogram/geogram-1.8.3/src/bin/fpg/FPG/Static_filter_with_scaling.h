#ifndef FPG_STATIC_FILTER_WITH_SCALING_H
#define FPG_STATIC_FILTER_WITH_SCALING_H

#include <FPG/Misc_visitors.h>
#include <FPG/Transformation.h>
#include <FPG/Abstract_interpretation_visitor.h>
#include <FPG/Group_index_value.h>
#include <FPG/Group_algebra.h>
#include <set>
#include <vector>

namespace Static_filter_with_scaling {

struct Group_cache {
    typedef std::set< Variable*>         Group;
    typedef std::map< Group, Variable* > Mapping;
    typedef std::map< Variable*, Group > Reverse_mapping;
    typedef Mapping::iterator            iterator;

    Mapping         groups;
    Reverse_mapping reverse_mapping;

    // if v ---> { a,b,c } then a,b,c are subsets of the set v
    // which means that the maxima for a,b,c have to be computed before max{v}
    std::map< Variable*, std::set< Variable* > > subset_relation;

    // returns true iff var appears in one of the subset_relation varsets,
    // ignoring all variables in 'ignore'
    //bool
    //has_superset( Variable *var, const std::set< Variable* >& ignore );

    // for each group g, check if another group f
    // is a subset/is equal to g. store this subset/dependency relation
    void
    make_partial_order();

    bool
    is_maximum( Variable* maxvar, std::vector< Variable* >& max_vars );

    unsigned int
    number_of_subsets( Variable* maxvar, std::vector< Variable* >& max_vars, std::set< Variable*> &ignore );

    Variable*
    find_maximum( std::vector< Variable* >& max_vars, std::set< Variable*> &ignore );


    // for the group g associated to maxvar:
    // check, if another group with index 0 exists, that is neither subset
    // nor superset, but shares some of its elements (non-zero intersection)
    bool
    is_intersection_free( Variable *maxvar );

    void
    add( const Group& key, Variable *value ) {
        groups[key] = value;
        reverse_mapping[value] = key;
    }

    iterator find( const Group& key ) { return groups.find( key );  }
    iterator begin()                  { return groups.begin();      }
    iterator end()                    { return groups.end();        }
    void     clear()                  { groups.clear();             }

};

struct Add_bound_variables : public Add_bound_variables_base  {
    Add_bound_variables( bool use_aposteriori_check = false );
    // same as transformationvisitor: intraprocedural
    virtual void visit( AST::UnaryFunction* );
    virtual void visit( AST::BinaryExpression * );
    virtual void visit( AST::TranslationUnit* );

    bool use_as_max( AST::Expression* e);



    Variable *epsilon_tmp_variable;
    Abstract_interpretation_visitor
        *absint_is_fresh,
        *absint_sfe,
        *absint_giv,
        *absint_max;
    Group_cache  group_cache;
    unsigned int max_var_counter;
    bool use_aposteriori_check;
    //std::map< AST::Expression*, AST::Expression* > group_index_to_maxvar;
};

struct Rewrite_sign_of_fresh_values : public Transformation_visitor {
    Rewrite_sign_of_fresh_values( Add_bound_variables *add_bounds )
       : add_bounds( add_bounds )
    {}

    virtual void visit( AST::UnaryFunction * );
    Add_bound_variables *add_bounds;
};

struct Rewrite_float_comparisons : public Transformation_visitor {
    Rewrite_float_comparisons( Add_bound_variables *add_bounds )
        : max_input(0.0), add_bounds( add_bounds )
    {}

    virtual void visit( AST::UnaryFunction * );
    virtual void visit( AST::FunctionDefinition * );

    AST::Expression*  make_max_term( Group_algebra::Group_item *leaf );

    void              compute_max_from( Variable *maxvar, Variable *other,
                                        Statement_addition_visitor::Position scope,
                                        bool use_fabs = true );
    void              make_max_computation( Variable *maxvar );
    void              make_max_groups( Group_index_value * giv,
                                       double              min_input,
                                       AST::Expression   *&result_cond_underflow,
                                       //AST::Expression   *&result_cond_zero,
                                       AST::Expression   *&result_cond_overflow );
    void              derive_min_max( unsigned int degree,
                                      std::vector< Variable* > &max_variables,
                                      Variable *& lb_var,
                                      Variable *& ub_var );
    void              reset();

    double max_input;
protected:
    // for each degree, store the respective lower/upper bound tmp variables
    std::map< unsigned int, Variable * > lb_vars, ub_vars;
    Add_bound_variables                 *add_bounds;
    std::map< Variable*, std::set< Variable* > > max_var_cover;
    std::set< Variable *> function_parameters;
};

}

#endif
