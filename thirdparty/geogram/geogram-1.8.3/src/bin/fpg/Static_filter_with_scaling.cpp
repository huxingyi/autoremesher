#include <FPG/Static_filter_with_scaling.h>
#include <FPG/Misc_visitors.h>
#include <FPG/Abstract_interpretation_visitor.h>
#include <FPG/Prettyprint_visitor.h>
#include <FPG/Transformation.h>
#include <FPG/Error_bound_value.h>
#include <FPG/Static_filter_error.h>
#include <FPG/Group_index_value.h>
#include <CGALmini/basic.h>

#include <cassert>
#include <vector>
#include <set>
#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>

extern "C" {
#include <fenv.h>
}

//#define DEBUG 0
#include <FPG/MSG.h>


using namespace Static_filter_with_scaling;

struct Contains_translation_on_fresh_values :
    public Attribute_visitor<bool>
{
    Abstract_interpretation_visitor *absint_is_fresh;

    // sfe knows, which AST::Expression*s are fresh
    Contains_translation_on_fresh_values( Abstract_interpretation_visitor *absint_is_fresh )
        : /* inter-procedural, */
          absint_is_fresh(absint_is_fresh)
    {
        set_default_value( false );
        set_shortcut_value( true );
    }

    virtual bool combine( bool a, bool b ) {
        return a || b;
    }

    virtual void visit( AST::BinaryExpression* bexp ) {
        Error_bound_value *is_fresh1 = dynamic_cast<Error_bound_value*>( absint_is_fresh->get_analysis_result( bexp->e1 ) );
        Error_bound_value *is_fresh2 = dynamic_cast<Error_bound_value*>( absint_is_fresh->get_analysis_result( bexp->e2 ) );
        assert( is_fresh1 != nullptr && is_fresh2 != nullptr );
        if( is_fresh1->is_fresh() && is_fresh2->is_fresh() && bexp->kind == AST::BinaryExpression::SUB ) {
            MSG("")
            //bexp->dump(0);
            result_value = true;
        } else
            Attribute_visitor<bool>::visit( bexp );
    }
};

struct Contains_float_compare_on_derived_values :
    public Attribute_visitor<bool>
{
    Abstract_interpretation_visitor *absint_is_fresh;

    // sfe knows, which AST::Expression*s are fresh
    Contains_float_compare_on_derived_values( Abstract_interpretation_visitor *absint_is_fresh )
        : /* inter-procedural, */
          absint_is_fresh(absint_is_fresh)
    {
        set_default_value( false );
        set_shortcut_value( true );
    }

    virtual bool combine( bool a, bool b ) {
        return a || b;
    }

    virtual void visit( AST::BinaryExpression* bexp ) {
        if( is_float( bexp->e1->getType() ) ||
            is_float( bexp->e2->getType() ) )
        {
            Error_bound_value *is_fresh1 = dynamic_cast<Error_bound_value*>( absint_is_fresh->get_analysis_result( bexp->e1 ) );
            Error_bound_value *is_fresh2 = dynamic_cast<Error_bound_value*>( absint_is_fresh->get_analysis_result( bexp->e2 ) );
            assert( is_fresh1 != nullptr && is_fresh2 != nullptr );
            bool is_fresh = is_fresh1->is_fresh() && is_fresh2->is_fresh();
            //MSG(is_fresh)
            switch( bexp->kind ) {
            case AST::BinaryExpression::EQ:
            case AST::BinaryExpression::NEQ:
            case AST::BinaryExpression::GEQ:
   	    case AST::BinaryExpression::LEQ: 
                if( !is_fresh )
                    throw RuntimeError("no equality compare for derived values!", bexp->location );
		// fallthrough
            case AST::BinaryExpression::LE:
            case AST::BinaryExpression::GR:
                if( !is_fresh ) {
                    result_value = true;
                    return;
                }
                break;
            default: break;
            }
        }
        Attribute_visitor<bool>::visit( bexp );
    }

    virtual void visit( AST::UnaryFunction *uf ) {
        Error_bound_value *is_fresh = dynamic_cast<Error_bound_value*>( absint_is_fresh->get_analysis_result( uf->e ) );
        assert( is_fresh != nullptr );
        if( uf->kind == AST::UnaryFunction::XSIGN && !is_fresh->is_fresh() )
            result_value = true;
        else
            Attribute_visitor<bool>::visit( uf );
    }
};



struct Subst_funcall_filter : Expression_filter {
    Add_bound_variables *add_bounds;

    Subst_funcall_filter( Add_bound_variables *add_bounds )
        : add_bounds(add_bounds)
    {}

    virtual bool operator()( AST::Expression *e ) {
        // this is not really efficient ....
        MSG("")
        //e->dump(0);
        AST::FunctionCall *funcall = dynamic_cast<AST::FunctionCall*>(e);
        assert( funcall != nullptr );
        if( funcall->fun_type->is_extern )
            return false;
        assert( funcall->called_function != nullptr );
        Contains_translation_on_fresh_values       has_fresh_sub ( add_bounds->absint_is_fresh );
        Contains_float_compare_on_derived_values   has_float_comp( add_bounds->absint_is_fresh );
        funcall->called_function->accept( &has_fresh_sub );
        funcall->called_function->accept( &has_float_comp );
        //std::cout << has_fresh_sub.result_value << has_float_comp.result_value << std::endl;
        return has_fresh_sub.result_value || has_float_comp.result_value;
    }
};

// for each group g, check if another group h
// is a subset/is equal to g. store this subset/dependency relation
void
Group_cache::make_partial_order() {
    subset_relation.clear();
    for( iterator g_it = begin(); g_it!= end(); ++g_it ) {
        const Group &g = g_it->first;
        Variable   *vg = g_it->second;
        /*std::cout << "group g:" << vg->id << " with: ";
        for( Group::iterator xx_it = g.begin(); xx_it != g.end(); ++xx_it )
            std::cout << (*xx_it)->id << " ";
        std::cout << std::endl;*/
        for( iterator h_it = begin(); h_it!= end(); ++h_it ) {
            if( g_it == h_it )
                continue;
            const Group &h = h_it->first;
            Variable   *vh = h_it->second;
            /*std::cout << "checking group " << vh->id << " with: ";
            for( Group::iterator xx_it = h.begin(); xx_it != h.end(); ++xx_it )
                std::cout << (*xx_it)->id << " ";
            std::cout << std::endl;*/

            // check if h is a subset of g (or equal to g)
            if( std::includes( g.begin(), g.end(), h.begin(), h.end() ) ) {
                // the variable vh can only appear once, so no need to check for
                // duplicates
                subset_relation[vg].insert( vh );
                //std::cout << " ++ var " << vh->id << " is subset" << std::endl;
            } //else
                //std::cout << " -- var " << vh->id << " no subset" << std::endl;
        }
    }
}

bool
Group_cache::is_intersection_free( Variable *maxvar /*, std::vector< Variable* >& other_max_vars */ ) {
    Reverse_mapping::iterator g_it = reverse_mapping.find( maxvar );
    assert( g_it != reverse_mapping.end() );
    const Group &g = g_it->second;
    for( iterator h_it = begin(); h_it!= end(); ++h_it ) {
        const Group &h = h_it->first;
        if( g == h )
            continue;
        std::set< Variable* > intersection;
        std::set_intersection( g.begin(), g.end(), h.begin(), h.end(),
                               std::inserter( intersection, intersection.begin() ) );
        if( intersection.empty() )
            continue;
        //bool g_includes_h = std::includes( g.begin(), g.end(), h.begin(), h.end() );
        //bool h_includes_g = std::includes( h.begin(), h.end(), g.begin(), g.end() );
        bool g_includes_h = subset_relation[g_it->first].find( h_it->second ) != subset_relation[g_it->first].end();
        bool h_includes_g = subset_relation[h_it->second].find( g_it->first ) != subset_relation[h_it->second].end();
        if( ! g_includes_h && ! h_includes_g ) {
            /*std::set< Variable* >::iterator it;
            std::cout << "g: ";
            for( it = g.begin(); it != g.end(); ++ it )
                std::cout << (*it)->id << " ";
            std::cout << std::endl << "h: ";
            for( it = h.begin(); it != h.end(); ++ it )
                std::cout << (*it)->id << " ";
            std::cout << std::endl;*/
            return false;
        }
    }
    return true;
}

bool
Group_cache::is_maximum( Variable* maxvar, std::vector< Variable* >& other_max_vars ) {
    std::vector< Variable* >::iterator begin = other_max_vars.begin(),
                                       end   = other_max_vars.end();
    for( ; begin != end; ++begin ) {
        if( maxvar == *begin )
            continue;
        //std::cout << "varname: " << (*begin)->id << std::endl;
        if( subset_relation[maxvar].find( *begin ) == subset_relation[maxvar].end() )
            return false;
    }
    return true;
}

unsigned int
Group_cache::number_of_subsets( Variable* maxvar, std::vector< Variable* >& max_vars, std::set< Variable*> &ignore ) {
    std::vector< Variable* >::iterator begin = max_vars.begin(),
                                       end   = max_vars.end();
    unsigned int counter = 0;
    for( ; begin != end; ++begin ) {
        if( maxvar == *begin )
            continue;
        //std::cout << "varname: " << (*begin)->id << std::endl;
        if( subset_relation[maxvar].find( *begin ) != subset_relation[maxvar].end() ) {
            ignore.insert( *begin );
            ++counter;
        }
    }
    return counter;
}

Variable*
Group_cache::find_maximum( std::vector< Variable* >& max_vars, std::set< Variable*> &ignore ) {
    std::vector< Variable* >::iterator begin = max_vars.begin(),
                                       end   = max_vars.end();
    Variable *result = nullptr;
    std::set< Variable*> ignore_tmp;
    unsigned int max_subsets = 0;
    for( ; begin != end; ++begin ) {
        //std::cout << "varname: " << (*begin)->id << std::endl;
        ignore_tmp.clear();
        unsigned int n = number_of_subsets( *begin, max_vars, ignore_tmp );
        if( n > max_subsets ) {
            result = *begin;
            // not terribly efficient ...
            ignore = ignore_tmp;
            max_subsets = n;
        }
    }
    return result;
}


// ----------------------
struct Use_as_max_value : public Abstract_value {
    Use_as_max_value(bool is_fresh   = true,
                     bool use_as_max = false)
        : is_fresh_(is_fresh),
          use_as_max_(use_as_max)
    {}

    Use_as_max_value*
    downcast( Abstract_value* value ) {
        Use_as_max_value *v= dynamic_cast<Use_as_max_value*>( value );
        assert( v!= nullptr );
        return v;
    }
    virtual Use_as_max_value*
    get_initial_value( Variable *var ) { argused(var); return new Use_as_max_value( true, true ); }

    virtual Use_as_max_value*
    add( Abstract_value* other ) {
        if( downcast(other)->is_fresh() && is_fresh() ) {
            downcast(other)->use_as_max_ = false;
            use_as_max_ = false;
            return new Use_as_max_value( false, true );
        } /*else {
            if( downcast(other)->is_fresh() ) {
                assert(!is_fresh());
                downcast(other)->use_as_max_ = true;
            } else if( is_fresh() ) {
                assert(!downcast(other)->is_fresh());
                use_as_max_ = true;
            }
        }*/
        return new Use_as_max_value(false,false);
    }

    virtual Use_as_max_value*
    sub( Abstract_value* other ) {
        return add( other );
    }


    virtual Use_as_max_value*
    mul( Abstract_value* other ) {
        argused(other);
        return new Use_as_max_value(false,false);
    }

    //virtual void funcall( AST::Expression* funcall) { if( is_fresh() ) use_as_max_ = true; }

    virtual Use_as_max_value* div( Abstract_value* other )  { argused(other); CGAL_error(); return nullptr; }
    virtual Use_as_max_value* sqrt()                        { CGAL_error(); return nullptr; }
    virtual Use_as_max_value* join( Abstract_value* other ) { argused(other); return this; }

    virtual Use_as_max_value* clone() { return new Use_as_max_value(*this); }
    // returns true if it is equivalent to a default constructed abstract value
    virtual bool is_fresh() { return is_fresh_; }

    virtual std::ostream& dump( std::ostream& out ) {
        out << "use as max: " << use_as_max_;
        return out;
    }

    bool      is_fresh_;
    bool      use_as_max_;
};

// --------------------------------------------------------------

struct CSE_max
    : public Common_subexpression_elimination
{
    CSE_max( Add_bound_variables *add_bounds ) : add_bounds(add_bounds) {}

    virtual bool filter( AST::BinaryExpression *e ) {
        return add_bounds->use_as_max( e );
    }

    virtual void visit( AST::AssignmentExpression *e ) {
        if( add_bounds->use_as_max( e ) )
            Transformation_visitor::push( e );
        else
            Transformation_visitor::visit( e );
    }

    virtual void update( AST::Node *node_old, AST::Node *node_new ) {
        Transformation_visitor::update( node_old, node_new );
        AST::Expression *e_old = dynamic_cast< AST::Expression* >(node_old);
        AST::Expression *e_new = dynamic_cast< AST::Expression* >(node_new);
        if( e_old != nullptr )
            assert( e_new != nullptr );
        add_bounds->absint_max->update( e_old, e_new );
    }

    Add_bound_variables *add_bounds;
};

struct Predicate_use_as_max : public Expression_filter {
    Predicate_use_as_max( Add_bound_variables *add_bounds ) : add_bounds(add_bounds) {}
    virtual bool operator()( AST::Expression *e ) {
        return add_bounds->use_as_max( e );
    }
    Add_bound_variables *add_bounds;
};


Add_bound_variables::Add_bound_variables( bool use_aposteriori_check )
   : epsilon_tmp_variable(nullptr),
     absint_max(new Abstract_interpretation_visitor( new Use_as_max_value )),
     max_var_counter(0),
     use_aposteriori_check(use_aposteriori_check)
{
    Expression_filter *f = new Predicate_use_as_max( this );

    Abstract_value *sfe = new Static_filter_error( f );
    absint_sfe = new Abstract_interpretation_visitor( sfe );

    Abstract_value *giv = new Group_index_value( f );
    absint_giv = new Abstract_interpretation_visitor( giv );

    Abstract_value *is_fresh = new Error_bound_value();
    absint_is_fresh = new Abstract_interpretation_visitor( is_fresh );
}


bool
Add_bound_variables::use_as_max( AST::Expression* e) {
    MSG("")
    Use_as_max_value *uamv= dynamic_cast< Use_as_max_value* >( absint_max->get_analysis_result( e ) );
    assert( uamv != nullptr );
    return uamv->use_as_max_ ;
}



static bool
find_max_input( AST::FunctionDefinition *fundef, double &max_input ) {
    max_input = std::numeric_limits<double>::max();
    Abstract_interpretation_visitor
        *absint = new Abstract_interpretation_visitor(
                      new Static_filter_error(new Expression_filter)
                  );
    unsigned int phase = 0;
    unsigned int counter = 0;
    do {
        ::feclearexcept( FE_DIVBYZERO | FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID );
        //std::cout << "trying with max_input = " << max_input << " .... " << std::endl;
        absint->clear( new Static_filter_error( new Expression_filter, CGAL::Static_filter_error(max_input) ) );
        absint->visit( fundef );
        bool overflow = ::fetestexcept ( FE_DIVBYZERO | FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID );
        if(  overflow == (phase % 2) ) {
            ++phase;
            //std::cout << "next phase: " << phase << std::endl;
        }
        if( max_input < 100.0 )
            return false;
        switch( phase ) {
        case 0  : max_input  = std::sqrt( max_input ); break;
        case 1  : max_input *= std::sqrt( max_input ); break;
        default : max_input /= 2.0; break;
        }
        if( ++counter > 1000000000 )
            return false;
    } while( phase <= 2 );
    //std::cout << "success with max_input = " << max_input << std::endl;
    delete absint;
    return true;
}

void
Add_bound_variables::visit( AST::TranslationUnit* tu) {
    Compute_call_count                       compute_call_count;
    Rewrite_float_comparisons                rewrite_float_comp( this );
    Rewrite_sign_of_fresh_values             rewrite_fresh_sign( this );
    Contains_float_compare_on_derived_values has_float_comp( absint_is_fresh );
    Subst_funcall_filter                     subst_filter( this );
    Substitute_funcalls                      subst_funcalls( &subst_filter );
    AST::ListOfFunctions::iterator           it;
    AST::ListOfFunctions::reverse_iterator   r_it;
    subst_funcalls.collect_variables = collect_variables;

    for( r_it = tu->functions.rbegin(); r_it != tu->functions.rend(); ++r_it ) {
        collect_variables->clear();
        absint_is_fresh->clear();
        has_float_comp.reset();
        AST::FunctionDefinition *fundef = *r_it;
        if( !fundef->type->is_extern ) {
            fundef->accept( collect_variables );
            fundef->accept( absint_is_fresh );
            fundef->accept( &has_float_comp );
            if( !has_float_comp.result_value )
                continue;
            //std::cerr << "substituting function calls in " << fundef->type->id << std::endl;
            // subst'ion works one funcall at a time, starting from the "outside":
            // function calls that are made in the called function have to be replaced
            // subsequently. repeat until no change has been made
            //Prettyprint_visitor pretty( std::cout );
            //fundef->accept( &pretty );
            do {
                subst_funcalls.is_dirty = false;
                fundef->accept( &subst_funcalls );
                fundef->accept( absint_is_fresh );
                /*if( subst_funcalls.is_dirty ) {
                    std::cerr << " +++ iteration: " << std::endl;
                    fundef->accept( &pretty );
                }*/
            } while( subst_funcalls.is_dirty );
            //std::cerr << "finished substituting. rewriting fresh sign expressions:" << std::endl;
            fundef->accept( &rewrite_fresh_sign );
            //fundef->accept( &pretty );
        }
    }

    // here, each function only contains function calls without
    // derived float comparisons. this allows to compute max groups
    // in only that toplevel function
    for( it = tu->functions.begin(); it != tu->functions.end(); ++it ) {
        collect_variables->clear();
        group_cache.clear();
        max_var_counter = 0;
        epsilon_tmp_variable = nullptr;
        rewrite_float_comp.reset();
        absint_is_fresh->clear();
        absint_sfe->clear();
        absint_giv->clear();
        absint_max->clear();
        has_float_comp.reset();
        AST::FunctionDefinition *fundef = *it;

        if( !fundef->type->is_extern ) {
            fundef->accept( collect_variables );
            fundef->accept( absint_is_fresh );
            fundef->accept( &has_float_comp );
            if( !has_float_comp.result_value )
                continue;
            std::cerr << "analyzing " << fundef->type->id << std::endl;

            if( !use_aposteriori_check  && !find_max_input( fundef, rewrite_float_comp.max_input ) )
                CGAL_error_msg("failed to compute max_input for which no overflow occurs");
            // find max expressions
            absint_max->visit( fundef );
            // for all max expressions: eliminate duplicate binary expressions
            CSE_max cse( this );
            fundef->accept( &cse );
            absint_max->clear();
            absint_max->visit( fundef );
            // compute floating point errors
            absint_sfe->visit( fundef );
            // compute group indices
            MSG("compute group indices of " << fundef->type->id )
            absint_giv->visit( fundef );
            handle( fundef );
            group_cache.make_partial_order();
            fundef->accept( &rewrite_float_comp );
            fundef->type->return_type = filtered_predicate_return_type;
        }
    }
}


// -----------------


/*
case
===========
groups: 1-3
group_indices: 1*2*2*3
compute max1
compute max2
compute max3
min/max of max123

case
============
group 1-4
group_indices: 0*0*0*0
remaining groups: 1 - 4
compute max0 (use variables from group 1 - 4)
min/max = max0

case
============
groups: 1 - 5
group_indices: 0*0*1*1*2*4
compute max1
compute max2
compute max4
remaining groups: 3,5
compute max0 (use only those variables that are in group 3 and 5)
compute min/max of max0124

==========> approach:
g = group_indices
zeroes = number of 0 in g
g = remove 0 from g

for each i in g
    if not already computed i
        max_i = compute max value(i)
    "eps = eps * max_i"
if zeroes > 0
    g = groups_used_in_predicate - g
    if g' is empty
        maxvar = max_1
    else
        max_0 = compute max value(all i in g)
        maxvar = max_0
    "eps = eps * (maxvar ^ zeroes)"

*/

void
Add_bound_variables::visit( AST::UnaryFunction* uf ) {
    Generic_visitor::visit( uf );
    // SQRT and SIGN need special treatment in Rewrite_visitor
    if( uf->kind == AST::UnaryFunction::XSIGN ) {
        Variable *new_var = add_tmp_result( uf );
        Statement_addition_visitor::add_stmt( new AST::VariableDeclaration( new_var ) );
        if( dynamic_cast<AST::IdentifierExpression*>(uf->e) == nullptr ) {
            new_var = add_tmp_result( uf->e );
            Statement_addition_visitor::add_stmt( new AST::VariableDeclaration( new_var ) );
        }
        if( epsilon_tmp_variable == nullptr ) {
            epsilon_tmp_variable = add_variable( "eps", type_float_bound );
            Statement_addition_visitor::add_stmt_toplevel( new AST::VariableDeclaration( epsilon_tmp_variable ) );
        }

        Abstract_value *absval_giv = absint_giv->get_analysis_result( uf->e );
        Group_index_value *giv = dynamic_cast< Group_index_value* >( absval_giv );
        assert( giv != nullptr );
        assert( giv->group_item->degree() > 0 );

        Group_algebra::Group_varset groups;
        giv->group_item->collect_groups( groups );
        Group_algebra::Group_varset::iterator it;
        for( it = groups.begin(); it != groups.end(); ++it ) {
            const Group_cache::Group &group = it->second;
            Group_cache::iterator gcit = group_cache.groups.find( group );
            if( gcit == group_cache.end() ) {
                // we have to allocate a new max variable:
                std::stringstream varname;
                varname << "max" << ++max_var_counter;
                group_cache.add( group, add_variable( varname.str(), type_float ) );
                MSG( "adding max var " << varname.str() )
            }
        }
    }
}

void
Add_bound_variables::visit( AST::BinaryExpression* bexp ) {
    Generic_visitor::visit( bexp );
    MSG("")
    if( is_float_or_int( bexp->e1->getType() ) ||
        is_float_or_int( bexp->e2->getType() )   )
    {
        switch( bexp->kind ) {
        case AST::BinaryExpression::EQ:
        case AST::BinaryExpression::GEQ:
        case AST::BinaryExpression::LEQ:
        case AST::BinaryExpression::NEQ:
        case AST::BinaryExpression::LE:
        case AST::BinaryExpression::GR:
        case AST::BinaryExpression::AND:
        case AST::BinaryExpression::OR: {
            Abstract_value *absval_sfe = absint_sfe->get_analysis_result( bexp->e1 );
            Static_filter_error *sfe1 =  dynamic_cast< Static_filter_error* >( absval_sfe );
            assert( sfe1 != nullptr );
            absval_sfe = absint_sfe->get_analysis_result( bexp->e1 );
            Static_filter_error *sfe2 =  dynamic_cast< Static_filter_error* >( absval_sfe );
            assert( sfe2 != nullptr );
            if( !sfe1->is_fresh() || !sfe2->is_fresh() )
                throw RuntimeError( "comparison between derived values not yet supported. use 'sign' or 'compare' instead.", bexp->location );
            break;
        }
        default: ;
        }
    }
}


void
Rewrite_sign_of_fresh_values::visit( AST::UnaryFunction *uf ) {
    Transformation_visitor::transform<AST::Expression>( uf->e );
    MSG("")
    AST::Expression *result = uf;
    if( uf->kind == AST::UnaryFunction::XSIGN && is_float(uf->e->getType()) ) {
        AST::BinaryExpression      *bexp = dynamic_cast<AST::BinaryExpression*    >(uf->e);
        AST::IdentifierExpression *idexp = dynamic_cast<AST::IdentifierExpression*>(uf->e);

        if( bexp != nullptr )
        {
            Error_bound_value *is_fresh1 = dynamic_cast<Error_bound_value*>( add_bounds->absint_is_fresh->get_analysis_result( bexp->e1 ) );
            Error_bound_value *is_fresh2 = dynamic_cast<Error_bound_value*>( add_bounds->absint_is_fresh->get_analysis_result( bexp->e2 ) );
            assert( is_fresh1 != nullptr && is_fresh2 != nullptr );
            if( is_fresh1->is_fresh() && is_fresh2->is_fresh() && bexp->kind == AST::BinaryExpression::SUB ) {
                AST::Expression *cond_e =
                    new AST::ConditionalExpression(
                        new AST::BinaryExpression( bexp->e1, bexp->e2, AST::BinaryExpression::GR ),
                        new AST::LiteralExpression( 1 ),
                        new AST::ConditionalExpression(
                            new AST::BinaryExpression( bexp->e1, bexp->e2, AST::BinaryExpression::LE ),
                            new AST::LiteralExpression( -1 ),
                            new AST::LiteralExpression( 0 )
                        )
                    );
                result = cond_e;
            }
        } else if( idexp != nullptr ) {
            Error_bound_value *is_fresh = dynamic_cast<Error_bound_value*>( add_bounds->absint_is_fresh->get_analysis_result( idexp ) );
            assert( is_fresh != nullptr );
            if( is_fresh->is_fresh() ) {
                AST::Expression *cond_e =
                    new AST::ConditionalExpression(
                        new AST::BinaryExpression( idexp, new AST::LiteralExpression(0.0), AST::BinaryExpression::GR ),
                        new AST::LiteralExpression( 1 ),
                        new AST::ConditionalExpression(
                            new AST::BinaryExpression( idexp, new AST::LiteralExpression(0.0), AST::BinaryExpression::LE ),
                            new AST::LiteralExpression( -1 ),
                            new AST::LiteralExpression( 0 )
                        )
                    );
                result = cond_e;
            }
        }
    }
    Transformation_visitor::push( result );
}

void
Rewrite_float_comparisons::compute_max_from(
    Variable *maxvar,
    Variable *other,
    Statement_addition_visitor::Position scope,
    bool use_fabs )
{
    assert( maxvar != nullptr );
    assert( other  != nullptr );
    //MSG( "maxvar: " << maxvar->id )
    //MSG( "othervar: " << other->id )
    AST::Expression *e = new AST::IdentifierExpression(other);
    if( use_fabs )
        e = new AST::UnaryFunction( e, AST::UnaryFunction::XABS );
    /*
    std::map< Variable*, std::set< Variable* > >::iterator
        begin = add_bounds->group_cache.subset_relation.begin(),
        end   = add_bounds->group_cache.subset_relation.end();
    */
    if( max_var_cover.find( maxvar ) == max_var_cover.end() ) {
        add_stmt_with_scope( make_assign_stmt( maxvar, e ), scope );
    } else {
        AST::Expression *cond_e =
            new AST::BinaryExpression(
                new AST::IdentifierExpression(maxvar),
                e,
                AST::BinaryExpression::LE );
        AST::Statement *s = new AST::ConditionalStatement(
                cond_e,
                make_assign_stmt( maxvar, e ) );
        add_stmt_with_scope( s, scope );
    }
}



void
Rewrite_float_comparisons::make_max_computation( Variable *maxvar ) {
    MSG( "maxvar: " << maxvar->id )
    if( max_var_cover.find( maxvar ) == max_var_cover.end() ) {
        MSG( maxvar->id << " not yet computed!" )
        std::set<Variable*>::iterator m_begin = add_bounds->group_cache.subset_relation[maxvar].begin();
        std::set<Variable*>::iterator m_end   = add_bounds->group_cache.subset_relation[maxvar].end();
        std::set<Variable*>::iterator v_begin = add_bounds->group_cache.reverse_mapping[ maxvar ].begin();
        std::set<Variable*>::iterator v_end   = add_bounds->group_cache.reverse_mapping[ maxvar ].end();
        std::set< Variable* > vars, vars_tmp;
        //std::copy( m_begin, m_end, std::inserter( vars_tmp, vars_tmp.begin() ) );
        std::copy( v_begin, v_end, std::inserter( vars_tmp, vars_tmp.begin() ) );
        std::set_difference( vars_tmp.begin(), vars_tmp.end(),
                             function_parameters.begin(), function_parameters.end(),
                             std::inserter( vars, vars.begin() ) );
        Statement_addition_visitor::Position scope = get_scope( vars );
        add_stmt_toplevel( new AST::VariableDeclaration( maxvar ) );
        for( ; m_begin != m_end; ++m_begin ) {
            Variable *othervar = *m_begin;
            MSG("iterate: " << othervar->id )
            make_max_computation( othervar );
            compute_max_from( maxvar, othervar, scope, false );
            std::copy( max_var_cover[othervar].begin(), max_var_cover[othervar].end(),
                       std::inserter( max_var_cover[maxvar], max_var_cover[maxvar].begin() ) );
        }
        MSG( maxvar->id << " first iteration ready")
        assert( add_bounds->group_cache.reverse_mapping.find( maxvar ) != add_bounds->group_cache.reverse_mapping.end() );
        for( ; v_begin != v_end; ++v_begin ) {
            if( max_var_cover.find( maxvar ) == max_var_cover.end() ||
                max_var_cover[maxvar].find( *v_begin ) == max_var_cover[maxvar].end() )
            {
                MSG("var left: " << (*v_begin)->id )
                compute_max_from( maxvar, *v_begin, scope );
                max_var_cover[maxvar].insert( *v_begin );
            }
        }
        MSG("finished")
    }
}

void
Rewrite_float_comparisons::make_max_groups( Group_index_value * giv,
                                            double              min_input,
                                            AST::Expression   *&result_cond_underflow,
                                            //AST::Expression   *&result_cond_zero,
                                            AST::Expression   *&result_cond_overflow  )
{
    assert( giv != nullptr );
    assert( giv->group_item != nullptr );
    Group_algebra::Group_varset groups;
    giv->group_item->collect_groups( groups );
    Group_algebra::Group_varset::iterator it;
    // for each input variable degree, store the set of maxvars:
    std::map< unsigned int, std::set< Variable* > > maxvars_per_degree;
    for( it = groups.begin(); it != groups.end(); ++it ) {
        unsigned int                degree = it->first;
        const std::set< Variable* > &group = it->second;
        Group_cache::iterator gcit = add_bounds->group_cache.find( group );
        assert( gcit != add_bounds->group_cache.end() );
        Variable *var = gcit->second;
        MSG( "using max var " << var->id );
        make_max_computation( var );
        maxvars_per_degree[ degree ].insert( var );
    }
    if( add_bounds->use_aposteriori_check )
        return;
    std::map< unsigned int, std::set< Variable* > >::iterator it2;
    for( it2 = maxvars_per_degree.begin(); it2 != maxvars_per_degree.end(); ++it2 ) {
        unsigned int          degree  = it2->first;
        std::set< Variable* > maxvar_set = it2->second;
        std::vector< Variable* > maxvars;
        std::copy( maxvar_set.begin(), maxvar_set.end(), std::back_inserter( maxvars ) );
        Variable *lb_var = nullptr, *ub_var = nullptr;
        derive_min_max( degree, maxvars, lb_var, ub_var );

        // compute actual allowable min/max input
        /* for each degree:
            o maintain a seperate lower/upper bound variable.
            o "distribute" the bounds uniformly: for degree 2 input variables v2,
              it is assumed that bound(v2) = bound(v1)^2 so that we have an over/underflow
              if for one of the different degree's lb/ub variables, the actual input bound
              violates the precomputed bound. example:

              if( lb_1 < 1e-60 || lb_2 < 1e-120 )
                 UNDERFLOW!

              where lb_1 is a degree 1 lower bound and lb_2 a degree 2 lower bound.
              of course, other mechanisms are imaginable, but this one is easy and works.
        */
        CGAL::FPU_CW_t fpu_backup = CGAL::FPU_get_and_set_cw(CGAL_FE_UPWARD);
        double actual_min_input = std::pow( min_input, (int)degree );
        CGAL::FPU_set_cw(CGAL_FE_DOWNWARD);
        double actual_max_input = std::pow( max_input, (int)degree );
        CGAL::FPU_set_cw(fpu_backup);

        AST::Expression *cond_underflow =
            new AST::BinaryExpression(
                new AST::IdentifierExpression( lb_var ),
                new AST::LiteralExpression( actual_min_input ),
                AST::BinaryExpression::LE
            );
        if( result_cond_underflow == nullptr )
            result_cond_underflow = cond_underflow;
        else
            result_cond_underflow =
                new AST::BinaryExpression(
                    result_cond_underflow,
                    cond_underflow,
                    AST::BinaryExpression::OR
                );

        /*AST::Expression *cond_zero =
            new AST::BinaryExpression(
                new AST::IdentifierExpression( lb_var ),
                new AST::LiteralExpression( 0.0 ),
                AST::BinaryExpression::EQ
            );
        if( result_cond_zero == nullptr )
            result_cond_zero = cond_zero;
        else
            result_cond_zero =
                new AST::BinaryExpression(
                    result_cond_zero,
                    cond_zero,
                    AST::BinaryExpression::AND
                );*/

        AST::Expression *cond_overflow =
            new AST::BinaryExpression(
                new AST::IdentifierExpression( ub_var ),
                new AST::LiteralExpression( actual_max_input ),
                AST::BinaryExpression::GR
            );
        if( result_cond_overflow == nullptr )
            result_cond_overflow = cond_overflow;
        else
            result_cond_overflow =
                new AST::BinaryExpression(
                    result_cond_overflow,
                    cond_overflow,
                    AST::BinaryExpression::OR
                );
    }
}

void
Rewrite_float_comparisons::derive_min_max( unsigned int degree,
                                           std::vector< Variable* > &max_variables,
                                           Variable *& lb_var,
                                           Variable *& ub_var )
{
    std::vector< Variable* >::iterator var_it,
                                       begin = max_variables.begin(),
                                       end   = max_variables.end();
    std::set< Variable*> max_ignore;
    if( max_variables.size() == 1 ) {
        ub_var = lb_var = max_variables.front();
        return;
    } else
        add_bounds->group_cache.find_maximum( max_variables, max_ignore );

    if( lb_vars.find( degree ) == lb_vars.end() ) {
        std::stringstream lb_varname, ub_varname;
        lb_varname << "lower_bound_" << degree;
        ub_varname << "upper_bound_" << degree;
        lb_var = lb_vars[degree] = add_bounds->add_variable( lb_varname.str(), type_float_bound );
        ub_var = ub_vars[degree] = add_bounds->add_variable( ub_varname.str(), type_float_bound );
        add_stmt_toplevel( new AST::VariableDeclaration( lb_var ) );
        add_stmt_toplevel( new AST::VariableDeclaration( ub_var ) );
    } else {
        lb_var = lb_vars[degree];
        ub_var = ub_vars[degree];
    }


    bool first = true;
    for( ; begin != end; ++begin ) {
        Variable *max_var = *begin;
        AST::Expression *id_max_var = new AST::IdentifierExpression( max_var );
        AST::Expression *id_lb_var = new AST::IdentifierExpression( lb_var );
        AST::Expression *id_ub_var = new AST::IdentifierExpression( ub_var );
        if( first ) {
            add_stmt( make_assign_stmt( lb_var, id_max_var ) );
            add_stmt( make_assign_stmt( ub_var, id_max_var ) );
            first = false;
        } else {
            AST::Expression *cond_max_gt_ub =
                new AST::BinaryExpression(
                    id_max_var,
                    id_ub_var,
                    AST::BinaryExpression::GR
                );
            AST::Expression *cond_max_lt_lb =
                new AST::BinaryExpression(
                    id_max_var,
                    id_lb_var,
                    AST::BinaryExpression::LE
                );

            bool ub_ignore = max_ignore.find( max_var ) != max_ignore.end();
            add_stmt(
                new AST::ConditionalStatement(
                    cond_max_lt_lb,
                    make_assign_stmt( lb_var, id_max_var ),
                    ub_ignore ? nullptr
                              : new AST::ConditionalStatement(
                                  cond_max_gt_ub,
                                  make_assign_stmt( ub_var, id_max_var ) )
                )
            );
        }
    }
}

AST::Expression*
Rewrite_float_comparisons::make_max_term( Group_algebra::Group_item *item ) {
    Group_algebra::Leaf_item *leaf = dynamic_cast<Group_algebra::Leaf_item*>(item);
    if( leaf != nullptr ) {
        Group_cache::iterator it = add_bounds->group_cache.find( leaf->variables );
        assert( it != add_bounds->group_cache.end() );
        return new AST::IdentifierExpression( it->second );
    }
    Group_algebra::Array_item *array = dynamic_cast<Group_algebra::Array_item*>(item);
    bool is_sum  = dynamic_cast<Group_algebra:: Sum_item*>(item) != nullptr;
    argused(is_sum);
    bool is_prod = dynamic_cast<Group_algebra::Product_item*>(item) != nullptr;
    assert( is_sum || is_prod  );
    assert( array->items.size() >= 2 );
    std::vector< Group_algebra::Group_item* >::iterator it = array->items.begin();
    AST::Expression *result = make_max_term( *it++ );
    do {
        AST::Expression *e2 = make_max_term( *it++ );
        if( is_prod ) result = new AST::BinaryExpression( result, e2, AST::BinaryExpression::MUL );
        else          result = AST::make_funcall( fun_std_max, result, e2 );
    } while( it != array->items.end() );
    return result;
}

// -----------------------------------

void
Rewrite_float_comparisons::visit( AST::FunctionDefinition * fundef ) {
    function_parameters.clear();
    std::copy( fundef->type->parameters.begin(), fundef->type->parameters.end(),
               std::inserter( function_parameters, function_parameters.begin() ) );
    Transformation_visitor::visit( fundef );
}

void
Rewrite_float_comparisons::visit( AST::UnaryFunction *uf ) {
    Transformation_visitor::transform<AST::Expression>( uf->e );

    if( uf->kind == AST::UnaryFunction::XSIGN && is_float(uf->e->getType()) ) {
        Abstract_value *absval_sfe = add_bounds->absint_sfe->get_analysis_result( uf->e );
        Static_filter_error *sfe =  dynamic_cast< Static_filter_error* >( absval_sfe );
        assert( sfe != nullptr );

        Abstract_value *absval_giv = add_bounds->absint_giv->get_analysis_result( uf->e );
        Group_index_value *giv = dynamic_cast< Group_index_value* >( absval_giv );
        assert( giv != nullptr );
        assert( giv->group_item != nullptr );

        Variable *tmp_result_var = add_bounds->tmp_result_variable( uf );
        Variable *tmp_arg_var = nullptr;
        if( add_bounds->has_tmp_result_variable( uf->e ) ) {
            tmp_arg_var = add_bounds->tmp_result_variable( uf->e );
            add_stmt( make_assign_stmt( tmp_arg_var, uf->e ) );
        } else {
            AST::IdentifierExpression *idexp = dynamic_cast<AST::IdentifierExpression*>(uf->e);
            assert( idexp != nullptr );
            tmp_arg_var = idexp->var;
        }
        assert( tmp_arg_var != nullptr );
        assert( tmp_result_var != nullptr );
        //assert( add_bounds->max_result_variables.size() > 0 );
        // correction due to the eps computation. for each multiplication, we need to add 'error*ulp'
        unsigned int degree = giv->group_item->degree();
        assert( degree > 0 );
        CGAL::FPU_CW_t fpu_backup = CGAL::FPU_get_and_set_cw(CGAL_FE_UPWARD);
        double epsilon = sfe->error().error() +
                         sfe->error().error() * CGAL::Static_filter_error::ulp() * (degree-1);
        double min_input = std::pow( std::numeric_limits<double>::min() / epsilon,
                                     1.0 / degree );
        double check_underflow = min_input;
        for( unsigned int i = 1; i < degree; ++i )
            check_underflow *= min_input;
        CGAL::FPU_set_cw(fpu_backup);
        assert( check_underflow > std::numeric_limits<double>::min() );
        //std::cout << "epsilon: " << epsilon << std::endl;
        AST::Expression *eps = new AST::LiteralExpression( epsilon );

        eps = new AST::BinaryExpression( eps, make_max_term( giv->group_item ), AST::BinaryExpression::MUL );
        //giv->dump(std::cout);
        //uf->e->dump(0);
        AST::Expression *cond_underflow = nullptr, /* *cond_zero = nullptr, */ *cond_overflow = nullptr;
        make_max_groups( giv, min_input, cond_underflow, /*cond_zero, */cond_overflow );

        static AST::Expression *plus_one  = new AST::LiteralExpression(  +1);
        static AST::Expression *minus_one = new AST::LiteralExpression(  -1);

        AST::Expression *cond_det_gt_eps =
            new AST::BinaryExpression(
                new AST::IdentifierExpression( tmp_arg_var ),
                new AST::IdentifierExpression( add_bounds->epsilon_tmp_variable ),
                AST::BinaryExpression::GR
            );
        AST::Expression *cond_det_lt_minus_eps =
            new AST::BinaryExpression(
                new AST::IdentifierExpression( tmp_arg_var ),
                new AST::UnaryExpression(
                    new AST::IdentifierExpression( add_bounds->epsilon_tmp_variable ),
                    AST::UnaryExpression::NEG
                ),
                AST::BinaryExpression::LE
            );

        // *** prepare for later use:
        AST::StatementList *compare_with_eps = new AST::StatementList;
        // check for possible overflow in computation of uf->e, and just return if unsure
        if( !add_bounds->use_aposteriori_check ) {
            compare_with_eps->add(
                new AST::ConditionalStatement(
                    cond_overflow,
                    new AST::Return( AST::uncertain_return_value )
                )
            );
        }

        // compute eps
        compare_with_eps->add( make_assign_stmt( add_bounds->epsilon_tmp_variable, eps ) );

        // compare +/- eps with uf->e, returning if unsure
        compare_with_eps->add(
            new AST::ConditionalStatement(
                cond_det_gt_eps,
                make_assign_stmt( tmp_result_var, plus_one ),
                new AST::ConditionalStatement(
                    cond_det_lt_minus_eps,
                    make_assign_stmt( tmp_result_var, minus_one ),
                    new AST::Return( AST::uncertain_return_value )
                )
            )
        );


        if( add_bounds->use_aposteriori_check ) {
            compare_with_eps->add(
                new AST::ConditionalStatement(
                    make_fe_condition(),
                    make_fe_clear_and_fail()
                )
            );
            add_stmt( compare_with_eps );
        } else {
            // check for underflow/exact zero:
            add_stmt(
                new AST::ConditionalStatement(
                    cond_underflow,
                    //new AST::ConditionalStatement(
                    //    cond_zero,
                    //    make_assign_stmt( tmp_result_var, new AST::LiteralExpression(0) ),
                        new AST::Return( AST::uncertain_return_value )
                    //)
                    ,
                    // else perform steps described above (see ***)
                    compare_with_eps
                )
            );
        }

        Transformation_visitor::push( new AST::IdentifierExpression( tmp_result_var ) );
    } else
        Transformation_visitor::push( uf );
}

void
Rewrite_float_comparisons::reset() {
    max_var_cover.clear();
    lb_vars.clear();
    ub_vars.clear();
}


