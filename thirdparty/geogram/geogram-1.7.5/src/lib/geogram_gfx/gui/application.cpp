/*
 *  Copyright (c) 2012-2016, Bruno Levy All rights reserved.
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

#include <geogram_gfx/gui/application.h>

#include <geogram_gfx/third_party/ImGui/imgui.h>

#include <geogram_gfx/third_party/ImGui/imgui_impl_opengl3.h>
#include <geogram_gfx/ImGui_ext/icon_font.h>

#include <geogram_gfx/lua/lua_glup.h>
#include <geogram_gfx/lua/lua_imgui.h>

#include <geogram_gfx/third_party/imgui_fonts/cousine_regular.h>
#include <geogram_gfx/third_party/imgui_fonts/roboto_medium.h>
#include <geogram_gfx/third_party/imgui_fonts/fa_solid.h>

#include <geogram/image/image.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/command_line_args.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/file_system.h>


#if defined(GEO_GLFW)
#  include <geogram_gfx/third_party/ImGui/imgui_impl_glfw.h>
// Too many documentation warnings in glfw
// (glfw uses tags that clang does not understand).
#  ifdef __clang__
#    pragma GCC diagnostic ignored "-Wdocumentation"
#  endif

#  if defined(GEO_USE_SYSTEM_GLFW3) || defined(GEO_OS_EMSCRIPTEN)
#    include <GLFW/glfw3.h>
#  else
#    include <third_party/glfw/include/GLFW/glfw3.h>
#  endif

#ifdef GEO_OS_EMSCRIPTEN
#  include <emscripten.h>
#endif


#endif


namespace GEO {

    void StyleColorsCorporateGrey(bool threeD);    
    /**
     * \brief Computes the pixel ratio for hidpi devices.
     * \details Uses the current GLFW window.
     */
    double compute_pixel_ratio();

    /**
     * \brief Computes the scaling factor for hidpi devices.
     * \details Uses the current GLFW window.
     */
    double compute_hidpi_scaling();
    

    /**
     * \brief If nothing happens during 100 frames, then
     * we (micro)-sleep instead of redrawing the
     * window.
     */
    const index_t NB_FRAMES_UPDATE_INIT = 100;
    
#if defined(GEO_GLFW)
    class ApplicationData {
    public:
	ApplicationData() {
	    window_ = nullptr;
	    GLFW_callbacks_initialized_ = false;
	    ImGui_callback_mouse_button = nullptr;
	    ImGui_callback_cursor_pos = nullptr;	
	    ImGui_callback_scroll = nullptr;
	    ImGui_callback_key = nullptr;	
	    ImGui_callback_char = nullptr;
	    ImGui_callback_drop = nullptr;
	    ImGui_callback_refresh = nullptr;
	}
	GLFWwindow* window_;
	bool GLFW_callbacks_initialized_;
	GLFWmousebuttonfun ImGui_callback_mouse_button;
	GLFWcursorposfun ImGui_callback_cursor_pos;
	GLFWscrollfun ImGui_callback_scroll;
	GLFWkeyfun ImGui_callback_key;
	GLFWcharfun ImGui_callback_char;
	GLFWdropfun ImGui_callback_drop;
	GLFWwindowrefreshfun ImGui_callback_refresh;
    };

#else
#  error "No windowing system"    
#endif    

}

namespace GEO {

    Application* Application::instance_ = nullptr;
    
    Application::Application(const std::string& name) {
	geo_assert(instance_ == nullptr);
	GEO::initialize();
	instance_ = this;
	name_ = name;
	data_ = new ApplicationData();
	ImGui_restart_ = false;
	ImGui_reload_font_ = false;
	ImGui_initialized_ = false;
	ImGui_firsttime_init_ = false;	
	width_ = 800;
	height_ = 800;
	frame_buffer_width_ = 800;
	frame_buffer_height_ = 800;
	in_main_loop_ = false;
	accept_drops_ = true;
	scaling_ = 1.0;
	font_size_ = 18;	
	nb_update_locks_ = 0;
	nb_frames_update_ = NB_FRAMES_UPDATE_INIT;
	hidpi_scaling_ = 1.0;
	pixel_ratio_ = 1.0;
	currently_drawing_gui_ = false;
	animate_ = false;
	menubar_visible_ = true;
	phone_screen_ = false;
	soft_keyboard_visible_ = false;
    }

    Application::~Application() {
	delete_window();
	geo_assert(instance_ == this);
	delete data_;
	data_ = nullptr;
	instance_ = nullptr;
    }

    void Application::start(int argc, char** argv) {
	if(argc != 0 && argv != nullptr) {
	    geogram_initialize(argc, argv);
	}
	if(CmdLine::arg_is_declared("gui:font_size")) {
	    set_font_size(CmdLine::get_arg_uint("gui:font_size"));
	}
	create_window();
	main_loop();
    }
    
    void Application::stop() {
	in_main_loop_ = false;
    }

    std::string Application::get_styles() {
#ifdef GEO_OS_ANDROID
	return "Light;Dark";	
#else
	return "Light;Dark;CorporateGrey";
#endif	
    }
    
    void Application::set_style(const std::string& style_name) {
	style_ = style_name;

	if(style_name == "Light") {
	    ImGui::StyleColorsLight();
	    ImGuiStyle& style = ImGui::GetStyle();
	    style.WindowRounding = 10.0f;
	    style.FrameRounding = 5.0f;
	    style.GrabRounding = 10.0f;
	    style.WindowBorderSize = 1.5f;
	    style.FrameBorderSize = 1.0f;
	    style.PopupBorderSize = 1.0f;
	    ImVec4* colors = style.Colors;
	    colors[ImGuiCol_Text]         = ImVec4(0.0f,  0.0f,  0.25f, 1.00f);
	    colors[ImGuiCol_TextDisabled] = ImVec4(0.25f, 0.25f, 0.75f, 1.00f);
	    colors[ImGuiCol_Separator]    = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
	} else if(style_name == "Dark") {
	    ImGuiStyle& style = ImGui::GetStyle();
	    style.WindowRounding = 10.0f;
	    style.FrameRounding = 5.0f;
	    style.GrabRounding = 10.0f;
	    style.WindowBorderSize = 1.5f;
	    style.FrameBorderSize = 0.0f;
	    style.PopupBorderSize = 1.0f;
	    ImGui::StyleColorsDark();
	} else if(style_name == "CorporateGrey") {
	    StyleColorsCorporateGrey(true);
	} else {
	    set_style("Light");
	    Logger::err("Skin") << style_name << ": no such style"
				<< std::endl;
	}

	ImGuiStyle& style = ImGui::GetStyle();
	if(CmdLine::get_arg_bool("gfx:transparent")) {
	    // Make ImGui windows opaque if background is
	    // transparent (else it becomes difficult to
	    // distinguish anything...)
	    style.Alpha = 1.0f;	    
	} else {
	    style.Alpha = 0.90f;
	}

	if(phone_screen_) {
	    style.ScrollbarSize = 10.0f * float(scaling_);
	    style.GrabMinSize   = 15.0f * float(scaling_);
	}
    }
    
    void Application::set_font_size(index_t value) {
	font_size_ = index_t(value);
	scaling_ = double(font_size_)/16.0;
	if(phone_screen_) {
	    scaling_ *= double(std::max(get_width(), get_height()))/600.0;
	}
	if(CmdLine::arg_is_declared("gui:font_size")) {	
	    CmdLine::set_arg("gui:font_size", String::to_string(value));
	}
	if(ImGui_initialized_) {
	    ImGui_reload_font_ = true;
	}
    }

    double Application::scaling() const {
        return scaling_ * hidpi_scaling_ / pixel_ratio_;
    }

    void Application::resize(
	index_t w, index_t h, index_t fb_w, index_t fb_h
    ) {
	width_ = w;
	height_ = h;
	frame_buffer_width_ = fb_w;
	frame_buffer_height_ = fb_h;
	scaling_ = double(font_size_)/16.0;
	if(phone_screen_) {
	    scaling_ *= double(std::max(get_width(), get_height()))/600.0;
	}
	update();
    }

    void Application::draw_gui() {
    }

    void Application::draw_graphics() {
    }
    
    void Application::mouse_button_callback(
	int button, int action, int mods, int source
    ) {
	geo_argused(button);
	geo_argused(action);
	geo_argused(mods);
	geo_argused(source);
    }

    void Application::scroll_callback(double xoffset, double yoffset) {
	geo_argused(xoffset);
	geo_argused(yoffset);
    }

    void Application::cursor_pos_callback(double x, double y, int source) {
	geo_argused(x);
	geo_argused(y);
	geo_argused(source);
    }
    
    void Application::drop_callback(int nb, const char** f) {
	geo_argused(nb);
	geo_argused(f);
    }
    
    void Application::char_callback(unsigned int c) {
	geo_argused(c);
    }
    
    void Application::key_callback(
	int key, int scancode, int action, int mods
    ) {
	geo_argused(key);
	geo_argused(scancode);
	geo_argused(action);
	geo_argused(mods);
    }
    
    void Application::draw_dock_space() {
	ImGuiIO& io = ImGui::GetIO();
	// Create window and dockspace for docking.
	if((io.ConfigFlags & ImGuiConfigFlags_DockingEnable) != 0) {
	    ImGuiViewport* viewport = ImGui::GetMainViewport();
	    ImGui::SetNextWindowPos(viewport->Pos);
	    ImGui::SetNextWindowSize(viewport->Size);
	    ImGui::SetNextWindowViewport(viewport->ID);
	    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	    ImGui::PushStyleVar(
		ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f)
	    );	    
	    static bool open = true;
	    ImGui::Begin(
		"DockSpace", &open,
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoNavFocus |
		ImGuiWindowFlags_NoBackground |
		(menubar_visible_ ? ImGuiWindowFlags_MenuBar : 0)
	    );
	    ImGui::PopStyleVar(3);
	    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
	    ImGui::DockSpace(
		dockspace_id, ImVec2(0.0f, 0.0f),
		ImGuiDockNodeFlags_None |
		ImGuiDockNodeFlags_PassthruCentralNode |
		ImGuiDockNodeFlags_AutoHideTabBar
	    );
	    ImGui::End();		
	}
    }
    
    void Application::draw() {
	if(!ImGui_initialized_) {
	    return;
	}
	update();	
	if(nb_update_locks_ == 0 && !Process::is_running_threads()) {
	    one_frame();
	}
    }
    
    void Application::GL_initialize() {
	GEO::Graphics::initialize();
	geo_assert(glupCurrentContext() == nullptr);
	glupMakeCurrent(glupCreateContext());
	if(glupCurrentContext() == nullptr) {
	    Logger::err("Skin") << "Could not create GLUP context"
				<< std::endl;
	    exit(-1);
	}
    }

    void Application::GL_terminate() {
	glupDeleteContext(glupCurrentContext());
	glupMakeCurrent(nullptr);
	GEO::Graphics::terminate();
    }
    
    void Application::ImGui_initialize() {
	geo_assert(!ImGui_initialized_ );
	ImGui::CreateContext();
	{
	    std::string state = CmdLine::get_arg("gui:state");
	    for(size_t i=0; i<state.length(); ++i) {
		if(state[i] == '\t') {
		    state[i] = '\n';
		}
	    }
	    ImGui::LoadIniSettingsFromMemory(state.c_str());
	}
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	// Viewports allow to drag ImGui windows outside the app's window,
	// but it is still a bit unstable, so deactivated it for now.
	// io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	
	// Note: NavKeyboard sets WantsCaptureKeyboard all the time and
	// thus prevents from nanosleeping !
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

#if defined(GEO_GLFW)
	
	// Second argument to true = install callbacks
	// (note that this function can be called multiple times,
	//  e.g. when ImGui is restarted after re-loading a saved
	//  window state, this mechanism prevents the callbacks to
	//  be installed multiple times, which would be an error since
	//  they are chained).
	ImGui_ImplGlfw_InitForOpenGL(
	    data_->window_, !data_->GLFW_callbacks_initialized_
	);
#endif	

#if defined(GEO_OS_APPLE)
	ImGui_ImplOpenGL3_Init("#version 330");	
#else
	ImGui_ImplOpenGL3_Init("#version 100");
#endif	
	callbacks_initialize();

	if(style_ != "") {
	    set_style(style_);
	} else if(Environment::instance()->has_value("gui:style")) {
            std::string style = Environment::instance()->get_value("gui:style");
            set_style(style);
        } else {
	    set_style("Light");
	}
        
	ImGui_load_fonts();
	ImGui_initialized_ = true;
	ImGui_firsttime_init_ = true;
    }

    void Application::ImGui_load_fonts() {
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr; 

	float s = 1.0f;
	if(phone_screen_) {
	    s = float(std::max(get_width(), get_height())) / 600.0f;
	}
	
	float font_size = s * float(double(font_size_) * hidpi_scaling_);

	// Default font
	io.FontDefault = io.Fonts->AddFontFromMemoryCompressedTTF(
	    roboto_medium_compressed_data,
	    roboto_medium_compressed_size, font_size
	);

	// Add icons to default font.
	{
#define ICON_MIN_FA 0xf000
#define ICON_MAX_FA 0xf63c

	    ImFontConfig config;
	    config.MergeMode = true;
	    
            // Make the icon monospaced	    
	    config.GlyphMinAdvanceX = 1.5f*font_size; 
	    config.GlyphOffset.y += 2.0f;
	    
	    static const ImWchar icon_ranges[] = {
		ICON_MIN_FA, ICON_MAX_FA, 0
	    };
	    
	    io.Fonts->AddFontFromMemoryCompressedTTF(
		fa_solid_compressed_data,
		fa_solid_compressed_size, font_size,
		&config, icon_ranges
	    );

	    init_icon_table();
	}

	// Fixed font for console and editor
	io.Fonts->AddFontFromMemoryCompressedTTF(
	    cousine_regular_compressed_data,
	    cousine_regular_compressed_size, font_size
	);

	// Larger font
	io.Fonts->AddFontFromMemoryCompressedTTF(
	    roboto_medium_compressed_data,
	    roboto_medium_compressed_size, font_size*1.5f
	);

	if(phone_screen_) {
	    // Smaller fixed font for console
	    io.Fonts->AddFontFromMemoryCompressedTTF(
		cousine_regular_compressed_data,
		cousine_regular_compressed_size, font_size*0.5f
	    );
	}
	
	io.FontGlobalScale = float(1.0 / pixel_ratio_);
    }
    
    void Application::ImGui_terminate() {
	geo_assert(ImGui_initialized_);
	ImGui_ImplOpenGL3_Shutdown();
#if defined(GEO_GLFW)
	// Note: normally, with the new ImGui (1.74), this
	// desinstalls user callbacks and reinstalls the previous
	// callbacks. I deactivated this mechanism to have the same
	// behavior as in ImGui 1.72. TODO: do that properly !
	ImGui_ImplGlfw_Shutdown();
#endif	
	ImGui::DestroyContext();
	ImGui_initialized_ = false;
    }
    
    void Application::ImGui_new_frame() {
	ImGui_ImplOpenGL3_NewFrame();
#if defined(GEO_GLFW)		
	ImGui_ImplGlfw_NewFrame();
#endif	
	ImGui::NewFrame();

    }

    void Application::geogram_initialize(int argc, char** argv) {
	GEO::initialize();
	CmdLine::import_arg_group("standard");
	CmdLine::import_arg_group("algo");
	CmdLine::import_arg_group("gfx");
	CmdLine::import_arg_group("gui");	    
	CmdLine::parse(argc, argv, filenames_);
	phone_screen_ = CmdLine::get_arg_bool("gui:phone_screen");
#ifndef GEO_OS_ANDROID	
	if(phone_screen_ && CmdLine::get_arg("gfx:geometry") == "1024x1024") {
	    CmdLine::set_arg("gfx:geometry", "768x1024");
	}
#endif	
    }
    
    bool Application::needs_to_redraw() const {
	return
	    animate_ ||
	    ImGui::GetIO().WantCaptureMouse ||
	    ImGui::GetIO().WantCaptureKeyboard ||
	    (nb_frames_update_ > 0);
    }

    void Application::update() {
	// We redraw several frames, in order to make
	// sure all events are properly processed.
	nb_frames_update_ = NB_FRAMES_UPDATE_INIT;
    }

    void Application::set_gui_state(std::string x) {
	CmdLine::set_arg("gui:state", x);
	if(!ImGui_initialized_) {
	    return;
	}
	ImGui_restart_ = true;
    }

    std::string Application::get_gui_state() const {
	std::string state;
	if(ImGui_initialized_) {
	    state = std::string(ImGui::SaveIniSettingsToMemory());
	    for(size_t i=0; i<state.length(); ++i) {
		if(state[i] == '\n') {
		    state[i] = '\t';
		}
	    }
	}
	return state;
    }

    /**************************** GLFW-specific code *********************/
#if defined(GEO_GLFW)


    void Application::pre_draw() {
    }

    void Application::post_draw() {
    }
    
    void Application::create_window() {
	if(!glfwInit()) {
	    Logger::err("Skin")
		<<  "Could not initialize GLFW" << std::endl;
	    exit(-1);
	}

	if(CmdLine::get_arg("gfx:GL_profile") == "core") {
	    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	}
	
    	if(CmdLine::get_arg_bool("gfx:GL_debug")) {
	    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);            
	}

	const char* title = name_.c_str();

	{
	    std::string geometry = CmdLine::get_arg("gfx:geometry");
	    std::vector<std::string> words;
	    String::split_string(geometry, 'x', words);
	    if(words.size() != 2) {
		Logger::err("Skin")
		    << "Invalid gfx:geometry:" << geometry << std::endl;
		exit(-1);
	    }
	    if(
		!String::string_to_unsigned_integer(words[0].c_str(), width_) ||
		!String::string_to_unsigned_integer(words[1].c_str(), height_)
	    ) {
		Logger::err("Skin")
		    << "Invalid gfx:geometry:" << geometry << std::endl;
		exit(-1);
	    }
	}
	
	if(CmdLine::get_arg_bool("gfx:transparent")) {
#ifdef GLFW_TRANSPARENT_FRAMEBUFFER
	    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);	    
#else
	    Logger::warn("Skin")
		<< "Transparent not supported by this version of GLFW"
		<< std::endl;
#endif	    
	}

	if(CmdLine::get_arg_bool("gfx:full_screen")) {
	    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	    const GLFWvidmode* vidmode = glfwGetVideoMode(monitor);
	    width_ = index_t(vidmode->width);
	    height_ = index_t(vidmode->height);

	    bool no_decoration = CmdLine::get_arg_bool("gfx:no_decoration");
	    
	    if(no_decoration) {
	       glfwWindowHint(GLFW_FOCUSED,GL_TRUE);	
	       glfwWindowHint(GLFW_DECORATED,GL_FALSE);
	       glfwWindowHint(GLFW_RESIZABLE,GL_FALSE);
	       glfwWindowHint(GLFW_AUTO_ICONIFY,GL_FALSE);
	       glfwWindowHint(GLFW_FLOATING,GL_FALSE);
	       glfwWindowHint(GLFW_MAXIMIZED,GL_TRUE);
	    }

	    data_->window_ = glfwCreateWindow(
		int(width_), int(height_), title,
		no_decoration ? glfwGetPrimaryMonitor() : nullptr, 
		nullptr
	    );
	    
	} else {
	    data_->window_ = glfwCreateWindow(
		int(width_), int(height_), title, nullptr, nullptr
	    );
	}
	
	if(data_->window_ == nullptr) {
	    Logger::err("Skin")
		<< "Could not create GLFW window" << std::endl;
	    exit(-1);
	}

	glfwSetWindowUserPointer(data_->window_, this);
	
	glfwMakeContextCurrent(data_->window_);
	glfwSwapInterval(1);

	hidpi_scaling_ = compute_hidpi_scaling();
	pixel_ratio_ = compute_pixel_ratio();

	Logger::out("Skin")
	    << "hidpi_scaling=" << hidpi_scaling_ << std::endl;
	Logger::out("Skin")
	    << "pixel_ratio=" << pixel_ratio_ << std::endl;
    }

    void Application::delete_window() {
	glfwDestroyWindow(data_->window_);
	data_->window_ = nullptr;
	glfwTerminate();
	in_main_loop_ = false;
    }

    
    void Application::one_frame() {
	// Avoid nested ImGui calls
	// (due to calling draw())
	if(currently_drawing_gui_) {
	    return;
	}
	
	// Can happen when ImGui Graphite application 
	// triggers update too soon.
	if(data_->window_ == nullptr) {
	    return;
	}

	if(glfwWindowShouldClose(data_->window_) || !in_main_loop_) {
	    return;
	}

	
	{
	    int cur_width, cur_height;
	    int cur_fb_width,  cur_fb_height;
	    
	    glfwGetWindowSize(data_->window_, &cur_width, &cur_height);
	    glfwGetFramebufferSize(
		data_->window_, &cur_fb_width, &cur_fb_height
	    );

	    if(
		int(width_) != cur_width ||
		int(height_) != cur_height ||
		int(frame_buffer_width_) != cur_fb_width ||
		int(frame_buffer_height_) != cur_fb_height
	    ) {
		resize(
		    index_t(cur_width), index_t(cur_height),
		    index_t(cur_fb_width), index_t(cur_fb_height)
		);
	    }
	}

	{
	    // Detect if hidpi scaling changed. This can happen when
	    // dragging the window from the laptop screen to an external
	    // monitor.
	    if(
		glfwGetCurrentContext() != nullptr && 
		compute_hidpi_scaling() != hidpi_scaling_
	    ) {
		hidpi_scaling_ = compute_hidpi_scaling();
		pixel_ratio_ = compute_pixel_ratio();
		set_font_size(font_size_); // This reloads the font.
	    }
	}
	
	
	glfwPollEvents();

	if(needs_to_redraw()) {
	    pre_draw();
	    currently_drawing_gui_ = true;
	    ImGui_new_frame();
	    draw_graphics();
	    draw_gui();
	    ImGui::Render();
	    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

#ifndef GEO_OS_EMSCRIPTEN	    
	    // Update and Render additional Platform Windows
	    // (see ImGui demo app).
	    if(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	    }
#endif
	    
	    currently_drawing_gui_ = false;

#ifdef GEO_OS_EMSCRIPTEN
	    // Set alpha channel to 1 else the image is composited 
	    // with the background 
	    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
	    glClear(GL_COLOR_BUFFER_BIT);
	    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
#endif	
	    
	    glfwSwapBuffers(data_->window_);
	    post_draw();
	    if(
		nb_frames_update_ > 0 && !animate_ &&
	        !(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	    ) {
		--nb_frames_update_;
	    }
	} else {
	    // Sleep for 0.2 seconds, to let the processor cold-down
	    // instead of actively waiting (be a good citizen for the
	    // other processes.
	    Process::sleep(20000);
	}

	// ImGui needs to be restarted whenever docking state is reloaded.
	if(ImGui_restart_) {
	    ImGui_restart_ = false;
	    ImGui_terminate();
	    if(CmdLine::arg_is_declared("gui:font_size")) {
		set_font_size(CmdLine::get_arg_uint("gui:font_size"));
		ImGui_reload_font_ = false;
	    }
	    ImGui_initialize();
	} else if(ImGui_reload_font_) {
	    ImGuiIO& io = ImGui::GetIO();	    
	    io.Fonts->Clear();
	    ImGui_load_fonts();
	    ImGui_ImplOpenGL3_DestroyDeviceObjects();
	    ImGui_reload_font_ = false;
	}
    }

#ifdef GEO_OS_EMSCRIPTEN

    void emscripten_load_latest_file();
    
    /**
     * \brief This function is called by the HTML shell each
     *  time a file is loaded.
     */
    void emscripten_load_latest_file() {
	if(Application::instance() == nullptr) {
	    return;
	}
	std::vector<std::string> all_files;
	GEO::FileSystem::get_directory_entries("/",all_files);
	std::string filename = "";
	Numeric::uint64 timestamp = 0;
	for(auto f: all_files) {
	    if(GEO::FileSystem::is_file(f)) {
		Numeric::uint64 f_timestamp =
		    GEO::FileSystem::get_time_stamp(f);
		if(f_timestamp > timestamp) {
		    timestamp = f_timestamp;
		    filename = f;
		}
	    }
	}
	if(filename != "") {
	    const char* filename_str = filename.c_str();
	    Application::instance()->drop_callback(1, &filename_str);
	}
    }
    
    
    /**
     * \brief The job to be done for each frame when running
     *  in Emscripten.
     * \details The browser keeps control of the main loop 
     *  in Emscripten. This function is a callback passed to
     *  the Emscripten runtime.
     */
    void emscripten_one_frame() {
	if(Application::instance() == nullptr) {
	    return;
	}
	Application::instance()->one_frame();
    }
#endif   
   
    void Application::main_loop() {
	in_main_loop_ = true;
#ifdef GEO_OS_EMSCRIPTEN
	FileSystem::set_file_system_changed_callback(
	    emscripten_load_latest_file
	);
	GL_initialize();
	ImGui_initialize();
	emscripten_load_latest_file();
	emscripten_set_main_loop(emscripten_one_frame, 0, 1);
#else       
	bool initialized = false;
	while (!glfwWindowShouldClose(data_->window_) && in_main_loop_) {
	    if(!initialized) {
		GL_initialize();
		ImGui_initialize();
		initialized = true;
	    }
	    one_frame();
	}
	if(initialized) {
	    ImGui_terminate();
	    GL_terminate();
	}
#endif       
    }

    namespace {
	void GLFW_callback_mouse_button(
	    GLFWwindow* w, int button, int action, int mods
	) {
	    Application* app = static_cast<Application*>(
		glfwGetWindowUserPointer(w)
	    );
	    app->update();
	    if(!ImGui::GetIO().WantCaptureMouse) {
		app->mouse_button_callback(button,action,mods);
	    }
	    // Note: when a menu is open and you click elsewhere, the
	    // WantCaptureMouse flag is still set, and the framework
	    // misses the "mouse button up" event. If a translation is
	    // active, it remains active later ("sticky translation" bug).
	    // The following code always generates a "mouse button up" event
	    // to solve this problem.
	    if(ImGui::GetIO().WantCaptureMouse && action==EVENT_ACTION_UP) {
		ImVec2 mouse_pos = ImGui::GetIO().MousePos;
		app->cursor_pos_callback(
		    double(mouse_pos.x), double(mouse_pos.y)
		);		
		app->mouse_button_callback(button,action,mods);
	    }
	    if(app->impl_data()->ImGui_callback_mouse_button != nullptr) {
		app->impl_data()->ImGui_callback_mouse_button(
		    w, button, action, mods
		);
	    }
	}

	void GLFW_callback_cursor_pos(
	    GLFWwindow* w, double xf, double yf
	) {
	    Application* app = static_cast<Application*>(
		glfwGetWindowUserPointer(w)
	    );
	    app->update();	
	    if(app->impl_data()->ImGui_callback_cursor_pos != nullptr) {
		app->impl_data()->ImGui_callback_cursor_pos(w, xf, yf);
	    }
	    if(!ImGui::GetIO().WantCaptureMouse) {
		app->cursor_pos_callback(xf, yf);
	    }
	}
    
	void GLFW_callback_scroll(
	    GLFWwindow* w, double xoffset, double yoffset
	) {
	    Application* app = static_cast<Application*>(
		glfwGetWindowUserPointer(w)
	    );
	    app->update();
	    if(!ImGui::GetIO().WantCaptureMouse) {
#if defined(GEO_OS_EMSCRIPTEN) || defined(GEO_OS_APPLE)
		app->scroll_callback(xoffset,-yoffset);
#else
		app->scroll_callback(xoffset, yoffset);		
#endif    
	    }	
	    if(app->impl_data()->ImGui_callback_scroll != nullptr) {
		app->impl_data()->ImGui_callback_scroll(w, xoffset, yoffset);
	    }
	}
    
	void GLFW_callback_drop(
	    GLFWwindow* w, int nb, const char** p
	) {
	    Application* app = static_cast<Application*>(
		glfwGetWindowUserPointer(w)
	    );
	    app->update();
	    if(app->impl_data()->ImGui_callback_drop != nullptr) {
		app->impl_data()->ImGui_callback_drop(w, nb, p);
	    }
	    app->drop_callback(nb, p);
	}

	void GLFW_callback_char(GLFWwindow* w, unsigned int c) {
	    Application* app = static_cast<Application*>(
		glfwGetWindowUserPointer(w)
	    );
	    app->update();
	    if(app->impl_data()->ImGui_callback_char != nullptr) {
		app->impl_data()->ImGui_callback_char(w, c);
	    }
	    if(!ImGui::GetIO().WantCaptureKeyboard) {	
		app->char_callback(c);
	    }
	}

	void GLFW_callback_key(
	    GLFWwindow* w, int key, int scancode, int action, int mods
	) {
	    Application* app = static_cast<Application*>(
		glfwGetWindowUserPointer(w)
	    );
	    app->update();
	    if(app->impl_data()->ImGui_callback_key != nullptr) {
		app->impl_data()->ImGui_callback_key(
		    w, key, scancode, action, mods
		);
	    }
	    if(!ImGui::GetIO().WantCaptureKeyboard) {
		app->key_callback(key,scancode,action,mods);
	    }
	}

	void GLFW_callback_refresh(GLFWwindow* w) {
	    Application* app = static_cast<Application*>(
		glfwGetWindowUserPointer(w)
	    );
	    app->update();
	    if(app->impl_data()->ImGui_callback_refresh != nullptr) {
		app->impl_data()->ImGui_callback_refresh(w);
	    }
	}
    }
    
    void Application::callbacks_initialize() {
	if(!data_->GLFW_callbacks_initialized_) {
	    GEO::Logger::out("ImGui") << "Viewer GUI init (GL3)"
				      << std::endl;
	}

	if(!data_->GLFW_callbacks_initialized_) {
	    // Get previous callbacks so that we can call them if ImGui
	    // wants to handle user input.
	    data_->ImGui_callback_mouse_button = glfwSetMouseButtonCallback(
		data_->window_,GLFW_callback_mouse_button
	    );
	    data_->ImGui_callback_cursor_pos = glfwSetCursorPosCallback(
		data_->window_,GLFW_callback_cursor_pos
	    );
	    data_->ImGui_callback_scroll = glfwSetScrollCallback(
		data_->window_,GLFW_callback_scroll
	    );	
	    data_->ImGui_callback_char = glfwSetCharCallback(
		data_->window_,GLFW_callback_char
	    );
	    data_->ImGui_callback_key = glfwSetKeyCallback(
		data_->window_,GLFW_callback_key
	    );
	    data_->ImGui_callback_drop = glfwSetDropCallback(
		data_->window_,GLFW_callback_drop
	    );
	    data_->ImGui_callback_refresh = glfwSetWindowRefreshCallback(
		data_->window_,GLFW_callback_refresh
	    );

#ifdef GEO_OS_EMSCRIPTEN
	    // It seems that Emscripten's implementation of
	    // glfwSetXXXCallback() does not always return previous
	    // callback bindings.
	    data_->ImGui_callback_mouse_button =
		ImGui_ImplGlfw_MouseButtonCallback;
	    data_->ImGui_callback_char = ImGui_ImplGlfw_CharCallback;
	    data_->ImGui_callback_key  = ImGui_ImplGlfw_KeyCallback;
	    data_->ImGui_callback_scroll  = ImGui_ImplGlfw_ScrollCallback;
#endif	    
	    data_->GLFW_callbacks_initialized_ = true;
	} 
    }
    
    void Application::set_window_icon(Image* icon_image) {
	GLFWimage glfw_image;
	glfw_image.width = int(icon_image->width());
	glfw_image.height = int(icon_image->height());
	glfw_image.pixels = icon_image->base_mem();
	glfwSetWindowIcon(data_->window_, 1, &glfw_image);
    }

    void Application::set_full_screen_mode(
	index_t w, index_t h, index_t Hz, index_t monitor
    ) {
	if(data_->window_ == nullptr) {
	    return;
	}
	int count;
	GLFWmonitor** monitors = glfwGetMonitors(&count);
	if(int(monitor) >= count) {
	    Logger::err("Application") << monitor << ": no such monitor"
	                               << std::endl;
	}
	if((w == 0) || (h == 0) || (Hz == 0)) {
	    Logger::out("Application")
		<< "Using default video mode" << std::endl;
	    const GLFWvidmode* mode = glfwGetVideoMode(monitors[monitor]);
	    w = index_t(mode->width);
	    h = index_t(mode->height);
	    Hz = index_t(mode->refreshRate);
	}
	glfwSetWindowMonitor(
	    data_->window_, monitors[monitor], 0, 0, int(w), int(h), int(Hz)
	);
	update();
    }

    void Application::set_windowed_mode(index_t w, index_t h) {
	if(w != 0 && h != 0) {
	    width_ = w;
	    height_ = w;
	}
	glfwSetWindowMonitor(
	    data_->window_, nullptr, 0, 0, int(width_), int(height_), 50
	);
	update();	
    }


    void Application::list_video_modes() {
	int nb_monitors = 0;
	GLFWmonitor** monitors = glfwGetMonitors(&nb_monitors);
	Logger::out("Application") << "Detected " << nb_monitors
				   << " monitor(s)"
				   << std::endl;
	for(int m=0; m<nb_monitors; ++m) {
	    GLFWmonitor* monitor = monitors[m];
	    Logger::out("Application") << "Monitor " << m << ":"
			       << glfwGetMonitorName(monitor)
			       << std::endl;
	    int nb_modes = 0;
	    const GLFWvidmode* modes = glfwGetVideoModes(monitor, &nb_modes);
	    for(int mm=0; mm<nb_modes; ++mm) {
		const GLFWvidmode& mode = modes[mm];
		Logger::out("Application") << "   mode " << mm << ":"
					   << mode.width << "x" << mode.height
					   << " " << mode.refreshRate << "Hz "
					   << "R" << mode.redBits
					   << "G" << mode.greenBits
					   << "B" << mode.blueBits
					   << std::endl;
	    }
	}
    }


    void Application::iconify() {
	if(data_->window_ == nullptr ){
	    return ;
	}
	glfwIconifyWindow(data_->window_);
    }
    
    void Application::restore() {
	if(data_->window_ == nullptr ){
	    return ;
	}
	
	// In full screen mode, glfwRestoreWindow()
	// does not seem to work, so we switch to
	// windowed mode, deiconify, then switch back
	// to full screen mode.
	if(get_full_screen()) {
	    set_full_screen(false);
	    glfwRestoreWindow(data_->window_);
	    set_full_screen(true);
	} else {
	    glfwRestoreWindow(data_->window_);
	}
    }

    bool Application::get_full_screen() const {
	return (data_->window_ != nullptr &&
		glfwGetWindowMonitor(data_->window_) != nullptr);
    }
    
    void Application::set_full_screen(bool x) {
	if(x != get_full_screen()) {
	    if(x) {
		set_full_screen_mode();
	    } else {
		set_windowed_mode();
	    }
	}
    }

    void* Application::impl_window() {
	return data_->window_;
    }

#else
# error "No windowing system"
#endif    


#ifdef GEO_GLFW
    const char* Application::key_to_string(int key) {
	if(key == GLFW_KEY_LEFT) {
	    return "left";
	}
	if(key == GLFW_KEY_RIGHT) {
	    return "right";
	}
	if(key == GLFW_KEY_UP) {
	    return "up";
	}
	if(key == GLFW_KEY_DOWN) {
	    return "down";
	}
	if(key == GLFW_KEY_F1) {
	    return "F1";
	}
	if(key == GLFW_KEY_F2) {
	    return "F2";
	}
	if(key == GLFW_KEY_F3) {
	    return "F3";
	}
	if(key == GLFW_KEY_F4) {
	    return "F4";
	}
	if(key == GLFW_KEY_F5) {
	    return "F5";
	}
	if(key == GLFW_KEY_F6) {
	    return "F6";
	}
	if(key == GLFW_KEY_F7) {
	    return "F7";
	}
	if(key == GLFW_KEY_F8) {
	    return "F8";
	}
	if(key == GLFW_KEY_F9) {
	    return "F9";
	}
	if(key == GLFW_KEY_F10) {
	    return "F10";
	}
	if(key == GLFW_KEY_F11) {
	    return "F11";
	}
	if(key == GLFW_KEY_F12) {
	    return "F12";
	}
	if(key == GLFW_KEY_LEFT_CONTROL) {
	    return "left_control";
	}
	if(key == GLFW_KEY_RIGHT_CONTROL) {
	    return "right_control";
	}
	if(key == GLFW_KEY_LEFT_ALT) {
	    return "left_alt";
	}
	if(key == GLFW_KEY_RIGHT_ALT) {
	    return "right_alt";
	}
	if(key == GLFW_KEY_LEFT_SHIFT) {
	    return "left_shift";
	}
	if(key == GLFW_KEY_RIGHT_SHIFT) {
	    return "right_shift";
	}
	if(key == GLFW_KEY_ESCAPE) {
	    return "escape";
	}
	if(key == GLFW_KEY_TAB) {
	    return "tab";
	}
	if(key == GLFW_KEY_BACKSPACE) {
	    return "backspace";
	}
	return "";
    }
#else
    const char* Application::key_to_string(int key) {
	return "";
    }
#endif    

}

/************************ Utilities *************************************/

namespace GEO {

    void StyleColorsCorporateGrey(bool threeD) {
	ImGuiStyle & style = ImGui::GetStyle();
	ImVec4 * colors = style.Colors;
	
	/// 0 = FLAT APPEARENCE
	/// 1 = MORE "3D" LOOK
	float is3D = threeD ? 1.0f : 0.0f;
		
	colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled]           = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
	colors[ImGuiCol_ChildBg]                = ImVec4(0.15f, 0.15f, 0.15f, 1.00f); //BL orig=0.25
	colors[ImGuiCol_WindowBg]               = ImVec4(0.20f, 0.20f, 0.20f, 1.00f); //BL orig=0.25
	colors[ImGuiCol_PopupBg]                = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_Border]                 = ImVec4(0.12f, 0.12f, 0.12f, 0.71f);
	colors[ImGuiCol_BorderShadow]           = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	colors[ImGuiCol_FrameBg]                = ImVec4(0.42f, 0.42f, 0.42f, 0.54f);
	colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.42f, 0.42f, 0.42f, 0.40f);
	colors[ImGuiCol_FrameBgActive]          = ImVec4(0.56f, 0.56f, 0.56f, 0.67f);
	colors[ImGuiCol_TitleBg]                = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
	colors[ImGuiCol_TitleBgActive]          = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.17f, 0.17f, 0.17f, 0.90f);
	colors[ImGuiCol_MenuBarBg]              = ImVec4(0.335f, 0.335f, 0.335f, 1.000f);
	colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.24f, 0.24f, 0.24f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
	colors[ImGuiCol_CheckMark]              = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
	colors[ImGuiCol_SliderGrab]             = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
	colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.64f, 0.64f, 0.64f, 1.00f);
	colors[ImGuiCol_Button]                 = ImVec4(0.54f, 0.54f, 0.54f, 0.35f);
	colors[ImGuiCol_ButtonHovered]          = ImVec4(0.52f, 0.52f, 0.52f, 0.59f);
	colors[ImGuiCol_ButtonActive]           = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
	colors[ImGuiCol_Header]                 = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
	colors[ImGuiCol_HeaderHovered]          = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
	colors[ImGuiCol_HeaderActive]           = ImVec4(0.76f, 0.76f, 0.76f, 0.77f);
	colors[ImGuiCol_Separator]              = ImVec4(0.000f, 0.000f, 0.000f, 0.137f);
	colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.700f, 0.671f, 0.600f, 0.290f);
	colors[ImGuiCol_SeparatorActive]        = ImVec4(0.702f, 0.671f, 0.600f, 0.674f);
	colors[ImGuiCol_ResizeGrip]             = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.73f, 0.73f, 0.73f, 0.35f);
	colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);

	style.PopupRounding = 3;

	style.WindowPadding = ImVec2(4, 4);
	style.FramePadding  = ImVec2(6, 4);
	style.ItemSpacing   = ImVec2(6, 2);

	style.ScrollbarSize = 18;

	style.WindowBorderSize = 1;
	style.ChildBorderSize  = 1;
	style.PopupBorderSize  = 1;
	style.FrameBorderSize  = is3D; 

	style.WindowRounding    = 3;
	style.ChildRounding     = 3;
	style.FrameRounding     = 3;
	style.ScrollbarRounding = 2;
	style.GrabRounding      = 3;

#ifdef IMGUI_HAS_DOCK 
	style.TabBorderSize = is3D; 
	style.TabRounding   = 3;

	colors[ImGuiCol_DockingEmptyBg]     = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
	colors[ImGuiCol_Tab]                = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_TabHovered]         = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
	colors[ImGuiCol_TabActive]          = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
	colors[ImGuiCol_TabUnfocused]       = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
	colors[ImGuiCol_DockingPreview]     = ImVec4(0.85f, 0.85f, 0.85f, 0.28f);

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
	    style.WindowRounding = 0.0f;
	    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
#endif
    }


#if defined(GEO_GLFW) && !defined(GEO_OS_EMSCRIPTEN)

    double compute_pixel_ratio() {
	int buf_size[2];
	int win_size[2];
	GLFWwindow* window = glfwGetCurrentContext();
	glfwGetFramebufferSize(window, &buf_size[0], &buf_size[1]);
	glfwGetWindowSize(window, &win_size[0], &win_size[1]);
	// The window may be iconified.
	if(win_size[0] == 0) {
	    return 1.0;
	}
	return double(buf_size[0]) / double(win_size[0]);
    }

    double compute_hidpi_scaling() {
	float xscale, yscale;
	GLFWwindow* window = glfwGetCurrentContext();
	glfwGetWindowContentScale(window, &xscale, &yscale);
	return 0.5 * double(xscale + yscale);
    }
    
#else

    double compute_pixel_ratio() {
	return 1.0;
    }

    double compute_hidpi_scaling() {
	return 1.0;
    }
    
#endif
    
}

