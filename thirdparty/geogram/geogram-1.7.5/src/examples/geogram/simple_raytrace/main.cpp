/*
 * GEOGRAM example program:
 * simple mesh raytracing using AABB tree.
 * disclaimer: not the fastest, lighting model is ridiculous, there is no
 *  antialiasing etc... (just a demo program for the mesh AABB class).
 */ 

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

#include <geogram/basic/common.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/command_line_args.h>
#include <geogram/basic/stopwatch.h>
#include "raytracing.h"

int main(int argc, char** argv) {
    using namespace GEO;
    GEO::initialize();
    CmdLine::import_arg_group("standard");    
    std::vector<std::string> filenames;
    if(!CmdLine::parse(argc, argv, filenames, "meshfile")) {
	return 1;
    }

    // 1) Load the mesh
    Mesh M;
    if(!mesh_load(filenames[0], M)) {
	return 1;
    }
    // 2) Normalize coordinates in the unit box (alternatively
    //  we could transform rays instead...)
    normalize_mesh(M);

    // 3) The scene
    Scene scene;


    scene.add_object(new MeshObject(M));                    // The mesh
    scene.add_object(new HorizontalCheckerboardPlane(0.0)); // The tradition !
    scene.add_object(                                       // A sphere
	new Sphere(vec3(-0.7, -0.7, 1.0),0.7)
    )->set_reflection_coefficient(vec3(0.8, 0.8, 0.8))
     ->set_diffuse_coefficient(vec3(0.2, 0.2, 0.2));

    // Let there be (two) lights !
    scene.add_object(new Light(
			 vec3(0.5, 1.3, 0.5), // Position
			 0.02,                // Radius
			 vec3(0.0, 0.5, 1.0)  // Color
		     )
    );
    scene.add_object(new Light(
			 vec3(1.0, 0.2, 1.0), // Position
			 0.02,                // Radius
			 vec3(1.5, 1.5, 0.5)  // Color
		     )
    );
    
    // 4) The camera and output image
    Camera camera(
	vec3(2.0, 2.0, 1.5), // Position
	vec3(0.5, 0.5, 0.5), // Target
	800, 800,            // Image size
	20.0                 // aperture angle (in degrees)
    );

    // 5) Let's trace some rays !!
    {
	Stopwatch Watch("Raytrace");

	parallel_for(
	    0, camera.image_height(),
	    [&camera, &scene](index_t Y) {
		for(index_t X=0; X<camera.image_width(); ++X) {
		    Ray R = camera.launch_ray(X,Y);
		    vec3 K = scene.raytrace(R);
		    camera.set_pixel(X,Y,K);
		}
	    }
	);
	Logger::out("Raytrace")
	    << double(camera.image_width()*camera.image_height()) /
	       Watch.elapsed_time() << " rays per second"
	    << std::endl;
    }

    // 6) Save the output image
    camera.save_image("output.ppm");
}

