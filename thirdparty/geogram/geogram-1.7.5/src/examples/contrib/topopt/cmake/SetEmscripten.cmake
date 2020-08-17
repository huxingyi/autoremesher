find_path(EMSCRIPTEN_DIR
      emcc
      HINTS
        ENV EMSCRIPTEN
      PATHS
        "C:/Program Files/emscripten"
         /usr/lib/emscripten
)

set(CMAKE_C_COMPILER "${EMSCRIPTEN_DIR}/emcc")
set(CMAKE_CXX_COMPILER "${EMSCRIPTEN_DIR}/em++")
set(CMAKE_AR "${EMSCRIPTEN_DIR}/emar")
set(CMAKE_RANLIB "${EMSCRIPTEN_DIR}/emranlib")
set(CMAKE_LINKER "${EMSCRIPTEN_DIR}/emcc")

include(${EMSCRIPTEN_DIR}/cmake/Modules/Platform/Emscripten.cmake)

# ideally one would use the following, but cmake removes duplicated flags such as "-s":
# add_library(emscripten INTERFACE)
# target_compile_options(emscripten INTERFACE -O3 -s USE_GLFW=3 -s TOTAL_MEMORY=256000000)

# -s USE_PTHREADS=2
set(CMAKE_C_FLAGS "-m64 -O3 -s USE_GLFW=3 -s TOTAL_MEMORY=256000000" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "-m64 -O3 -s USE_GLFW=3 -s TOTAL_MEMORY=256000000" CACHE STRING "" FORCE)
