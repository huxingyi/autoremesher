/*
 *  Copyright (c) 2020 Jeremy HU <jeremy-at-dust3d dot org>. All rights reserved. 
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
#include <QtGlobal>
#include <QFile>
#include <QDebug>
#include <QtEndian>
#include <QByteArray>
#include <QOpenGLPixelTransferOptions>
#include "ddsfile.h"

#ifndef _WIN32
typedef quint32 DWORD;
typedef quint32 UINT;
#endif

// DDS data struct copy from
// https://docs.microsoft.com/en-us/windows/win32/direct3ddds/dx-graphics-dds-pguide

typedef struct {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwFourCC;
    DWORD dwRGBBitCount;
    DWORD dwRBitMask;
    DWORD dwGBitMask;
    DWORD dwBBitMask;
    DWORD dwABitMask;
} DDS_PIXELFORMAT;

typedef enum {
    DDSCAPS2_CUBEMAP = 0x200,
    DDSCAPS2_CUBEMAP_POSITIVEX = 0x400,
    DDSCAPS2_CUBEMAP_NEGATIVEX = 0x800,
    DDSCAPS2_CUBEMAP_POSITIVEY = 0x1000,
    DDSCAPS2_CUBEMAP_NEGATIVEY = 0x2000,
    DDSCAPS2_CUBEMAP_POSITIVEZ = 0x4000,
    DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x8000,
    DDSCAPS2_VOLUME = 0x200000
} DDS_CAPS2_FLAGS;

typedef struct {
    DWORD           dwSize;
    DWORD           dwFlags;
    DWORD           dwHeight;
    DWORD           dwWidth;
    DWORD           dwPitchOrLinearSize;
    DWORD           dwDepth;
    DWORD           dwMipMapCount;
    DWORD           dwReserved1[11];
    DDS_PIXELFORMAT ddspf;
    DWORD           dwCaps;
    DWORD           dwCaps2;
    DWORD           dwCaps3;
    DWORD           dwCaps4;
    DWORD           dwReserved2;
} DDS_HEADER;

typedef enum {
    DXGI_FORMAT_UNKNOWN,
    DXGI_FORMAT_R32G32B32A32_TYPELESS,
    DXGI_FORMAT_R32G32B32A32_FLOAT,
    DXGI_FORMAT_R32G32B32A32_UINT,
    DXGI_FORMAT_R32G32B32A32_SINT,
    DXGI_FORMAT_R32G32B32_TYPELESS,
    DXGI_FORMAT_R32G32B32_FLOAT,
    DXGI_FORMAT_R32G32B32_UINT,
    DXGI_FORMAT_R32G32B32_SINT,
    DXGI_FORMAT_R16G16B16A16_TYPELESS,
    DXGI_FORMAT_R16G16B16A16_FLOAT,
    DXGI_FORMAT_R16G16B16A16_UNORM,
    DXGI_FORMAT_R16G16B16A16_UINT,
    DXGI_FORMAT_R16G16B16A16_SNORM,
    DXGI_FORMAT_R16G16B16A16_SINT,
    DXGI_FORMAT_R32G32_TYPELESS,
    DXGI_FORMAT_R32G32_FLOAT,
    DXGI_FORMAT_R32G32_UINT,
    DXGI_FORMAT_R32G32_SINT,
    DXGI_FORMAT_R32G8X24_TYPELESS,
    DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
    DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS,
    DXGI_FORMAT_X32_TYPELESS_G8X24_UINT,
    DXGI_FORMAT_R10G10B10A2_TYPELESS,
    DXGI_FORMAT_R10G10B10A2_UNORM,
    DXGI_FORMAT_R10G10B10A2_UINT,
    DXGI_FORMAT_R11G11B10_FLOAT,
    DXGI_FORMAT_R8G8B8A8_TYPELESS,
    DXGI_FORMAT_R8G8B8A8_UNORM,
    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
    DXGI_FORMAT_R8G8B8A8_UINT,
    DXGI_FORMAT_R8G8B8A8_SNORM,
    DXGI_FORMAT_R8G8B8A8_SINT,
    DXGI_FORMAT_R16G16_TYPELESS,
    DXGI_FORMAT_R16G16_FLOAT,
    DXGI_FORMAT_R16G16_UNORM,
    DXGI_FORMAT_R16G16_UINT,
    DXGI_FORMAT_R16G16_SNORM,
    DXGI_FORMAT_R16G16_SINT,
    DXGI_FORMAT_R32_TYPELESS,
    DXGI_FORMAT_D32_FLOAT,
    DXGI_FORMAT_R32_FLOAT,
    DXGI_FORMAT_R32_UINT,
    DXGI_FORMAT_R32_SINT,
    DXGI_FORMAT_R24G8_TYPELESS,
    DXGI_FORMAT_D24_UNORM_S8_UINT,
    DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
    DXGI_FORMAT_X24_TYPELESS_G8_UINT,
    DXGI_FORMAT_R8G8_TYPELESS,
    DXGI_FORMAT_R8G8_UNORM,
    DXGI_FORMAT_R8G8_UINT,
    DXGI_FORMAT_R8G8_SNORM,
    DXGI_FORMAT_R8G8_SINT,
    DXGI_FORMAT_R16_TYPELESS,
    DXGI_FORMAT_R16_FLOAT,
    DXGI_FORMAT_D16_UNORM,
    DXGI_FORMAT_R16_UNORM,
    DXGI_FORMAT_R16_UINT,
    DXGI_FORMAT_R16_SNORM,
    DXGI_FORMAT_R16_SINT,
    DXGI_FORMAT_R8_TYPELESS,
    DXGI_FORMAT_R8_UNORM,
    DXGI_FORMAT_R8_UINT,
    DXGI_FORMAT_R8_SNORM,
    DXGI_FORMAT_R8_SINT,
    DXGI_FORMAT_A8_UNORM,
    DXGI_FORMAT_R1_UNORM,
    DXGI_FORMAT_R9G9B9E5_SHAREDEXP,
    DXGI_FORMAT_R8G8_B8G8_UNORM,
    DXGI_FORMAT_G8R8_G8B8_UNORM,
    DXGI_FORMAT_BC1_TYPELESS,
    DXGI_FORMAT_BC1_UNORM,
    DXGI_FORMAT_BC1_UNORM_SRGB,
    DXGI_FORMAT_BC2_TYPELESS,
    DXGI_FORMAT_BC2_UNORM,
    DXGI_FORMAT_BC2_UNORM_SRGB,
    DXGI_FORMAT_BC3_TYPELESS,
    DXGI_FORMAT_BC3_UNORM,
    DXGI_FORMAT_BC3_UNORM_SRGB,
    DXGI_FORMAT_BC4_TYPELESS,
    DXGI_FORMAT_BC4_UNORM,
    DXGI_FORMAT_BC4_SNORM,
    DXGI_FORMAT_BC5_TYPELESS,
    DXGI_FORMAT_BC5_UNORM,
    DXGI_FORMAT_BC5_SNORM,
    DXGI_FORMAT_B5G6R5_UNORM,
    DXGI_FORMAT_B5G5R5A1_UNORM,
    DXGI_FORMAT_B8G8R8A8_UNORM,
    DXGI_FORMAT_B8G8R8X8_UNORM,
    DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM,
    DXGI_FORMAT_B8G8R8A8_TYPELESS,
    DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
    DXGI_FORMAT_B8G8R8X8_TYPELESS,
    DXGI_FORMAT_B8G8R8X8_UNORM_SRGB,
    DXGI_FORMAT_BC6H_TYPELESS,
    DXGI_FORMAT_BC6H_UF16,
    DXGI_FORMAT_BC6H_SF16,
    DXGI_FORMAT_BC7_TYPELESS,
    DXGI_FORMAT_BC7_UNORM,
    DXGI_FORMAT_BC7_UNORM_SRGB,
    DXGI_FORMAT_AYUV,
    DXGI_FORMAT_Y410,
    DXGI_FORMAT_Y416,
    DXGI_FORMAT_NV12,
    DXGI_FORMAT_P010,
    DXGI_FORMAT_P016,
    DXGI_FORMAT_420_OPAQUE,
    DXGI_FORMAT_YUY2,
    DXGI_FORMAT_Y210,
    DXGI_FORMAT_Y216,
    DXGI_FORMAT_NV11,
    DXGI_FORMAT_AI44,
    DXGI_FORMAT_IA44,
    DXGI_FORMAT_P8,
    DXGI_FORMAT_A8P8,
    DXGI_FORMAT_B4G4R4A4_UNORM,
    DXGI_FORMAT_P208,
    DXGI_FORMAT_V208,
    DXGI_FORMAT_V408,
    DXGI_FORMAT_FORCE_UINT
} DXGI_FORMAT;

static const char *DxgiFormatToString(DXGI_FORMAT dxgiFormat)
{
    static const char *names[] = {
        "DXGI_FORMAT_UNKNOWN",
        "DXGI_FORMAT_R32G32B32A32_TYPELESS",
        "DXGI_FORMAT_R32G32B32A32_FLOAT",
        "DXGI_FORMAT_R32G32B32A32_UINT",
        "DXGI_FORMAT_R32G32B32A32_SINT",
        "DXGI_FORMAT_R32G32B32_TYPELESS",
        "DXGI_FORMAT_R32G32B32_FLOAT",
        "DXGI_FORMAT_R32G32B32_UINT",
        "DXGI_FORMAT_R32G32B32_SINT",
        "DXGI_FORMAT_R16G16B16A16_TYPELESS",
        "DXGI_FORMAT_R16G16B16A16_FLOAT",
        "DXGI_FORMAT_R16G16B16A16_UNORM",
        "DXGI_FORMAT_R16G16B16A16_UINT",
        "DXGI_FORMAT_R16G16B16A16_SNORM",
        "DXGI_FORMAT_R16G16B16A16_SINT",
        "DXGI_FORMAT_R32G32_TYPELESS",
        "DXGI_FORMAT_R32G32_FLOAT",
        "DXGI_FORMAT_R32G32_UINT",
        "DXGI_FORMAT_R32G32_SINT",
        "DXGI_FORMAT_R32G8X24_TYPELESS",
        "DXGI_FORMAT_D32_FLOAT_S8X24_UINT",
        "DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS",
        "DXGI_FORMAT_X32_TYPELESS_G8X24_UINT",
        "DXGI_FORMAT_R10G10B10A2_TYPELESS",
        "DXGI_FORMAT_R10G10B10A2_UNORM",
        "DXGI_FORMAT_R10G10B10A2_UINT",
        "DXGI_FORMAT_R11G11B10_FLOAT",
        "DXGI_FORMAT_R8G8B8A8_TYPELESS",
        "DXGI_FORMAT_R8G8B8A8_UNORM",
        "DXGI_FORMAT_R8G8B8A8_UNORM_SRGB",
        "DXGI_FORMAT_R8G8B8A8_UINT",
        "DXGI_FORMAT_R8G8B8A8_SNORM",
        "DXGI_FORMAT_R8G8B8A8_SINT",
        "DXGI_FORMAT_R16G16_TYPELESS",
        "DXGI_FORMAT_R16G16_FLOAT",
        "DXGI_FORMAT_R16G16_UNORM",
        "DXGI_FORMAT_R16G16_UINT",
        "DXGI_FORMAT_R16G16_SNORM",
        "DXGI_FORMAT_R16G16_SINT",
        "DXGI_FORMAT_R32_TYPELESS",
        "DXGI_FORMAT_D32_FLOAT",
        "DXGI_FORMAT_R32_FLOAT",
        "DXGI_FORMAT_R32_UINT",
        "DXGI_FORMAT_R32_SINT",
        "DXGI_FORMAT_R24G8_TYPELESS",
        "DXGI_FORMAT_D24_UNORM_S8_UINT",
        "DXGI_FORMAT_R24_UNORM_X8_TYPELESS",
        "DXGI_FORMAT_X24_TYPELESS_G8_UINT",
        "DXGI_FORMAT_R8G8_TYPELESS",
        "DXGI_FORMAT_R8G8_UNORM",
        "DXGI_FORMAT_R8G8_UINT",
        "DXGI_FORMAT_R8G8_SNORM",
        "DXGI_FORMAT_R8G8_SINT",
        "DXGI_FORMAT_R16_TYPELESS",
        "DXGI_FORMAT_R16_FLOAT",
        "DXGI_FORMAT_D16_UNORM",
        "DXGI_FORMAT_R16_UNORM",
        "DXGI_FORMAT_R16_UINT",
        "DXGI_FORMAT_R16_SNORM",
        "DXGI_FORMAT_R16_SINT",
        "DXGI_FORMAT_R8_TYPELESS",
        "DXGI_FORMAT_R8_UNORM",
        "DXGI_FORMAT_R8_UINT",
        "DXGI_FORMAT_R8_SNORM",
        "DXGI_FORMAT_R8_SINT",
        "DXGI_FORMAT_A8_UNORM",
        "DXGI_FORMAT_R1_UNORM",
        "DXGI_FORMAT_R9G9B9E5_SHAREDEXP",
        "DXGI_FORMAT_R8G8_B8G8_UNORM",
        "DXGI_FORMAT_G8R8_G8B8_UNORM",
        "DXGI_FORMAT_BC1_TYPELESS",
        "DXGI_FORMAT_BC1_UNORM",
        "DXGI_FORMAT_BC1_UNORM_SRGB",
        "DXGI_FORMAT_BC2_TYPELESS",
        "DXGI_FORMAT_BC2_UNORM",
        "DXGI_FORMAT_BC2_UNORM_SRGB",
        "DXGI_FORMAT_BC3_TYPELESS",
        "DXGI_FORMAT_BC3_UNORM",
        "DXGI_FORMAT_BC3_UNORM_SRGB",
        "DXGI_FORMAT_BC4_TYPELESS",
        "DXGI_FORMAT_BC4_UNORM",
        "DXGI_FORMAT_BC4_SNORM",
        "DXGI_FORMAT_BC5_TYPELESS",
        "DXGI_FORMAT_BC5_UNORM",
        "DXGI_FORMAT_BC5_SNORM",
        "DXGI_FORMAT_B5G6R5_UNORM",
        "DXGI_FORMAT_B5G5R5A1_UNORM",
        "DXGI_FORMAT_B8G8R8A8_UNORM",
        "DXGI_FORMAT_B8G8R8X8_UNORM",
        "DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM",
        "DXGI_FORMAT_B8G8R8A8_TYPELESS",
        "DXGI_FORMAT_B8G8R8A8_UNORM_SRGB",
        "DXGI_FORMAT_B8G8R8X8_TYPELESS",
        "DXGI_FORMAT_B8G8R8X8_UNORM_SRGB",
        "DXGI_FORMAT_BC6H_TYPELESS",
        "DXGI_FORMAT_BC6H_UF16",
        "DXGI_FORMAT_BC6H_SF16",
        "DXGI_FORMAT_BC7_TYPELESS",
        "DXGI_FORMAT_BC7_UNORM",
        "DXGI_FORMAT_BC7_UNORM_SRGB",
        "DXGI_FORMAT_AYUV",
        "DXGI_FORMAT_Y410",
        "DXGI_FORMAT_Y416",
        "DXGI_FORMAT_NV12",
        "DXGI_FORMAT_P010",
        "DXGI_FORMAT_P016",
        "DXGI_FORMAT_420_OPAQUE",
        "DXGI_FORMAT_YUY2",
        "DXGI_FORMAT_Y210",
        "DXGI_FORMAT_Y216",
        "DXGI_FORMAT_NV11",
        "DXGI_FORMAT_AI44",
        "DXGI_FORMAT_IA44",
        "DXGI_FORMAT_P8",
        "DXGI_FORMAT_A8P8",
        "DXGI_FORMAT_B4G4R4A4_UNORM",
        "DXGI_FORMAT_P208",
        "DXGI_FORMAT_V208",
        "DXGI_FORMAT_V408",
        "DXGI_FORMAT_FORCE_UINT",
    };
    int index = (int)dxgiFormat;
    if (index >= 0 && index < sizeof(names) / sizeof(names[0]))
        return names[index];
    return "(Unknown)";
}

typedef enum {
    D3D10_RESOURCE_DIMENSION_UNKNOWN,
    D3D10_RESOURCE_DIMENSION_BUFFER,
    D3D10_RESOURCE_DIMENSION_TEXTURE1D,
    D3D10_RESOURCE_DIMENSION_TEXTURE2D,
    D3D10_RESOURCE_DIMENSION_TEXTURE3D
} D3D10_RESOURCE_DIMENSION;

static const char *ResourceDimensionToString(D3D10_RESOURCE_DIMENSION resourceDimension)
{
    static const char *names[] = {
        "D3D10_RESOURCE_DIMENSION_UNKNOWN",
        "D3D10_RESOURCE_DIMENSION_BUFFER",
        "D3D10_RESOURCE_DIMENSION_TEXTURE1D",
        "D3D10_RESOURCE_DIMENSION_TEXTURE2D",
        "D3D10_RESOURCE_DIMENSION_TEXTURE3D",
    };
    int index = (int)resourceDimension;
    if (index >= 0 && index < sizeof(names) / sizeof(names[0]))
        return names[index];
    return "(Unknown)";
}

typedef enum {
    D3D10_RESOURCE_MISC_GENERATE_MIPS,
    D3D10_RESOURCE_MISC_SHARED,
    D3D10_RESOURCE_MISC_TEXTURECUBE,
    D3D10_RESOURCE_MISC_SHARED_KEYEDMUTEX,
    D3D10_RESOURCE_MISC_GDI_COMPATIBLE
} D3D10_RESOURCE_MISC_FLAG;

static const char *MiscFlagToString(UINT miscFlag)
{
    static const char *names[] = {
        "D3D10_RESOURCE_MISC_GENERATE_MIPS",
        "D3D10_RESOURCE_MISC_SHARED",
        "D3D10_RESOURCE_MISC_TEXTURECUBE",
        "D3D10_RESOURCE_MISC_SHARED_KEYEDMUTEX",
        "D3D10_RESOURCE_MISC_GDI_COMPATIBLE",
    };
    int index = (int)miscFlag;
    if (index >= 0 && index < sizeof(names) / sizeof(names[0]))
        return names[index];
    return "(Unknown)";
}

typedef struct {
    DXGI_FORMAT              dxgiFormat;
    D3D10_RESOURCE_DIMENSION resourceDimension;
    UINT                     miscFlag;
    UINT                     arraySize;
    UINT                     miscFlags2;
} DDS_HEADER_DXT10;

typedef struct {
    DWORD               dwMagic;
    DDS_HEADER          header;
    DDS_HEADER_DXT10    header10;
} DDS_FILE_HEADER;

DdsFileReader::DdsFileReader(const QString &filename) :
    m_filename(filename)
{     
}

QOpenGLTexture *DdsFileReader::createOpenGLTexture()
{
    QFile file(m_filename);
    
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Open" << m_filename << "failed";
        return nullptr;
    }
    
    DDS_FILE_HEADER fileHeader;
    if (sizeof(fileHeader) != file.read((char *)&fileHeader, sizeof(fileHeader))) {
        qDebug() << "Read DDS file hader failed";
        return nullptr;
    }
    
    if (0x20534444 != qFromLittleEndian<quint32>(&fileHeader.dwMagic)) {
        qDebug() << "Not a DDS file";
        return nullptr;
    }
    
    if (0x30315844 != qFromLittleEndian<quint32>(&fileHeader.header.ddspf.dwFourCC)) {
        qDebug() << "Unsupported DDS file, expected DX10 file";
        return nullptr;
    }
    
    auto caps2 = qFromLittleEndian<quint32>(&fileHeader.header.dwCaps2);
    if (!(DDSCAPS2_CUBEMAP & caps2)) {
        qDebug() << "Unsupported DDS file, expected CUBEMAP file";
        return nullptr;
    }
    
    int width = qFromLittleEndian<quint32>(&fileHeader.header.dwWidth);
    int height = qFromLittleEndian<quint32>(&fileHeader.header.dwHeight);
    
    auto arraySize = qFromLittleEndian<quint32>(&fileHeader.header10.arraySize);
    
    auto mipMapCount = qFromLittleEndian<quint32>(&fileHeader.header.dwMipMapCount);
    
    DXGI_FORMAT dxgiFormat = (DXGI_FORMAT)qFromLittleEndian<quint32>(&fileHeader.header10.dxgiFormat);
    
    auto faces = 0;
    if (DDSCAPS2_CUBEMAP_POSITIVEX & caps2) {
        ++faces;
    }
    if (DDSCAPS2_CUBEMAP_NEGATIVEX & caps2) {
        ++faces;
    }
    if (DDSCAPS2_CUBEMAP_POSITIVEY & caps2) {
        ++faces;
    }
    if (DDSCAPS2_CUBEMAP_NEGATIVEY & caps2) {
        ++faces;
    }
    if (DDSCAPS2_CUBEMAP_POSITIVEZ & caps2) {
        ++faces;
    }
    if (DDSCAPS2_CUBEMAP_NEGATIVEZ & caps2) {
        ++faces;
    }
    
    if (6 != faces) {
        qDebug() << "Unsupported DDS file, expected six faces";
        return nullptr;
    }
    
    if (1 != arraySize) {
        qDebug() << "Unsupported DDS file, expected one layer";
        return nullptr;
    }
    
    if (DXGI_FORMAT_R16G16B16A16_FLOAT != dxgiFormat) {
        qDebug() << "Unsupported DDS file, expected dxgi format: DXGI_FORMAT_R16G16B16A16_FLOAT";
        return nullptr; 
    }
    int components = 8;
    int oneFaceSize = 0;
    auto calculateOneFaceSizeAtLevel = [=](int level) {
        return qMax(width >> level, 1) * qMax(height >> level, 1) * components;
    };
    for (quint32 level = 0; level < mipMapCount; ++level) {
        oneFaceSize += calculateOneFaceSizeAtLevel(level);
    }
    int totalSize = arraySize * faces * oneFaceSize;
    const QByteArray data = file.read(totalSize);
    if (data.size() < totalSize) {
        qDebug() << "DDS file invalid, expected total size:" << totalSize << "read size:" << data.size();
        return nullptr;
    }
    
    int depth = 1;
    
    QOpenGLTexture* texture = new QOpenGLTexture(QOpenGLTexture::TargetCubeMap);
    texture->setFormat(QOpenGLTexture::RGBA16F);
    texture->setSize(width, height, depth);
    texture->setAutoMipMapGenerationEnabled(false);
    texture->setMipBaseLevel(0);
    texture->setMipMaxLevel(mipMapCount - 1);
    texture->setMipLevels(mipMapCount);
    
    if (!texture->create()) {
        qDebug() << "QOpenGLTexture::create failed";
        delete texture;
        return nullptr;
    }
    
    texture->allocateStorage();
    if (!texture->isStorageAllocated()) {
        qDebug() << "QOpenGLTexture::isStorageAllocated false";
        delete texture;
        return nullptr;
    }
    
    uint64_t dataOffset = 0;
    for (quint32 layer = 0; layer < arraySize; ++layer) {
        for (int face = 0; face < faces; ++face) {
            for (quint32 level = 0; level < mipMapCount; ++level) {
                QOpenGLPixelTransferOptions uploadOptions;
                uploadOptions.setAlignment(1);
                texture->setData(level, 
                    layer, 
                    static_cast<QOpenGLTexture::CubeMapFace>(QOpenGLTexture::CubeMapPositiveX + face), 
                    QOpenGLTexture::RGBA, 
                    QOpenGLTexture::Float16, 
                    data.constData() + dataOffset, 
                    &uploadOptions);
                dataOffset += calculateOneFaceSizeAtLevel(level);
            }
        }
    }
    
    return texture;
}