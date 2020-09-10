##
## SPDX-License-Identifier: BSD-3-Clause
## Copyright Contributors to the OpenEXR Project.
##

## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libIlmImf.la

libIlmImf_la_SOURCES = ImfForward.h ImfAttribute.cpp ImfBoxAttribute.cpp ImfCRgbaFile.cpp \
		       ImfChannelList.cpp ImfChannelListAttribute.cpp \
		       ImfFloatAttribute.cpp ImfFrameBuffer.cpp \
		       ImfHeader.cpp ImfIO.cpp ImfInputFile.cpp \
		       ImfIntAttribute.cpp ImfLineOrderAttribute.cpp \
		       ImfMatrixAttribute.cpp ImfOpaqueAttribute.cpp \
		       ImfOutputFile.cpp ImfRgbaFile.cpp \
		       ImfStringAttribute.cpp ImfVecAttribute.cpp ImfHuf.cpp \
		       ImfThreading.cpp \
		       ImfWav.cpp ImfLut.cpp ImfCompressor.cpp \
		       ImfRleCompressor.cpp ImfZipCompressor.cpp \
		       ImfPizCompressor.cpp ImfB44Compressor.cpp \
		       ImfDwaCompressor.cpp ImfMisc.cpp \
		       ImfCompressionAttribute.cpp ImfDoubleAttribute.cpp \
		       ImfAttribute.h ImfBoxAttribute.h \
		       ImfCRgbaFile.h ImfChannelList.h \
		       ImfChannelListAttribute.h \
		       ImfCompressionAttribute.h \
		       ImfDoubleAttribute.h ImfFloatAttribute.h \
		       ImfFrameBuffer.h ImfHeader.h ImfIO.h \
		       ImfInputFile.h ImfIntAttribute.h \
		       ImfLineOrderAttribute.h ImfMatrixAttribute.h \
		       ImfOpaqueAttribute.h ImfOutputFile.h \
		       ImfRgbaFile.h ImfStringAttribute.h \
		       ImfVecAttribute.h ImfHuf.h ImfWav.h ImfLut.h \
		       ImfArray.h ImfCompression.h ImfLineOrder.h \
		       ImfName.h ImfPixelType.h ImfVersion.h ImfXdr.h \
		       ImfCompressor.h ImfRleCompressor.h ImfZipCompressor.h \
		       ImfPizCompressor.h ImfDwaCompressor.h \
	               ImfDwaCompressorSimd.h ImfAutoArray.h \
		       ImfConvert.cpp ImfConvert.h ImfPreviewImage.cpp \
		       ImfPreviewImage.h ImfPreviewImageAttribute.cpp \
		       ImfPreviewImageAttribute.h ImfVersion.cpp \
		       ImfChromaticities.cpp ImfChromaticities.h \
		       ImfChromaticitiesAttribute.cpp \
		       ImfChromaticitiesAttribute.h \
		       ImfKeyCode.cpp ImfKeyCode.h \
		       ImfKeyCodeAttribute.cpp ImfKeyCodeAttribute.h \
		       ImfTimeCode.cpp ImfTimeCode.h \
		       ImfTimeCodeAttribute.cpp ImfTimeCodeAttribute.h \
		       ImfRational.cpp ImfRational.h \
		       ImfRationalAttribute.cpp ImfRationalAttribute.h \
		       ImfFramesPerSecond.cpp ImfFramesPerSecond.h \
		       ImfStandardAttributes.cpp ImfStandardAttributes.h \
		       ImfStdIO.cpp ImfStdIO.h ImfEnvmap.cpp ImfEnvmap.h \
		       ImfEnvmapAttribute.cpp ImfEnvmapAttribute.h \
		       ImfInt64.h ImfRgba.h ImfScanLineInputFile.cpp \
		       ImfScanLineInputFile.h ImfTiledInputFile.cpp \
		       ImfTiledMisc.cpp ImfTiledOutputFile.cpp \
		       ImfTiledRgbaFile.cpp ImfTileDescriptionAttribute.cpp \
		       ImfTileOffsets.cpp ImfTileDescription.h \
		       ImfTileDescriptionAttribute.h ImfTileOffsets.h \
		       ImfTiledInputFile.h ImfTiledMisc.h \
		       ImfTiledOutputFile.h ImfTiledRgbaFile.h \
		       ImfRgbaYca.cpp ImfRgbaYca.h \
		       ImfPxr24Compressor.cpp ImfPxr24Compressor.h \
		       ImfTestFile.cpp ImfTestFile.h ImfThreading.h \
		       ImfStringVectorAttribute.cpp ImfStringVectorAttribute.h \
		       ImfMultiView.cpp ImfMultiView.h \
		       ImfAcesFile.cpp ImfAcesFile.h \
		       ImfCheckedArithmetic.h \
		       ImfMultiPartOutputFile.cpp ImfMultiPartOutputFile.h \
		       ImfGenericOutputFile.cpp ImfGenericOutputFile.h \
		       ImfOutputStreamMutex.h ImfInputStreamMutex.h\
		       ImfOutputPartData.h ImfOutputPartData.cpp \
		       ImfMultiPartInputFile.cpp ImfMultiPartInputFile.h \
		       ImfGenericInputFile.cpp ImfGenericInputFile.h \
		       ImfPartType.cpp ImfPartType.h \
		       ImfInputPartData.cpp ImfInputPartData.h \
		       ImfOutputPart.cpp ImfOutputPart.h \
		       ImfTiledOutputPart.cpp ImfTiledOutputPart.h \
		       ImfInputPart.cpp ImfInputPart.h \
		       ImfTiledInputPart.cpp ImfTiledInputPart.h \
		       ImfDeepScanLineInputPart.h ImfDeepScanLineInputPart.cpp \
		       ImfDeepScanLineOutputPart.cpp ImfDeepScanLineOutputPart.h \
		       ImfDeepScanLineInputFile.cpp ImfDeepScanLineInputFile.h \
		       ImfDeepScanLineOutputFile.cpp ImfDeepScanLineOutputFile.h \
		       ImfDeepTiledInputPart.h ImfDeepTiledInputPart.cpp \
		       ImfDeepTiledOutputPart.h ImfDeepTiledOutputPart.cpp \
		       ImfDeepTiledInputFile.h ImfDeepTiledInputFile.cpp \
		       ImfDeepTiledOutputFile.h ImfDeepTiledOutputFile.cpp \
		       ImfDeepFrameBuffer.cpp ImfDeepFrameBuffer.h \
		       ImfDeepCompositing.h ImfDeepCompositing.cpp \
		       ImfCompositeDeepScanLine.h ImfCompositeDeepScanLine.cpp \
		       ImfDeepImageStateAttribute.h ImfDeepImageStateAttribute.cpp \
	               ImfFastHuf.h ImfFastHuf.cpp \
	               ImfFloatVectorAttribute.h ImfFloatVectorAttribute.cpp \
	               ImfRle.h ImfRle.cpp ImfSimd.h \
	               ImfSystemSpecific.cpp ImfZip.h ImfZip.cpp


libIlmImf_la_LDFLAGS = @ILMBASE_LDFLAGS@ -version-info @LIBTOOL_VERSION@ \
			-no-undefined 


if LIB_SUFFIX_EXISTS
libIlmImf_la_LDFLAGS += -release @LIB_SUFFIX@
endif


libIlmImf_la_LIBADD = $(ZLIB_LIBS) $(ILMBASE_LIBS)

libIlmImfincludedir = $(includedir)/OpenEXR

libIlmImfinclude_HEADERS = ImfForward.h ImfAttribute.h ImfBoxAttribute.h \
			   ImfCRgbaFile.h ImfChannelList.h \
			   ImfChannelListAttribute.h \
			   ImfCompressionAttribute.h \
			   ImfDoubleAttribute.h ImfFloatAttribute.h ImfFloatVectorAttribute.h \
			   ImfFrameBuffer.h ImfHeader.h ImfIO.h \
			   ImfInputFile.h ImfIntAttribute.h \
			   ImfLineOrderAttribute.h ImfMatrixAttribute.h \
			   ImfOpaqueAttribute.h ImfOutputFile.h \
			   ImfRgbaFile.h ImfStringAttribute.h \
			   ImfVecAttribute.h ImfHuf.h ImfWav.h ImfLut.h \
			   ImfArray.h ImfCompression.h ImfLineOrder.h \
			   ImfName.h ImfPixelType.h ImfVersion.h ImfXdr.h \
			   ImfConvert.h ImfPreviewImage.h \
			   ImfPreviewImageAttribute.h ImfChromaticities.h \
			   ImfChromaticitiesAttribute.h \
			   ImfKeyCode.h ImfKeyCodeAttribute.h \
			   ImfTimeCode.h ImfTimeCodeAttribute.h \
			   ImfRational.h ImfRationalAttribute.h \
			   ImfFramesPerSecond.h \
			   ImfStandardAttributes.h \
			   ImfStdIO.h \
			   ImfEnvmap.h \
			   ImfEnvmapAttribute.h \
			   ImfInt64.h ImfRgba.h \
			   ImfTileDescription.h \
			   ImfTileDescriptionAttribute.h \
			   ImfTiledInputFile.h \
			   ImfTiledOutputFile.h ImfTiledRgbaFile.h \
			   ImfRgbaYca.h \
			   ImfTestFile.h ImfThreading.h \
			   ImfB44Compressor.h ImfStringVectorAttribute.h \
			   ImfMultiView.h \
			   ImfAcesFile.h \
			   ImfMultiPartOutputFile.h \
			   ImfGenericOutputFile.h \
			   ImfMultiPartInputFile.h \
			   ImfGenericInputFile.h \
			   ImfPartType.h \
			   ImfOutputPart.h \
			   ImfTiledOutputPart.h \
			   ImfInputPart.h \
			   ImfTiledInputPart.h \
			   ImfDeepScanLineOutputFile.h \
			   ImfDeepScanLineOutputPart.h \
			   ImfDeepScanLineInputFile.h \
			   ImfDeepScanLineInputPart.h \
			   ImfDeepTiledInputFile.h \
			   ImfDeepTiledInputPart.h \
			   ImfDeepTiledOutputFile.h \
			   ImfDeepTiledOutputPart.h \
			   ImfDeepFrameBuffer.h \
			   ImfDeepCompositing.h ImfCompositeDeepScanLine.h \
			   ImfNamespace.h ImfForward.h ImfExport.h \
			   ImfPartHelper.h \
			   ImfDeepImageState.h \
			   ImfDeepImageStateAttribute.h

noinst_HEADERS = ImfCompressor.h    \
		 ImfRleCompressor.h \
		 ImfZipCompressor.h \
		 ImfPizCompressor.h \
		 ImfDwaCompressor.h \
		 ImfDwaCompressorSimd.h \
		 ImfAutoArray.h     \
		 ImfTiledMisc.h     \
		 ImfTileOffsets.h   \
		 ImfScanLineInputFile.h \
		 ImfPxr24Compressor.h   \
		 ImfCheckedArithmetic.h \
		 ImfInputStreamMutex.h  \
		 ImfOutputStreamMutex.h \
		 ImfInputPartData.h     \
		 ImfOutputPartData.h    \
		 ImfScanLineInputFile.h \
		 ImfSystemSpecific.h    \
		 ImfOptimizedPixelReading.h


EXTRA_DIST = $(noinst_HEADERS) b44ExpLogTable.cpp b44ExpLogTable.h dwaLookups.cpp dwaLookups.h CMakeLists.txt


AM_CPPFLAGS = @ILMBASE_CXXFLAGS@ \
	   -I$(top_builddir) \
	   -I$(top_srcdir)/config \
	   $(ILMBASE_CFLAGS) $(ZLIB_CFLAGS)

CLEANFILES = b44ExpLogTable b44ExpLogTable.h dwaLookups dwaLookups.h

b44ExpLogTable_SOURCES = b44ExpLogTable.cpp
b44ExpLogTable_LDADD = @ILMBASE_LDFLAGS@ $(ILMBASE_LIBS)

b44ExpLogTable.h: b44ExpLogTable
	./b44ExpLogTable > b44ExpLogTable.h

dwaLookups_SOURCES = dwaLookups.cpp
dwaLookups_LDADD = @ILMBASE_LDFLAGS@ $(ILMBASE_LIBS)

dwaLookups.h: dwaLookups
	./dwaLookups > dwaLookups.h

BUILT_SOURCES = b44ExpLogTable.h dwaLookups.h

noinst_PROGRAMS = b44ExpLogTable dwaLookups
