// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#ifndef OPENVDB_UNITTEST_UTIL_HAS_BEEN_INCLUDED
#define OPENVDB_UNITTEST_UTIL_HAS_BEEN_INCLUDED

#include <openvdb/openvdb.h>
#include <openvdb/math/Math.h> // for math::Random01
#include <openvdb/tools/Prune.h>// for pruneLevelSet
#include <sstream>

namespace unittest_util {

enum SphereMode { SPHERE_DENSE, SPHERE_DENSE_NARROW_BAND, SPHERE_SPARSE_NARROW_BAND };

/// @brief Generates the signed distance to a sphere located at @a center
/// and with a specified @a radius (both in world coordinates). Only voxels
/// in the domain [0,0,0] -> @a dim are considered. Also note that the
/// level set is either dense, dense narrow-band or sparse narrow-band.
///
/// @note This method is VERY SLOW and should only be used for debugging purposes!
/// However it works for any transform and even with open level sets.
/// A faster approch for closed narrow band generation is to only set voxels
/// sparsely and then use grid::signedFloodFill to define the sign
/// of the background values and tiles! This is implemented in openvdb/tools/LevelSetSphere.h
template<class GridType>
inline void
makeSphere(const openvdb::Coord& dim, const openvdb::Vec3f& center, float radius,
           GridType& grid, SphereMode mode)
{
    typedef typename GridType::ValueType ValueT;
    const ValueT
        zero = openvdb::zeroVal<ValueT>(),
        outside = grid.background(),
        inside = -outside;

    typename GridType::Accessor acc = grid.getAccessor();
    openvdb::Coord xyz;
    for (xyz[0]=0; xyz[0]<dim[0]; ++xyz[0]) {
        for (xyz[1]=0; xyz[1]<dim[1]; ++xyz[1]) {
            for (xyz[2]=0; xyz[2]<dim[2]; ++xyz[2]) {
                const openvdb::Vec3R p =  grid.transform().indexToWorld(xyz);
                const float dist = float((p-center).length() - radius);
                OPENVDB_NO_TYPE_CONVERSION_WARNING_BEGIN
                ValueT val = ValueT(zero + dist);
                OPENVDB_NO_TYPE_CONVERSION_WARNING_END
                switch (mode) {
                case SPHERE_DENSE:
                    acc.setValue(xyz, val);
                    break;
                case SPHERE_DENSE_NARROW_BAND:
                    acc.setValue(xyz, val < inside ? inside : outside < val ? outside : val);
                    break;
                case SPHERE_SPARSE_NARROW_BAND:
                    if (val < inside)
                        acc.setValueOff(xyz, inside);
                    else if (outside < val)
                        acc.setValueOff(xyz, outside);
                    else
                        acc.setValue(xyz, val);
                }
            }
        }
    }
    //if (mode == SPHERE_SPARSE_NARROW_BAND) grid.tree().prune();
    if (mode == SPHERE_SPARSE_NARROW_BAND) openvdb::tools::pruneLevelSet(grid.tree());
}

// Template specialization for boolean trees (mostly a dummy implementation)
template<>
inline void
makeSphere<openvdb::BoolGrid>(const openvdb::Coord& dim, const openvdb::Vec3f& center,
                              float radius, openvdb::BoolGrid& grid, SphereMode)
{
    openvdb::BoolGrid::Accessor acc = grid.getAccessor();
    openvdb::Coord xyz;
    for (xyz[0]=0; xyz[0]<dim[0]; ++xyz[0]) {
        for (xyz[1]=0; xyz[1]<dim[1]; ++xyz[1]) {
            for (xyz[2]=0; xyz[2]<dim[2]; ++xyz[2]) {
                const openvdb::Vec3R p =  grid.transform().indexToWorld(xyz);
                const float dist = static_cast<float>((p-center).length() - radius);
                if (dist <= 0) acc.setValue(xyz, true);
            }
        }
    }
}

// This method will soon be replaced by the one above!!!!!
template<class GridType>
inline void
makeSphere(const openvdb::Coord& dim, const openvdb::Vec3f& center, float radius,
           GridType &grid, float dx, SphereMode mode)
{
    grid.setTransform(openvdb::math::Transform::createLinearTransform(/*voxel size=*/dx));
    makeSphere<GridType>(dim, center, radius, grid, mode);
}

// Generate random points by uniformly distributing points
// on a unit-sphere.
inline void genPoints(const int numPoints, std::vector<openvdb::Vec3R>& points)
{
    // init
    openvdb::math::Random01 randNumber(0);
    const int n = int(std::sqrt(double(numPoints)));
    const double xScale = (2.0 * M_PI) / double(n);
    const double yScale = M_PI / double(n);

    double x, y, theta, phi;
    openvdb::Vec3R pos;

    points.reserve(n*n);

    // loop over a [0 to n) x [0 to n) grid.
    for (int a = 0; a < n; ++a) {
        for (int b = 0; b < n; ++b) {

            // jitter, move to random pos. inside the current cell
            x = double(a) + randNumber();
            y = double(b) + randNumber();

            // remap to a lat/long map
            theta = y * yScale; // [0 to PI]
            phi   = x * xScale; // [0 to 2PI]

            // convert to cartesian coordinates on a unit sphere.
            // spherical coordinate triplet (r=1, theta, phi)
            pos[0] = std::sin(theta)*std::cos(phi);
            pos[1] = std::sin(theta)*std::sin(phi);
            pos[2] = std::cos(theta);

            points.push_back(pos);
        }
    }
}

// @todo makePlane

} // namespace unittest_util

#endif // OPENVDB_UNITTEST_UTIL_HAS_BEEN_INCLUDED
