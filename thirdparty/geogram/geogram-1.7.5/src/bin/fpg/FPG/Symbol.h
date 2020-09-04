#ifndef SYMBOL_H
#define SYMBOL_H

#include <FPG/Error.h>
#include <map>
#include <string>
#include <list>
#include <vector>
#include <set>


struct Variable;
struct Type;
struct FunctionType;

struct SymbolEnvironment;

typedef std::map< const std::string, Variable* >     Variables;
typedef std::map< const std::string, Type* >         Types;
typedef std::map< const std::string, std::list<FunctionType*> > Functions;

struct Block {
    Block() {}
    Variables    vars;
    Types        types;

    Variable* findVariable(const std::string& id) {
        Variables::iterator it = vars.find( id );
        if( it != vars.end() )
            return it->second;
        else return nullptr;
    }
};



struct Symbol {
    Symbol() {}
    Symbol( const std::string &id ) : id( id ) {}
    std::string id;
};

struct Type : public Symbol {
    Type( const std::string &id, std::size_t size = 1 )
      : Symbol( id ),size(size) {}
    virtual ~Type() {}

    virtual bool isConvertible( Type *target_type ) const = 0;
    virtual bool isEqual( Type *target_type ) const;
    // returns size in stack cells, that would be required if a variable of this type
    // is stored. default: 1 cell
    virtual std::size_t sizeOf()  { return size; }

    virtual std::string name() const { return id; }

protected:
    std::size_t size;
};


struct Variable : public Symbol {
    Variable( const std::string &id, Type *type )
       : Symbol( id ),
         type( type ),
         is_constant(false),
         value(0),
         group_index(0),
         degree(1)
    {}

    void make_constant(int value) {
        this->value = value;
        is_constant = true;
    }

    Type *type;
    bool is_constant;
    int value;
    unsigned int group_index;
    unsigned int degree;
};

struct FunctionType: public Type {
    typedef std::list< Variable* > ParameterList;

    FunctionType( const std::string &id, Type *return_type )
      : Type(id), return_type(return_type), is_inline(false), is_extern(false)
    {}

    FunctionType( const FunctionType& other );
    void addParameter( const std::string &name, Type *type )
    { addParameter( new Variable( name, type ) );  }
    void addParameter( Variable *var );
    std::string name() const;
    int numberOfParameters() const { return int(parameters.size()); }
    Type *getReturnType() const { return return_type; }
    virtual bool isConvertible( Type *target_type ) const;
    // only count non-bonud types
    unsigned int non_bound_arity();

    Type *return_type;
    ParameterList parameters;
    bool is_inline;
    bool is_extern;

};

struct BaseType : public Type {
    enum Kind { INTTYPE, FLOATTYPE, VOIDTYPE };
    BaseType( const std::string &id, Kind kind, std::size_t size = 1 )
       : Type( id, size ), kind( kind ) {}
    Kind kind;

    virtual bool isConvertible( Type *target_type ) const;
    virtual bool isEqual( Type *target_type ) const;
};

struct PointerType : public Type {
    Type *base_type;

    PointerType( Type *base_type )
      : Type(base_type->name()), base_type( base_type )
    {}

    virtual bool isConvertible( Type *target_type ) const;
    virtual bool isEqual( Type *target_type ) const;
    virtual std::string name() const { return id + "*"; }
};

struct AliasType : public Type {
    Type *base_type;

    AliasType( std::string name, Type *base_type)
      : Type(name), base_type( base_type )
    {}

    virtual bool isConvertible( Type *target_type ) const;
    virtual bool isEqual( Type *target_type ) const;
    virtual std::string name() const { return base_type->name(); }
};

struct EnumType : public Type {
    EnumType ( const std::string &id ) : Type(id), counter(0) {};

    virtual bool isConvertible( Type *target_type ) const;
    virtual bool isEqual ( Type *target_type ) const;

    int add_value(const std::string& name);
    int add_value(const std::string& name, int);


protected:
    typedef std::set<std::string> EnumValues;

    int        counter;
    EnumValues values; // names for values
};

extern bool is_float( Type *t );
extern bool is_int  ( Type *t );
extern bool is_float_or_int( Type *t );
extern bool is_bound( Type *t );

extern Type*         type_int;
extern Type*         type_bool;
extern Type*         type_float;
extern Type*         type_float_bound;
extern Type*         type_float_bound_ptr;
extern Type*         type_void;
extern FunctionType* fun_std_min;
extern FunctionType* fun_std_max;
extern FunctionType* fun_std_swap;

extern Type*         filtered_predicate_return_type;



#endif
