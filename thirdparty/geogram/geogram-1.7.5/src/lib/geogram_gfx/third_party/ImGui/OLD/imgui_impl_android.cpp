// ImGui Platform Binding for: Android
// [Bruno Levy] Sun Aug 19 08:01:39 CEST 2018
// Note: not part (yet) of the official ImGui distribution
// 
// Note: to use, include in the CMakeLists.txt that compiles this file:
// if(ANDROID)
//   target_include_directories(geogram_gfx_third_party PRIVATE
//      ${ANDROID_NDK}/sources/android/native_app_glue
//   )
// endif()

// What works:
//   Rendering with OpenGL ES 2.x
//   Fingers/Stylus/Mouse interaction
//   Virtual and physical keyboard interaction

// TODO (Bugs to be fixed):
// ------------------------
//  - soft keyboard directional keys do not always work
//     (it depends on the used keyboard,
//     for some keyboards, they work a little bit, randomly,
//     for some others they work...)
//  - right mouse button is not super-responsive

// TODO (Improvements):
// --------------------
//  - UTF8 text input (probably not very difficult to add).
//
//  - mouse cursors (https://developer.android.com/about/versions/nougat/android-7.0#custom_pointer_api)
//    (need to overload Java function, cannot do that with native_glue I think, unless we can change
//     methods of an existing Java object with JNI)
//
//  - setMousePos


#ifdef __ANDROID__

#include "imgui.h"
#include "imgui_impl_android.h"

#include <EGL/egl.h>
#include <GLES/gl.h>
#include <android/log.h>

#include <time.h>
#include <cassert>
#include <stdexcept>
#include <cctype>
#include <cmath>

#include <string>
#include <geogram/basic/string.h>
#include <geogram/basic/android_utils.h>
#include <geogram_gfx/gui/application.h>

// Pulled this dependency on Application:
// - for the constants in geogram_gfx/gui/events.h
// - to quit the application if the back key is pressed
// (by calling Application::instance()->stop()
// (TODO: it is probably possible to use the 'default behavior' of
//  the 'back' button instead, by returning the correct value in
//  the event handler, to be investigated)

using namespace GEO;

namespace {
    inline int decode_action(int action) {
	switch(action) {
	    case AMOTION_EVENT_ACTION_BUTTON_PRESS:
	    case AMOTION_EVENT_ACTION_DOWN:
		return EVENT_ACTION_DOWN;
	    case AMOTION_EVENT_ACTION_BUTTON_RELEASE:	    
	    case AMOTION_EVENT_ACTION_UP:
		return EVENT_ACTION_UP;
	    case AMOTION_EVENT_ACTION_MOVE:
		return EVENT_ACTION_DRAG;
	}
	return EVENT_ACTION_UNKNOWN;
    }

    struct android_app* g_app = nullptr;
    double g_Time = 0.0;
    float g_mouseX = 0.0f;
    float g_mouseY = 0.0f;
    bool  g_mousePressed[5]     = {false, false, false, false, false};
    int   g_mouseJustPressed[5] = {0, 0, 0, 0, 0};    
    bool  g_resetKeys = false;
    ImGui_ImplAndroid_MouseUserCallback g_mouse_CB = nullptr;

    inline ImVec2 barycenter(const ImVec2& p1, const ImVec2& p2) {
	return ImVec2(0.5f*(p1.x+p1.x), 0.5f*(p1.y+p2.y));
    }

    inline float distance(const ImVec2& p1, const ImVec2& p2) {
	return ::sqrtf((p2.x-p1.x)*(p2.x-p1.x) + (p2.y-p1.y)*(p2.y-p1.y));
    }

}

void ImGui_ImplAndroid_SetMouseUserCallback(
    ImGui_ImplAndroid_MouseUserCallback CB
) {
    g_mouse_CB = CB;
}

bool ImGui_ImplAndroid_Init(struct android_app* app) {
    g_app = app;
    g_Time = 0.0;
    g_mouseX = 0.0f;
    g_mouseY = 0.0f;
    for (int i = 0; i < IM_ARRAYSIZE(g_mousePressed); i++) {
	g_mousePressed[i]     = false;
	g_mouseJustPressed[i] = 0;	
    }
    // TODO: mouse cursor
    // TODO: setmousepos ?

    ImGuiIO& io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = AKEYCODE_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = AKEYCODE_DPAD_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = AKEYCODE_DPAD_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = AKEYCODE_DPAD_UP;
    io.KeyMap[ImGuiKey_DownArrow] = AKEYCODE_DPAD_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = AKEYCODE_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = AKEYCODE_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = AKEYCODE_MOVE_HOME;
    io.KeyMap[ImGuiKey_End] = AKEYCODE_MOVE_END;
    io.KeyMap[ImGuiKey_Insert] = AKEYCODE_INSERT;
    io.KeyMap[ImGuiKey_Delete] = AKEYCODE_FORWARD_DEL;
    io.KeyMap[ImGuiKey_Backspace] = AKEYCODE_DEL;
    io.KeyMap[ImGuiKey_Space] = AKEYCODE_SPACE;
    io.KeyMap[ImGuiKey_Enter] = AKEYCODE_ENTER;
    io.KeyMap[ImGuiKey_Escape] = AKEYCODE_ESCAPE;
    io.KeyMap[ImGuiKey_A] = AKEYCODE_A;
    io.KeyMap[ImGuiKey_C] = AKEYCODE_C;
    io.KeyMap[ImGuiKey_V] = AKEYCODE_V;
    io.KeyMap[ImGuiKey_X] = AKEYCODE_X;
    io.KeyMap[ImGuiKey_Y] = AKEYCODE_Y;
    io.KeyMap[ImGuiKey_Z] = AKEYCODE_Z;
    

    // Install callbacks
    if(app != nullptr) {
	app->onInputEvent = ImGui_ImplAndroid_InputEvent;
    }
    
    return true;
}

void ImGui_ImplAndroid_Shutdown() {
    // TODO: destroy mouse cursors.
}

static void ImGui_ImplAndroid_UpdateMousePosAndButtons()
{
    ImGuiIO& io = ImGui::GetIO();

    for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) {
	// We do the same thing as in imgui_impl_glfw.cpp:
        // If a mouse press event came, always pass it as
	// "mouse held this frame", so we don't miss click-release
	// events that are shorter than 1 frame.
	// (unlike in imgui_impl_glfw.cpp, we do that during several
	//  frames instead of a single one).
        io.MouseDown[i] = (g_mouseJustPressed[i] != 0) || g_mousePressed[i];
	if(g_mouseJustPressed[i] != 0) {
	    --g_mouseJustPressed[i];
	}
    }
    io.MousePos = ImVec2(g_mouseX, g_mouseY);
}

static void ImGui_ImplAndroid_UpdateMouseCursor() {
    // TODO...
}

void ImGui_ImplAndroid_NewFrame() {
    ImGuiIO& io = ImGui::GetIO();
    // Font atlas needs to be built, call renderer _NewFrame() function
    // e.g. ImGui_ImplOpenGL3_NewFrame()     
    IM_ASSERT(io.Fonts->IsBuilt());     

    // Get current display size
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    EGLSurface surface = eglGetCurrentSurface(EGL_DRAW);
    int w=0;
    int h=0;
    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);
    int display_w = w;
    int display_h = h;

    io.DisplaySize = ImVec2((float)w, (float)h);
    io.DisplayFramebufferScale =
	ImVec2(
	    w > 0 ?((float)display_w / w) : 0,
	    h > 0 ? ((float)display_h / h) : 0
    );

    // Setup time step
    timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    double current_time = double(now.tv_sec) + double(now.tv_nsec) * 1e-9;
    
    io.DeltaTime = g_Time > 0.0 ? float(current_time - g_Time) : 1.0f/60.0f;
    g_Time = current_time;

    ImGui_ImplAndroid_UpdateMousePosAndButtons();
    ImGui_ImplAndroid_UpdateMouseCursor();

    // TODO: Gamepad navigation mapping ?
}


void ImGui_ImplAndroid_EndFrame() {
    // g_resetKeys is set when the latest key event came from the soft keyboard,
    // then we need to reset the keys.
    if(g_resetKeys) {
	ImGuiIO& io = ImGui::GetIO();
	for(int key = 0; key < IM_ARRAYSIZE(io.KeysDown); ++key) {
	    io.KeysDown[key] = false;	    
	}
	io.KeyShift = false;
	io.KeyCtrl = false;
	io.KeyAlt = false;
	io.KeySuper = false;
	g_resetKeys = false;
    }
}

// Emulates mouse buttons using multiple fingers:
//   emulated mouse button is determined by number of fingers
//   coordinates are defined by last finger
int32_t  ImGui_ImplAndroid_FingerEvent(
    struct android_app* app, AInputEvent* event
) {
    int32_t action = AMotionEvent_getAction(event);
    bool down_or_move = (action == AMOTION_EVENT_ACTION_DOWN ||
			 action == AMOTION_EVENT_ACTION_MOVE );

    int nb_fingers = int(AMotionEvent_getPointerCount(event));

    int btn = nb_fingers-1;
    for(int i=0; i<IM_ARRAYSIZE(g_mousePressed); ++i) {
	if(i == btn) {
	    g_mousePressed[i] = down_or_move;
	    if(action == AMOTION_EVENT_ACTION_DOWN) {
		// Notify that the mouse was just pressed during two
		// frames. This is needed else ImGui does not notice
		// that the button was pressed (I think it needs one
		// frame to notice that the cursor moved to the button,
		// and another frame to notice that the button was
		// pressed). This is because fingers do not generate
		// events when hovering the touch screen.
		g_mouseJustPressed[i] = 2;
	    }
	} else {
	    g_mousePressed[i] = false;
	}
    }

    g_mouseX = AMotionEvent_getX(event, nb_fingers-1);
    g_mouseY = AMotionEvent_getY(event, nb_fingers-1);

    if(g_mouse_CB != nullptr) {
	static int last_button_ = -1;
	
	if(nb_fingers == 1) {
	    if(last_button_ != -1 && last_button_ != 0) {
		g_mouse_CB(
		    g_mouseX, g_mouseY, last_button_,
		    EVENT_ACTION_UP, EVENT_SOURCE_FINGER
		);
	    }
	    last_button_ = 0;
	    g_mouse_CB(
		g_mouseX, g_mouseY, 0,
		decode_action(action), EVENT_SOURCE_FINGER
	    );
	} else if(nb_fingers == 2) {
	    // Two-fingers interactions: does both zoom (button 1) and
	    // translation (button 2). The chosen action depends on the
	    // variation of the distance between the two fingers and the
	    // displacement of the centroid of the two fingers:
	    // if distance varies most -> zoom
	    // if centroid moves most  -> translation
	    
	    if(last_button_ != -1 && last_button_ != 2) {
		g_mouse_CB(
		    g_mouseX, g_mouseY, last_button_,
		    EVENT_ACTION_UP, EVENT_SOURCE_FINGER
		);
	    }
	    ImVec2 finger1(
		AMotionEvent_getX(event, 0),
		AMotionEvent_getY(event, 0)
	    );
	    ImVec2 finger2(
		AMotionEvent_getX(event, 1),
		AMotionEvent_getY(event, 1)
	    );
	    float length = distance(finger1, finger2);
	    ImVec2 center = barycenter(finger1, finger2);
	    
	    static float last_length = 0.0f;
	    static ImVec2 last_center;

	    if(action == AMOTION_EVENT_ACTION_MOVE) {
		if(distance(center, last_center) > ::fabs(length-last_length)) {
		    // Translation: synthetise press btn 1, move, release btn 1
		    g_mouse_CB(
			last_center.x, last_center.y, 1,
			EVENT_ACTION_DOWN, EVENT_SOURCE_FINGER
		    );
		    g_mouse_CB(
			center.x, center.y, 1,
			EVENT_ACTION_DRAG, EVENT_SOURCE_FINGER
		    );
		    g_mouse_CB(
			center.x, center.y, 1,
			EVENT_ACTION_UP, EVENT_SOURCE_FINGER
		    );
		} else {
		    // Zoom: synthetise press btn 2, move, release btn 2
		    g_mouse_CB(
			0.0f, last_length, 2,
			EVENT_ACTION_DOWN, EVENT_SOURCE_FINGER
		    );
		    g_mouse_CB(
			0.0f, length, 2,
			EVENT_ACTION_DRAG, EVENT_SOURCE_FINGER
		    );
		    g_mouse_CB(
			0.0f, length, 2,
			EVENT_ACTION_UP, EVENT_SOURCE_FINGER
		    );
		}
	    }
	    last_length = length;
	    last_center = center;
	    last_button_ = 2;
	} else if(nb_fingers == 3) {
	    if(last_button_ != -1 && last_button_ != 1) {
		g_mouse_CB(
		    g_mouseX, g_mouseY, last_button_,
		    EVENT_ACTION_UP, EVENT_SOURCE_FINGER
		);
	    }
	    if(last_button_ != 1) {
		last_button_ = 1;
		g_mouse_CB(
		    g_mouseX, g_mouseY, 1,
		    EVENT_ACTION_DOWN, EVENT_SOURCE_FINGER
		);
	    } else {
		g_mouse_CB(
		    g_mouseX, g_mouseY, 1,
		    decode_action(action), EVENT_SOURCE_FINGER
		);
	    }
	}

    }

    return 1;
}

// Handles stylus input, like Galaxy SPen. Uses the tiny button on
// the pen to emulate second mouse button.
int32_t ImGui_ImplAndroid_StylusEvent(
    struct android_app* app, AInputEvent* event
) {
    int32_t action = AMotionEvent_getAction(event);
    bool down_or_move = (action == AMOTION_EVENT_ACTION_DOWN ||
			 action == AMOTION_EVENT_ACTION_MOVE );

    int btn = (
	(AMotionEvent_getButtonState(event) &
	    AMOTION_EVENT_BUTTON_STYLUS_PRIMARY) != 0
    ) ? 1 : 0;

    for(int i=0; i<IM_ARRAYSIZE(g_mousePressed); ++i) {
	if(i == btn) {
	    g_mousePressed[i] = down_or_move;
	    if(action == AMOTION_EVENT_ACTION_DOWN) {
		g_mouseJustPressed[i] = 1;
	    }
	} else {
	    g_mousePressed[i] = false;
	}
    }
    g_mouseX = AMotionEvent_getX(event, 0);
    g_mouseY = AMotionEvent_getY(event, 0);

    if(g_mouse_CB != nullptr) {
	g_mouse_CB(
	    g_mouseX, g_mouseY, btn, decode_action(action), EVENT_SOURCE_STYLUS
	);
    }
	
    return 1;    
}

// Declared as static global so that key handler can 'push' button 1
// when the back key event is synthetized by a right mouse click
// (but this does not fully work, to be investigated...)
static int mouse_handler_btn = -1;

// Handles a standard USB or bluetooth mouse connected to the phone.
int32_t  ImGui_ImplAndroid_MouseEvent(
    struct android_app* app, AInputEvent* event
) {
    int32_t buttons = AMotionEvent_getButtonState(event);
    for(int i=0; i<IM_ARRAYSIZE(g_mousePressed); ++i) {
	g_mousePressed[i] = false;
    }
    g_mousePressed[0] = (buttons &  AMOTION_EVENT_BUTTON_PRIMARY) != 0;
    g_mousePressed[1] = (buttons &  AMOTION_EVENT_BUTTON_SECONDARY) != 0;
    g_mousePressed[2] = (buttons &  AMOTION_EVENT_BUTTON_TERTIARY) != 0;
    // TODO: g_mouseJustPressed
    g_mouseX = AMotionEvent_getX(event, 0);
    g_mouseY = AMotionEvent_getY(event, 0);

    // Mouse wheel
    int32_t action = AMotionEvent_getAction(event);

    if(action == AMOTION_EVENT_ACTION_SCROLL) {
	float hscroll = AMotionEvent_getAxisValue(
	    event, AMOTION_EVENT_AXIS_HSCROLL, 0
        );
	float vscroll = AMotionEvent_getAxisValue(
	    event, AMOTION_EVENT_AXIS_VSCROLL, 0
        );
	ImGuiIO& io = ImGui::GetIO();
	io.MouseWheelH += hscroll;
	io.MouseWheel  += vscroll;
    }

    if(g_mouse_CB != nullptr) {
	int32_t action = AMotionEvent_getAction(event);
	if(action == AMOTION_EVENT_ACTION_SCROLL) {
	    // Synthetize btn 2 push, move, btn 2 release
	    ImGuiIO& io = ImGui::GetIO();	    
	    g_mouse_CB(
		g_mouseX, g_mouseY, 2,
		EVENT_ACTION_DOWN, EVENT_SOURCE_MOUSE
	    );
	    g_mouse_CB(
		g_mouseX + io.MouseWheelH, g_mouseY - 6.0f * io.MouseWheel, 2,
		EVENT_ACTION_DRAG, EVENT_SOURCE_MOUSE
	    );	    
	    g_mouse_CB(
		g_mouseX + io.MouseWheelH, g_mouseY - 6.0f * io.MouseWheel, 2,
		EVENT_ACTION_UP, EVENT_SOURCE_MOUSE
	    );	    
	} else {
	    // TODO2: does not seem to work with right button,
	    //   ... to be investigated (does it generate the
	    //  event or does it only generate a 'back' keypress)
	    // TODO3: AMotionEvent_getActionButton(event) would
	    // be better, but it does not seem to be defined.
	    if(
		action == AMOTION_EVENT_ACTION_BUTTON_PRESS ||
		action ==  AMOTION_EVENT_ACTION_DOWN
	    ) {
		if((buttons &  AMOTION_EVENT_BUTTON_PRIMARY) != 0) {
		    mouse_handler_btn = 0;
		} else if(((buttons &  AMOTION_EVENT_BUTTON_SECONDARY) != 0)) {
		    mouse_handler_btn = 1;
		} else if(((buttons &  AMOTION_EVENT_BUTTON_TERTIARY) != 0)) {
		    mouse_handler_btn = 2;
		}
	    }
	    g_mouse_CB(
		g_mouseX, g_mouseY, mouse_handler_btn,
		decode_action(action), EVENT_SOURCE_MOUSE
	    );	    
	}
    }    
    
    return 1;    
}

int32_t ImGui_ImplAndroid_MotionEvent(
    struct android_app* app, AInputEvent* event
) {
    int32_t result = 0;
    switch(AMotionEvent_getToolType(event,0)) {
	case AMOTION_EVENT_TOOL_TYPE_FINGER:
	    result = ImGui_ImplAndroid_FingerEvent(app, event);
	    break;
	case AMOTION_EVENT_TOOL_TYPE_STYLUS:
	    result = ImGui_ImplAndroid_StylusEvent(app, event);	    
	    break;
	case AMOTION_EVENT_TOOL_TYPE_MOUSE:
	    result = ImGui_ImplAndroid_MouseEvent(app, event);	    	    
	    break;
	default:
	    break;
    }
    return result;
}

int32_t ImGui_ImplAndroid_KeyEvent(
    struct android_app* app, AInputEvent* event
) {
    // Note: important to return 1 on BACK key pressed,
    // else this triggers default behavior that stops the
    // application brutally.

    ImGuiIO& io = ImGui::GetIO();
    
    int32_t action = AKeyEvent_getAction(event);
    int32_t key = AKeyEvent_getKeyCode(event);
    int32_t modifiers = AKeyEvent_getMetaState(event);
    int32_t device = AInputEvent_getDeviceId(event);    

    if(key >= 0 && key < IM_ARRAYSIZE(io.KeysDown)) {
	if((AKeyEvent_getFlags(event) & AKEY_EVENT_FLAG_SOFT_KEYBOARD)) {
	    // The soft keyboard generates Push/Release events when the
	    // key is released. Thus we mark the key as pushed, and
	    // set g_resetKeys so that ImGui_ImplAndroid_EndFrame()
	    // will mark the key as released after ImGui could do what
	    // it has to do with the key.
	    io.KeysDown[key] = true;
	    g_resetKeys = true;
	} else {
	    io.KeysDown[key] = (action == AKEY_EVENT_ACTION_DOWN);
	    g_resetKeys = false;	    
	}
	io.KeyShift = ((modifiers & AMETA_SHIFT_ON) != 0);
	io.KeyCtrl = ((modifiers & AMETA_CTRL_ON) != 0);
	io.KeyAlt = ((modifiers & AMETA_ALT_ON) != 0);
	io.KeySuper = ((modifiers & AMETA_META_ON) != 0);
    }

    // WIP: right mouse handler (does not work yet)
    // Detect whether it was triggered by right mouse click
    // (if it was the case, re-route it).
    if(action == AKEY_EVENT_ACTION_UP &&
       key == AKEYCODE_BACK &&
       AInputEvent_getSource(event) == AINPUT_SOURCE_MOUSE &&
       g_mouse_CB != nullptr 
     ) {
	mouse_handler_btn = -1;
	g_mouse_CB(g_mouseX, g_mouseY, 1, EVENT_ACTION_UP, EVENT_SOURCE_MOUSE);
    }
    
    if(action == AKEY_EVENT_ACTION_DOWN) {
	if(key == AKEYCODE_BACK) {
	    // WIP: right mouse handler (does not work yet)	    
	    // Detect whether it was triggered by right mouse click
	    // (if it was the case, re-route it).
	    if(AInputEvent_getSource(event) != AINPUT_SOURCE_MOUSE) {
		// If real back button, quit application
		// (normally, returning 0 should do the same, but
		//  it does seem to work, to be understood...).
		if(Application::instance() != nullptr) {
		    Application::instance()->stop();
		}
	    } else {
		if(g_mouse_CB != nullptr) {
		    mouse_handler_btn = 1;
		    g_mouse_CB(
			g_mouseX, g_mouseY, 1,
			EVENT_ACTION_DOWN, EVENT_SOURCE_MOUSE
		    );
		}
	    }
	} else {
	    jint unicode = AndroidUtils::keycode_to_unicode(
		app, device, key, modifiers
	    );
	    // TODO: use AddInputCharactersUTF8()
	    char c = char(unicode);
	    if(isprint(c)) {
		io.AddInputCharacter(c);
	    }
	}
    }
    
    return 1;
}

int32_t ImGui_ImplAndroid_InputEvent(
    struct android_app* app, AInputEvent* event
) {
    int32_t result = 0;
    switch(AInputEvent_getType(event)) {
	case AINPUT_EVENT_TYPE_MOTION:
	    result = ImGui_ImplAndroid_MotionEvent(app, event);
	    break;
	case AINPUT_EVENT_TYPE_KEY:
	    result = ImGui_ImplAndroid_KeyEvent(app, event);
	    break;
	default:
	    break;
    }
    return result;
}
    
#endif

/********************************************************************/


