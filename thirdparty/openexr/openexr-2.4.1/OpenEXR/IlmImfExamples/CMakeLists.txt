# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the OpenEXR Project.

add_executable(IlmImfExamples
  drawImage.cpp
  generalInterfaceExamples.cpp
  generalInterfaceTiledExamples.cpp
  lowLevelIoExamples.cpp
  main.cpp
  previewImageExamples.cpp
  rgbaInterfaceExamples.cpp
  rgbaInterfaceTiledExamples.cpp
)
target_link_libraries(IlmImfExamples OpenEXR::IlmImf)

# Examples
install(
  FILES
    main.cpp
    drawImage.cpp
    rgbaInterfaceExamples.cpp
    rgbaInterfaceTiledExamples.cpp
    generalInterfaceExamples.cpp
    lowLevelIoExamples.cpp
    previewImageExamples.cpp
    generalInterfaceTiledExamples.cpp
    generalInterfaceTiledExamples.h
    drawImage.h
    rgbaInterfaceExamples.h
    generalInterfaceExamples.h
    rgbaInterfaceTiledExamples.h
    lowLevelIoExamples.h
    previewImageExamples.h
    namespaceAlias.h
  DESTINATION
    ${CMAKE_INSTALL_DOCDIR}/examples
  )
