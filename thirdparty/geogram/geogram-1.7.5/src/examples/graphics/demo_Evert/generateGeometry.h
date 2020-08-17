/*
    This file is part of "sphereEversion",
    a program by Michael McGuffin.
    The code in this file was almost entirely taken
    (with slight adaptations) from the source code of
    "evert", a program written by Nathaniel Thurston.
    evert's source code can be downloaded from
        http://www.geom.umn.edu/docs/outreach/oi/software.html
        http://www.geom.uiuc.edu/docs/outreach/oi/software.html

    Grateful acknowledgements go out to Nathaniel Thurston,
    Silvio Levy, and the Geometry Center (University of Minnesota)
    for making evert's source code freely available to the public.

    Update: June 2016: Bruno Levy - ported to GLUP and Doxygen documentation.
             Made many small optimizations: using references whenever possible.
             Removed all unnecessary memory allocations. Create vertices in
             one single contiguous chunk of memory.

    It seems that the original geometry center webpage no longer exists, but
    some information is still available on Michael McGuffin's webpage:
        http://www.dgp.toronto.edu/~mjmcguff/eversion/
*/

#ifndef GENERATE_GEOMETRY_H
#define GENERATE_GEOMETRY_H

/**
 * \brief Generates an array of points (and optionally normals) that corresponds
 *  to the sphere eversion at a given timestep.
 * \details The u parameter corresponds to latitude.
 *   Parallels [90 degrees north, 90 degrees south] are mapped to [0.0, 2.0].
 * The v parameter corresponds to longitude.
 * Meridians [0 degrees, 180 degrees west] are mapped to [0, numStrips].
 * \param[out] points an array of (1 + u_count) x (1 + v_count) x 3
 *  floating point numbers that receives the geometry of the grid points.
 * \param[out] normals an array of (1 + u_count) x (1 + v_count) x 3
 *  floating point numbers that receives the normal vectors at each grid point.
 *  If set to nullptr, then no normal is computed.
 * \param[in] time the eversion time, between 0.0 and 1.0
 * \param[in] numStrips number of corrugations. If this is too small, there will
 *   be pinches in the eversion.
 * \param[in] u_min set to 0.0 to start at north pole
 * \param[in] u_count recommended value: 12*(u_max-u_min)
 * \param[in] u_max set to 1.0 to stop at equator, or 2.0 to stop at south pole
 * \param[in] v_min origin is at 0.0
 * \param[in] v_count recommended value: 12*(v_max-v_min)
 * \param[in] v_max set to 1.0 for full sphere
 * \param[in] bendtime -1 means don't do bendtime at all
 * \param[in] corrStart , pushStart , twistStart, unpushStart , uncorrStart
 *  The eversion has five phases: corrugation, push, twist, unpush, 
 *  uncorrugation. These five parameters indicate the time at which each phase
 *  starts. Motion is smoothly interpolated between each phase.
 */
void generateGeometry(
    float* points,
    float* normals,
    double time = 0.0,   
    int numStrips = 8,   
    double u_min = 0.0,   
    int u_count = 12,      
    double u_max = 1.0,   
    double v_min = 0.0,
    int v_count = 12,  
    double v_max = 1.0,
    double bendtime = -1.0,   
    double corrStart   = 0.00,
    double pushStart   = 0.10, 
    double twistStart  = 0.23, 
    double unpushStart = 0.60, 
    double uncorrStart = 0.93  
);

#endif

