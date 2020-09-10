# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the OpenEXR Project.

add_executable(b44ExpLogTable b44ExpLogTable.cpp)
target_link_libraries(b44ExpLogTable PRIVATE OpenEXR::Config IlmBase::Half IlmBase::IlmThread IlmBase::Iex)
set_target_properties(b44ExpLogTable PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
# TODO: Old file had logic to skip these if the file already exists
add_custom_command(
  OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/b44ExpLogTable.h
  COMMAND $<TARGET_FILE:b44ExpLogTable> > ${CMAKE_CURRENT_BINARY_DIR}/b44ExpLogTable.h
  DEPENDS b44ExpLogTable
)


add_executable(dwaLookups dwaLookups.cpp)
target_link_libraries(dwaLookups PRIVATE OpenEXR::Config IlmBase::Imath IlmBase::Half IlmBase::IlmThread IlmBase::Iex)
set_target_properties(dwaLookups PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
# TODO: Old file had logic to skip these if the file already exists
add_custom_command(
  OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/dwaLookups.h
  COMMAND $<TARGET_FILE:dwaLookups> > ${CMAKE_CURRENT_BINARY_DIR}/dwaLookups.h
  DEPENDS dwaLookups
)

openexr_define_library(IlmImf
  PRIV_EXPORT ILMIMF_EXPORTS
  CURDIR ${CMAKE_CURRENT_SOURCE_DIR}
  SOURCES
    ${CMAKE_CURRENT_BINARY_DIR}/b44ExpLogTable.h
    ${CMAKE_CURRENT_BINARY_DIR}/dwaLookups.h
    ImfAttribute.cpp
    ImfBoxAttribute.cpp
    ImfCRgbaFile.cpp
    ImfChannelList.cpp
    ImfChannelListAttribute.cpp
    ImfFloatAttribute.cpp
    ImfFrameBuffer.cpp
    ImfHeader.cpp
    ImfIO.cpp
    ImfInputFile.cpp
    ImfIntAttribute.cpp
    ImfLineOrderAttribute.cpp
    ImfMatrixAttribute.cpp
    ImfOpaqueAttribute.cpp
    ImfOutputFile.cpp
    ImfRgbaFile.cpp
    ImfStringAttribute.cpp
    ImfVecAttribute.cpp
    ImfHuf.cpp
    ImfThreading.cpp
    ImfWav.cpp
    ImfLut.cpp
    ImfCompressor.cpp
    ImfRleCompressor.cpp
    ImfZipCompressor.cpp
    ImfPizCompressor.cpp
    ImfB44Compressor.cpp
    ImfDwaCompressor.cpp
    ImfMisc.cpp
    ImfCompressionAttribute.cpp
    ImfDoubleAttribute.cpp
    ImfConvert.cpp
    ImfPreviewImage.cpp
    ImfPreviewImageAttribute.cpp
    ImfVersion.cpp
    ImfChromaticities.cpp
    ImfChromaticitiesAttribute.cpp
    ImfKeyCode.cpp
    ImfKeyCodeAttribute.cpp
    ImfTimeCode.cpp
    ImfTimeCodeAttribute.cpp
    ImfRational.cpp
    ImfRationalAttribute.cpp
    ImfFramesPerSecond.cpp
    ImfStandardAttributes.cpp
    ImfStdIO.cpp
    ImfEnvmap.cpp
    ImfEnvmapAttribute.cpp
    ImfScanLineInputFile.cpp
    ImfTiledInputFile.cpp
    ImfTiledMisc.cpp
    ImfTiledOutputFile.cpp
    ImfTiledRgbaFile.cpp
    ImfTileDescriptionAttribute.cpp
    ImfTileOffsets.cpp
    ImfRgbaYca.cpp
    ImfPxr24Compressor.cpp
    ImfTestFile.cpp
    ImfStringVectorAttribute.cpp
    ImfMultiView.cpp
    ImfAcesFile.cpp
    ImfMultiPartOutputFile.cpp
    ImfGenericOutputFile.cpp
    ImfOutputPartData.cpp
    ImfMultiPartInputFile.cpp
    ImfGenericInputFile.cpp
    ImfPartType.cpp
    ImfInputPartData.cpp
    ImfOutputPart.cpp
    ImfTiledOutputPart.cpp
    ImfInputPart.cpp
    ImfTiledInputPart.cpp
    ImfDeepScanLineInputPart.cpp
    ImfDeepScanLineOutputPart.cpp
    ImfDeepScanLineInputFile.cpp
    ImfDeepScanLineOutputFile.cpp
    ImfDeepTiledInputPart.cpp
    ImfDeepTiledOutputPart.cpp
    ImfDeepTiledInputFile.cpp
    ImfDeepTiledOutputFile.cpp
    ImfDeepFrameBuffer.cpp
    ImfDeepCompositing.cpp
    ImfCompositeDeepScanLine.cpp
    ImfDeepImageStateAttribute.cpp
    ImfFastHuf.cpp
    ImfFloatVectorAttribute.cpp
    ImfRle.cpp
    ImfSystemSpecific.cpp
    ImfZip.cpp
  HEADERS
    ImfForward.h
    ImfExport.h
    ImfAttribute.h
    ImfBoxAttribute.h
    ImfCRgbaFile.h
    ImfChannelList.h
    ImfChannelListAttribute.h
    ImfCompressionAttribute.h
    ImfDoubleAttribute.h
    ImfFloatAttribute.h
    ImfFrameBuffer.h
    ImfHeader.h
    ImfIO.h
    ImfInputFile.h
    ImfIntAttribute.h
    ImfLineOrderAttribute.h
    ImfMatrixAttribute.h
    ImfOpaqueAttribute.h
    ImfOutputFile.h
    ImfRgbaFile.h
    ImfStringAttribute.h
    ImfVecAttribute.h
    ImfHuf.h
    ImfWav.h
    ImfLut.h
    ImfArray.h
    ImfCompression.h
    ImfLineOrder.h
    ImfName.h
    ImfPixelType.h
    ImfVersion.h
    ImfXdr.h
    ImfConvert.h
    ImfPreviewImage.h
    ImfPreviewImageAttribute.h
    ImfChromaticities.h
    ImfChromaticitiesAttribute.h
    ImfKeyCode.h
    ImfKeyCodeAttribute.h
    ImfTimeCode.h
    ImfTimeCodeAttribute.h
    ImfRational.h
    ImfRationalAttribute.h
    ImfFramesPerSecond.h
    ImfStandardAttributes.h
    ImfStdIO.h
    ImfEnvmap.h
    ImfEnvmapAttribute.h
    ImfInt64.h
    ImfRgba.h
    ImfTileDescription.h
    ImfTileDescriptionAttribute.h
    ImfTiledInputFile.h
    ImfTiledOutputFile.h
    ImfTiledRgbaFile.h
    ImfRgbaYca.h
    ImfTestFile.h
    ImfThreading.h
    ImfB44Compressor.h
    ImfStringVectorAttribute.h
    ImfMultiView.h
    ImfAcesFile.h
    ImfMultiPartOutputFile.h
    ImfGenericOutputFile.h
    ImfMultiPartInputFile.h
    ImfGenericInputFile.h
    ImfPartType.h
    ImfPartHelper.h
    ImfOutputPart.h
    ImfTiledOutputPart.h
    ImfInputPart.h
    ImfTiledInputPart.h
    ImfDeepScanLineOutputFile.h
    ImfDeepScanLineOutputPart.h
    ImfDeepScanLineInputFile.h
    ImfDeepScanLineInputPart.h
    ImfDeepTiledInputFile.h
    ImfDeepTiledInputPart.h
    ImfDeepTiledOutputFile.h
    ImfDeepTiledOutputPart.h
    ImfDeepFrameBuffer.h
    ImfDeepCompositing.h
    ImfCompositeDeepScanLine.h
    ImfNamespace.h
    ImfDeepImageState.h
    ImfDeepImageStateAttribute.h
    ImfFloatVectorAttribute.h
  DEPENDENCIES
    OpenEXR::Config
    IlmBase::Iex
    IlmBase::Half
    IlmBase::Imath
    IlmBase::IlmThread
    ZLIB::ZLIB
  )
