#ifndef DECLARATOR_H
#define DECLARATOR_H

#include <string>
#include <vector>
#include <algorithm> // for pair

#include <FPG/Symbol.h>
#include <FPG/AST.h>

// a declarator is a specification, how to build up a type,
// using some base type. the identifier is set while traversing the tree,
// if an identifier-declarator is encountered. this name can be used to
//   * declare a variable of that type
//   * define a type alias

struct Declarator  {
    virtual Type* declare( Type *base_type, std::string &identifier ) const = 0;
    virtual ~Declarator() {};

  /*it may be necessary to initialize a variable
    as structs may also be initialized this
    initialization may not be entirely describable
    with an expression */
    AST::Statement* initializer;
};

// leaf node
struct IdentifierDeclarator : public Declarator  {
    IdentifierDeclarator( const std::string &id ) : id( id ) {}
    Type* declare( Type *base_type, std::string &id ) const;
    const std::string id;
};

struct FunctionDeclarator : public Declarator {
    typedef std::pair< Type*, Declarator* > ParameterDecl;
    typedef std::vector< ParameterDecl* >   ParameterDeclList;

    FunctionDeclarator( Declarator *decl, ParameterDeclList *params = nullptr ) : decl(decl), params(params) {}
    Type* declare( Type *base_type, std::string &id ) const;
    Declarator *decl;
    ParameterDeclList *params;
};



#endif
