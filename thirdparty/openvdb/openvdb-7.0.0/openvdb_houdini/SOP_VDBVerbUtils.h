// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

/*
 * Copyright (c)
 *      Side Effects Software Inc.  All rights reserved.
 */

#ifndef OPENVDB_HOUDINI_SOP_VDBVERBUTILS_HAS_BEEN_INCLUDED
#define OPENVDB_HOUDINI_SOP_VDBVERBUTILS_HAS_BEEN_INCLUDED

#include <UT/UT_Version.h>
#include <GOP/GOP_Manager.h>
#include <SOP/SOP_NodeParmsOptions.h> // for SOP_NodeCacheOptions
#include <openvdb/Types.h>
#include <string>


////////////////////////////////////////


/// @brief SOP_NodeCacheOptions subclass that adds methods specific to SOP_NodeVDB
class SOP_VDBCacheOptions: public SOP_NodeCacheOptions
{
public:
            SOP_VDBCacheOptions() {}
    virtual ~SOP_VDBCacheOptions() {}

    openvdb::Vec3f evalVec3f(const char* name, fpreal time) const
    {
        return openvdb::Vec3f(static_cast<float>(evalFloat(name, 0, time)),
                              static_cast<float>(evalFloat(name, 1, time)),
                              static_cast<float>(evalFloat(name, 2, time)));
    }
    openvdb::Vec3R evalVec3R(const char* name, fpreal time) const
    {
        return openvdb::Vec3R(evalFloat(name, 0, time),
                              evalFloat(name, 1, time),
                              evalFloat(name, 2, time));
    }
    openvdb::Vec3i evalVec3i(const char* name, fpreal time) const
    {
        using IntT = openvdb::Vec3i::ValueType;
        return openvdb::Vec3i(static_cast<IntT>(evalInt(name, 0, time)),
                              static_cast<IntT>(evalInt(name, 1, time)),
                              static_cast<IntT>(evalInt(name, 2, time)));
    }
    openvdb::Vec2R evalVec2R(const char* name, fpreal time) const
    {
        return openvdb::Vec2R(evalFloat(name, 0, time),
                              evalFloat(name, 1, time));
    }
    openvdb::Vec2i evalVec2i(const char* name, fpreal time) const
    {
        using IntT = openvdb::Vec2i::ValueType;
        return openvdb::Vec2i(static_cast<IntT>(evalInt(name, 0, time)),
                              static_cast<IntT>(evalInt(name, 1, time)));
    }

    std::string evalStdString(const char* name, fpreal time, int index = 0) const
    {
        UT_String str;
        evalString(str, name, index, time);
        return str.toStdString();
    }

    const GA_PrimitiveGroup *matchGroup(const GU_Detail &gdp, const UT_StringRef &groupname)
    {
        const GA_PrimitiveGroup *group = 0;
        if (groupname.isstring())
        {
            bool success = false;
            group = gop.parseOrderedPrimitiveDetached(groupname, &gdp, false, success);
            if (!success)
            {
                UT_StringHolder error;
                error = "Invalid group (";
                error += groupname;
                error += ")";
                throw std::runtime_error(error.c_str());
            }
        }
        return group;
    }

    const GA_PrimitiveGroup *
    parsePrimitiveGroups(const UT_StringRef &maskStr, const GroupCreator &maskGeo)
    {
        return gop.parsePrimitiveGroups(maskStr, maskGeo);
    }

    GA_PrimitiveGroup *
    parsePrimitiveGroupsCopy(const UT_StringRef &maskStr, const GroupCreator &maskGeo)
    {
        return gop.parsePrimitiveGroupsCopy(maskStr, maskGeo);
    }

    const GA_PointGroup *
    parsePointGroups(const UT_StringRef &maskStr, const GroupCreator &maskGeo)
    {
        return gop.parsePointGroups(maskStr, maskGeo);
    }

    const GA_PointGroup *
    parsePointGroups(const UT_StringRef &maskStr, const GU_Detail *gdp)
    {
        return parsePointGroups(maskStr, GroupCreator(gdp));
    }

protected:
    virtual OP_ERROR cook(OP_Context &context) override final
    {
        auto result = cookMySop(context);
        gop.destroyAdhocGroups();
        return result;
    }

    virtual OP_ERROR cookVDBSop(OP_Context&) = 0;
    OP_ERROR cookMySop(OP_Context& context) { return cookVDBSop(context); }

    // Handles ad-hoc group creation.
    GOP_Manager         gop;
}; // class SOP_VDBCacheOptions

#endif // OPENVDB_HOUDINI_SOP_VDBVERBUTILS_HAS_BEEN_INCLUDED
