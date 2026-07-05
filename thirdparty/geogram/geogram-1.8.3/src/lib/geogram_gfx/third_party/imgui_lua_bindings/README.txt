ImGUI LUA bindings
Downloaded from: https://github.com/patrickriordan/imgui_lua_bindings
Fri Aug 25 08:27:26 CEST 2017

./generate_imgui_bindings.pl <../ImGui/imgui.h >imgui_iterator.h

Changes in imgui_lua_bindings.cpp:

* silenced some warnings for ICC, gcc, clang
* geogram include path
* #include "imgui_iterator.cpp" -> #include "imgui_iterator.h"
  (else CMake will want to compile imgui_iterator.cpp directly)
* END_INT_POINTER and END_UINT_POINTER: use lua_pushinteger() instead
  of lua_pushnumber()  