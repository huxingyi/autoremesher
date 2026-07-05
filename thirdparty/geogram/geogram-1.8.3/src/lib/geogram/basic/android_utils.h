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

#ifndef GEOGRAM_BASIC_ANDROID_UTILS
#define GEOGRAM_BASIC_ANDROID_UTILS

#include <geogram/basic/common.h>
#include <geogram/basic/numeric.h>
#include <string>

/**
 * \file geogram/basic/android_utils.h
 * \brief Functions for Android.
 */

struct android_app;

namespace GEO {
    namespace AndroidUtils {

	/**
	 * \brief Shows the software keyboard on the phone.
	 * \param[in] app a pointer to the android app.
	 */
	void GEOGRAM_API show_soft_keyboard(android_app* app);

	/**
	 * \brief Hides the software keyboard on the phone.
	 * \param[in] app a pointer to the android app.
	 */
	void GEOGRAM_API hide_soft_keyboard(android_app* app);

	/**
	 * \brief Converts a keycode to a Unicode character.
	 * \param[in] app a pointer to the android app.
	 * \param[in] deviceId , keyCode , metaState obtained
	 *  from the InputEvent.
	 */
	Numeric::int32 GEOGRAM_API keycode_to_unicode(
	    android_app* app,
	    Numeric::int32 deviceId,
	    Numeric::int32 keyCode,
	    Numeric::int32 metaState
	);

	/**
	 * \brief Tests whether a permission is granted.
	 * \param[in] app a pointer to the android app.
	 * \param[in] perm the name of the permission, e.g.,
	 *   "READ_EXTERNAL_STORAGE", "WRITE_EXTERNAL_STORAGE".
	 * \retval true if the permission is granted.
	 * \retval false otherwise.
	 * \note Requires Android API level 23 (Marshmallow, May 2015)
	 */
	bool GEOGRAM_API has_permission(android_app* app, const char* perm);

	/**
	 * \brief Request permissions.
	 * \details This opens the system dialog that lets the user
	 *  grant (or deny) the permission.
	 * \param[in] app a pointer to the android app.
	 * \param[in] nb_perms number of requested permissions.
	 * \param[in] perms the names of the permission, e.g.,
	 *   "READ_EXTERNAL_STORAGE", "WRITE_EXTERNAL_STORAGE".
	 * \note Requires Android API level 23 (Marshmallow, May 2015)
	 */
	void GEOGRAM_API request_permissions(
	    android_app* app, int nb_perms, const char** perms
	);

	/**
	 * \brief Gets the path for temporary file.
	 * \param[in] app a pointer to the android app.
	 * \return a std::string with the path where to write
	 *  temporary files.
	 */
	std::string GEOGRAM_API temp_folder(android_app* app);
    }
}



#endif
