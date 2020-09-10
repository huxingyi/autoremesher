// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

/*
 * Copyright (c) Side Effects Software Inc.
 *
 * Produced by:
 *      Side Effects Software Inc
 *      123 Front Street West, Suite 1401
 *      Toronto, Ontario
 *      Canada   M5J 2M2
 *      416-504-9876
 *
 * NAME:        GT_GEOPrimCollectVDB.h ( GT Library, C++)
 *
 * COMMENTS:
 */

#ifndef __GT_GEOPrimCollectVDB__
#define __GT_GEOPrimCollectVDB__

#include <GT/GT_GEOPrimCollect.h>
#include <openvdb/Platform.h>

namespace openvdb_houdini {

class OPENVDB_HOUDINI_API GT_GEOPrimCollectVDB : public GT_GEOPrimCollect
{
public:
                GT_GEOPrimCollectVDB(const GA_PrimitiveTypeId &id);
    virtual     ~GT_GEOPrimCollectVDB();

    static void registerPrimitive(const GA_PrimitiveTypeId &id);

    virtual GT_GEOPrimCollectData *
                beginCollecting(
                        const GT_GEODetailListHandle &,
                        const GT_RefineParms *) const;

    virtual GT_PrimitiveHandle
                collect(
                        const GT_GEODetailListHandle &geometry,
                        const GEO_Primitive *const* prim_list,
                        int nsegments,
                        GT_GEOPrimCollectData *data) const;

    virtual GT_PrimitiveHandle
                endCollecting(
                        const GT_GEODetailListHandle &geometry,
                        GT_GEOPrimCollectData *data) const;
private:

    GA_PrimitiveTypeId          myId;

};

} // namespace openvdb_houdini

#endif // __GT_GEOPrimCollectVDB__
