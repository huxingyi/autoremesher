#ifndef SYMBOL_ENVIRONMENT_H
#define SYMBOL_ENVIRONMENT_H

#include <string>
#include <vector>
//#include <ext/hash_map>
#include <map>

#include <FPG/Symbol.h>

namespace AST {
  struct FunctionDefinition;
  struct Node;
}

struct Collect_variables;


struct SymbolEnvironment {
    SymbolEnvironment();

    // corresponds to compound blocks.
    void enterBlock();
    void leaveBlock();

    // add item to current block.
    void add( Variable* var );
    void add( Type* type );
    // only allowed on global level <=> env.size() == 1
    void add( FunctionType* function );
    void add_fun_def( AST::FunctionDefinition* fun_def );

    // only when leaving a block, symbols are removed. so, no need for remove methods.

    // if symbol not found, nullptr is returned
    Variable* findVariable( const std::string& id );
    // if an alias type is registered that refers to t, t itself is returned (not the alias)
    // alias types cannot refer to another alias type, so the type returned here is a proper type
    Type*  findType( const std::string& id );
    FunctionType* findFunction( const std::string& id, unsigned int arity );
    // returns true if there is at least one fun for id
    bool hasFunction( const std::string& id );
    AST::FunctionDefinition* findFunctionDef( const std::string& id, unsigned int arity );
    AST::FunctionDefinition* findFunctionDef( FunctionType *fun_type );
    int numberOfBlocks() const { return int(env.size()); }

    Block* getCurrentBlock() ;

    // in AST::FunctionCall* funcall, set called_function to the AST::FunctionDefinition* under given
    // symbol environment
    void resolve_function_calls( AST::Node *n  );
    std::string make_fresh_function_name( const std::string& prefix );
    void clear();
    // needed for function type declarations, where the variable names can be different
    // from the actual function definition. overwritten, as soon the definition is seen
    void replace_function_type( FunctionType* t );

    void rename_function( FunctionType* t, std::string new_id );
protected:
    std::vector< Block* > env;
    Functions functions;

    typedef std::map< FunctionType*, AST::FunctionDefinition*> Function_definitions;
    Function_definitions function_definitions;
};

extern SymbolEnvironment symbol_env;
extern std::string random_identifier( unsigned int size = 7 );

#endif
