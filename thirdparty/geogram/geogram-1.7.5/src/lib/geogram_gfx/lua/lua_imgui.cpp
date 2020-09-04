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

#include <geogram_gfx/lua/lua_imgui.h>
#include <geogram_gfx/ImGui_ext/imgui_ext.h>
#include <geogram_gfx/ImGui_ext/icon_font.h>
#include <geogram/lua/lua_wrap.h>
#include <geogram/basic/string.h>
#include <geogram/basic/logger.h>
#include <map>

extern void LoadImguiBindings();
extern lua_State* lState;

namespace {
    using namespace GEO;

    
    int wrapper_TextInput(lua_State* L) {
	
	if(
	    lua_gettop(L) != 2 &&
	    lua_gettop(L) != 3
	) {
	    return luaL_error(
		L, "'imgui.TextInput()' invalid number of arguments"
	    );
	}
	
	if(!lua_isstring(L,1)) {
	    return luaL_error(
		L, "'imgui.TextInput()' argument 1 should be a string"
	    );
	}
	
	if(!lua_isstring(L,2)) {
	    return luaL_error(
		L, "'imgui.TextInput()' argument 2 should be a string"
	    );
	}

	ImGuiInputTextFlags flags = 0;
	
	if(lua_gettop(L) == 3) {
	    if(!lua_isnumber(L,3)) {
		return luaL_error(
		    L, "'imgui.TextInput()' argument 3 should be a number"
		);
	    }
	    flags = ImGuiInputTextFlags(lua_tonumber(L,3));
	}
	
	const char* label  = lua_tostring(L,1);
	const char* str = lua_tostring(L,2);	
	static char buff[geo_imgui_string_length];
	strcpy(buff,str);
	bool result = ImGui::InputText(
	    label, buff, geo_imgui_string_length, flags
	);
	lua_pushboolean(L,result);
	lua_pushstring(L,buff);
	
	
	return 2;
    }


    int wrapper_Combo(lua_State* L) {
	if(lua_gettop(L) != 3) {
	    return luaL_error(
		L, "'imgui.Combo()' invalid number of arguments"
	    );
	}
	
	if(!lua_isstring(L,1)) {
	    return luaL_error(
		L, "'imgui.Combo()' argument should be a string"
	    );
	}
	
	if(!lua_isstring(L,2)) {
	    return luaL_error(
		L, "'imgui.Combo()' argument should be a string"
	    );
	}

	if(!lua_isstring(L,3)) {
	    return luaL_error(
		L, "'imgui.Combo()' argument should be a string"
	    );
	}
	
	const char* label = lua_tostring(L,1);
	const char* current_item = lua_tostring(L,2);
	const char* items = lua_tostring(L,3);		

	char* lua_items = (char*)alloca(strlen(items)+2);
	strcpy(lua_items,items);
	{
	    size_t n = strlen(lua_items);
	    lua_items[n] = ';';
	    lua_items[n+1] = '\0';
	}
	
	int lua_current_item=0;
	
	const char* prev_item = lua_items;
	int nb_items = 0;

	char* p = lua_items;
	while(*p != '\0') {
	    if(*p == ';') {
		*p = '\0';
		if(!strcmp(prev_item, current_item)) {
		    lua_current_item = nb_items;
		}
		prev_item = p+1;
		++nb_items;
	    }
	    ++p;
	}
	*p = '\0'; // Double '\0' to indicate end of item list to lua.

	bool result = ImGui::Combo(label, &lua_current_item, lua_items);

	current_item = lua_items;
	while(lua_current_item > 0) {
	    while(*current_item) {
		++current_item;
	    }
	    ++current_item;
	    --lua_current_item;
	}

	lua_pushboolean(L, result);
	lua_pushstring(L, current_item);

	return 2;
    }

    int wrapper_ColorEdit3WithPalette(
	lua_State* L	
    ) {
	if(lua_gettop(L) != 4) {
	    return luaL_error(
		L, "'imgui.ColorEdit3WithPalette()' invalid number of arguments"
	    );
	}
	
	if(!lua_isstring(L,1)) {
	    return luaL_error(
		L, "'imgui.ColorEdit3WithPalette()' argument 1 should be a string"
	    );
	}

	if(!lua_isnumber(L,2)) {
	    return luaL_error(
		L, "'imgui.ColorEdit3WithPalette()' argument 2 should be a number"
	    );
	}

	if(!lua_isnumber(L,3)) {
	    return luaL_error(
		L, "'imgui.ColorEdit3WithPalette()' argument 3 should be a number"
	    );
	}

	if(!lua_isnumber(L,4)) {
	    return luaL_error(
		L, "'imgui.ColorEdit3WithPalette()' argument 4 should be a number"
	    );
	}

	const char* label = lua_tostring(L,1);
	
	float rgb[3];
	rgb[0] = float(lua_tonumber(L,2));
	rgb[1] = float(lua_tonumber(L,3));
	rgb[2] = float(lua_tonumber(L,4));	

	bool sel = ImGui::ColorEdit3WithPalette(
	    label, rgb
	);

	lua_pushboolean(L,sel);
	lua_pushnumber(L,double(rgb[0]));
	lua_pushnumber(L,double(rgb[1]));
	lua_pushnumber(L,double(rgb[2]));

	return 4;
    }

    int wrapper_ColorEdit4WithPalette(
	lua_State* L	
    ) {
	if(lua_gettop(L) != 5) {
	    return luaL_error(
		L, "'imgui.ColorEdit3WithPalette()' invalid number of arguments"
	    );
	}
	
	if(!lua_isstring(L,1)) {
	    return luaL_error(
		L, "'imgui.ColorEdit3WithPalette()' argument 1 should be a string"
	    );
	}

	if(!lua_isnumber(L,2)) {
	    return luaL_error(
		L, "'imgui.ColorEdit3WithPalette()' argument 2 should be a number"
	    );
	}

	if(!lua_isnumber(L,3)) {
	    return luaL_error(
		L, "'imgui.ColorEdit3WithPalette()' argument 3 should be a number"
	    );
	}

	if(!lua_isnumber(L,4)) {
	    return luaL_error(
		L, "'imgui.ColorEdit3WithPalette()' argument 4 should be a number"
	    );
	}

	if(!lua_isnumber(L,5)) {
	    return luaL_error(
		L, "'imgui.ColorEdit3WithPalette()' argument 5 should be a number"
	    );
	}
	
	const char* label = lua_tostring(L,1);
	
	float rgb[4];
	rgb[0] = float(lua_tonumber(L,2));
	rgb[1] = float(lua_tonumber(L,3));
	rgb[2] = float(lua_tonumber(L,4));
	rgb[3] = float(lua_tonumber(L,5));		

	bool sel = ImGui::ColorEdit4WithPalette(
	    label, rgb
	);

	lua_pushboolean(L,sel);
	lua_pushnumber(L,double(rgb[0]));
	lua_pushnumber(L,double(rgb[1]));
	lua_pushnumber(L,double(rgb[2]));
	lua_pushnumber(L,double(rgb[3]));	

	return 5;
    }

    
    int wrapper_OpenFileDialog(
	lua_State* L
    ) {
	if(lua_gettop(L) != 4) {
	    return luaL_error(
		L, "'imgui.OpenFileDialog()' invalid number of arguments"
	    );
	}

	if(!lua_isstring(L,1)) {
	    return luaL_error(
		L, "'imgui.OpenFileDialog()' argument 1 should be a string"
	    );
	}

	if(!lua_isstring(L,2)) {
	    return luaL_error(
		L, "'imgui.OpenFileDialog()' argument 2 should be a string"
	    );
	}

	if(!lua_isstring(L,3)) {
	    return luaL_error(
		L, "'imgui.OpenFileDialog()' argument 3 should be a string"
	    );
	}

	if(!lua_isnumber(L,4)) {
	    return luaL_error(
		L, "'imgui.OpenFileDialog()' argument 4 should be a number"
	    );
	}

	const char* label      = lua_tostring(L,1);
	const char* extensions = lua_tostring(L,2);
	const char* filename   = lua_tostring(L,3);
	ImGuiExtFileDialogFlags flags =
	    ImGuiExtFileDialogFlags(lua_tonumber(L,4));

	ImGui::OpenFileDialog(label, extensions, filename, flags);
	
	return 0;
    }

    int wrapper_FileDialog(
	lua_State* L
    ) {
	if(lua_gettop(L) != 2) {
	    return luaL_error(
		L, "'imgui.FileDialog()' invalid number of arguments"
	    );
	}

	if(!lua_isstring(L,1)) {
	    return luaL_error(
		L, "'imgui.OpenFileDialog()' argument 1 should be a string"
	    );
	}

	if(!lua_isstring(L,2)) {
	    return luaL_error(
		L, "'imgui.OpenFileDialog()' argument 2 should be a string"
	    );
	}

	const char* label      = lua_tostring(L,1);
	char filename[geo_imgui_string_length];

	const char* filename_in = lua_tostring(L,2);
	if(filename_in != nullptr) {
	    if(strlen(filename_in) > geo_imgui_string_length + 1) {
		Logger::err("ImGui") << "Max file name length exceeded"
				     << std::endl;
		return false;
	    }
	    strcpy(filename, filename_in);
	} else {
	    filename[0] = '\0';
	}
	
	bool result =
	    ImGui::FileDialog(label, filename, geo_imgui_string_length);

	lua_pushboolean(L,result);
	lua_pushstring(L, result ? filename : filename_in);
	
	return 2;
    }

    int wrapper_SetNextWindowPos(lua_State* L) {
	if(
	    lua_gettop(L) != 2 &&
	    lua_gettop(L) != 3
	) {
	    return luaL_error(
		L, "'imgui.SetNextWindowPos()' invalid number of arguments"
	    );
	}

	if(!lua_isnumber(L,1)) {
	    return luaL_error(
		L, "'imgui.SetNextWindowPos()' argument 1 should be a number"
	    );
	}

	if(!lua_isnumber(L,2)) {
	    return luaL_error(
		L, "'imgui.SetNextWindowPos()' argument 2 should be a number"
	    );
	}

	ImGuiCond cond = 0;
	if(lua_gettop(L) == 3) {
	    if(!lua_isnumber(L,3)) {
		return luaL_error(
		    L,
		    "'imgui.SetNextWindowPos()' argument 3 should be a number"
		);
	    }
	    cond = ImGuiCond(lua_tonumber(L,3));
	}

	
	ImGui::SetNextWindowPos(
	    ImVec2(float(lua_tonumber(L,1)), float(lua_tonumber(L,2))),
	    cond
	);

	return 0;
    }

    int wrapper_SetNextWindowSize(lua_State* L) {
	if(
	    lua_gettop(L) != 2 &&
	    lua_gettop(L) != 3
	) {
	    return luaL_error(
		L, "'imgui.SetNextWindowSize()' invalid number of arguments"
	    );
	}

	if(!lua_isnumber(L,1)) {
	    return luaL_error(
		L, "'imgui.SetNextWindowSize()' argument 1 should be a number"
	    );
	}

	if(!lua_isnumber(L,2)) {
	    return luaL_error(
		L, "'imgui.SetNextWindowSize()' argument 2 should be a number"
	    );
	}

	ImGuiCond cond = 0;
	if(lua_gettop(L) == 3) {
	    if(!lua_isnumber(L,3)) {
		return luaL_error(
		    L,
		    "'imgui.SetNextWindowSize()' argument 3 should be a number"
		);
	    }
	    cond = ImGuiCond(lua_tonumber(L,3));
	}

	
	ImGui::SetNextWindowSize(
	    ImVec2(float(lua_tonumber(L,1)), float(lua_tonumber(L,2))),
	    cond
	);

	return 0;
    }


    int wrapper_IsItemHovered(lua_State* L) {
	if(lua_gettop(L) != 0) {
	    return luaL_error(
		L, "'imgui.IsItemHovered()' invalid number of arguments"
	    );
	}
	lua_pushboolean(L,ImGui::IsItemHovered());
	return 1;
    }

    int wrapper_Text(lua_State* L) {
	if(lua_gettop(L) < 1) {
	    return luaL_error(
		L, "'imgui.Text()' invalid number of arguments"
	    );
	}
	const char* str = lua_tostring(L,1);
	ImGui::Text("%s",str);
	return 0;
    }

    int wrapper_SetTooltip(lua_State* L) {
	if(lua_gettop(L) != 1) {
	    return luaL_error(
		L, "'imgui.SetTooltip()' invalid number of arguments"
	    );
	}
	const char* str = lua_tostring(L,1);
	ImGui::SetTooltip("%s",str);
	return 0;
    }

    int wrapper_ShowStyleEditor(lua_State* L) {
	if(lua_gettop(L) != 0) {
	    return luaL_error(
		L, "'imgui.ShowStyleEditor()' invalid number of arguments"
	    );
	}
	ImGui::ShowStyleEditor(nullptr);
	return 0;
    }

    int wrapper_PushFont(lua_State* L) {
	if(lua_gettop(L) != 1) {
	    return luaL_error(
		L, "'imgui.PushFont()' invalid number of arguments"
	    );
	}
	if(!lua_isinteger(L,1)) {
	    return luaL_error(
		L, "'imgui.PushFont()' argument is not an integer"
	    );
	}
	int idx = int(lua_tointeger(L,1));
	if(idx < 0 || idx >= ImGui::GetIO().Fonts->Fonts.size()) {
	    return luaL_error(
		L, "'imgui.PushFont()' invalid font index"
	    );
	}
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[idx]);
	return 0;
    }

    int wrapper_font_icon(lua_State* L) {
	if(lua_gettop(L) != 1) {
	    return luaL_error(
		L, "'imgui.font_icon()' invalid number of arguments"
	    );
	}
	if(!lua_isstring(L,1)) {
	    return luaL_error(
		L, "'imgui.font_icon()' argument is not a string"
	    );
	}
	const char* K = lua_tostring(L,1);
	wchar_t result[2];
	result[0] = icon_wchar(K);
	result[1] = '\0';	
	std::string result_str = String::wchar_to_UTF8(result);
	lua_pushstring(L, result_str.c_str());
	return 1;
    }

    int wrapper_BeginTabBar(lua_State* L) {
	if(lua_gettop(L) != 1) {
	    return luaL_error(
		L, "'imgui.BeginTabBar()' invalid number of arguments"
	    );
	}
	if(!lua_isstring(L,1)) {
	    return luaL_error(
		L, "'imgui.BeinTabBar()' argument is not a string"
	    );
	}
	const char* K = lua_tostring(L,1);
	ImGui::BeginTabBar(K);
	return 0;
    }

    int wrapper_BeginTabItem(lua_State* L) {
	if(lua_gettop(L) != 1) {
	    return luaL_error(
		L, "'imgui.BeginTabItem()' invalid number of arguments"
	    );
	}
	if(!lua_isstring(L,1)) {
	    return luaL_error(
		L, "'imgui.BeginTabItem()' argument is not a string"
	    );
	}
	const char* K = lua_tostring(L,1);
	lua_pushboolean(L,ImGui::BeginTabItem(K));
	return 1;
    }

    int wrapper_SimpleButton(lua_State* L) {
	if(lua_gettop(L) != 1) {
	    return luaL_error(
		L, "'imgui.SimpleButton()' invalid number of arguments"
	    );
	}
	if(!lua_isstring(L,1)) {
	    return luaL_error(
		L, "'imgui.SimpleButton()' argument is not a string"
	    );
	}
	const char* K = lua_tostring(L,1);
	lua_pushboolean(L,ImGui::SimpleButton(K));
	return 1;
    }

    int wrapper_GetMousePos(lua_State* L) {
	if(lua_gettop(L) != 0) {
	    return luaL_error(
		L, "'imgui.GetMousePos()' invalid number of arguments"
	    );
	}
	lua_pushnumber(L,double(ImGui::GetIO().MousePos.x));
	lua_pushnumber(L,double(ImGui::GetIO().MousePos.y));	
	return 2;
    }
    
}

namespace GEO {
    
    /**
     * \brief Specialization of lua_push() for ImDrawList*
     */
    template<> inline void lua_push(
	lua_State* L, ImDrawList* x
    ) {
	lua_pushlightuserdata(L,(void*)x);
    }

    /**
     * \brief lua_to specialization for ImDrawList*
     */
    template<> class lua_to<ImDrawList*> {
      public:
	lua_to(lua_State* L, int idx) {
	    x_ = (ImDrawList*)(lua_touserdata(L,idx));
	}
	static bool can_convert(lua_State* L, int idx) {
	    return lua_check_type(L, idx, my_lua_islightuserdata);
	}
	operator ImDrawList*() const {
	    return x_;
	}
      private:
	ImDrawList* x_;
    };
    
}

// WIP: export all ImGuiDrawList functions to LUA

namespace ImGuiDrawAdapters {

    // Needed by lua_bindwrapper because ImGui::GetBackgroundDrawList()
    // takes an ImViewport (default = nullptr) as an argunemt.
    static ImDrawList* GetBackgroundDrawList() {
	return ImGui::GetBackgroundDrawList();
    }

    // Needed by lua_bindwrapper because ImGui::GetForegroundDrawList()
    // takes an ImViewport (default = nullptr) as an argunemt.
    static ImDrawList* GetForegroundDrawList() {
	return ImGui::GetForegroundDrawList();
    }

    static void PushClipRect(
	ImDrawList* list,
	float xmin, float ymin,
	float xmax, float ymax,
	bool intersect
    ) {
	list->PushClipRect(
	    ImVec2(xmin, ymin),
	    ImVec2(xmax, ymax),
	    intersect
	);
    }

    static void PushClipRectFullScreen(ImDrawList* list) {
	list->PushClipRectFullScreen();
    }

    static void PopClipRect(ImDrawList* list) {
	list->PopClipRect();
    }

    static void PushTextureID(ImDrawList* list, index_t id) {
	union {
	    ImTextureID im_texture_id;
	    index_t gl_texture_id;
	};
	gl_texture_id = id;
	list->PushTextureID(im_texture_id);
    }
    
    static void PopTextureID(ImDrawList* list) {
	list->PopClipRect();
    }
    
    static void AddLine(
	ImDrawList* list, float x1, float y1, float x2, float y2,
	Numeric::uint32 color, float thickness
    ) {
	list->AddLine(
	    ImVec2(x1,y1), ImVec2(x2,y2),
	    color, thickness
	);
    }

    static void AddRect(
	ImDrawList* list,
	float x1, float y1, float x2, float y2,
	Numeric::uint32 color, float rounding, int rounding_corners,
	float thickness
    ) {
	list->AddRect(
	    ImVec2(x1,y1), ImVec2(x2,y2), color, rounding,
	    ImDrawCornerFlags(rounding_corners), thickness
	);
    }

    static void AddRectFilled(
	ImDrawList* list,
	float x1, float y1, float x2, float y2,
	Numeric::uint32 color, float rounding, int rounding_corners
    ) {
	list->AddRectFilled(
	    ImVec2(x1,y1), ImVec2(x2,y2), color, rounding,
	    ImDrawCornerFlags(rounding_corners)
	);
    }

    static void AddRectFilledMultiColor(
	ImDrawList* list,
	float x1, float y1, float x2, float y2,
	Numeric::uint32 color1,	Numeric::uint32 color2,
	Numeric::uint32 color3,	Numeric::uint32 color4
    ) {
	list->AddRectFilledMultiColor(
	    ImVec2(x1,y1), ImVec2(x2,y2),	    
	    color1, color2, color3, color4
	);
    }

    static void AddQuad(
	ImDrawList* list,
	float x1, float y1, float x2, float y2,
	float x3, float y3, float x4, float y4,	
	Numeric::uint32 color,
	float thickness
    ) {
	list->AddQuad(
	    ImVec2(x1,y1), ImVec2(x2,y2), ImVec2(x3,y3), ImVec2(x4,y4),	    
	    color, thickness
	);
    }

    static void AddQuadFilled(
	ImDrawList* list, float x1, float y1, float x2, float y2,
	float x3, float y3, float x4, float y4,	Numeric::uint32 color
    ) {
	list->AddQuad(
	    ImVec2(x1,y1), ImVec2(x2,y2), ImVec2(x3,y3), ImVec2(x4,y4), color
	);
    }

    static void AddTriangle(
	ImDrawList* list, float x1, float y1, float x2, float y2,
	float x3, float y3, Numeric::uint32 color, float thickness
    ) {
	list->AddTriangle(
	    ImVec2(x1,y1), ImVec2(x2,y2), ImVec2(x3,y3), color, thickness
	);
    }

    static void AddTriangleFilled(
	ImDrawList* list, float x1, float y1, float x2, float y2,
	float x3, float y3, Numeric::uint32 color
    ) {
	list->AddTriangleFilled(ImVec2(x1,y1), ImVec2(x2,y2), ImVec2(x3,y3), color);
    }

    static void AddCircle(
	ImDrawList* list, float x, float y, float radius, Numeric::uint32 color,
	int num_segments, float thickness
    ) {
	list->AddCircle(ImVec2(x,y), radius, color, num_segments, thickness);
    }

    static void AddCircleFilled(
	ImDrawList* list, float x, float y, float radius, Numeric::uint32 color,
	int num_segments
    ) {
	list->AddCircleFilled(ImVec2(x,y), radius, color, num_segments);
    }

    static void AddText(
	ImDrawList* list, float x, float y, index_t color, const char* text
    ) {
	list->AddText(ImVec2(x,y), color, text);
    }

    static void AddText2(
	ImDrawList* list, index_t font, float font_size, float x, float y,
	index_t color, const char* text
    ) {
	if(int(font) >= ImGui::GetIO().Fonts->Fonts.size()) {
	    return;
	}
	ImFont* imfont = ImGui::GetIO().Fonts->Fonts[int(font)];
	list->AddText(imfont, font_size, ImVec2(x,y), color, text);
    }

    static void AddBezierCurve(
	ImDrawList* list, float x1, float y1, float x2, float y2,
	float x3, float y3, float x4, float y4, index_t color,
	float thickness, int num_segments
    ) {
	list->AddBezierCurve(
	    ImVec2(x1,y1), ImVec2(x2,y2), ImVec2(x3,y3), ImVec2(x4,y4),
	    color, thickness, num_segments
	);
    }

    static void AddImage(
	ImDrawList* list, index_t id,
	float x1, float y1, float x2, float y2,
	float u1, float v1, float u2, float v2,
	index_t color
    ) {
	union {
	    ImTextureID im_texture_id;
	    index_t gl_texture_id;
	};
	gl_texture_id = id;
	list->AddImage(
	    im_texture_id,
	    ImVec2(x1,y1), ImVec2(x2,y2),
	    ImVec2(u1,v1), ImVec2(u2,v2),
	    color
	);
    }

    static void AddImageRounded(
	ImDrawList* list, index_t id,
	float x1, float y1, float x2, float y2,
	float u1, float v1, float u2, float v2,
	index_t color, float rounding, int rounding_corners
    ) {
	union {
	    ImTextureID im_texture_id;
	    index_t gl_texture_id;
	};
	gl_texture_id = id;
	list->AddImageRounded(
	    im_texture_id,
	    ImVec2(x1,y1), ImVec2(x2,y2),
	    ImVec2(u1,v1), ImVec2(u2,v2),
	    color, rounding, ImDrawCornerFlags(rounding_corners)
	);
    }

    static void PathClear(ImDrawList* list) {
	list->PathClear();
    }

    static void PathLineTo(ImDrawList* list, float x, float y) {
	list->PathLineTo(ImVec2(x,y));
    }

    static void PathLineToMergeDuplicate(ImDrawList* list, float x, float y) {
	list->PathLineToMergeDuplicate(ImVec2(x,y));
    }

    static void PathFillConvex(ImDrawList* list, index_t color) {
	list->PathFillConvex(color);
    }

    static void PathStroke(
	ImDrawList* list, index_t color, bool closed, float thickness
    ) {
	list->PathStroke(color, closed, thickness);
    }

    static void PathArcTo(
	ImDrawList* list, float cx, float cy, float radius, float a1, float a2,
	int num_segments
    ) {
	list->PathArcTo(ImVec2(cx,cy), radius, a1, a2, num_segments);
    }

    static void PathArcToFast(
	ImDrawList* list, float cx, float cy, float radius, int a1, int a2
    ) {
	list->PathArcToFast(ImVec2(cx,cy), radius, a1, a2);
    }

    static void PathBezierCurveTo(
	ImDrawList* list,
	float x1, float y1, float x2, float y2, float x3, float y3,
	int num_segments
    ) {
	list->PathBezierCurveTo(
	    ImVec2(x1,y1), ImVec2(x2,y2), ImVec2(x3,y3), num_segments
	);
    }

    static void PathRect(
	ImDrawList* list,
	float x1, float y1, float x2, float y2,
	float rounding, int rounding_corners	
    ) {
	list->PathRect(
	    ImVec2(x1,y1), ImVec2(x2,y2),
	    rounding, ImDrawCornerFlags(rounding_corners)
	);
    }
}


#define DECLARE_IMGUI_CONSTANT(C) \
	lua_pushinteger(L,C);     \
	lua_setglobal(L,#C)


void init_lua_imgui(lua_State* L) {
    lState = L;
    LoadImguiBindings();

    DECLARE_IMGUI_CONSTANT(ImGuiExtFileDialogFlags_Load);
    DECLARE_IMGUI_CONSTANT(ImGuiExtFileDialogFlags_Save);
    DECLARE_IMGUI_CONSTANT(ImGuiCond_Always);
    DECLARE_IMGUI_CONSTANT(ImGuiCond_Once);
    DECLARE_IMGUI_CONSTANT(ImGuiCond_FirstUseEver);
    DECLARE_IMGUI_CONSTANT(ImGuiCond_Appearing);
    DECLARE_IMGUI_CONSTANT(ImGuiSelectableFlags_AllowDoubleClick);

    DECLARE_IMGUI_CONSTANT(ImGuiCol_Text);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_TextDisabled);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_WindowBg);              
    DECLARE_IMGUI_CONSTANT(ImGuiCol_ChildBg);               
    DECLARE_IMGUI_CONSTANT(ImGuiCol_PopupBg);               
    DECLARE_IMGUI_CONSTANT(ImGuiCol_Border);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_BorderShadow);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_FrameBg);               
    DECLARE_IMGUI_CONSTANT(ImGuiCol_FrameBgHovered);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_FrameBgActive);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_TitleBg);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_TitleBgActive);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_TitleBgCollapsed);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_MenuBarBg);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_ScrollbarBg);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_ScrollbarGrab);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_ScrollbarGrabHovered);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_ScrollbarGrabActive);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_CheckMark);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_SliderGrab);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_SliderGrabActive);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_Button);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_ButtonHovered);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_ButtonActive);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_Header);                
    DECLARE_IMGUI_CONSTANT(ImGuiCol_HeaderHovered);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_HeaderActive);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_Separator);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_SeparatorHovered);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_SeparatorActive);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_ResizeGrip);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_ResizeGripHovered);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_ResizeGripActive);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_Tab);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_TabHovered);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_TabActive);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_TabUnfocused);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_TabUnfocusedActive);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_DockingPreview);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_DockingEmptyBg);        
    DECLARE_IMGUI_CONSTANT(ImGuiCol_PlotLines);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_PlotLinesHovered);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_PlotHistogram);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_PlotHistogramHovered);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_TextSelectedBg);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_DragDropTarget);
    DECLARE_IMGUI_CONSTANT(ImGuiCol_NavHighlight);          
    DECLARE_IMGUI_CONSTANT(ImGuiCol_NavWindowingHighlight); 
    DECLARE_IMGUI_CONSTANT(ImGuiCol_NavWindowingDimBg);     
    DECLARE_IMGUI_CONSTANT(ImGuiCol_ModalWindowDimBg);      
    
    lua_getglobal(L, "imgui");

    lua_pushliteral(L,"TextInput");
    lua_pushcfunction(L,wrapper_TextInput); 
    lua_settable(L,-3);

    lua_pushliteral(L,"Combo");
    lua_pushcfunction(L,wrapper_Combo); 
    lua_settable(L,-3);

    lua_pushliteral(L,"ColorEdit3WithPalette");
    lua_pushcfunction(L,wrapper_ColorEdit3WithPalette); 
    lua_settable(L,-3);

    lua_pushliteral(L,"ColorEdit4WithPalette");
    lua_pushcfunction(L,wrapper_ColorEdit4WithPalette); 
    lua_settable(L,-3);
    
    lua_pushliteral(L,"OpenFileDialog");
    lua_pushcfunction(L,wrapper_OpenFileDialog);
    lua_settable(L,-3);

    lua_pushliteral(L,"FileDialog");
    lua_pushcfunction(L,wrapper_FileDialog);
    lua_settable(L,-3);

    lua_pushliteral(L,"SetNextWindowPos");
    lua_pushcfunction(L,wrapper_SetNextWindowPos);
    lua_settable(L,-3);

    lua_pushliteral(L,"SetNextWindowSize");
    lua_pushcfunction(L,wrapper_SetNextWindowSize);
    lua_settable(L,-3);
    
    lua_pushliteral(L,"IsItemHovered");
    lua_pushcfunction(L,wrapper_IsItemHovered);
    lua_settable(L,-3);

    lua_pushliteral(L,"Text");
    lua_pushcfunction(L,wrapper_Text);
    lua_settable(L,-3);

    lua_pushliteral(L,"SetTooltip");
    lua_pushcfunction(L,wrapper_SetTooltip);
    lua_settable(L,-3);

    lua_pushliteral(L,"ShowStyleEditor");
    lua_pushcfunction(L,wrapper_ShowStyleEditor);
    lua_settable(L,-3);

    lua_pushliteral(L,"PushFont");
    lua_pushcfunction(L,wrapper_PushFont);
    lua_settable(L,-3);

    lua_pushliteral(L,"font_icon");
    lua_pushcfunction(L,wrapper_font_icon);
    lua_settable(L,-3);

    lua_pushliteral(L,"BeginTabBar");
    lua_pushcfunction(L,wrapper_BeginTabBar);
    lua_settable(L,-3);

    lua_pushliteral(L,"BeginTabItem");
    lua_pushcfunction(L,wrapper_BeginTabItem);
    lua_settable(L,-3);

    lua_pushliteral(L,"SimpleButton");
    lua_pushcfunction(L,wrapper_SimpleButton);
    lua_settable(L,-3);

    lua_pushliteral(L,"GetMousePos");
    lua_pushcfunction(L,wrapper_GetMousePos);
    lua_settable(L,-3);
    
    /*****************************************************************/
    
    lua_bindwrapper(L,ImGui::GetWindowDrawList);
    lua_bindwrapper(L,ImGuiDrawAdapters::GetBackgroundDrawList);
    lua_bindwrapper(L,ImGuiDrawAdapters::GetForegroundDrawList);
    
    lua_bindwrapper(L,ImGuiDrawAdapters::PushClipRect);
    lua_bindwrapper(L,ImGuiDrawAdapters::PushClipRectFullScreen);    
    lua_bindwrapper(L,ImGuiDrawAdapters::PopClipRect);
    
    lua_bindwrapper(L,ImGuiDrawAdapters::PushTextureID);
    lua_bindwrapper(L,ImGuiDrawAdapters::PopTextureID);
    
    lua_bindwrapper(L,ImGuiDrawAdapters::AddLine);
    lua_bindwrapper(L,ImGuiDrawAdapters::AddRect);
    lua_bindwrapper(L,ImGuiDrawAdapters::AddRectFilled);
    lua_bindwrapper(L,ImGuiDrawAdapters::AddRectFilledMultiColor);            
    lua_bindwrapper(L,ImGuiDrawAdapters::AddQuad);
    lua_bindwrapper(L,ImGuiDrawAdapters::AddQuadFilled);
    lua_bindwrapper(L,ImGuiDrawAdapters::AddTriangle);
    lua_bindwrapper(L,ImGuiDrawAdapters::AddTriangleFilled);
    lua_bindwrapper(L,ImGuiDrawAdapters::AddCircle);
    lua_bindwrapper(L,ImGuiDrawAdapters::AddCircleFilled);
    lua_bindwrapper(L,ImGuiDrawAdapters::AddText);
    lua_bindwrapper(L,ImGuiDrawAdapters::AddText2);
    lua_bindwrapper(L,ImGuiDrawAdapters::AddBezierCurve);
    lua_bindwrapper(L,ImGuiDrawAdapters::AddImage);
    lua_bindwrapper(L,ImGuiDrawAdapters::AddImageRounded);         
    lua_bindwrapper(L,ImGuiDrawAdapters::PathClear);
    lua_bindwrapper(L,ImGuiDrawAdapters::PathLineTo);
    lua_bindwrapper(L,ImGuiDrawAdapters::PathLineToMergeDuplicate);
    lua_bindwrapper(L,ImGuiDrawAdapters::PathFillConvex);
    lua_bindwrapper(L,ImGuiDrawAdapters::PathStroke);
    lua_bindwrapper(L,ImGuiDrawAdapters::PathArcTo);
    lua_bindwrapper(L,ImGuiDrawAdapters::PathArcToFast);        
    lua_bindwrapper(L,ImGuiDrawAdapters::PathBezierCurveTo);
    lua_bindwrapper(L,ImGuiDrawAdapters::PathRect);                
    
    lua_pop(L,1);

    DECLARE_IMGUI_CONSTANT(ImDrawCornerFlags_None);
    DECLARE_IMGUI_CONSTANT(ImDrawCornerFlags_TopLeft);
    DECLARE_IMGUI_CONSTANT(ImDrawCornerFlags_TopRight);
    DECLARE_IMGUI_CONSTANT(ImDrawCornerFlags_BotLeft);
    DECLARE_IMGUI_CONSTANT(ImDrawCornerFlags_BotRight);
    DECLARE_IMGUI_CONSTANT(ImDrawCornerFlags_Top);
    DECLARE_IMGUI_CONSTANT(ImDrawCornerFlags_Bot);
    DECLARE_IMGUI_CONSTANT(ImDrawCornerFlags_Left);
    DECLARE_IMGUI_CONSTANT(ImDrawCornerFlags_Right);
    DECLARE_IMGUI_CONSTANT(ImDrawCornerFlags_All);
    
}

