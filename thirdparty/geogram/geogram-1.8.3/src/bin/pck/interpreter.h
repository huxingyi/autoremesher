#ifndef H__PCK__INTERPRETER__H
#define H__PCK__INTERPRETER__H

#include <string>

struct lua_State;

namespace GEO {
    
    class Expr;
    
    bool lua_isexpr(lua_State* L, int index);
    void lua_pushexpr(lua_State* L, Expr* expr);
    Expr* lua_toexpr(lua_State* L, int index);
    
    class Interpreter {
      public:
	Interpreter();
	~Interpreter();
	void run_file(const std::string& filename);
	lua_State* lua_state() { return lua_state_; }
	static Interpreter* instance() { return instance_; }
      private:
	lua_State* lua_state_;
	static Interpreter* instance_; 
    };
}

#endif
