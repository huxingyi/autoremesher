#include "expr.h"
#include "interpreter.h"
#include <geogram/basic/string.h>

extern "C" {
#include <geogram/third_party/lua/lauxlib.h>
#include <geogram/third_party/lua/lualib.h>
}



namespace GEO {

    namespace String {
	std::string to_string(Expr* e) {
	    return e->atomic() ? e->to_string() : "(" + e->to_string() + ")";
	}
    }

    /**************************************************************************/
    
    Expr::~Expr() { }
    
    std::string Expr::name() const {
	std::string result;

	// Try to find this variable in Lua's global scope,
	// if found, use its name.
	lua_State* L = Interpreter::instance()->lua_state();
	lua_pushglobaltable(L);
	int index = lua_gettop(L);
	lua_pushnil(L);
	while(lua_next(L,index) != 0) {
	    if(lua_isexpr(L,-1) && lua_toexpr(L,-1) == this) {
		result = std::string(lua_tostring(L,-2));

	    }
	    lua_pop(L,1);
	}

	// If not found in Lua's global scope, generate a unique ID
	// from this Expr's address.
	if(result.length() == 0) {
	    result = "expr@" + String::to_string(this);	    
	}
	return result;
    }

    /**************************************************************************/
    
    Expr::Type Constant::type() const {
	return SCALAR;
    }

    index_t Constant::dim() const {
	return 1;
    }

    std::string Constant::to_string() const {
	return String::to_string(value());
    }

    bool Constant::atomic() const {
	return true;
    }
    
    /**************************************************************************/
    
    Expr::Type Variable::type() const {
	return type_;
    }

    bool Variable::atomic() const {
	return true;
    }
    
    /**************************************************************************/
    
    index_t ScalarVar::dim() const {
	return 1;
    }    

    std::string ScalarVar::to_string() const {
	// return name() + ":scalar";
	return name();
    }

    /**************************************************************************/
    
    index_t VectorVar::default_dim_ = 3;
    
    index_t VectorVar::dim() const {
	return dim_ == index_t(-1) ? default_dim_ : dim_;
    }

    std::string VectorVar::to_string() const {
	std::string result = name();
	/*
	  result += ":vec";
	  if(dim_ != index_t(-1)) {
	     result += ("[" + String::to_string(dim_) + "]");
	  }
	*/
	return result;
    }

    /**************************************************************************/

    std::string VectorComponent::to_string() const {
	return String::to_string(vector()) + "[" + String::to_string(component_) + "]";
    }

    bool VectorComponent::atomic() const {
	return true;
    }

    Expr::Type VectorComponent::type() const {
	return SCALAR;
    }

    index_t VectorComponent::dim() const {
	return 1;
    }
    
    /**************************************************************************/    
    
    index_t SignVar::dim() const {
	return 0;
    }

    std::string SignVar::to_string() const {
	return name() + ":sign";
    }

    /**************************************************************************/

    Expr::Type Sum::type() const {
	return type_;
    }

    index_t Sum::dim() const {
	return dim_;
    }

    std::string Sum::to_string() const {
	std::string result;
	for(index_t i=0; i<nb_terms(); ++i) {
	    result += String::to_string(ith_term(i));
	    if(i != nb_terms()-1) {
		result += " + ";
	    }
	}
	return result;
    }

    void Sum::add_term(Expr* term) {
	{
	    Sum* sum = dynamic_cast<Sum*>(term);
	    if(sum != nullptr) {
		for(index_t i=0; i<sum->nb_terms(); ++i) {
		    add_term(sum->ith_term(i));
		}
		return;
	    }
	}
	{
	    Constant* co1 = dynamic_cast<Constant*>(term);
	    if(co1 != nullptr) {
		if(co1->value() == 0.0) {
		    return;
		}
		for(index_t i=0; i<nb_terms(); ++i) {
		    Constant* co2 = dynamic_cast<Constant*>(ith_term(i));
		    if(co2 != nullptr) {
			terms_[i] = new Constant(co1->value() + co2->value());
			return;
		    }
		}
	    }
	}
	terms_.push_back(term);
    }

    bool Sum::atomic() const {
	return false;
    }
    
    /**************************************************************************/

    Expr::Type Product::type() const {
	return type_;
    }

    index_t Product::dim() const {
	return dim_;
    }

    std::string Product::to_string() const {
	std::string result;
	for(index_t i=0; i<nb_factors(); ++i) {
	    result += String::to_string(ith_factor(i));
	    if(i != nb_factors()-1) {
		result += " * ";
	    }
	}
	return result;
    }

    void Product::mult_factor(Expr* factor) {
	{
	    for(index_t i=0; i<nb_factors(); ++i) {
		if(factor == ith_factor(i)) {
		    factors_[i] = new Pow(factor,2);
		    return;
		}
		Pow* pow = dynamic_cast<Pow*>(ith_factor(i));
		if(pow != nullptr && pow->arg() == factor) {
		    factors_[i] = new Pow(factor,pow->exponent()+1);
		    return;
		}
	    }
	}
	{
	    Product* prod = dynamic_cast<Product*>(factor);
	    if(prod != nullptr) {
		for(index_t i=0; i<prod->nb_factors(); ++i) {
		    mult_factor(prod->ith_factor(i));
		}
		return;
	    }
	}
	{
	    Constant* co1 = dynamic_cast<Constant*>(factor);
	    if(co1 != nullptr) {
		if(co1->value() == 1.0) {
		    return;
		}
		for(index_t i=0; i<nb_factors(); ++i) {
		    Constant* co2 = dynamic_cast<Constant*>(ith_factor(i));
		    if(co2 != nullptr) {
			factors_[i] = new Constant(co1->value() * co2->value());
			return;
		    }
		}
	    }
	}
	factors_.push_back(factor);
    }

    bool Product::atomic() const {
	return false;
    }
    
    /**************************************************************************/

    Expr::Type Pow::type() const {
	return type_;
    }

    index_t Pow::dim() const {
	return dim_;
    }

    std::string Pow::to_string() const {
	return String::to_string(arg()) + "^" + String::to_string(exponent());
    }

    bool Pow::atomic() const {
	return false;
    }
    
    /**************************************************************************/
}
