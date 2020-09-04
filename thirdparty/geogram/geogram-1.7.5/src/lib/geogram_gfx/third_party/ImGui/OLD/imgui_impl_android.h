/*
 * ImGui Platform Binding for: Android
 * Author: Bruno Levy   Sun Aug 19 08:01:39 CEST 2018
 * Note: not part (yet) of the official ImGui distribution
 */ 

#ifdef __ANDROID__
#include <android_native_app_glue.h>

// param app: if non-null, registers input handler to specified app.
IMGUI_IMPL_API bool     ImGui_ImplAndroid_Init(
    struct android_app* app = nullptr
);

IMGUI_IMPL_API void     ImGui_ImplAndroid_Shutdown();

// Needs to be called at the beginning of each frame,
// before ImGui::NewFrame().
IMGUI_IMPL_API void     ImGui_ImplAndroid_NewFrame();

// Needs to be called at the end of each frame,
// after all other ImGui functions.
IMGUI_IMPL_API void     ImGui_ImplAndroid_EndFrame();

// The event handler, to be used if not registered by ImGui_ImplAndroid_Init().
IMGUI_IMPL_API int32_t  ImGui_ImplAndroid_InputEvent(
    struct android_app* app, AInputEvent* event
);

// x,y window coordinats (0..width-1 x 0..height-1)
// button: one of 0:left, 1:right, 2:middle
// action: one of 0:UP, 1:DOWN, 2:DRAG
// source: one of 0:KEYBOARD, 1:MOUSE, 2:FINGER, 3:STYLUS
typedef void (*ImGui_ImplAndroid_MouseUserCallback)(
    float x, float y, int button, int action, int source
);

//   Registers a user mouse event handler.
// Note: the mouse handler needs to test the ImGui::GetIO().WantCaptureMouse
// flag to determine whether the event should be processed. The reason why
// it is not tested before the handler is because when a menu is open and
// the user clicks outside the menu, the flag is still set (this situation
// needs special code to be handled properly).
IMGUI_IMPL_API void ImGui_ImplAndroid_SetMouseUserCallback(
    ImGui_ImplAndroid_MouseUserCallback CB
);

#endif
