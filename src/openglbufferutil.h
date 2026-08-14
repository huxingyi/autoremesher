/*
 *  Copyright (c) 2026 Jeremy HU <jeremy-at-dust3d dot org>. All rights reserved. 
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:

 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.

 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */
#ifndef AUTO_REMESHER_OPENGL_BUFFER_UTIL_H
#define AUTO_REMESHER_OPENGL_BUFFER_UTIL_H
#include <QDebug>
#include <QOpenGLBuffer>
#include <cstddef>
#include <limits>

// QOpenGLBuffer::allocate() describes the upload with an int, so a request of
// INT_MAX bytes or more wraps around to a negative size. glBufferData then
// fails with GL_INVALID_VALUE and leaves the buffer without any storage, and
// the later draw call dereferences a null base pointer inside the driver.
// Refuse the upload instead, and report whether the buffer is safe to draw.
inline bool allocateOpenGLBuffer(QOpenGLBuffer& buffer, const void* data, size_t bytes)
{
    if (bytes > (size_t)std::numeric_limits<int>::max()) {
        qWarning() << "Refusing to upload" << bytes << "bytes, buffer size limit is"
                   << std::numeric_limits<int>::max();
        return false;
    }
    buffer.allocate(data, (int)bytes);
    return buffer.size() == (int)bytes;
}

#endif
