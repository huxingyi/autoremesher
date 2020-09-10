**Security and OpenVDB**

The OpenVDB Technical Steering Committee (TSC) takes security very
seriously.

OpenVDB was not originally built as an outward-facing library.
Users should exercise caution when working with untrusted data.
Code injection bugs have been found in much simpler data structures,
so it would be foolish to assume OpenVDB is immune.

OpenVDB is also focused on high performance.   It will rely on
incoming parameters being valid.  For example, array bounds checking is
intentionally avoided.  Likewise, integer overflow concerns are
intentionally not addressed.

***Reporting***

If you discover a security vulnerability that you feel cannot
be disclosed publicly, please submit it to security@openvdb.org.
This will go to a private mailing list of the TSC where we will
endeavour to triage and respond to your issue in a timely manner.

***Outstanding Security Issues***

None

***Addressed Security Issues***

None

***File Format Expectations***

Attempting to read a .vdb file will:
* Return success and produce a valid VDB data structure in memory
* Fail with an error
* Execute forever
* Run out of memory

The last two options may be surprising.  VDBs, however, are designed
as open-ended containers of production data that may be terabytes in size.

It is a bug if some file causes the library to crash.  It is a serious
security issue if some file causes arbitrary code execution.

***Runtime Library Expectations***

We consider the library to run with the same privilege as the linked
code.  As such, we do not guarantee any safety against malformed
arguments.   Provided functions are called with well-formed
parameters, we expect the same set of behaviors as with file
loading.

It is a bug if calling a function with well-formed arguments causes
the library to crash.  It is a security issue if calling a function
with well-formed arguments causes arbitrary code execution.

We do not consider this as severe as file format issues because
in most deployments the parameter space is not exposed to potential
attackers.

***Proper Data Redaction***

A common concern when working with sensitive data is to ensure
that distributed files are clean and do not possess any hidden
data.   There are a few surprising ways in which OpenVDB can
maintain data that appears erased.

The best practice for building a clean VDB is populate an
empty grid voxel-by-voxel with the desired data and only
copy known and trusted metadata fields.

****Inactive Voxels****

When voxels are marked inactive in the grid, they are not cleared
to the background value.  If you rely on the data being deleted, you
should overwrite the voxel's values as well as deactivating them.
In addition, calling pruneInactive will free deactivated tiles.  This
is particularly important when passing a VDB to another process.

****Topology****

It is important to note the general topology of the grid provides
a 1-bit image of the data in question.  By taking the expected bandwidth
into account, a close approximation of an SDF can be recreated by just
the topology data.  Building a new tree with a new topology of only
the desired data can avoid this.

****Metadata****

VDBs will try to preserve metadata through most operations.  This can
provide an unexpected sidechannel for communication.

****Steganographic****

Most image-based steganographic techniques can be applied to VDBs.
Narrow band SDF have an additional concern, however.   Since only
the zero crossing affects the perceived geometry, there is considerable
room for information hiding in the off-band voxels.  Conversion to
polygon and reconstruction from the polygonal mesh should eliminate
those channels.

Most VDB algorithms ignore inactive values.  Hidden data stored in
inactive voxels may thus be preserved by the VDB tools, even for
non-SDF grids.  tools::changeBackground can be run to clear all inactive
voxels, and tools::pruneInactive to ensure minimal topology.
