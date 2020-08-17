#include <FPG/SymbolEnvironment.h>
#include <FPG/AST.h>

#include <FPG/Error.h>
#include <FPG/Generic_visitor.h>
#include <FPG/Misc_visitors.h>

#include <cassert>
#include <sstream>
#include <cmath>
#include <malloc.h>
#include <stdlib.h>

//#define DEBUG 0
#include <FPG/MSG.h>

SymbolEnvironment symbol_env;

// env[0] is global environment.
SymbolEnvironment::SymbolEnvironment()
{
    enterBlock();
}

Block*
SymbolEnvironment::getCurrentBlock() {
    if(env.size()<1)
        throw InternalError("environment unexpectedly empty when attempting to get fields for struct");
    return env.back();
}

void
SymbolEnvironment::enterBlock( ) {
    MSG("+block");
    env.push_back( new Block() );
}

void
SymbolEnvironment::leaveBlock() {
    MSG("-block");
    assert( env.size() > 1 );
    Block *b = getCurrentBlock();
    env.pop_back();
    delete b;
}

void
SymbolEnvironment::add( Variable* var ) {
    assert( env.size() > 0 );
    Variables &vars = env.back()->vars;
    if( vars.find( var->id ) != vars.end() )
        throw ParseError( "variable " + (var->id) + " already defined!", AST::current_location );
    vars[var->id] = var;

    MSG( "adding new variable " << var->id << " to block " << (env.size()-1) );
}


void
SymbolEnvironment::add( Type* type ) {
    assert( env.size() > 0 );
    Types &types = env.back()->types;
    if( types.find( type->id ) != types.end() )
        throw ParseError( "type " + (type->id) + " already defined" );
    MSG( "adding type: " << type->id << " " );
    env.back()->types[type->id] = type;
}

void
SymbolEnvironment::add( FunctionType* function ) {
    MSG( function->id );
    assert( env.size() == 1 );
    if( findFunction( function->id, function->non_bound_arity() ) != nullptr )
        throw ParseError( "function " + (function->id) + " already declared!", AST::current_location );
    functions[function->id].push_back( function );
}

void
SymbolEnvironment::replace_function_type( FunctionType* t ) {
    assert( env.size() == 1 );
    Functions::iterator it = functions.find( t->id );
    if( it != functions.end() ) {
        std::list<FunctionType*>::iterator it2 = it->second.begin();
        for( ; it2 != it->second.end(); ++it2 ) {
            if( (*it2)->non_bound_arity() == t->non_bound_arity() ) {
                *it2 = t;
                return;
            }
        }
    }
    add( t );
}

void
SymbolEnvironment::add_fun_def( AST::FunctionDefinition* fun_def ) {
    MSG( fun_def->type->id );
    assert( env.size() == 1 );
    std::string id = fun_def->type->id;
    unsigned int arity = fun_def->type->non_bound_arity();
    if( findFunctionDef( id, arity ) != nullptr )
        throw ParseError( "function " + (id) + " already defined!", AST::current_location );
    function_definitions[ fun_def->type ] = fun_def;
}


Variable*
SymbolEnvironment::findVariable( const std::string& id )
{
    MSG("var access " << id );
    assert( env.size() > 0 );
    for( int block = int(env.size()) - 1; block >= 0; --block ) {
        //MSG( "looking in block " << block );
        Variable* var = env[size_t(block)]->findVariable( id );
    if(var)
       return var;
    }
    return nullptr;
}

Type*
SymbolEnvironment::findType( const std::string& id ) {
    assert( env.size() > 0 );
    for( int block = int(env.size()) - 1; block >= 0; --block ) {
        //MSG( "looking in block " << block );
        Types::iterator it = env[size_t(block)]->types.find( id );
        if( it != env[size_t(block)]->types.end() )
            return it->second;
    }
    return nullptr;
}

FunctionType*
SymbolEnvironment::findFunction( const std::string& id, unsigned int arity ) {
    assert( env.size() > 0 );
    Functions::iterator it = functions.find( id );
    if( it != functions.end() ) {
        std::list<FunctionType*>::iterator it2 = it->second.begin();
        for( ; it2 != it->second.end(); ++it2 ) {
            if( (*it2)->non_bound_arity() == arity )
                return *it2;
        }
    }
    return nullptr;
}

bool
SymbolEnvironment::hasFunction( const std::string& id ) {
    return( functions.find( id ) != functions.end() );
}


AST::FunctionDefinition*
SymbolEnvironment::findFunctionDef( const std::string& id, unsigned int arity ) {
    FunctionType *t = findFunction( id, arity );
    if( t != nullptr )
        return function_definitions[t];
    return nullptr;
}

AST::FunctionDefinition*
SymbolEnvironment::findFunctionDef( FunctionType *fun_type ) {
    return function_definitions[fun_type];
}

void
SymbolEnvironment::rename_function( FunctionType* t, std::string new_id ) {
    Functions::iterator it = functions.find( t->id );
    assert( it != functions.end() );
    it->second.remove( t );
    t->id = new_id;
    add( t );
}

namespace {
struct Resolve_function_calls : public Generic_visitor {
    Resolve_function_calls( SymbolEnvironment& env )
       : env(env)
    {}

    virtual void visit( AST::FunctionCall* funcall ) {
        MSG(funcall->fun_type->id)
        // handle function arguments also:
        Generic_visitor::visit( funcall );
        /*if( funcall->fun_type == nullptr ) {
            FunctionType *fun_type = env.findFunction( funcall->id, funcall->exp_list->size() );
            if( fun_type == nullptr ) {
                std::stringstream errmsg;
                errmsg << "no function declaration found for " << funcall->id << " and arity " << funcall->exp_list->size();
                throw RuntimeError( errmsg.str(), funcall->location );
            }
            funcall->fun_type = fun_type;
        }*/

        if( funcall->fun_type->is_extern )
            funcall->called_function = nullptr;
        else {
            AST::FunctionDefinition *fun_def = env.findFunctionDef( funcall->fun_type );
            if( fun_def == nullptr ) {
                std::stringstream errmsg;
                errmsg << "no definition found for " << funcall->fun_type->id << " and arity " << funcall->exp_list->size();
                throw RuntimeError( errmsg.str(), funcall->location );
            }
            funcall->called_function = fun_def;
        }
    }

    SymbolEnvironment &env;
};
}

void
SymbolEnvironment::resolve_function_calls( AST::Node *n ) {
    Resolve_function_calls v( *this );
    n->accept( &v );
}

std::string
SymbolEnvironment::make_fresh_function_name( const std::string& prefix ) {
    std::string suffix;
    // NOTE: this should quickly terminate ... 60^7 is a fairly large number!
    // use a while loop: maybe, we dont even need the suffix in some cases
    while( hasFunction( prefix + suffix ) ) {
        suffix = "_" + random_identifier();
    }
    return prefix + suffix;
}

void
SymbolEnvironment::clear() {
    assert( env.size() == 1 );
    env.back()->vars.clear();
    env.back()->types.clear();
    function_definitions.clear();
    functions.clear();
}

std::string
random_identifier( unsigned int size ) {
    char *result = static_cast<char*>(malloc((size+3)/4 + 1));
    assert( result != nullptr );
    const char pool[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghjklmnopqrstuvwxyz0123456789";
    result[size] = 0;
    while( size > 0 ) {
        // prevent rounding towards nearest => floor
        unsigned int i = (unsigned int)(std::floor(drand48() * sizeof(pool)));
        assert( i < sizeof(pool) );
        --size;
        result[size] = pool[i];
    }
    std::string result_string( result );
    free( result );
    return result_string;
}
