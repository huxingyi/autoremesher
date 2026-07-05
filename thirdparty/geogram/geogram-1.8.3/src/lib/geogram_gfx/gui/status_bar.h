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

#ifndef H_GEOGRAM_GFX_GUI_STATUS_BAR_H
#define H_GEOGRAM_GFX_GUI_STATUS_BAR_H

#include <geogram_gfx/basic/common.h>
#include <geogram/basic/progress.h>

/**
 * \file geogram_gfx/gui/status_bar.h
 * \brief Implementation of the status bar.
 */

namespace GEO {

    /**
     * \brief StatusBar displays the progress bar.
     */
    class GEOGRAM_GFX_API StatusBar : public GEO::ProgressClient {
    public:

        /**
         * \brief StatusBar constructor.
         */
        StatusBar();
        
        /**
         * \copydoc GEO::ProgressClient::begin()
         */
	void begin() override;
        
        /**
         * \copydoc GEO::ProgressClient::progress()
         */
	void progress(GEO::index_t step, GEO::index_t percent) override;
        
        /**
         * \copydoc GEO::ProgressClient::end()
         */
	void end(bool canceled) override;

        /**
         * \brief Draws the status bar and handles the GUI.
         */
        void draw();

        /**
         * \brief Tests whether this status bar should be displayed.
         * \details The status bar is displayed whenever there is an
         *  active progress bar.
         */
        bool active() const {
            return (nb_active_ > 0);
        }

	/**
	 * \brief Redraws the GUI.
	 */
	virtual void update();

	/**
	 * \brief Gets the height of the status bar window.
	 * \return the height of the window.
	 * \details Needs to have drawn the window at least once,
	 *  else it returns 0.
	 */
	float get_window_height() const {
	    return height_;
	}
	
      private:
        bool progress_;
        index_t step_;
        index_t percent_;
        bool canceled_;
        index_t nb_active_;
	float height_;
    };

    typedef SmartPointer<StatusBar> StatusBar_var;
}

#endif
