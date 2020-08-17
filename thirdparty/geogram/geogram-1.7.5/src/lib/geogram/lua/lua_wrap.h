/*
 *  Copyright (c) 2012-2016, Bruno Levy
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *  * Neither the name of the ALICE Project-Team nor the names of its
 *  contributors may be used to endorse or promote products derived from this
 *  software without specific prior written permission.
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Bruno Levy
 *
 *     Bruno.Levy@inria.fr
 *     http://www.loria.fr/~levy
 *
 *     ALICE Project
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 */

#ifndef GEOGRAM_LUA_LUA_WRAP

#include <geogram/basic/common.h>
#include <geogram/basic/assert.h>
#include <geogram/basic/numeric.h>
#include <geogram/basic/string.h>
#include <geogram/basic/memory.h>

extern "C" {
#include <geogram/third_party/lua/lua.h>    
#include <geogram/third_party/lua/lauxlib.h>
#include <geogram/third_party/lua/lualib.h>
}

/**
 * \file geogram/lua/lua_wrap.h
 * \brief Utilities to write lua bindings.
 */

namespace GEO {

    /**
     * \brief A pointer to a LUA function to test an argument
     *  in the LUA stack.
     */
    typedef int (*lua_test_func)(lua_State* L, int idx);

    
    /**
     * \brief Tests whether a LUA variable is a boolean.
     * \details lua_isboolean() is a macro, and we needed
     *  a true function here (to be passed to lua_check_type()).
     * \param[in] L a pointer to the LUA state
     * \param[in] idx an index in the LUA stack
     * \retval a non-zero integer if the variable 
     *  at index \p idx in the LUA
     *  state \p L is a boolean.
     * \retval 0 otherwise.
     */
    inline int my_lua_isboolean(lua_State* L, int idx) {
	return lua_isboolean(L,idx);
    }

    /**
     * \brief Tests whether a LUA variable is a light user data.
     * \details lua_isuserdata() is a macro, and we needed
     *  a true function here (to be passed to lua_check_type()).
     * \param[in] L a pointer to the LUA state
     * \param[in] idx an index in the LUA stack
     * \retval a non-zero integer if the variable 
     *  at index \p idx in the LUA
     *  state \p L is a light user data.
     * \retval 0 otherwise.
     */
    inline int my_lua_islightuserdata(lua_State* L, int idx) {
	return lua_islightuserdata(L,idx);
    }
    
    /**
     * \brief Tests whether a LUA variable is a positive integer.
     * \param[in] L a pointer to the LUA state
     * \param[in] idx an index in the LUA stack
     * \retval a non-zero integer if the variable 
     *  at index \p idx in the LUA
     *  state \p L is a positive integer.
     * \retval 0 otherwise.
     */
    inline int my_lua_ispositiveinteger(lua_State* L, int idx) {
	if(!lua_isinteger(L,idx)) {
	    return 0;
	}
	lua_Integer x = lua_tointeger(L,idx);
	return (x > 0) ? 1 : 0;
    }

    /**
     * \brief Memorizes an error message in LUA registry.
     * \details This is used by C++/LUA interoperability
     *  functions to memorize an error. The error message 
     *  can be passed back to LUA when exiting a wrapper.
     * \param[in] L a pointer to the LUA state.
     * \param[in] error the error message. It will be copied.
     */
    inline void lua_set_error(lua_State* L, const char* error) {
	lua_pushstring(L, error);
	lua_setfield(L,LUA_REGISTRYINDEX,"last_geogram_error");	
    }

    /**
     * \brief Memorizes an error message in LUA registry.
     * \details This is used by C++/LUA interoperability
     *  functions to memorize an error. The error message 
     *  can be passed back to LUA when exiting a wrapper.
     * \param[in] L a pointer to the LUA state.
     * \param[in] error the error message. It will be copied.
     */
    inline void lua_set_error(lua_State* L, const std::string& error) {
	lua_set_error(L,error.c_str());
    }

    /**
     * \brief Clears the last error message in LUA registry.
     * \param[in] L a pointer to the LUA state.
     */
    inline void lua_clear_error(lua_State* L) {
	lua_pushnil(L);
	lua_setfield(L,LUA_REGISTRYINDEX,"last_geogram_error");
    }

    /**
     * \brief Tests whether an error message was memorized in the
     *  registry.
     * \param[in] L a pointer to the LUA state.
     */
    inline bool lua_has_error(lua_State* L) {
	lua_getfield(L,LUA_REGISTRYINDEX,"last_geogram_error");
	bool result = !lua_isnil(L,-1);
	lua_pop(L,1);
	return result;
    }

    /**
     * \brief Tests whether a LUA variable has the correct type.
     * \details If the LUA variable does not have the correct type,
     *  then an error message is memorized in the registry.
     * \param[in] L a pointer to the LUA state
     * \param[in] idx the index of the variable
     * \param[in] test a function to test the type, taking as argument
     *  \p L and \p idx, and returning 1 if the type of the argument
     *  is correct and 0 otherwise.
     * \retval true if the type is corret
     * \retval false otherwise
     */
    inline bool lua_check_type(lua_State* L, int idx, lua_test_func test) {
	if(!test(L,idx)) {
	    std::string error = std::string("Argument ") +
		String::to_string(idx) + ": wrong argument type (" +
		"got " + lua_typename(L,lua_type(L,idx)) + ")";
	    lua_set_error(L, error.c_str());
	    return false;
	}
	return true;
    }

    /**
     * \brief Tests whether the expected number of arguments was pushed
     *  onto the stack.
     * \details If the number of elements on the stack does not match
     *  the expected number of arguments, then an error message is 
     *  memorized in the registry.
     * \param[in] L a pointer to the LUA state
     * \param[in] expected_nb_args the expected number of arguments
     * \retval true if the expected number of arguments were pushed onto
     *  the stack
     * \retval false otherwise
     */
    inline bool lua_check_nb_args(lua_State* L, int expected_nb_args) {
	if(lua_gettop(L) != expected_nb_args) {
	    std::string error =
		"Expected " + String::to_string(expected_nb_args)
		+ " arg(s), got " + String::to_string(lua_gettop(L));
	    lua_set_error(L,error.c_str());
	    return false;
	}
	return true;
    }

    /**
     * \brief Takes the last error message memorized in the registry
     *  and sends it back to LUA.
     * \details This function is called by wrappers right before returning
     *  each time an error is detected.
     * \retval the return value supposed to be returned by the wrapper,
     *  as follows:
     * \code
     *   if(error occured) {
     *     return lua_notify_last_error(L);
     *   }
     * \endcode
     */
    inline int lua_notify_last_error(lua_State* L) {
	std::string error;
	lua_getfield(L,LUA_REGISTRYINDEX,"last_geogram_error");
	if(lua_isstring(L,-1)) {
	    error += lua_tostring(L,-1);
	}
	lua_pop(L,1);
	lua_clear_error(L);
	return luaL_error(L,error.c_str());
    }

    /**********************************************************************/

    /**
     * \brief Converts LUA variables to C++ variables.
     * \details This version is a placeholder, the actual implementation
     *  is done in the template specializations.
     */
    template <class T> class lua_to {
    public:
	/**
	 * \brief lua_to constructor.
	 * \details Does the actual conversion. The converted variable
	 *  is memorized in a member. It can be accesed by a conversion
	 *  operator. Supposing that my_function() takes a std::string and
	 *  an int as arguments, it can be called as follows:
	 * \code
	 *    my_function(lua_to<std::string>(L,1), lua_to<int>(L,2));
	 * \endcode
	 * \param[in] L a pointer to the LUA state.
	 * \param[in] idx the index of the variable to be converted.
	 */
	lua_to(lua_State* L, int idx) {
	    geo_argused(L);
	    geo_argused(idx);
	    geo_assert_not_reached;
	}

	/**
	 * \brief Tests whether a LUA variable can be converted to 
	 *  a C++ variable.
	 * \note It would have been possible to make the constructor throw
	 *  an exception on conversion error, but exceptions are not well
	 *  supported by Emscripten so we have separated validity check from
	 *  conversion.
	 * \param[in] L a pointer to the LUA state.
	 * \param[in] idx the index of the variable to be converted.
	 * \retval true if the LUA variable can be converted to a C++ variable.
	 * \retval false otherwise.
	 */
	static bool can_convert(lua_State* L, int idx) {
	    geo_argused(L);
	    geo_argused(idx);
	    geo_assert_not_reached;
	}

	/**
	 * \brief Gets the converted value.
	 * \return the converted value.
	 */
	operator T() const {
	    geo_assert_not_reached;
	}
    protected:
	T x_;
    };

    /**
     * \brief lua_to specialization for int.
     */
    template<> class lua_to<int> {
      public:
	lua_to(lua_State* L, int idx) {
	    x_ = int(lua_tointeger(L,idx));
	}
	static bool can_convert(lua_State* L, int idx) {
	    return lua_check_type(L, idx, lua_isinteger);
	}
	operator int() const {
	    return x_;
	}
      private:
	int x_;
    };

    /**
     * \brief lua_to specialization for Numeric::uint32.
     */
    template<> class lua_to<Numeric::uint32> {
      public:
	lua_to(lua_State* L, int idx) {
	    x_ = Numeric::uint32(lua_tointeger(L,idx));
	}
	static bool can_convert(lua_State* L, int idx) {
	    return lua_check_type(L, idx, my_lua_ispositiveinteger);
	}
	operator Numeric::uint32() const {
	    return x_;
	}
      private:
	Numeric::uint32 x_;
    };

    /**
     * \brief lua_to specialization for Numeric::uint64.
     */
    template<> class lua_to<Numeric::uint64> {
      public:
	lua_to(lua_State* L, int idx) {
	    x_ = Numeric::uint64(lua_tointeger(L,idx));
	}
	static bool can_convert(lua_State* L, int idx) {
	    return lua_check_type(L, idx, my_lua_ispositiveinteger);
	}
	operator Numeric::uint64() const {
	    return x_;
	}
      private:
	Numeric::uint64 x_;
    };

    
    /**
     * \brief lua_to specialization for Numeric::int64.
     */
    template<> class lua_to<Numeric::int64> {
      public:
	lua_to(lua_State* L, int idx) {
	    x_ = Numeric::int64(lua_tointeger(L,idx));
	}
	static bool can_convert(lua_State* L, int idx) {
	    return lua_check_type(L, idx, lua_isinteger);
	}
	operator Numeric::int64() const {
	    return x_;
	}
      private:
	Numeric::int64 x_;
    };
    
    /**
     * \brief lua_to specialization for float.
     */
    template<> class lua_to<float> {
      public:
	lua_to(lua_State* L, int idx) {
	    x_ = float(lua_tonumber(L,idx));
	}
	static bool can_convert(lua_State* L, int idx) {
	    return lua_check_type(L, idx, lua_isnumber);
	}
	operator float() const {
	    return x_;
	}
      private:
	float x_;
    };

    /**
     * \brief lua_to specialization for double.
     */
    template<> class lua_to<double> {
      public:
	lua_to(lua_State* L, int idx) {
	    x_ = double(lua_tonumber(L,idx));
	}
	static bool can_convert(lua_State* L, int idx) {
	    return lua_check_type(L, idx, lua_isnumber);
	}
	operator double() const {
	    return x_;
	}
      private:
	double x_;
    };

    /**
     * \brief lua_to specialization for bool.
     */
    template<> class lua_to<bool> {
      public:
	lua_to(lua_State* L, int idx) {
	    x_ = (lua_toboolean(L,idx) != 0);
	}
	static bool can_convert(lua_State* L, int idx) {
	    return lua_check_type(L, idx, my_lua_isboolean);
	}
	operator bool() const {
	    return x_;
	}
      private:
	bool x_;
    };

    /**
     * \brief lua_to specialization for raw string (const char*).
     */
    template<> class lua_to<const char*> {
      public:
	lua_to(lua_State* L, int idx) {
	    x_ = lua_tostring(L,idx);
	}
	static bool can_convert(lua_State* L, int idx) {
	    return lua_check_type(L, idx, lua_isstring);
	}
	operator const char*() const {
	    return x_;
	}
      private:
	const char* x_;
    };

    /**
     * \brief lua_to specialization for reference to std::string.
     */
    template<> class lua_to<const std::string&> {
      public:
	lua_to(lua_State* L, int idx) {
	    x_ = lua_tostring(L,idx);
	}
	static bool can_convert(lua_State* L, int idx) {
	    return lua_check_type(L, idx, lua_isstring);
	}
	operator const std::string&() const {
	    return x_;
	}
      private:
	std::string x_;
    };

    /**
     * \brief lua_to specialization for std::string.
     */
    template<> class lua_to<std::string> {
      public:
	lua_to(lua_State* L, int idx) {
	    x_ = lua_tostring(L,idx);
	}
	static bool can_convert(lua_State* L, int idx) {
	    return lua_check_type(L, idx, lua_isstring);
	}
	operator std::string() const {
	    return x_;
	}
      private:
	std::string x_;
    };

    /**********************************************************************/

    /**
     * \brief Converts and pushes a C++ variable onto the LUA stack.
     * \details This version is a placeholder. The actual implementation
     *  is done in the specializations.
     * \note Just using function overloading (instead of template partializations)
     *  works with gcc, but does not work with clang and MSVC.
     * \param[in] L a pointer to the LUA state.
     * \param[in] x the variable to be pushed.
     */
    template<class T> inline void lua_push(lua_State* L, T x) {
	geo_argused(L);
	geo_argused(x);
	geo_assert_not_reached;
    }

    /**
     * \brief Specialization of lua_push() for int.
     */
    template<> inline void lua_push(lua_State* L, int x) {
	lua_pushinteger(L,lua_Integer(x));
    }


    /**
     * \brief Specialization of lua_push() for Numeric::uint32.
     */
    template<> inline void lua_push(lua_State* L, Numeric::uint32 x) {
	lua_pushinteger(L,lua_Integer(x));
    }

    /**
     * \brief Specialization of lua_push() for Numeric::uint64.
     */
    template<> inline void lua_push(lua_State* L, Numeric::uint64 x) {
	lua_pushinteger(L,lua_Integer(x));
    }
    
    /**
     * \brief Specialization of lua_push() for Numeric::int64.
     */
    template<> inline void lua_push(lua_State* L, Numeric::int64 x) {
	lua_pushinteger(L,lua_Integer(x));
    }
    
    /**
     * \brief Specialization of lua_push() for float.
     */
    template<> inline void lua_push(lua_State* L, float x) {
	lua_pushnumber(L,lua_Number(x));
    }

    /**
     * \brief Specialization of lua_push() for double.
     */
    template<> inline void lua_push(lua_State* L, double x) {
	lua_pushnumber(L,lua_Number(x));
    }

    /**
     * \brief Specialization of lua_push() for bool.
     */
    template<> inline void lua_push(lua_State* L, bool x) {
	lua_pushboolean(L,x?1:0);
    }

    /**
     * \brief Specialization of lua_push() for raw string (const char*).
     */
    template<> inline void lua_push(lua_State* L, const char* x) {
	lua_pushstring(L,x);
    }

    /**
     * \brief Specialization of lua_push() for reference to std::string.
     */
    template<> inline void lua_push(lua_State* L, const std::string& x) {
	lua_pushstring(L,x.c_str());
    }

    /**
     * \brief Specialization of lua_push() for std::string.
     */
    template<> inline void lua_push(lua_State* L, std::string x) {
	lua_pushstring(L,x.c_str());
    }

    /**
     * \brief Specialization of lua_push() for vectors.
     * \details It pushes a table, indexed by integers, from 1 to n (rather than
     *  0 to n-1, following LUA indexing convention).
     */
    template<class T> inline void lua_push(
	lua_State* L, const std::vector<T>& x
    ) {
	lua_newtable(L);
	for(size_t i=0; i<x.size(); ++i) {
	    lua_push(L,x[i]);
	    lua_seti(L,-2,lua_Integer(i+1));
	}
    }

/**
 * \brief Declares a new enum type that can be used by LUA wrappers.
 * \details enum types that can be used in wrapped functions need to be
 *  explicitely declared before using lua_bindwrapper() and 
 *  lua_bindwrapperglobal(). This will be no longer the case when we will
 *  switch to C++11 (but for now, geogram needs to remain compatible with
 *  C++98). 
 * \note LUA_DECLAREENUMTYPE cannot be done from within a function.
 * \param[in] T the C++ type name of the enum.
 */
#define LUA_DECLAREENUMTYPE(T)                                \
    template<> inline void lua_push(lua_State* L, T x) {      \
	lua_push(L, int(x));                                  \
    }                                                         \
                                                              \
    template<> class lua_to<T> : public GEO::lua_to<int> {    \
      public:                                                 \
	lua_to(lua_State* L, int idx) : lua_to<int>(L,idx) {  \
	}                                                     \
	operator T() const {                                  \
	    return T(lua_to<int>::operator int());            \
	}                                                     \
    }
    
    /**********************************************************************/

    /**
     * \brief Calls a C++ function from LUA.
     * \details The arguments are converted from the LUA stack, and the
     *  return value pushed onto the LUA stack. Whenever an error occurs,
     *  (invalid number of arguments or type error), it is captured and
     *  an error message is returned to the caller.
     */
    template <class R> inline int lua_wrap(lua_State* L, R (*fptr)(void)) {
	if(!lua_check_nb_args(L,0)) {
	    return lua_notify_last_error(L);
	}
	R retval = fptr();
	lua_push(L,retval);
	return 1;
    }

    /**
     * \brief Calls a C++ function from LUA.
     * \details The arguments are converted from the LUA stack, and the
     *  return value pushed onto the LUA stack. Whenever an error occurs,
     *  (invalid number of arguments or type error), it is captured and
     *  an error message is returned to the caller.
     */
    template <class R, class T1> inline int lua_wrap(
	lua_State* L, R (*fptr)(T1)
    ) {
	if(
	   !lua_check_nb_args(L,1) ||
	   !lua_to<T1>::can_convert(L,1)
	) {
	    return lua_notify_last_error(L);
	}
	R retval = fptr(
	    lua_to<T1>(L,1)
	);
        lua_push(L,retval);
	return 1;
    }

    /**
     * \brief Calls a C++ function from LUA.
     * \details The arguments are converted from the LUA stack, and the
     *  return value pushed onto the LUA stack. Whenever an error occurs,
     *  (invalid number of arguments or type error), it is captured and
     *  an error message is returned to the caller.
     */
    template <class R, class T1, class T2> inline int lua_wrap(
	lua_State* L, R (*fptr)(T1,T2)
    ) {
	if(
	   !lua_check_nb_args(L,2) ||
	   !lua_to<T1>::can_convert(L,1) ||
	   !lua_to<T2>::can_convert(L,2)	   
	) {
	    return lua_notify_last_error(L);
	}
        R retval = fptr(
	    lua_to<T1>(L,1),
	    lua_to<T2>(L,2)
	);
        lua_push(L,retval);
        return 1;
    }

    /**
     * \brief Calls a C++ function from LUA.
     * \details The arguments are converted from the LUA stack, and the
     *  return value pushed onto the LUA stack. Whenever an error occurs,
     *  (invalid number of arguments or type error), it is captured and
     *  an error message is returned to the caller.
     */
    template <class R, class T1, class T2, class T3> inline int lua_wrap(
	lua_State* L, R (*fptr)(T1,T2,T3)
    ) {
	if(
	   !lua_check_nb_args(L,3) ||
	   !lua_to<T1>::can_convert(L,1) ||
	   !lua_to<T2>::can_convert(L,2) ||
	   !lua_to<T3>::can_convert(L,3)	   	   
	) {
	    return lua_notify_last_error(L);
	}
        R retval = fptr(
	    lua_to<T1>(L,1),
	    lua_to<T2>(L,2),
	    lua_to<T3>(L,3)
	);
        lua_push(L,retval);
        return 1;
    }

    /**
     * \brief Calls a C++ function from LUA.
     * \details The arguments are converted from the LUA stack, and the
     *  return value pushed onto the LUA stack. Whenever an error occurs,
     *  (invalid number of arguments or type error), it is captured and
     *  an error message is returned to the caller.
     */
    template <class R, class T1, class T2, class T3, class T4>
    inline int lua_wrap(lua_State* L, R (*fptr)(T1,T2,T3,T4)) {
	if(
	   !lua_check_nb_args(L,4) ||
	   !lua_to<T1>::can_convert(L,1) ||
	   !lua_to<T2>::can_convert(L,2) ||
	   !lua_to<T3>::can_convert(L,3) ||
	   !lua_to<T4>::can_convert(L,4) 	   
	) {
	    return lua_notify_last_error(L);
	}
        R retval = fptr(
	    lua_to<T1>(L,1),
	    lua_to<T2>(L,2),
	    lua_to<T3>(L,3),
	    lua_to<T4>(L,4)	    
	);
        lua_push(L,retval);
        return 1;
    }
    
    /*************************************************************************/

    /**
     * \brief Calls a C++ function from LUA.
     * \details The arguments are converted from the LUA stack. 
     *  Whenever an error occurs, (invalid number of arguments or type error), 
     *  it is captured and an error message is returned to the caller.
     */
    template <> inline int lua_wrap(lua_State* L, void (*fptr)(void)) {
	if(!lua_check_nb_args(L,0)) {
	    return lua_notify_last_error(L);
	}
	fptr();
	return 0;
    }

    /**
     * \brief Calls a C++ function from LUA.
     * \details The arguments are converted from the LUA stack. 
     *  Whenever an error occurs, (invalid number of arguments or type error), 
     *  it is captured and an error message is returned to the caller.
     */
    template <class T1> inline int lua_wrap(lua_State* L, void (*fptr)(T1)) {
	if(
	   !lua_check_nb_args(L,1) ||
	   !lua_to<T1>::can_convert(L,1)
	) {
	    return lua_notify_last_error(L);
	}
	fptr(
	    lua_to<T1>(L,1)
	);	    
	return 0;
    }

    /**
     * \brief Calls a C++ function from LUA.
     * \details The arguments are converted from the LUA stack. 
     *  Whenever an error occurs, (invalid number of arguments or type error), 
     *  it is captured and an error message is returned to the caller.
     */
    template <class T1, class T2> inline int lua_wrap(
	lua_State* L, void (*fptr)(T1,T2)
    ) {
	if(
	   !lua_check_nb_args(L,2) ||
	   !lua_to<T1>::can_convert(L,1) ||
	   !lua_to<T2>::can_convert(L,2)	   
	) {
	    return lua_notify_last_error(L);
	}
	fptr(
	    lua_to<T1>(L,1),
	    lua_to<T2>(L,2)
	);
        return 0;
    }

    /**
     * \brief Calls a C++ function from LUA.
     * \details The arguments are converted from the LUA stack. 
     *  Whenever an error occurs, (invalid number of arguments or type error), 
     *  it is captured and an error message is returned to the caller.
     */
    template <class T1, class T2, class T3>
    inline int lua_wrap(lua_State* L, void (*fptr)(T1,T2,T3)) {
	if(
	   !lua_check_nb_args(L,3) ||
	   !lua_to<T1>::can_convert(L,1) ||
	   !lua_to<T2>::can_convert(L,2) ||
	   !lua_to<T3>::can_convert(L,3)	   	   
	) {
	    return lua_notify_last_error(L);
	}
        fptr(
	    lua_to<T1>(L,1),
	    lua_to<T2>(L,2),
	    lua_to<T3>(L,3)
	);
        return 0;
    }

    /**
     * \brief Calls a C++ function from LUA.
     * \details The arguments are converted from the LUA stack. 
     *  Whenever an error occurs, (invalid number of arguments or type error), 
     *  it is captured and an error message is returned to the caller.
     */
    template <class T1, class T2, class T3, class T4>
    inline int lua_wrap(lua_State* L, void (*fptr)(T1,T2,T3,T4)) {
	if(
	   !lua_check_nb_args(L,4) ||
	   !lua_to<T1>::can_convert(L,1) ||
	   !lua_to<T2>::can_convert(L,2) ||
	   !lua_to<T3>::can_convert(L,3) ||
	   !lua_to<T4>::can_convert(L,4) 	   	   	   
	) {
	    return lua_notify_last_error(L);
	}
        fptr(
	    lua_to<T1>(L,1),
	    lua_to<T2>(L,2),
	    lua_to<T3>(L,3),
	    lua_to<T4>(L,4)	    
	);
        return 0;
    }

    /**
     * \brief Calls a C++ function from LUA.
     * \details The arguments are converted from the LUA stack. 
     *  Whenever an error occurs, (invalid number of arguments or type error), 
     *  it is captured and an error message is returned to the caller.
     */
    template <
	class T1, class T2, class T3, class T4, class T5
    >
    inline int lua_wrap(
	lua_State* L, void (*fptr)(T1,T2,T3,T4,T5)
    ) {
	if(
	   !lua_check_nb_args(L,5) ||
	   !lua_to<T1>::can_convert(L,1) ||
	   !lua_to<T2>::can_convert(L,2) ||
	   !lua_to<T3>::can_convert(L,3) ||
	   !lua_to<T4>::can_convert(L,4) ||
	   !lua_to<T5>::can_convert(L,5) 
	) {
	    return lua_notify_last_error(L);
	}
        fptr(
	    lua_to<T1>(L,1),
	    lua_to<T2>(L,2),
	    lua_to<T3>(L,3),
	    lua_to<T4>(L,4),
	    lua_to<T5>(L,5)
	);
        return 0;
    }
    
    /**
     * \brief Calls a C++ function from LUA.
     * \details The arguments are converted from the LUA stack. 
     *  Whenever an error occurs, (invalid number of arguments or type error), 
     *  it is captured and an error message is returned to the caller.
     */
    template <
	class T1, class T2, class T3, class T4, class T5, class T6
    >
    inline int lua_wrap(
	lua_State* L, void (*fptr)(T1,T2,T3,T4,T5,T6)
    ) {
	if(
	   !lua_check_nb_args(L,6) ||
	   !lua_to<T1>::can_convert(L,1) ||
	   !lua_to<T2>::can_convert(L,2) ||
	   !lua_to<T3>::can_convert(L,3) ||
	   !lua_to<T4>::can_convert(L,4) ||
	   !lua_to<T5>::can_convert(L,5) ||
	   !lua_to<T6>::can_convert(L,6) 	   
	) {
	    return lua_notify_last_error(L);
	}
        fptr(
	    lua_to<T1>(L,1),
	    lua_to<T2>(L,2),
	    lua_to<T3>(L,3),
	    lua_to<T4>(L,4),
	    lua_to<T5>(L,5),
	    lua_to<T6>(L,6)
	);
        return 0;
    }
    
    /**
     * \brief Calls a C++ function from LUA.
     * \details The arguments are converted from the LUA stack. 
     *  Whenever an error occurs, (invalid number of arguments or type error), 
     *  it is captured and an error message is returned to the caller.
     */
    template <
	class T1, class T2, class T3, class T4, class T5,
	class T6, class T7
    >
    inline int lua_wrap(
	lua_State* L, void (*fptr)(T1,T2,T3,T4,T5,T6,T7)
    ) {
	if(
	   !lua_check_nb_args(L,7) ||
	   !lua_to<T1>::can_convert(L,1) ||
	   !lua_to<T2>::can_convert(L,2) ||
	   !lua_to<T3>::can_convert(L,3) ||
	   !lua_to<T4>::can_convert(L,4) ||
	   !lua_to<T5>::can_convert(L,5) ||
	   !lua_to<T6>::can_convert(L,6) ||
	   !lua_to<T7>::can_convert(L,7) 
	) {
	    return lua_notify_last_error(L);
	}
        fptr(
	    lua_to<T1>(L,1),
	    lua_to<T2>(L,2),
	    lua_to<T3>(L,3),
	    lua_to<T4>(L,4),
	    lua_to<T5>(L,5),
	    lua_to<T6>(L,6),
	    lua_to<T7>(L,7)
	);
        return 0;
    }


    /**
     * \brief Calls a C++ function from LUA.
     * \details The arguments are converted from the LUA stack. 
     *  Whenever an error occurs, (invalid number of arguments or type error), 
     *  it is captured and an error message is returned to the caller.
     */
    template <
	class T1, class T2, class T3, class T4, class T5,
	class T6, class T7, class T8
    >
    inline int lua_wrap(
	lua_State* L, void (*fptr)(T1,T2,T3,T4,T5,T6,T7,T8)
    ) {
	if(
	   !lua_check_nb_args(L,8) ||
	   !lua_to<T1>::can_convert(L,1) ||
	   !lua_to<T2>::can_convert(L,2) ||
	   !lua_to<T3>::can_convert(L,3) ||
	   !lua_to<T4>::can_convert(L,4) ||
	   !lua_to<T5>::can_convert(L,5) ||
	   !lua_to<T6>::can_convert(L,6) ||
	   !lua_to<T7>::can_convert(L,7) ||
	   !lua_to<T8>::can_convert(L,8) 
	) {
	    return lua_notify_last_error(L);
	}
        fptr(
	    lua_to<T1>(L,1),
	    lua_to<T2>(L,2),
	    lua_to<T3>(L,3),
	    lua_to<T4>(L,4),
	    lua_to<T5>(L,5),
	    lua_to<T6>(L,6),
	    lua_to<T7>(L,7),
	    lua_to<T8>(L,8)
	);
        return 0;
    }


    /**
     * \brief Calls a C++ function from LUA.
     * \details The arguments are converted from the LUA stack. 
     *  Whenever an error occurs, (invalid number of arguments or type error), 
     *  it is captured and an error message is returned to the caller.
     */
    template <
	class T1, class T2, class T3, class T4, class T5,
	class T6, class T7, class T8, class T9
    >
    inline int lua_wrap(
	lua_State* L, void (*fptr)(T1,T2,T3,T4,T5,T6,T7,T8,T9)
    ) {
	if(
	   !lua_check_nb_args(L,9) ||
	   !lua_to<T1>::can_convert(L,1) ||
	   !lua_to<T2>::can_convert(L,2) ||
	   !lua_to<T3>::can_convert(L,3) ||
	   !lua_to<T4>::can_convert(L,4) ||
	   !lua_to<T5>::can_convert(L,5) ||
	   !lua_to<T6>::can_convert(L,6) ||
	   !lua_to<T7>::can_convert(L,7) ||
	   !lua_to<T8>::can_convert(L,8) ||
	   !lua_to<T9>::can_convert(L,9) 
	) {
	    return lua_notify_last_error(L);
	}
        fptr(
	    lua_to<T1>(L,1),
	    lua_to<T2>(L,2),
	    lua_to<T3>(L,3),
	    lua_to<T4>(L,4),
	    lua_to<T5>(L,5),
	    lua_to<T6>(L,6),
	    lua_to<T7>(L,7),
	    lua_to<T8>(L,8),
	    lua_to<T9>(L,9)
	);
        return 0;
    }

    
    /**
     * \brief Calls a C++ function from LUA.
     * \details The arguments are converted from the LUA stack. 
     *  Whenever an error occurs, (invalid number of arguments or type error), 
     *  it is captured and an error message is returned to the caller.
     */
    template <
	class T1, class T2, class T3, class T4, class T5,
	class T6, class T7, class T8, class T9, class T10
    >
    inline int lua_wrap(
	lua_State* L, void (*fptr)(T1,T2,T3,T4,T5,T6,T7,T8,T9,T10)
    ) {
	if(
	   !lua_check_nb_args(L,10) ||
	   !lua_to<T1>::can_convert(L,1) ||
	   !lua_to<T2>::can_convert(L,2) ||
	   !lua_to<T3>::can_convert(L,3) ||
	   !lua_to<T4>::can_convert(L,4) ||
	   !lua_to<T5>::can_convert(L,5) ||
	   !lua_to<T6>::can_convert(L,6) ||
	   !lua_to<T7>::can_convert(L,7) ||
	   !lua_to<T8>::can_convert(L,8) ||
	   !lua_to<T9>::can_convert(L,9) ||
	   !lua_to<T10>::can_convert(L,10) 	   
	) {
	    return lua_notify_last_error(L);
	}
        fptr(
	    lua_to<T1>(L,1),
	    lua_to<T2>(L,2),
	    lua_to<T3>(L,3),
	    lua_to<T4>(L,4),
	    lua_to<T5>(L,5),
	    lua_to<T6>(L,6),
	    lua_to<T7>(L,7),
	    lua_to<T8>(L,8),
	    lua_to<T9>(L,9),
	    lua_to<T10>(L,10)	    	    
	);
        return 0;
    }
    

    /**
     * \brief Calls a C++ function from LUA.
     * \details The arguments are converted from the LUA stack. 
     *  Whenever an error occurs, (invalid number of arguments or type error), 
     *  it is captured and an error message is returned to the caller.
     */
    template <
	class T1, class T2, class T3, class T4, class T5,
	class T6, class T7, class T8, class T9, class T10,
	class T11
    >
    inline int lua_wrap(
	lua_State* L, void (*fptr)(T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11)
    ) {
	if(
	   !lua_check_nb_args(L,11) ||
	   !lua_to<T1>::can_convert(L,1) ||
	   !lua_to<T2>::can_convert(L,2) ||
	   !lua_to<T3>::can_convert(L,3) ||
	   !lua_to<T4>::can_convert(L,4) ||
	   !lua_to<T5>::can_convert(L,5) ||
	   !lua_to<T6>::can_convert(L,6) ||
	   !lua_to<T7>::can_convert(L,7) ||
	   !lua_to<T8>::can_convert(L,8) ||
	   !lua_to<T9>::can_convert(L,9) ||
	   !lua_to<T10>::can_convert(L,10) ||
	   !lua_to<T11>::can_convert(L,11) 	   
	) {
	    return lua_notify_last_error(L);
	}
        fptr(
	    lua_to<T1>(L,1),
	    lua_to<T2>(L,2),
	    lua_to<T3>(L,3),
	    lua_to<T4>(L,4),
	    lua_to<T5>(L,5),
	    lua_to<T6>(L,6),
	    lua_to<T7>(L,7),
	    lua_to<T8>(L,8),
	    lua_to<T9>(L,9),
	    lua_to<T10>(L,10),
	    lua_to<T11>(L,11)	    	    	    
	);
        return 0;
    }

    /**
     * \brief Calls a C++ function from LUA.
     * \details The arguments are converted from the LUA stack. 
     *  Whenever an error occurs, (invalid number of arguments or type error), 
     *  it is captured and an error message is returned to the caller.
     */
    template <
	class T1, class T2, class T3, class T4, class T5,
	class T6, class T7, class T8, class T9, class T10,
	class T11, class T12
    >
    inline int lua_wrap(
	lua_State* L, void (*fptr)(T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12)
    ) {
	if(
	   !lua_check_nb_args(L,12) ||
	   !lua_to<T1>::can_convert(L,1) ||
	   !lua_to<T2>::can_convert(L,2) ||
	   !lua_to<T3>::can_convert(L,3) ||
	   !lua_to<T4>::can_convert(L,4) ||
	   !lua_to<T5>::can_convert(L,5) ||
	   !lua_to<T6>::can_convert(L,6) ||
	   !lua_to<T7>::can_convert(L,7) ||
	   !lua_to<T8>::can_convert(L,8) ||
	   !lua_to<T9>::can_convert(L,9) ||
	   !lua_to<T10>::can_convert(L,10) ||
	   !lua_to<T11>::can_convert(L,11) ||
	   !lua_to<T12>::can_convert(L,12) 	   
	) {
	    return lua_notify_last_error(L);
	}
        fptr(
	    lua_to<T1>(L,1),
	    lua_to<T2>(L,2),
	    lua_to<T3>(L,3),
	    lua_to<T4>(L,4),
	    lua_to<T5>(L,5),
	    lua_to<T6>(L,6),
	    lua_to<T7>(L,7),
	    lua_to<T8>(L,8),
	    lua_to<T9>(L,9),
	    lua_to<T10>(L,10),
	    lua_to<T11>(L,11),
	    lua_to<T12>(L,12)	    
	);
        return 0;
    }


    /**
     * \brief Calls a C++ function from LUA.
     * \details The arguments are converted from the LUA stack. 
     *  Whenever an error occurs, (invalid number of arguments or type error), 
     *  it is captured and an error message is returned to the caller.
     */
    template <
	class T1, class T2, class T3, class T4, class T5,
	class T6, class T7, class T8, class T9, class T10,
	class T11, class T12, class T13
    >
    inline int lua_wrap(
	lua_State* L, void (*fptr)(T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13)
    ) {
	if(
	   !lua_check_nb_args(L,13) ||
	   !lua_to<T1>::can_convert(L,1) ||
	   !lua_to<T2>::can_convert(L,2) ||
	   !lua_to<T3>::can_convert(L,3) ||
	   !lua_to<T4>::can_convert(L,4) ||
	   !lua_to<T5>::can_convert(L,5) ||
	   !lua_to<T6>::can_convert(L,6) ||
	   !lua_to<T7>::can_convert(L,7) ||
	   !lua_to<T8>::can_convert(L,8) ||
	   !lua_to<T9>::can_convert(L,9) ||
	   !lua_to<T10>::can_convert(L,10) ||
	   !lua_to<T11>::can_convert(L,11) ||
	   !lua_to<T12>::can_convert(L,12) ||
	   !lua_to<T12>::can_convert(L,13)
	) {
	    return lua_notify_last_error(L);
	}
        fptr(
	    lua_to<T1>(L,1),
	    lua_to<T2>(L,2),
	    lua_to<T3>(L,3),
	    lua_to<T4>(L,4),
	    lua_to<T5>(L,5),
	    lua_to<T6>(L,6),
	    lua_to<T7>(L,7),
	    lua_to<T8>(L,8),
	    lua_to<T9>(L,9),
	    lua_to<T10>(L,10),
	    lua_to<T11>(L,11),
	    lua_to<T12>(L,12),
	    lua_to<T13>(L,13)	    
	);
        return 0;
    }
    
    /**
     * \brief Specialization of the wrapper for functions that 
     *  are "already wrapped".
     * \note Normally not used, since there is a specialization of
     *  lua_pushwrapper() for lua_CFunction.
     */
    template<> inline int lua_wrap(lua_State* L, lua_CFunction fptr) {
	return fptr(L);
    }

    
    /*************************************************************************/

    /**
     * \brief Manages wrappers around C++ functions to be called from LUA.
     * \details This class should not be used directly by client code. 
     *  Client code will rather use the high-level macro lua_bindwrapper()
     *  or the lower-level functions lua_bindwrapperwithname() and 
     *  lua_pushwrapper().
     */
    template <class FPTR> class lua_wrapper {
      public:

	/**
	 * \brief Implementation of the wrapper. 
	 * \details This is the C functions to be declared to LUA. 
	 *  It is typed by the signature of the wrapped C++ function, 
	 *  and the pointer to the  actual wrapped C++ function is stored 
	 *  in an upvalue.
	 * \param[in] L a pointer to the LUA state.
	 */
	static int call(lua_State* L) {
	    FPTR f = FPTR(
		Memory::generic_pointer_to_function_pointer(
		    lua_touserdata(L, lua_upvalueindex(1))
		)
	    );
	    return lua_wrap(L, f);
	}

	/**
	 * \brief Pushes a wrapper for a given C++ function onto the LUA stack.
	 * \param[in] L a pointer to the LUA state.
	 * \param[in] f a pointer to the C++ function to be pushed. It cannot be
	 *  a non-static object's member function.
	 */
	static void push(lua_State* L, FPTR f) {
	    lua_pushlightuserdata(
		L,
		Memory::function_pointer_to_generic_pointer(
		    Memory::function_pointer(f)
		)
	    );
	    lua_pushcclosure(L, lua_wrapper<FPTR>::call, 1);
	}
    };

    /**************************************************************************/

    /**
     * \brief Pushes a wrapper for a given C++ function onto the LUA stack.
     * \param[in] L a pointer to the LUA state.
     * \param[in] f a pointer to the C++ function to be pushed. It cannot be
     *  a non-static object member function.
     */
    template <class FPTR> inline void lua_pushwrapper(lua_State* L, FPTR f) {
	lua_wrapper<FPTR>::push(L,f);
    }

    /**
     * \brief Specialization for lua_CFunction.
     * \details No need for a wrapper if it is already a LUA function.
     */
    template<> inline void lua_pushwrapper(lua_State* L, lua_CFunction f) {
	lua_pushcfunction(L,f);
    }
    
    /**************************************************************************/

    /**
     * \brief Binds a wrapper to a name in the table at the top 
     *  of the LUA stack.
     * \pre the object on the top of the stack is a table.
     * \param[in] L a pointer to the LUA state.
     * \param[in] f a pointer to the C++ function to be wrapped. It cannot be
     *  a non-static object member function.
     */
    template <class FPTR> inline void lua_bindwrapperwithname(
	lua_State* L, FPTR f, const std::string& name
    ) {
	geo_assert(lua_gettop(L) > 0);
	geo_assert(lua_istable(L,-1));
	lua_pushstring(L,name.c_str());
	lua_pushwrapper(L,f);
	lua_settable(L,-3);
    }


    /**
     * \brief Binds a wrapper to a name in the global scole.
     * \param[in] L a pointer to the LUA state.
     * \param[in] f a pointer to the C++ function to be wrapped. It cannot be
     *  a non-static object member function.
     */
    template <class FPTR> inline void lua_bindwrapperwithnameglobal(
	lua_State* L, FPTR f, const std::string& name
    ) {
	lua_pushwrapper(L,f);
	lua_setglobal(L,name.c_str());
    }

    /**************************************************************************/

    /**
     * \brief Converts a C++ function name into a LUA function name.
     * \details Removes all namespaces from the name.
     * \param[in] L a pointer to the LUA state (unused).
     * \param[in] functionname the C++ function name.
     * \return a std::string with the function name with all namespaces removed.
     */
    inline std::string lua_wrappername(lua_State* L, const char* functionname) {
	geo_argused(L);
	std::string result(functionname);
	size_t pos = result.find_last_of(":");
	if(pos != std::string::npos) {
	    result = result.substr(pos+1, result.length()-pos);
	}
	return result;
    }

    /**************************************************************************/

    /**
     * \brief Binds a LUA wrapper around a C++ function to the table on the top
     *  of the LUA stack.
     * \details The arguments are automatically converted from the LUA stack
     *  to the function arguments. If the function is non-void, the return
     *  value is automatically converted to LUA and pushed onto the LUA stack.
     *  If the function is already a LUA interface (takes a lua_State* as an
     *  argument and returns an integer), then it will be directly called,
     *  without any conversion.
     *  The name of the function is obtained by removing all namespaces
     *  from \p f.
     * \param[in] L a pointer to the LUA state.
     * \param[in] f a pointer to the C++ function to be wrapped. It cannot be
     *  a non-static object member function.
     */
     #define lua_bindwrapper(L, f) lua_bindwrapperwithname( \
        (L),(f),GEO::lua_wrappername(L,#f)                 \
     )

    /**
     * \brief Binds a LUA wrapper around a C++ function to the global scope.
     * \details The arguments are automatically converted from the LUA stack
     *  to the function arguments. If the function is non-void, the return
     *  value is automatically converted to LUA and pushed onto the LUA stack.
     *  If the function is already a LUA interface (takes a lua_State* as an
     *  argument and returns an integer), then it will be directly called,
     *  without any conversion.
     *  The name of the function is obtained by removing all namespaces
     *  from \p f.
     * \param[in] L a pointer to the LUA state.
     * \param[in] f a pointer to the C++ function to be wrapped. It cannot be
     *  a non-static object member function.
     */
     #define lua_bindwrapperglobal(L, f) lua_bindwrapperwithnameglobal(\
   	(L),(f),GEO::lua_wrappername(L,#f)                        \
     )

    
    /*************************************************************************/
    
}

#endif
