#!/bin/sh

headers_GLUP="
GLUP/constants.h
GLUP/defs.h
GLUP/stdglup.h
GLUP/fragment_shader_utils.h
GLUP/fragment_ray_tracing.h
GLUP/portable_fragment_shader.h
GLUP/ShaderToy.h
GLUPES/fragment_shader_utils.h
GLUPES/vertex_shader_state.h
GLUPES/fragment_shader_state.h
GLUPES/points_vertex_shader.h
GLUPES/points_fragment_shader.h
GLUPES/lines_fragment_shader.h
GLUPES/vertex_shader.h
GLUPES/fragment_shader.h
GLUPES/spheres_vertex_shader.h
GLUPES/spheres_fragment_shader.h
GLUPGLSL/state.h
GLUPGLSL/points_vertex_shader.h
GLUPGLSL/points_fragment_shader.h
GLUPGLSL/lines_fragment_shader.h
GLUPGLSL/vertex_shader.h
GLUPGLSL/gather_vertex_shader.h
GLUPGLSL/fragment_shader.h
GLUPGLSL/geometry_shader_preamble.h
GLUPGLSL/tess_evaluation_shader.h
GLUPGLSL/marching_cells.h
GLUPGLSL/spheres_vertex_shader.h
GLUPGLSL/spheres_fragment_shader.h
"

headers_fullscreen="
fullscreen/ambient_occlusion_fragment_shader.h
fullscreen/blur_fragment_shader.h
fullscreen/depth_dependent_blur_fragment_shader.h
fullscreen/unsharp_masking_fragment_shader.h
fullscreen/vertex_shader.h
"

cat <<EOF
/*
 * This file was automatically generated, do not edit.
 */

#include <geogram_gfx/GLUP/shaders/embedded_shaders.h>
#include <geogram_gfx/basic/common.h>
#include <geogram_gfx/basic/GLSL.h>

namespace GLUP {
   void register_embedded_shaders_GLUP() {
EOF
for header in $headers_GLUP
do
    echo "     GEO::GLSL::register_GLSL_include_file(\"$header\","
    cat $header | sed -e 's|^|        \"|' -e 's| *$| \\n\"|'
    echo "     );"
    echo
done
echo "   }"


echo "   void register_embedded_shaders_fullscreen() {"
for header in $headers_fullscreen
do
    echo "     GEO::GLSL::register_GLSL_include_file(\"$header\","
    cat $header | sed -e 's|^|        \"|' -e 's| *$| \\n\"|'
    echo "     );"
    echo
done
echo "   }"

echo "}"
