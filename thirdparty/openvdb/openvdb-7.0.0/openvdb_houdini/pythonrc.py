# Copyright Contributors to the OpenVDB Project
# SPDX-License-Identifier: MPL-2.0

# Startup script to set the visibility of (and otherwise customize)
# open-source (ASWF) OpenVDB nodes and their native Houdini equivalents
#
# To be installed as <dir>/python2.7libs/pythonrc.py,
# where <dir> is a path in $HOUDINI_PATH.

import hou
import os


# Construct a mapping from ASWF SOP names to names of equivalent
# native Houdini SOPs.
sopcategory = hou.sopNodeTypeCategory()
namemap = {}
for name, sop in sopcategory.nodeTypes().items():
    try:
        nativename = sop.spareData('nativename')
        if nativename:
            namemap[name] = nativename
    except AttributeError:
        pass

# Print the list of correspondences.
#from pprint import pprint
#pprint(namemap)


# Determine which VDB SOPs should be visible in the Tab menu:
# - If $OPENVDB_OPHIDE_POLICY is set to 'aswf', hide AWSF SOPs for which
#   a native Houdini equivalent exists.
# - If $OPENVDB_OPHIDE_POLICY is set to 'native', hide native Houdini SOPs
#   for which an ASWF equivalent exists.
# - Otherwise, show both the ASWF and the native SOPs.
names = []
ophide = os.getenv('OPENVDB_OPHIDE_POLICY', 'none').strip().lower()
if ophide == 'aswf':
    names = namemap.keys()
elif ophide == 'native':
    names = namemap.values()

for name in names:
    sop = sopcategory.nodeType(name)
    if sop:
        sop.setHidden(True)


# Customize SOP visibility with code like the following:
#
#     # Hide the ASWF Clip SOP.
#     sopcategory.nodeType('DW_OpenVDBClip').setHidden(True)
#
#     # Show the native VDB Clip SOP.
#     sopcategory.nodeType('vdbclip').setHidden(False)
#
#     # Hide all ASWF advection SOPs for which a native equivalent exists.
#     for name in namemap.keys():
#         if 'Advect' in name:
#             sopcategory.nodeType(name).setHidden(True)
