#ifndef MISC_VISITORS_H
#define MISC_VISITORS_H

#include <list>
#include <map>
#include <set>

#include <FPG/Generic_visitor.h>
#include <FPG/Attribute_visitor.h>

struct Contains_function_call :
    public Attribute_visitor<bool>
{
    // this is by definition intra-procedural: as soon as we see a funcall,
    // we can stop traversing the AST
    Contains_function_call();

    virtual bool combine( bool a, bool b );
    virtual void visit( AST::FunctionCall* );
};

bool
contains_function_call( AST::Node *n );
// ---------------------------

struct Contains_floatingpoint_comparison :
    public Attribute_visitor<bool>
{
    Contains_floatingpoint_comparison( bool interprocedural = true );
    virtual bool combine( bool a, bool b );
    virtual void visit( AST::BinaryExpression* );
    virtual void visit( AST::UnaryFunction* );
};

bool
contains_floatingpoint_comparison( AST::Node *n, bool interprocedural = true );

// label those functions that need to be filtered ( which is currently
// equivalent to "contains_floatingpoint_comparison" )
struct Label_filtered_functions :
    public Generic_visitor
{
    virtual void visit( AST::FunctionDefinition* );
    // useful during make_unique:
    void add( AST::FunctionDefinition* fundef );
    bool contains( AST::FunctionDefinition* fundef );
    void clear() { filtered.clear(); }
protected:
    std::set< AST::FunctionDefinition* > filtered;
};

struct Collect_function_calls :
    public Generic_visitor
{
    Collect_function_calls( bool interprocedural = true );
    virtual void visit( AST::FunctionCall* );

    void clear();
    // to avoid duplicates in the list, efficiently
    std::set < AST::FunctionCall* > set_of_funcalls;
    // f appears before g <==> f is called by g
    std::list< AST::FunctionCall* > funcalls;
};

struct Collect_variables :
    public Generic_visitor
{
    // intraprocedural only
    Collect_variables();
    virtual void visit( AST::VariableDeclaration* );
    virtual void visit( AST::FunctionDefinition* );

    void add( Variable *var );
    bool contains( Variable *var );
    bool contains( const std::string& varname );
    unsigned int number_of_variable_names();
    void clear();

    // postcond: fresh name does not collide with global function names + local variable names
    std::string make_fresh_variable_name( const std::string& prefix );


    std::set< Variable*   > variables;
    std::set< std::string > variable_names;
};

// compute, how many different call-sites there are, per function definition
// TODO: actually, we are only interested in the number of function_definitions
// that have a different set of abstract values, which might be lower than the number
// of call-sites. leave it as an optimization task for later
struct Compute_call_count : public Generic_visitor
{
    typedef std::map< AST::FunctionDefinition*, unsigned int > Callcount_map;
    Callcount_map callcount;
    unsigned int initial_value;

    Compute_call_count( unsigned int initial_value = 0 );
    virtual void visit( AST::FunctionCall* );
};


#endif
