/*
 *  Copyright (c) 2012-2014, Bruno Levy
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

/*
 *
 * b_stream.cpp
 *
 */

#include <geogram/basic/b_stream.h>
#include <geogram/basic/assert.h>
#include <geogram/basic/logger.h>

#include <iostream>
#include <stdlib.h>

namespace GEO {

    BinaryStream::BinaryStream(int stream_endian) {
        has_record_markers_ = true;
        detect_machine_endian();
        set_stream_endian(stream_endian);
    }

    void BinaryStream::detect_machine_endian() {

        Numeric::int32 data = 0x04030201;
        Numeric::uint8* pointer = (Numeric::uint8*) &data;

        if(
            pointer[0] == 1 &&
            pointer[1] == 2 &&
            pointer[2] == 3 &&
            pointer[3] == 4
        ) {
            machine_endian_ = GEO_LITTLE_ENDIAN;
        } else if(
            pointer[0] == 4 &&
            pointer[1] == 3 &&
            pointer[2] == 2 &&
            pointer[3] == 1) {
            machine_endian_ = GEO_BIG_ENDIAN;
        } else {
            Logger::err("BinaryStream")
                << "invalid processor type" << std::endl;
            // Should not occur, nowadays machines are either little or
            // big endian.
            ::abort();
        }
    }

    void BinaryStream::set_stream_endian(int stream_endian) {
        stream_endian_ = stream_endian;
        swapped_ = stream_endian_ != machine_endian_;
    }

    /************************************************************************/

    BinaryInputStream::BinaryInputStream(
        const std::string& file_name, int stream_endian_in
    ) :
        BinaryStream(stream_endian_in) {
        record_OK_ = true;
        record_count_ = 0;
        input_ = new std::ifstream(
            file_name.c_str(),
            std::fstream::in | std::fstream::binary
        );
        owns_input_ = true;
        if(!input_) {
            delete input_;
            input_ = nullptr;
        }
    }

    BinaryInputStream::BinaryInputStream(
        std::istream& input, int stream_endian_in
    ) :
        BinaryStream(stream_endian_in) {
        record_OK_ = true;
        record_count_ = 0;
        input_ = &input;
        owns_input_ = false;
    }

    bool BinaryInputStream::OK() const {
        return record_OK_ && input_ != nullptr && input_->good();
    }

    bool BinaryInputStream::more() const {
        return !input_->eof();
    }

    void BinaryInputStream::begin_record() {
        if(input_->eof()) {
            record_OK_ = false;
        } else {
            if(has_record_markers()) {
                (*this) >> count1_;
            }
        }
    }

    void BinaryInputStream::end_record() {
        record_count_++;
        if(input_->eof()) {
            record_OK_ = false;
        } else {
            if(has_record_markers()) {
                (*this) >> count2_;
                if(count1_ != count2_) {
                    record_OK_ = false;
                    Logger::err("BinaryStream")
                        << "Invalid record length in record #"
                        << record_count_ << std::endl;
                    Logger::err("BinaryStream")
                        << " count1=" << count1_
                        << " count2=" << count2_ << std::endl;
                }
            }
        }
    }

    BinaryInputStream::~BinaryInputStream() {
        if(input_ != nullptr && owns_input_) {
            delete input_;
        }
        input_ = nullptr;
    }

    BinaryInputStream& BinaryInputStream::read(
        char* data, size_t n, ItemSize<2>
    ) {
        const size_t size = 2;
        if(!swapped_) {
            input_->read(data, (std::streamsize) (size * n));
        } else {
            char buffer[size];
            for(; n != 0; --n) {
                input_->read(buffer, size);
                data[0] = buffer[1];
                data[1] = buffer[0];
                data += size;
            }
        }
        return *this;
    }

    BinaryInputStream& BinaryInputStream::read(
        char* data, size_t n, ItemSize<4>
    ) {
        const size_t size = 4;
        if(!swapped_) {
            input_->read(data, (std::streamsize) (size * n));
        } else {
            char buffer[size];
            for(; n != 0; --n) {
                input_->read(buffer, size);
                data[0] = buffer[3];
                data[1] = buffer[2];
                data[2] = buffer[1];
                data[3] = buffer[0];
                data += size;
            }
        }
        return *this;
    }

    BinaryInputStream& BinaryInputStream::read(
        char* data, size_t n, ItemSize<8>
    ) {
        const size_t size = 8;
        if(!swapped_) {
            input_->read(data, (std::streamsize) (size * n));
        } else {
            char buffer[size];
            for(; n != 0; --n) {
                input_->read(buffer, size);
                data[0] = buffer[7];
                data[1] = buffer[6];
                data[2] = buffer[5];
                data[3] = buffer[4];
                data[4] = buffer[3];
                data[5] = buffer[2];
                data[6] = buffer[1];
                data[7] = buffer[0];
                data += size;
            }
        }
        return *this;
    }

    /************************************************************************/

    BinaryOutputStream::BinaryOutputStream(
        const std::string& file_name, int stream_endian_in
    ) :
        BinaryStream(stream_endian_in) {
        output_ = new std::ofstream(
            file_name.c_str(),
            std::fstream::out | std::fstream::trunc | std::fstream::binary
        );
        if(!output_) {
            delete output_;
            output_ = nullptr;
        }
        owns_output_ = true;
    }

    BinaryOutputStream::BinaryOutputStream(
        std::ostream& output, int stream_endian_in
    ) :
        BinaryStream(stream_endian_in) {
        output_ = &output;
        owns_output_ = false;
    }

    bool BinaryOutputStream::OK() const {
        return output_ != nullptr && output_->good();
    }

    void BinaryOutputStream::begin_record() {
        if(has_record_markers()) {
            count_ = 0;
            pos_ = output_->tellp();
            write_marker(Numeric::uint32(count_));
        }
    }

    void BinaryOutputStream::end_record() {
        if(has_record_markers()) {
            write_marker(Numeric::uint32(count_));
            std::streamoff pos = output_->tellp();
            output_->seekp(pos_);
            write_marker(Numeric::uint32(count_));
            output_->seekp(pos);
        }
    }

    BinaryOutputStream::~BinaryOutputStream() {
        if(output_ != nullptr && owns_output_) {
            delete output_;
        }
        output_ = nullptr;
    }

    void BinaryOutputStream::write_marker(Numeric::uint32 x) {
        geo_assert(output_ != nullptr);
        BinaryOutputStream::operator<< (x);

        // Warning: operator<<() increments count_!
        count_ -= sizeof(x);
    }

    BinaryOutputStream& BinaryOutputStream::write(
        const char* data, size_t n, ItemSize<2>
    ) {
        const size_t size = 2;
        if(!swapped_) {
            output_->write(data, (std::streamsize) (size * n));
        } else {
            char buffer[size];
            for(; n != 0; --n) {
                buffer[0] = data[1];
                buffer[1] = data[0];
                output_->write(buffer, size);
                data += size;
            }
        }
        count_ += size * n;
        return *this;
    }

    BinaryOutputStream& BinaryOutputStream::write(
        const char* data, size_t n, ItemSize<4>
    ) {
        const size_t size = 4;
        if(!swapped_) {
            output_->write(data, (std::streamsize) (size * n));
        } else {
            char buffer[size];
            for(; n != 0; --n) {
                buffer[0] = data[3];
                buffer[1] = data[2];
                buffer[2] = data[1];
                buffer[3] = data[0];
                output_->write(buffer, size);
                data += size;
            }
        }
        count_ += size * n;
        return *this;
    }

    BinaryOutputStream& BinaryOutputStream::write(
        const char* data, size_t n, ItemSize<8>
    ) {
        const size_t size = 8;
        if(!swapped_) {
            output_->write(data, (std::streamsize) (size * n));
        } else {
            char buffer[size];
            for(; n != 0; --n) {
                buffer[0] = data[7];
                buffer[1] = data[6];
                buffer[2] = data[5];
                buffer[3] = data[4];
                buffer[4] = data[3];
                buffer[5] = data[2];
                buffer[6] = data[1];
                buffer[7] = data[0];
                output_->write(buffer, size);
                data += size;
            }
        }
        count_ += size * n;
        return *this;
    }
}

