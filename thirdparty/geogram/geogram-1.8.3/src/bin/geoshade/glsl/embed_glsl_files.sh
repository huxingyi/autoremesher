#!/bin/sh

glsl_files="
  course/raytrace_step1.glsl
  course/raytrace_step2.glsl
  course/raytrace_step3.glsl
  course/raytrace_step4.glsl
  course/raytrace_step5.glsl
  course/raytrace_step6.glsl  
ShaderToy/AlloyPlatedVoronoi.glsl
ShaderToy/AndromedaJewel.glsl
ShaderToy/Circuits.glsl
ShaderToy/ContouredLayers.glsl
ShaderToy/FractalLand.glsl
ShaderToy/GeodesicTiling.glsl
ShaderToy/Geomechanical.glsl
ShaderToy/HappyJumping.glsl
ShaderToy/HexFlow.glsl
ShaderToy/JellyTubes.glsl
ShaderToy/MengerTunnel.glsl
ShaderToy/ProteanClouds.glsl
ShaderToy/QuadTreeTruchet.glsl
ShaderToy/rabbit.glsl
ShaderToy/RayMarchingPrimitives.glsl
ShaderToy/SeaScape.glsl
ShaderToy/RoundedVoronoiEdges.glsl
ShaderToy/RounderVoronoi.glsl
ShaderToy/SiggraphLogo.glsl
ShaderToy/TentacleObject.glsl
ShaderToy/ThePopularShader.glsl
ShaderToy/TracedMinkwskiTube.glsl
ShaderToy/VoxelPacMan.glsl
ShaderToy/Voxels.glsl
"

cat <<EOF
/*
 * This file was automatically generated, do not edit.
 */

#include <geogram/basic/file_system.h>

void register_embedded_glsl_files(
   GEO::FileSystem::MemoryNode* n
);

void register_embedded_glsl_files(
   GEO::FileSystem::MemoryNode* n
) {
EOF

for f in $glsl_files
do
    echo "     n->create_file(\"$f\","
    cat $f | sed -e 's|\\|\\\\|' \
                 -e 's|"|\\\"|g' \
	         -e 's|^|        \"|' \
	         -e 's| *$|\\n\"|' 
    echo "     );"
    echo
done

echo "}"
