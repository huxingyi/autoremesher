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
 
#ifndef OGF_IMAGE_IO_IMAGE_SERIALIZER_XPM
#define OGF_IMAGE_IO_IMAGE_SERIALIZER_XPM

#include <geogram/basic/common.h>
#include <geogram/image/image_serializer.h>

namespace GEO {

//_________________________________________________________

    class GEOGRAM_API ImageSerializer_xpm : public ImageSerializer {
    public:
        Image* serialize_read(std::istream& stream) override;
        bool read_supported() const override;
        bool binary() const override;

	/**
	 * \brief Creates an image from XPM data.
	 * \param[in] s the string with the XPM data.
	 * \return the created image. 
	 */
	static Image* create_image_from_xpm_data(const char* s);

    protected:
	
	static Image* serialize_read_static(std::istream& stream);

        static Image* read_xpm_1_byte_per_pixel(
            int width, int height, int num_colors,
            std::istream& input
        );

        static Image* read_xpm_2_bytes_per_pixel(
            int width, int height, int num_colors,
            std::istream& input
        );

        static char* next_xpm_data(std::istream& input);
    };

//_________________________________________________________

}
#endif

