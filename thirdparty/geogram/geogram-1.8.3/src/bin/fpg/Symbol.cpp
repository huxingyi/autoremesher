#include <FPG/Symbol.h>
#include <FPG/SymbolEnvironment.h>
#include <FPG/Error.h>
#include <FPG/AST.h>

#include <sstream>
#include <cassert>

#define DEBUG
#include <FPG/MSG.h>

Type *type_int   = new BaseType("int",    BaseType::INTTYPE   );
Type *type_bool  = new AliasType("bool",  type_int            );
Type *type_float = new BaseType("double", BaseType::FLOATTYPE );
Type *type_void  = new BaseType("void",   BaseType::VOIDTYPE  );

Type *type_float_bound     = new BaseType("double", BaseType::FLOATTYPE );
Type *type_float_bound_ptr = new PointerType( type_float_bound );

Type *filtered_predicate_return_type = new BaseType("int", BaseType::INTTYPE );

FunctionType *fun_std_max, *fun_std_min, *fun_std_swap;

namespace {
struct Funtype_init {
    Funtype_init() {
        fun_std_min  = new FunctionType( "std_min", type_float );
        fun_std_max  = new FunctionType( "std_max", type_float );
        fun_std_swap = new FunctionType( "std_swap", type_void );
        fun_std_max->addParameter( "a", type_float );
        fun_std_max->addParameter( "b", type_float );
        fun_std_max->is_extern = true;
        fun_std_swap->addParameter( "a", type_float );
        fun_std_swap->addParameter( "b", type_float );
        fun_std_max->is_extern = true;
    }
};

Funtype_init bla;
}

bool is_float( Type *t ) {
    return t->isEqual(type_float);
}

bool is_int( Type *t ) {
    return t->isEqual(type_int);
}

bool is_float_or_int( Type *t ) {
    return is_float(t) || is_int(t);
}

bool is_bound( Type *t ) {
    return t == type_float_bound || t == type_float_bound_ptr;
}

bool
Type::isEqual( Type *target_type ) const {
    throw InternalError( name() + " is not comparable to " + target_type->name() );
}


FunctionType::FunctionType( const FunctionType& other )
    : Type( other )
{
    // we dont clone here
    return_type = other.return_type;
    is_inline = other.is_inline;
    // but, we have to clone variables because data may be attached to them via maps
    for( ParameterList::const_iterator it  = other.parameters.begin();
                                       it != other.parameters.end(); ++it )
    {
        const Variable *var = *it;
        Variable *new_var = new Variable( var->id, var->type );
        addParameter( new_var );
    }
}

void
FunctionType::addParameter( Variable *var )
{
    assert( var != nullptr );
    parameters.push_back( var );
}


unsigned int
FunctionType::non_bound_arity() {
    unsigned int non_bound_args = 0;
    ParameterList::iterator param_iter = parameters.begin();
    // only count original parameters (not the added bound parameters) as float params
    for( ; param_iter != parameters.end();
         ++param_iter )
    {
        Variable *var = *param_iter;
        if( !is_bound( var->type ) )
            ++non_bound_args;
    }
    return non_bound_args;
}


bool
FunctionType::isConvertible( Type *target_type ) const
{
    argused(target_type);
    throw InternalError("function types cannot be compared");
}

std::string
FunctionType::name() const {
    std::string inline_option;
    //if( is_inline )
        inline_option = "inline ";
    return inline_option + return_type->name() + " " + id;
}

bool
BaseType::isConvertible( Type *target_type ) const
{
    BaseType   *base_type = dynamic_cast< BaseType * >(target_type);
    AliasType *alias_type = dynamic_cast< AliasType* >(target_type);
    if( base_type )
        return (this == type_void) == (target_type == type_void);
    else if( alias_type )
        return isConvertible( alias_type->base_type );
    else
        return false;
}

bool
BaseType::isEqual( Type *target_type ) const {
    return this == target_type;
}

bool
PointerType::isConvertible( Type *target_type ) const {
    PointerType *pt = dynamic_cast< PointerType* >( target_type );
    if( pt != nullptr )
        return base_type->isConvertible( pt->base_type );
    else
        return false;
}

bool
PointerType::isEqual( Type *target_type ) const {
    PointerType *pt = dynamic_cast< PointerType* >( target_type );
    if( pt != nullptr )
        return base_type->isEqual( pt->base_type );
    else
        return false;
}

bool
AliasType::isConvertible( Type *target_type ) const {
    return base_type->isConvertible( target_type );
}

bool
AliasType::isEqual( Type *target_type ) const {
    return base_type->isEqual( target_type );
}


bool
EnumType::isConvertible( Type *target_type ) const {
    return dynamic_cast<EnumType*>(target_type)
        || is_int(target_type);
}

bool
EnumType::isEqual ( Type *target_type ) const {
    return target_type == this;
}

int
EnumType::add_value(const std::string& name, int i ) {
    EnumValues::iterator it = values.find(name);
    counter = i + 1;
    if( it == values.end() )
        return i;
    else
        throw ParseError("redefinition of enum value" /*, AST::current_location */ );
}

int
EnumType::add_value(const std::string& name) {
    return add_value( name, counter );
}
