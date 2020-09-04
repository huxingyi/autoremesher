/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000 Bruno Levy
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Bruno Levy
 *
 *     levy@loria.fr
 *
 *     ISA Project
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 *  Note that the GNU General Public License does not permit incorporating
 *  the Software into proprietary programs. 
 */
 
#include <geogram/image/image_library.h>
#include <geogram/image/image.h>
#include <geogram/image/image_serializer.h>

#include <geogram/basic/file_system.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/string.h>

// For clipboard 
#ifdef GEO_OS_WINDOWS
#include <windows.h>
#endif

namespace GEO {

//_________________________________________________________

    ImageLibrary* ImageLibrary::instance_ = nullptr ;

    ImageLibrary* ImageLibrary::instance() {
        return instance_;
    }

    void ImageLibrary::initialize() {
        geo_assert(instance_ == nullptr) ;
        instance_ = new ImageLibrary() ;
        Environment::instance()->add_environment(instance_) ;
    }

    void ImageLibrary::terminate() {
        // Note: instance is not deleted here. Since it
        // is declared in the Environment, now Environment
        // has the ownership and will delete it.
        geo_assert(instance_ != nullptr) ;
        instance_ = nullptr ;
    }

    ImageLibrary::ImageLibrary() {
    }

    ImageLibrary::~ImageLibrary() {
    }


    bool ImageLibrary::bind_image_serializer(
        const std::string& extension, ImageSerializer* serializer
    ) {
        std::string upper_extension = extension ;
        String::to_uppercase(upper_extension) ;
        if(
            resolve_image_serializer(extension) != nullptr ||
            resolve_image_serializer(upper_extension) != nullptr 
        ) {
            return false ;
        }
        image_serializers_[extension] = serializer ;
        image_serializers_[upper_extension] = serializer ;
        Environment::notify_observers("image_read_extensions") ;
        Environment::notify_observers("image_write_extensions") ;
        return true ;
    }

    ImageSerializer* ImageLibrary::resolve_image_serializer(
        const std::string& extension
    ) const {
        auto it = image_serializers_.find(extension) ;
        if(it == image_serializers_.end()) {
            return nullptr ;
        }
        return it->second ;
    }

    bool ImageLibrary::bind_image(const std::string& name, Image* image) {
        if(resolve_image(name) != nullptr) {
            return false ;
        }
        images_[name] = image ;
        return true ;
    }

    bool ImageLibrary::unbind_image(const std::string& name) {
        auto it = images_.find(name) ;
        if(it == images_.end()) {
            return false ;
        }
        images_.erase(it) ;
        return true ;
    }

    Image* ImageLibrary::resolve_image(const std::string& name) const {
        auto it = images_.find(name) ;
        if(it == images_.end()) {
            return nullptr ;
        }
        return it->second ;
    }

    Image* ImageLibrary::load_image(const std::string& file_name) {
        std::string extension = FileSystem::extension(file_name) ;
        if(extension.length() == 0) {
            Image* result = resolve_image(file_name) ;
            if(result != nullptr) { result->acquire(); return result ; }
            Logger::err("ImageLibrary") 
                << "no extension in file name and no such registered image" << std::endl ;
            return nullptr ;
        }
        
        ImageSerializer* serializer = resolve_image_serializer(extension) ;
        if(serializer == nullptr) {
            Logger::err("ImageLibrary") 
                << "could not find serializer for extension \'"
                << extension << "\'" << std::endl ;
            return nullptr ;
        }

        if(!serializer->read_supported()) {
            Logger::err("ImageLibrary") 
                << "serializer for extension \'"
                << extension << "\' does not have a \'read\' function" 
                << std::endl ;
            return nullptr ;
        }

        return serializer->serialize_read(file_name) ;
    }

    bool ImageLibrary::save_image(
        const std::string& file_name, Image* image
    ) {

        std::string extension = FileSystem::extension(file_name) ;
        if(extension.length() == 0) {
            Logger::err("ImageLibrary") 
                << "no extension in file name" << std::endl ;
            return false ;
        }
        
        ImageSerializer* serializer = resolve_image_serializer(extension) ;
        if(serializer == nullptr) {
            Logger::err("ImageLibrary") 
                << "could not find serializer for extension \'"
                << extension << "\'" << std::endl ;
            return false ;
        }

        if(!serializer->write_supported()) {
            Logger::err("ImageLibrary") 
                << "serializer for extension \'"
                << extension << "\' does not have a \'write\' function" 
                << std::endl ;
            return false ;
        }
        
        return serializer->serialize_write(file_name, image) ;
    }

    void ImageLibrary::copy_image_to_clipboard(Image* image) {
        geo_argused(image);
        
#ifdef GEO_OS_WINDOWS

        if(image->color_encoding() != Image::RGB) {
            Logger::err("ImageLibrary")
                << "copy_image_to_clipboard() "
                << "not implemented for this color encoding" 
                << std::endl ;
            return ;
        }
        
        // Thanks to Pierre Alliez for his help with
        // Windows clipboard programming.
        
        // Step 1: Try to open Window's clipboard
        //   nullptr -> bind to current process
        if(!::OpenClipboard( nullptr )) {
            return ;
        }
        
        index_t h = image->height() ;
        index_t w = image->width() ;
        
        // Step 2: Prepare the image for Windows:
        //   flip the image and flip rgb -> bgr
        {
            index_t row_len = image->width() * 3 ;
            for(index_t j=0; j< h/2; j++) {
                Memory::pointer row1 =
                    image->base_mem() + j * row_len ;
                Memory::pointer row2 =
                    image->base_mem() + (h - 1 - j) * row_len ;
                for(index_t i=0; i<w; i++) {
                    std::swap(row1[3*i+2], row2[3*i  ]) ;
                    std::swap(row1[3*i+1], row2[3*i+1]) ;
                    std::swap(row1[3*i  ], row2[3*i+2]) ;
                }
            }
        }
        
        // Step 3: create a shared memory segment, with
        // a DIB (Device Independent Bitmap) in it.
        HANDLE handle;
        
        index_t image_size = 3 * image->width() * image->height();
        index_t size = index_t(sizeof(BITMAPINFOHEADER)) + image_size ;
        
        handle = (HANDLE)::GlobalAlloc(GHND,size);
        if(handle != nullptr) {
            char *pData = (char *) ::GlobalLock((HGLOBAL)handle);
            BITMAPINFOHEADER header ;
            header.biSize          = sizeof(BITMAPINFOHEADER);
            header.biWidth         = (LONG)(image->width()) ;
	    header.biHeight        = (LONG)(image->height()) ;
            header.biPlanes        = 1 ;
            header.biBitCount      = 24 ;
            header.biCompression   = BI_RGB ;
            header.biSizeImage     = 0 ;
            header.biXPelsPerMeter = 1000000 ;
            header.biYPelsPerMeter = 1000000 ;
            header.biClrUsed       = 0 ;
            header.biClrImportant  = 0 ;
            ::memcpy(pData,&header,sizeof(BITMAPINFOHEADER));    
            ::memcpy(
                pData+sizeof(BITMAPINFOHEADER),image->base_mem(),image_size
            ) ;

            // Step 4: put the data in the clipboard.
            ::GlobalUnlock((HGLOBAL)handle);
            ::EmptyClipboard() ;
            ::SetClipboardData(CF_DIB,handle);
            ::CloseClipboard();

            // Step 5: restore the image
            {
                index_t row_len = image->width() * 3 ;
                for(index_t j=0; j< h/2; j++) {
                    Memory::pointer
                        row1 = image->base_mem() + j * row_len ;
                    Memory::pointer
                        row2 = image->base_mem() + (h - 1 - j) * row_len ;
                    for(index_t i=0; i<w; i++) {
                        std::swap(row1[3*i+2], row2[3*i  ]) ;
                        std::swap(row1[3*i+1], row2[3*i+1]) ;
                        std::swap(row1[3*i  ], row2[3*i+2]) ;
                    }
                }
            }
        }
#else
        Logger::err("ImageLibrary") << "copy_image_to_clipboard() "
                                    << "not implemented for this OS" 
                                    << std::endl ;
#endif
    }

    bool ImageLibrary::get_local_value(
        const std::string& name, std::string& value
    ) const {
        if(name == "image_read_extensions") {
            value = "" ;
            for(auto& it : image_serializers_) {
                if(it.second->read_supported()) {
                    if(value.length() != 0) {
                        value += ";" ;
                    }
                    value += "*." ;
                    value += it.first ;
                }
            }
            return true ;
        } else if(name=="image_write_extensions") {
            value = "" ;
            for(auto& it : image_serializers_) {
                if(it.second->write_supported()) {
                    if(value.length() != 0) {
                        value += ";" ;
                    }
                    value += "*." ;
                    value += it.first ;
                }
            }
            return true ;
        } else {
            return false ;
        }
    }

    bool ImageLibrary::set_local_value(const std::string& name, const std::string& value) {
        geo_argused(name);
        geo_argused(value);
        return false;
    }

    
//_________________________________________________________

}

