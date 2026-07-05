#include "interpreter.h"
#include "expr.h"

#include <geogram/lua/lua_io.h>
#include <geogram/basic/logger.h>

extern "C" {
#include <geogram/third_party/lua/lauxlib.h>
#include <geogram/third_party/lua/lualib.h>
}


namespace GEO {

    bool lua_isexpr(lua_State* L, int index) {
	if(lua_islightuserdata(L,index)) {
	    return false;
	}
	if(!lua_isuserdata(L,index)) {
	    return false;
	}
	lua_getmetatable(L,index);
	lua_getfield(L,LUA_REGISTRYINDEX,"expr_vtbl");
	bool result = (lua_compare(L, -1, -2, LUA_OPEQ) != 0);
	lua_pop(L,2);
	return result;
    }

    struct ExprRef {
	Expr* expr;
    };

    void lua_pushexpr(lua_State* L, Expr* expr) {
	if(expr == nullptr) {
	    lua_pushnil(L);
	    return;
	}
	void* p = lua_newuserdata(L,sizeof(ExprRef));
	ExprRef* ER = static_cast<ExprRef*>(p);
	ER->expr = expr;
	ER->expr->ref();
	lua_getfield(L,LUA_REGISTRYINDEX,"expr_vtbl");
	lua_setmetatable(L,-2);
    }

    Expr* lua_toexpr(lua_State* L, int index) {
	geo_assert(lua_isexpr(L,index));
	return static_cast<ExprRef*>(
	    lua_touserdata(L,index)
	)->expr;
    }

    Expr* lua_convert_toexpr(lua_State* L, int index) {
	if(lua_isexpr(L,index)) {
	    return lua_toexpr(L,index);
	}
	if(lua_isnumber(L,index)) {
	    return new Constant(lua_tonumber(L,index));
	}
	return nullptr;
    }
    
    int expr_gc(lua_State* L) {
	geo_debug_assert(lua_isexpr(L,1));
	ExprRef* ER = static_cast<ExprRef*>(
	    lua_touserdata(L,1)
	);
	if(ER->expr != nullptr) {
	    ER->expr->unref();
	}
	ER->expr = nullptr;
	return 0;
    }

    int expr_add(lua_State* L) {
	Expr* op1 = lua_convert_toexpr(L,-2);
	Expr* op2 = lua_convert_toexpr(L,-1);
	if(op1 == nullptr || op2 == nullptr) {
	    Logger::err("PCK") << "+: invalid operand types"
			       << std::endl;
	    return 0;
	}
	SmartPointer<Sum> result = new Sum;
	result->add_term(op1);
	result->add_term(op2);
	if(result->nb_terms() == 1) {
	    lua_pushexpr(L,result->ith_term(0));	    
	} else {
	    lua_pushexpr(L,result);
	}
	return 1;
    }

    int expr_sub(lua_State* L) {
	Expr* op1 = lua_convert_toexpr(L,-2);
	Expr* op2 = lua_convert_toexpr(L,-1);
	if(op1 == nullptr || op2 == nullptr) {
	    Logger::err("PCK") << "-: invalid operand types"
			       << std::endl;
	    return 0;
	}

	SmartPointer<Sum> result = new Sum;
	result->add_term(op1);
	
	Constant* cop2 = dynamic_cast<Constant*>(op2);
	if(cop2 != nullptr) {
	    result->add_term(new Constant(-cop2->value()));
	} else {
	    Product* mop2 = new Product;
	    mop2->mult_factor(new Constant(-1.0));
	    mop2->mult_factor(op2);
	    result->add_term(mop2);
	}

	if(result->nb_terms() == 1) {
	    lua_pushexpr(L,result->ith_term(0));	    
	} else {
	    lua_pushexpr(L,result);
	}
	return 1;
    }

    int expr_mul(lua_State* L) {
	Expr* op1 = lua_convert_toexpr(L,-2);
	Expr* op2 = lua_convert_toexpr(L,-1);
	if(op1 == nullptr || op2 == nullptr) {
	    Logger::err("PCK") << "+: invalid operand types"
			       << std::endl;
	    return 0;
	}
	SmartPointer<Product> result = new Product;
	result->mult_factor(op1);
	result->mult_factor(op2);
	
	if(result->nb_factors() == 1) {
	    lua_pushexpr(L,result->ith_factor(0));	    
	} else {
	    lua_pushexpr(L,result);
	}
	return 1;
    }

    int expr_pow(lua_State* L) {
	Expr* arg = lua_convert_toexpr(L,-2);
	lua_Integer expo = -1;
	if(lua_isinteger(L,-1)) {
	    expo = lua_tointeger(L,-1);
	}
	if(arg == nullptr || expo == -1) {
	    Logger::err("PCK") << "^: invalid operand types"
			       << std::endl;
	    return 0;
	}
	lua_pushexpr(L, new Pow(arg, index_t(expo)));
	return 1;
    }

    int expr_index(lua_State* L) {
	Expr* arg = lua_convert_toexpr(L,-2);
	lua_Integer index = -1;
	const char* name = nullptr;
	if(lua_isinteger(L,-1)) {
	    index = lua_tointeger(L,-1);
	} else if(lua_isstring(L,-1)) {
	    name = lua_tostring(L,-1);
	}
	if(
	    arg == nullptr ||
	    arg->type() != Expr::VECTOR ||
	    (index == -1 && name == nullptr)
	) {
	    Logger::err("PCK") << "index: invalid operand types"
			       << std::endl;
	    return 0;
	}
	if(index == -1) {
	           if(!strcmp(name,"x") || !strcmp(name,"r")) {
	      index = 0;
	    } else if(!strcmp(name,"y") || !strcmp(name,"g")) {
	      index = 1;		       
	    } else if(!strcmp(name,"z") || !strcmp(name,"b")) {
	      index = 2;		       
	    } else if(!strcmp(name,"w") || !strcmp(name,"a")) {
	      index = 3;		       
	    }
	}
	if(index < 0 || index >= arg->dim()) {
	    Logger::err("PCK") << "index: invalid index"
			       << std::endl;
	    return 0;
	}
	lua_pushexpr(L,new VectorComponent(arg, index_t(index)));
	return 1;
    }

    int expr_len(lua_State* L) {
	Expr* arg = lua_convert_toexpr(L,-1);
	if(arg == nullptr) {
	    Logger::err("PCK") << "len: invalid operand types"
			       << std::endl;
	    return 0;
	}
	lua_pushinteger(L,arg->dim());
	return 1;
    }

    int PCK_print(lua_State* L) {
	if(lua_isexpr(L,1)) {
	    Expr* expr = lua_toexpr(L,1);
	    if(expr == nullptr) {
		Logger::out("LUA") << "nil expr" << std::endl;
	    } else {
		Logger::out("LUA") << expr->to_string() << std::endl;
	    }
	} else {
	    Logger::out("LUA") << "Not an expr" << std::endl;
	}
	return 0;
    }

    int PCK_scalar(lua_State* L) {
	lua_pushexpr(L, new ScalarVar);
	return 1;
    }

    int PCK_vec(lua_State* L) {
	lua_pushexpr(L, new VectorVar);
	return 1;
    }
    
    int PCK_vec2(lua_State* L) {
	lua_pushexpr(L, new VectorVar(2));
	return 1;
    }

    int PCK_vec3(lua_State* L) {
	lua_pushexpr(L, new VectorVar(3));
	return 1;
    }

    int PCK_vec4(lua_State* L) {
	lua_pushexpr(L, new VectorVar(4));
	return 1;
    }

    int PCK_Sign(lua_State* L) {
	lua_pushexpr(L, new SignVar);
	return 1;
    }

    int PCK_set_default_dim(lua_State* L) {
	if(lua_gettop(L) != 1) {
	    Logger::err("PCK") << "set_defaut_dim(): invalid number of operants"
			       << std::endl;
	    return 0;
	}
	if(!lua_isinteger(L,-1)) {
	    Logger::err("PCK") << "set_defaut_dim(): invalid operand types"
			       << std::endl;
	}
	lua_Integer dim = lua_tointeger(L,-1);
	if(dim < 1) {
	    Logger::err("PCK") << "set_defaut_dim(): invalid operand value"
			       << std::endl;
	}
	VectorVar::set_default_dim(index_t(dim));
	return 0;
    }
}

namespace GEO {

    Interpreter* Interpreter::instance_ = nullptr;
    
    Interpreter::Interpreter() {
	geo_assert(instance_ == nullptr);
	instance_ = this;
	
	lua_state_ = luaL_newstate();
	luaL_openlibs(lua_state_);
	init_lua_io(lua_state_);

	// Expr member functions
	lua_newtable(lua_state_);

	lua_pushliteral(lua_state_,"__gc");
	lua_pushcfunction(lua_state_,expr_gc);
	lua_settable(lua_state_,-3);

	lua_pushliteral(lua_state_,"__add");
	lua_pushcfunction(lua_state_,expr_add);
	lua_settable(lua_state_,-3);

	lua_pushliteral(lua_state_,"__sub");
	lua_pushcfunction(lua_state_,expr_sub);
	lua_settable(lua_state_,-3);

	lua_pushliteral(lua_state_,"__mul");
	lua_pushcfunction(lua_state_,expr_mul);
	lua_settable(lua_state_,-3);

	lua_pushliteral(lua_state_,"__pow");
	lua_pushcfunction(lua_state_,expr_pow);
	lua_settable(lua_state_,-3);

	lua_pushliteral(lua_state_,"__index");
	lua_pushcfunction(lua_state_,expr_index);
	lua_settable(lua_state_,-3);

	lua_pushliteral(lua_state_,"__len");
	lua_pushcfunction(lua_state_,expr_len);
	lua_settable(lua_state_,-3);
	
	lua_setfield(lua_state_, LUA_REGISTRYINDEX, "expr_vtbl");

	/**********************************************************/
	
	// global functions

	lua_pushcfunction(lua_state_,PCK_scalar);
	lua_setglobal(lua_state_, "scalar");

	lua_pushcfunction(lua_state_,PCK_vec);
	lua_setglobal(lua_state_, "vec");
	
	lua_pushcfunction(lua_state_,PCK_vec2);
	lua_setglobal(lua_state_, "vec2");

	lua_pushcfunction(lua_state_,PCK_vec3);
	lua_setglobal(lua_state_, "vec3");

	lua_pushcfunction(lua_state_,PCK_vec4);
	lua_setglobal(lua_state_, "vec4");

	lua_pushcfunction(lua_state_,PCK_Sign);
	lua_setglobal(lua_state_, "Sign");

	
	/**********************************************************/
	
	// PCK functions
	lua_newtable(lua_state_);
	
	lua_pushliteral(lua_state_,"print");
	lua_pushcfunction(lua_state_,PCK_print);
	lua_settable(lua_state_,-3);

	lua_pushliteral(lua_state_,"set_default_dim");
	lua_pushcfunction(lua_state_,PCK_set_default_dim);
	lua_settable(lua_state_,-3);
	
	
	lua_setglobal(lua_state_, "PCK");
    }

    Interpreter::~Interpreter() {
	lua_close(lua_state_);
	instance_ = nullptr;
    }

    void Interpreter::run_file(const std::string& filename) {
	if(luaL_dofile(lua_state_,filename.c_str())) {
	    const char* msg = lua_tostring(lua_state_,-1);
	    Logger::err("Lua") << msg << std::endl;
	}
    }

}


