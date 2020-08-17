#include <FPG/Declarator.h>
#include <FPG/Error.h>

Type*
IdentifierDeclarator::declare( Type *base_type, std::string &id ) const {
    id = this->id;

    return base_type;
}

Type*
FunctionDeclarator::declare( Type *base_type, std::string &id ) const {
    // decl should be some form of IdentifierDeclarator. type not needed.
    decl->declare( nullptr, id );

    // prepare function type
    FunctionType *fun = new FunctionType( id, base_type );

    if( params ) {
        for( ParameterDeclList::iterator it = params->begin(); it != params->end(); ++it ) {
            Type *type = (*it)->first;
            Declarator *declarator = (*it)->second;
            std::string name;
            // construct type
            type = declarator->declare( type, name );
            fun->addParameter( name, type );
        }
    }
    return fun;
}
