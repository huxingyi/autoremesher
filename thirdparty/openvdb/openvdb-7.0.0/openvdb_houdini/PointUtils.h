// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

/// @file PointUtils.h
///
/// @authors Dan Bailey, Nick Avramoussis, Richard Kwok
///
/// @brief Utility classes and functions for OpenVDB Points Houdini plugins

#ifndef OPENVDB_HOUDINI_POINT_UTILS_HAS_BEEN_INCLUDED
#define OPENVDB_HOUDINI_POINT_UTILS_HAS_BEEN_INCLUDED

#include <openvdb/math/Vec3.h>
#include <openvdb/Types.h>
#include <openvdb/points/PointDataGrid.h>

#include <GA/GA_Attribute.h>
#include <GU/GU_Detail.h>
#include <PRM/PRM_ChoiceList.h>

#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <vector>


#ifdef SESI_OPENVDB
    #ifdef OPENVDB_HOUDINI_API
        #undef OPENVDB_HOUDINI_API
        #define OPENVDB_HOUDINI_API
    #endif
#endif


namespace openvdb_houdini {

using OffsetList = std::vector<GA_Offset>;
using OffsetListPtr = std::shared_ptr<OffsetList>;

using OffsetPair = std::pair<GA_Offset, GA_Offset>;
using OffsetPairList = std::vector<OffsetPair>;
using OffsetPairListPtr = std::shared_ptr<OffsetPairList>;

// note that the bool parameter here for toggling in-memory compression is now deprecated
using AttributeInfoMap = std::map<openvdb::Name, std::pair<int, bool>>;

using WarnFunc = std::function<void (const std::string&)>;

/// Metadata name for viewport groups
const std::string META_GROUP_VIEWPORT = "group_viewport";


/// Enum to store available compression types for point grids
enum POINT_COMPRESSION_TYPE
{
    COMPRESSION_NONE = 0,
    COMPRESSION_TRUNCATE,
    COMPRESSION_UNIT_VECTOR,
    COMPRESSION_UNIT_FIXED_POINT_8,
    COMPRESSION_UNIT_FIXED_POINT_16,
};


// forward declaration
class Interrupter;


/// @brief Compute a voxel size from a Houdini detail
///
/// @param  detail           GU_Detail to compute the voxel size from
/// @param  pointsPerVoxel   the target number of points per voxel, must be positive and non-zero
/// @param  matrix           voxel size will be computed using this transform
/// @param  decimalPlaces    for readability, truncate voxel size to this number of decimals
/// @param  interrupter      a Houdini interrupter
OPENVDB_HOUDINI_API
float
computeVoxelSizeFromHoudini(
    const GU_Detail& detail,
    const openvdb::Index pointsPerVoxel,
    const openvdb::math::Mat4d& matrix,
    const openvdb::Index decimalPlaces,
    Interrupter& interrupter);


/// @brief Convert a Houdini detail into a VDB Points grid
///
/// @param  detail         GU_Detail to convert the points and attributes from
/// @param  compression    position compression to use
/// @param  attributes     a vector of VDB Points attributes to be included
///                        (empty vector defaults to all)
/// @param  transform      transform to use for the new point grid
/// @param  warnings       list of warnings to be added to the SOP
OPENVDB_HOUDINI_API
openvdb::points::PointDataGrid::Ptr
convertHoudiniToPointDataGrid(
    const GU_Detail& detail,
    const int compression,
    const AttributeInfoMap& attributes,
    const openvdb::math::Transform& transform,
    const WarnFunc& warnings = [](const std::string&){});


/// @brief Convert a VDB Points grid into Houdini points and append them to a Houdini Detail
///
/// @param  detail         GU_Detail to append the converted points and attributes to
/// @param  grid           grid containing the points that will be converted
/// @param  attributes     a vector of VDB Points attributes to be included
///                        (empty vector defaults to all)
/// @param  includeGroups  a vector of VDB Points groups to be included
///                        (empty vector defaults to all)
/// @param  excludeGroups  a vector of VDB Points groups to be excluded
///                        (empty vector defaults to none)
/// @param inCoreOnly      true if out-of-core leaf nodes are to be ignored
OPENVDB_HOUDINI_API
void
convertPointDataGridToHoudini(
    GU_Detail& detail,
    const openvdb::points::PointDataGrid& grid,
    const std::vector<std::string>& attributes = {},
    const std::vector<std::string>& includeGroups = {},
    const std::vector<std::string>& excludeGroups = {},
    const bool inCoreOnly = false);


/// @brief Populate VDB Points grid metadata from Houdini detail attributes
///
/// @param  grid           grid to be populated with metadata
/// @param  detail         GU_Detail to extract the detail attributes from
/// @param  warnings       list of warnings to be added to the SOP
OPENVDB_HOUDINI_API
void
populateMetadataFromHoudini(
    openvdb::points::PointDataGrid& grid,
    const GU_Detail& detail,
    const WarnFunc& warnings = [](const std::string&){});


/// @brief Convert VDB Points grid metadata into Houdini detail attributes
///
/// @param  detail         GU_Detail to add the Houdini detail attributes
/// @param  metaMap        the metamap to create the Houdini detail attributes from
/// @param  warnings       list of warnings to be added to the SOP
OPENVDB_HOUDINI_API
void
convertMetadataToHoudini(
    GU_Detail& detail,
    const openvdb::MetaMap& metaMap,
    const WarnFunc& warnings = [](const std::string&){});


/// @brief Returns supported tuple sizes for conversion from GA_Attribute
OPENVDB_HOUDINI_API
int16_t
attributeTupleSize(const GA_Attribute* const attribute);


/// @brief Returns supported Storage types for conversion from GA_Attribute
OPENVDB_HOUDINI_API
GA_Storage
attributeStorageType(const GA_Attribute* const attribute);


///////////////////////////////////////


/// @brief If the given grid is a PointDataGrid, add node specific info text to
///        the stream provided. This is used to populate the MMB window in Houdini
///        versions 15 and earlier, as well as the Operator Information Window.
OPENVDB_HOUDINI_API
void
pointDataGridSpecificInfoText(std::ostream&, const openvdb::GridBase&);

/// @brief  Populates string data with information about the provided OpenVDB
///         Points grid.
/// @param  grid          The OpenVDB Points grid to retrieve information from.
/// @param  countStr      The total point count as a formatted integer.
/// @param  groupStr      The list of comma separated groups (or "none" if no
///                       groups exist). Enclosed by parentheses.
/// @param  attributeStr  The list of comma separated attributes (or "none" if
///                       no attributes exist). Enclosed by parentheses.
///                       Each attribute takes the form "name [type] [code]
///                       [stride]" where code and stride are added for non
///                       default attributes.
OPENVDB_HOUDINI_API
void
collectPointInfo(const openvdb::points::PointDataGrid& grid,
    std::string& countStr,
    std::string& groupStr,
    std::string& attributeStr);


///////////////////////////////////////


// VDB Points group name drop-down menu

OPENVDB_HOUDINI_API extern const PRM_ChoiceList VDBPointsGroupMenuInput1;
OPENVDB_HOUDINI_API extern const PRM_ChoiceList VDBPointsGroupMenuInput2;
OPENVDB_HOUDINI_API extern const PRM_ChoiceList VDBPointsGroupMenuInput3;
OPENVDB_HOUDINI_API extern const PRM_ChoiceList VDBPointsGroupMenuInput4;

/// @note   Use this if you have more than 4 inputs, otherwise use
///         the input specific menus instead which automatically
///         handle the appropriate spare data settings.
OPENVDB_HOUDINI_API extern const PRM_ChoiceList VDBPointsGroupMenu;

} // namespace openvdb_houdini

#endif // OPENVDB_HOUDINI_POINT_UTILS_HAS_BEEN_INCLUDED
