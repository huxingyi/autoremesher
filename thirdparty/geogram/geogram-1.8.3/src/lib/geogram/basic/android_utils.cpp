/*
 *  Copyright (c) 2000-2022 Inria
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
 *  Contact: Bruno Levy
 *
 *     https://www.inria.fr/fr/bruno-levy
 *
 *     Inria,
 *     Domaine de Voluceau,
 *     78150 Le Chesnay - Rocquencourt
 *     FRANCE
 *
 */

#ifdef __ANDROID__

#include <geogram/basic/android_utils.h>
#include <geogram/basic/argused.h>
#include <android_native_app_glue.h>

namespace {
    using namespace GEO;

/******************************************************************************/
   //Protocol to call a JNI function in an Android App:
   //https://groups.google.com/forum/?fromgroups=#!topic/android-ndk/Tk3g00wLKhk
   //   (see messages about how to attach/detach thread).

    /**
     * \brief To be called at the beginning of each function that uses the JNI.
     * \details Gets the JNIEnv and attaches the current thread to 
     *  the JavaVM if need be.
     * \param[in] app a pointer to the android_app
     * \param[out] jni_env a pointer to the JNIEnv
     * \param[out] thread_attached true if this thread was attached to the VM, 
     *  false otherwise.
     */
    inline void enter_JNI_function(
	android_app* app, JNIEnv*& jni_env, bool& thread_attached
    ) {
	jni_env = nullptr; 
	thread_attached = false;
	JavaVM* java_vm = app->activity->vm;
	// Get JNIEnv from the JavaVM using GetEnv to test whether
	// thread is attached or not to the VM. If not, attach it
	// (and note that it will need to be detached at the end
	//  of the function).
	switch (java_vm->GetEnv((void**)&jni_env, JNI_VERSION_1_6)) {
	    case JNI_OK:
		break;
	    case JNI_EDETACHED: {
		jint attach_result =
		    java_vm->AttachCurrentThread(&jni_env, nullptr);
		if(attach_result == JNI_ERR) {
		    throw std::runtime_error("Could not attach current thread");
		}
		thread_attached = true;
	    } break;
	    case JNI_EVERSION:
		throw std::runtime_error("Invalid java version");
	}
    }

    /**
     * \brief To be called at the endof each function that uses the JNI.
     * \details Detaches the current thread from the JavaVM if need be.
     * \param[in] app a pointer to the android_app
     * \param[in] jni_env a pointer to the JNIEnv
     * \param[in] thread_attached true if this thread was attached to the VM
     *   by enter_JNI_function(), false otherwise.
     */
    inline void leave_JNI_function(
	android_app* app, JNIEnv* jni_env, bool thread_attached
    ) {
	geo_argused(jni_env);
	if(thread_attached) {
	    app->activity->vm->DetachCurrentThread();
	}
    }

/******************************************************************************/
   // Display soft keyboard programmatically
   //https://groups.google.com/forum/?fromgroups=#!topic/android-ndk/Tk3g00wLKhk
   // There is normally an NDK function supposed to do that: 
   // ANativeActivity_showSoftInput(
   //   mApplication->activity,ANATIVEACTIVITY_SHOW_SOFT_INPUT_FORCED
   // );
   // (or ANATIVEACTIVITY_SHOW_SOFT_INPUT_IMPLICIT) 
   // but I did not manage to make it work....
    
    /**
     * \brief Shows or hides the soft keyboard.
     * \param[in] app a pointer to the android_app
     * \param[in) pShow true if the keyboard should be
     *  shown, false if it should be hidden.
     */
    void set_soft_keyboard_visibility(android_app* app, bool pShow) {
	JNIEnv* lJNIEnv = nullptr;
	bool thread_attached = false;
	enter_JNI_function(app, lJNIEnv, thread_attached);

	// Retrieves NativeActivity.
	jobject lNativeActivity = app->activity->clazz;
	jclass ClassNativeActivity = lJNIEnv->GetObjectClass(lNativeActivity);

	// Retrieves Context.INPUT_METHOD_SERVICE.
	jclass ClassContext = lJNIEnv->FindClass("android/content/Context");
	jfieldID FieldINPUT_METHOD_SERVICE =
	    lJNIEnv->GetStaticFieldID(
		ClassContext,"INPUT_METHOD_SERVICE", "Ljava/lang/String;"
	);
	jobject INPUT_METHOD_SERVICE =
	    lJNIEnv->GetStaticObjectField(
		ClassContext, FieldINPUT_METHOD_SERVICE
	);

	// Runs getSystemService(Context.INPUT_METHOD_SERVICE).
	jclass ClassInputMethodManager = lJNIEnv->FindClass(
	    "android/view/inputmethod/InputMethodManager"
	);
	jmethodID MethodGetSystemService = lJNIEnv->GetMethodID(
	    ClassNativeActivity, "getSystemService",
	    "(Ljava/lang/String;)Ljava/lang/Object;"
	);
	jobject lInputMethodManager = lJNIEnv->CallObjectMethod(
	    lNativeActivity, MethodGetSystemService,
	    INPUT_METHOD_SERVICE
	);

	// Runs getWindow().getDecorView().
	jmethodID MethodGetWindow = lJNIEnv->GetMethodID(
	    ClassNativeActivity, "getWindow",
	    "()Landroid/view/Window;"
	);
	jobject lWindow = lJNIEnv->CallObjectMethod(
	    lNativeActivity, MethodGetWindow
	);
	jclass ClassWindow = lJNIEnv->FindClass("android/view/Window");
	jmethodID MethodGetDecorView = lJNIEnv->GetMethodID(
	    ClassWindow, "getDecorView", "()Landroid/view/View;"
	);
	jobject lDecorView = lJNIEnv->CallObjectMethod(
	    lWindow, MethodGetDecorView
	);

	if (pShow) {
	    // Runs lInputMethodManager.showSoftInput(...).
	    jmethodID MethodShowSoftInput = lJNIEnv->GetMethodID(
		ClassInputMethodManager, "showSoftInput",
		"(Landroid/view/View;I)Z"
	    );
	    lJNIEnv->CallBooleanMethod(
		lInputMethodManager, MethodShowSoftInput,
		lDecorView, 0
	    );
	} else {
	    // Runs lWindow.getViewToken()
	    jclass ClassView = lJNIEnv->FindClass(
		"android/view/View"
	    );
	    jmethodID MethodGetWindowToken = lJNIEnv->GetMethodID(
		ClassView, "getWindowToken", "()Landroid/os/IBinder;"
	    );
	    jobject lBinder = lJNIEnv->CallObjectMethod(
		lDecorView, MethodGetWindowToken
	    );

	    // lInputMethodManager.hideSoftInput(...).
	    jmethodID MethodHideSoftInput = lJNIEnv->GetMethodID(
		ClassInputMethodManager, "hideSoftInputFromWindow",
		"(Landroid/os/IBinder;I)Z"
	    );
	    lJNIEnv->CallBooleanMethod(
		lInputMethodManager, MethodHideSoftInput,
		lBinder, 0
	    );
	}
	
	leave_JNI_function(app, lJNIEnv, thread_attached);	
    }

    /**
     * \brief Gets the internal name for an android permission.
     * \param[in] lJNIEnv a pointer to the JNI environment
     * \param[in] perm_name the name of the permission, e.g.,
     *   "READ_EXTERNAL_STORAGE", "WRITE_EXTERNAL_STORAGE".
     * \return a jstring with the internal name of the permission,
     *   to be used with android Java functions 
     *   Context.checkSelfPermission() or Activity.requestPermissions()
     */
    jstring android_permission_name(
	JNIEnv* lJNIEnv, const char* perm_name
    ) {
	// nested class permission in class android.Manifest,
	// hence android 'slash' Manifest 'dollar' permission
	jclass ClassManifestpermission = lJNIEnv->FindClass(
	    "android/Manifest$permission"
	);

	jfieldID lid_PERM = lJNIEnv->GetStaticFieldID(
	    ClassManifestpermission, perm_name, "Ljava/lang/String;"
	);

	return (jstring)(lJNIEnv->GetStaticObjectField(
	     ClassManifestpermission, lid_PERM
	));
    }
}

namespace GEO {

    namespace AndroidUtils {
	
	void show_soft_keyboard(android_app* app) {
	    set_soft_keyboard_visibility(app, true);
	}

	void hide_soft_keyboard(android_app* app) {
	    set_soft_keyboard_visibility(app, false);	
	}
    
	Numeric::int32 keycode_to_unicode(
	    android_app* app,
	    Numeric::int32 pDeviceId,
	    Numeric::int32 pKeyCode,
	    Numeric::int32 pMetaState
	) {
	    jint result = 0;

	    // Early exit for special keys
	    // (works without it, but well, why calling all that
	    //  Java stuff if we now in advance that we do not need
	    //  to ?).
	    if(
		pKeyCode == AKEYCODE_TAB ||
		pKeyCode == AKEYCODE_DPAD_LEFT ||
		pKeyCode == AKEYCODE_DPAD_RIGHT ||
		pKeyCode == AKEYCODE_DPAD_UP ||
		pKeyCode == AKEYCODE_DPAD_DOWN ||
		pKeyCode == AKEYCODE_PAGE_UP ||
		pKeyCode == AKEYCODE_PAGE_DOWN ||
		pKeyCode == AKEYCODE_MOVE_HOME ||
		pKeyCode == AKEYCODE_MOVE_END ||
		pKeyCode == AKEYCODE_INSERT ||
		pKeyCode == AKEYCODE_FORWARD_DEL ||
		pKeyCode == AKEYCODE_DEL ||
		pKeyCode == AKEYCODE_ENTER ||
		pKeyCode == AKEYCODE_ESCAPE
	    ) {
		return result;
	    }
	
	    JNIEnv* lJNIEnv = nullptr;
	    bool thread_attached = false;
	    enter_JNI_function(app, lJNIEnv, thread_attached);

	    jclass ClassKeyCharacterMap = lJNIEnv->FindClass(
		"android/view/KeyCharacterMap"
	    );

	    jmethodID MethodLoad = lJNIEnv->GetStaticMethodID(
		ClassKeyCharacterMap, "load",
		"(I)Landroid/view/KeyCharacterMap;"
	    );

	    jobject lKeyCharacterMap = lJNIEnv->CallStaticObjectMethod(
		ClassKeyCharacterMap, MethodLoad, jint(pDeviceId)
	    );

	    jmethodID MethodGet = lJNIEnv->GetMethodID(
		ClassKeyCharacterMap, "get",
		"(II)I"
	    );

	    result = lJNIEnv->CallIntMethod(
		lKeyCharacterMap, MethodGet,
		jint(pKeyCode), jint(pMetaState)
	    );
	
	    leave_JNI_function(app, lJNIEnv, thread_attached);
	    return result;
	}

	bool has_permission(android_app* app, const char* perm) {
	    JNIEnv* lJNIEnv = nullptr;
	    bool thread_attached = false;
	    enter_JNI_function(app, lJNIEnv, thread_attached);
	    
	    // Get the symbolic value PERMISSION_GRANTED
	    jclass ClassPackageManager = lJNIEnv->FindClass(
		"android/content/pm/PackageManager"
	    );
	    jfieldID lid_PERMISSION_GRANTED = lJNIEnv->GetStaticFieldID(
		ClassPackageManager, "PERMISSION_GRANTED", "I"
	    );
	    jint PERMISSION_GRANTED = lJNIEnv->GetStaticIntField(
		ClassPackageManager, lid_PERMISSION_GRANTED
	    );

	    // Call checkSelfPermission
	    jobject activity = app->activity->clazz;
	    jclass ClassContext = lJNIEnv->FindClass(
		"android/content/Context"
	    );
	    jmethodID MethodcheckSelfPermission = lJNIEnv->GetMethodID(
		ClassContext, "checkSelfPermission", "(Ljava/lang/String;)I"
	    );
	    jint int_result = lJNIEnv->CallIntMethod(
		activity, MethodcheckSelfPermission,
		android_permission_name(lJNIEnv, perm)
	    );
	    bool result = (int_result == PERMISSION_GRANTED);
	    
	    leave_JNI_function(app, lJNIEnv, thread_attached);
	    return result;
	}
	
	void request_permissions(
	    android_app* app, int nb_perms, const char** perms
	) {
	    JNIEnv* lJNIEnv = nullptr;
	    bool thread_attached = false;
	    enter_JNI_function(app, lJNIEnv, thread_attached);

	    jobjectArray perm_array = lJNIEnv->NewObjectArray(
		nb_perms,
		lJNIEnv->FindClass("java/lang/String"),
		lJNIEnv->NewStringUTF("")
	    );

	    for(int i=0; i<nb_perms; ++i) {
		lJNIEnv->SetObjectArrayElement(
		    perm_array, i,
		    android_permission_name(lJNIEnv, perms[i])
		);
	    }

	    jobject activity = app->activity->clazz;
	
	    jclass ClassActivity = lJNIEnv->FindClass(
		"android/app/Activity"
	    );

	    jmethodID MethodrequestPermissions = lJNIEnv->GetMethodID(
		ClassActivity, "requestPermissions", "([Ljava/lang/String;I)V"
	    );

	    // Last arg (0) is just for the callback (that I do not use)
	    lJNIEnv->CallVoidMethod(
		activity, MethodrequestPermissions, perm_array, 0
	    );
	    
	    leave_JNI_function(app, lJNIEnv, thread_attached);
	}


   // See https://stackoverflow.com/questions/7595324/creating-temporary-files-in-android-with-ndk/10334111
	std::string temp_folder(android_app* app) {
	    JNIEnv* env = nullptr;
	    bool thread_attached = false;
	    enter_JNI_function(app, env, thread_attached);

	    jclass activityClass = env->FindClass(
		"android/app/NativeActivity"
	    );
	    jmethodID getCacheDir = env->GetMethodID(
		activityClass, "getCacheDir", "()Ljava/io/File;"
	    );
	    jobject cache_dir = env->CallObjectMethod(
		app->activity->clazz, getCacheDir
	    );

	    jclass fileClass = env->FindClass("java/io/File");
	    jmethodID getPath = env->GetMethodID(
		fileClass, "getPath", "()Ljava/lang/String;"
	    );
	    jstring path_string = (jstring)env->CallObjectMethod(
		cache_dir, getPath
	    );

	    const char *path_chars = env->GetStringUTFChars(
		path_string,nullptr
	    );
	    std::string result(path_chars);
	    
	    leave_JNI_function(app, env, thread_attached);
	    return result;
	}
	
    }
}

#endif
