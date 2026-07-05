

!----------------------------------------------------------
!
!                       LIBMESH V 7.56
! 
!----------------------------------------------------------
!
!   Description:        handles .meshb file format I/O
!   Author:             Loic MARECHAL
!   Creation date:      dec 08 2015
!   Last modification:  nov 27 2020
!
!----------------------------------------------------------

module libmeshb7
  
  use, intrinsic :: iso_c_binding, only: c_int,c_long,c_loc,c_ptr
  
  implicit none
  
 !Procedures definition
  external gmfopenmesh
  external gmfclosemesh
  external gmfstatkwd
  external gmfsetkwd
  external gmfgotokwd
  external gmfgetlin
  external gmfsetlin
  external gmfgetblock
  external gmfsetblock
  external gmfsethonodesordering
 !external gmfreadbyteflow
 !external gmfwritebyteflow
 !external gmfgetfloatprecision
 !external gmfsetfloatprecision

  integer(8) :: gmfopenmesh
  integer(4) :: gmfclosemesh
  integer(4) :: gmfstatkwd
  integer(4) :: gmfsetkwd
  integer(4) :: gmfgotokwd
  integer(4) :: gmfgetlin
  integer(4) :: gmfsetlin
  integer(4) :: gmfgetblock
  integer(4) :: gmfsetblock
  integer(4) :: gmfsethonodesordering
 !integer(4) :: gmfreadbyteflow
 !integer(4) :: gmfwritebyteflow
 !integer(4) :: gmfgetfloatprecision
 !integer(4) :: gmfsetfloatprecision


 !Parameters definition
  integer(4) :: gmfmaxtyp
  integer(4) :: gmfmaxkwd
  integer(4) :: gmfread
  integer(4) :: gmfwrite
  integer(4) :: gmfsca
  integer(4) :: gmfvec
  integer(4) :: gmfsymmat
  integer(4) :: gmfmat
  integer(4) :: gmffloat
  integer(4) :: gmfdouble
  integer(4) :: gmfint
  integer(4) :: gmflong
  integer(4) :: gmfinttab
  integer(4) :: gmflongtab
  integer(4) :: gmffloatvec
  integer(4) :: gmfdoublevec
  integer(4) :: gmfintvec
  integer(4) :: gmflongvec
  integer(4) :: gmfargtab
  integer(4) :: gmfarglst

  parameter (gmfmaxtyp=1000)
  parameter (gmfmaxkwd=103)
  parameter (gmfread=1)
  parameter (gmfwrite=2)
  parameter (gmfsca=1)
  parameter (gmfvec=2)
  parameter (gmfsymmat=3)
  parameter (gmfmat=4)
  parameter (gmffloat=1)
  parameter (gmfdouble=2)
  parameter (gmfint=3)
  parameter (gmflong=4)
  parameter (gmfinttab=7)
  parameter (gmflongtab=8)
  parameter (gmffloatvec=5)
  parameter (gmfdoublevec=6)
  parameter (gmfintvec=7)
  parameter (gmflongvec=8)
  parameter (gmfargtab=100)
  parameter (gmfarglst=101)


 !Keywords list
  integer(4) :: gmfmeshversionformatted
  integer(4) :: gmfdimension
  integer(4) :: gmfvertices
  integer(4) :: gmfedges
  integer(4) :: gmftriangles
  integer(4) :: gmfquadrilaterals
  integer(4) :: gmftetrahedra
  integer(4) :: gmfprisms
  integer(4) :: gmfhexahedra
  integer(4) :: gmfiterationsall
  integer(4) :: gmftimesall
  integer(4) :: gmfcorners
  integer(4) :: gmfridges
  integer(4) :: gmfrequiredvertices
  integer(4) :: gmfrequirededges
  integer(4) :: gmfrequiredtriangles
  integer(4) :: gmfrequiredquadrilaterals
  integer(4) :: gmftangentatedgevertices
  integer(4) :: gmfnormalatvertices
  integer(4) :: gmfnormalattrianglevertices
  integer(4) :: gmfnormalatquadrilateralvertices
  integer(4) :: gmfangleofcornerbound
  integer(4) :: gmftrianglesp2
  integer(4) :: gmfedgesp2
  integer(4) :: gmfsolatpyramids
  integer(4) :: gmfquadrilateralsq2
  integer(4) :: gmfisolatpyramids
  integer(4) :: gmfsubdomainfromgeom
  integer(4) :: gmftetrahedrap2
  integer(4) :: gmffault_neartri
  integer(4) :: gmffault_inter
  integer(4) :: gmfhexahedraq2
  integer(4) :: gmfextraverticesatedges
  integer(4) :: gmfextraverticesattriangles
  integer(4) :: gmfextraverticesatquadrilaterals
  integer(4) :: gmfextraverticesattetrahedra
  integer(4) :: gmfextraverticesatprisms
  integer(4) :: gmfextraverticesathexahedra
  integer(4) :: gmfverticesongeometricvertices
  integer(4) :: gmfverticesongeometricedges
  integer(4) :: gmfverticesongeometrictriangles
  integer(4) :: gmfverticesongeometricquadrilaterals
  integer(4) :: gmfedgesongeometricedges
  integer(4) :: gmffault_freeedge
  integer(4) :: gmfpolyhedra
  integer(4) :: gmfpolygons
  integer(4) :: gmffault_overlap
  integer(4) :: gmfpyramids
  integer(4) :: gmfboundingbox
  integer(4) :: gmfbody
  integer(4) :: gmfprivatetable
  integer(4) :: gmffault_badshape
  integer(4) :: gmfend
  integer(4) :: gmftrianglesongeometrictriangles
  integer(4) :: gmftrianglesongeometricquadrilaterals
  integer(4) :: gmfquadrilateralsongeometrictriangles
  integer(4) :: gmfquadrilateralsongeometricquadrilaterals
  integer(4) :: gmftangents
  integer(4) :: gmfnormals
  integer(4) :: gmftangentatvertices
  integer(4) :: gmfsolatvertices
  integer(4) :: gmfsolatedges
  integer(4) :: gmfsolattriangles
  integer(4) :: gmfsolatquadrilaterals
  integer(4) :: gmfsolattetrahedra
  integer(4) :: gmfsolatprisms
  integer(4) :: gmfsolathexahedra
  integer(4) :: gmfdsolatvertices
  integer(4) :: gmfisolatvertices
  integer(4) :: gmfisolatedges
  integer(4) :: gmfisolattriangles
  integer(4) :: gmfisolatquadrilaterals
  integer(4) :: gmfisolattetrahedra
  integer(4) :: gmfisolatprisms
  integer(4) :: gmfisolathexahedra
  integer(4) :: gmfiterations
  integer(4) :: gmftime
  integer(4) :: gmffault_smalltri
  integer(4) :: gmfcoarsehexahedra
  integer(4) :: gmfcomments
  integer(4) :: gmfperiodicvertices
  integer(4) :: gmfperiodicedges
  integer(4) :: gmfperiodictriangles
  integer(4) :: gmfperiodicquadrilaterals
  integer(4) :: gmfprismsp2
  integer(4) :: gmfpyramidsp2
  integer(4) :: gmfquadrilateralsq3
  integer(4) :: gmfquadrilateralsq4
  integer(4) :: gmftrianglesp3
  integer(4) :: gmftrianglesp4
  integer(4) :: gmfedgesp3
  integer(4) :: gmfedgesp4
  integer(4) :: gmfirefgroups
  integer(4) :: gmfdrefgroups
  integer(4) :: gmftetrahedrap3
  integer(4) :: gmftetrahedrap4
  integer(4) :: gmfhexahedraq3
  integer(4) :: gmfhexahedraq4
  integer(4) :: gmfpyramidsp3
  integer(4) :: gmfpyramidsp4
  integer(4) :: gmfprismsp3
  integer(4) :: gmfprismsp4
  integer(4) :: gmfhosolatedgesp1
  integer(4) :: gmfhosolatedgesp2
  integer(4) :: gmfhosolatedgesp3
  integer(4) :: gmfhosolattrianglesp1
  integer(4) :: gmfhosolattrianglesp2
  integer(4) :: gmfhosolattrianglesp3
  integer(4) :: gmfhosolatquadrilateralsq1
  integer(4) :: gmfhosolatquadrilateralsq2
  integer(4) :: gmfhosolatquadrilateralsq3
  integer(4) :: gmfhosolattetrahedrap1
  integer(4) :: gmfhosolattetrahedrap2
  integer(4) :: gmfhosolattetrahedrap3
  integer(4) :: gmfhosolatpyramidsp1
  integer(4) :: gmfhosolatpyramidsp2
  integer(4) :: gmfhosolatpyramidsp3
  integer(4) :: gmfhosolatprismsp1
  integer(4) :: gmfhosolatprismsp2
  integer(4) :: gmfhosolatprismsp3
  integer(4) :: gmfhosolathexahedraq1
  integer(4) :: gmfhosolathexahedraq2
  integer(4) :: gmfhosolathexahedraq3
  integer(4) :: gmfbezierbasis
  integer(4) :: gmfbyteflow
  integer(4) :: gmfedgesp2ordering
  integer(4) :: gmfedgesp3ordering
  integer(4) :: gmftrianglesp2ordering
  integer(4) :: gmftrianglesp3ordering
  integer(4) :: gmfquadrilateralsq2ordering
  integer(4) :: gmfquadrilateralsq3ordering
  integer(4) :: gmftetrahedrap2ordering
  integer(4) :: gmftetrahedrap3ordering
  integer(4) :: gmfpyramidsp2ordering
  integer(4) :: gmfpyramidsp3ordering
  integer(4) :: gmfprismsp2ordering
  integer(4) :: gmfprismsp3ordering
  integer(4) :: gmfhexahedraq2ordering
  integer(4) :: gmfhexahedraq3ordering
  integer(4) :: gmfedgesp1ordering
  integer(4) :: gmfedgesp4ordering
  integer(4) :: gmftrianglesp1ordering
  integer(4) :: gmftrianglesp4ordering
  integer(4) :: gmfquadrilateralsq1ordering
  integer(4) :: gmfquadrilateralsq4ordering
  integer(4) :: gmftetrahedrap1ordering
  integer(4) :: gmftetrahedrap4ordering
  integer(4) :: gmfpyramidsp1ordering
  integer(4) :: gmfpyramidsp4ordering
  integer(4) :: gmfprismsp1ordering
  integer(4) :: gmfprismsp4ordering
  integer(4) :: gmfhexahedraq1ordering
  integer(4) :: gmfhexahedraq4ordering
  integer(4) :: gmffloatingpointprecision
  integer(4) :: gmfhosolatedgesp4
  integer(4) :: gmfhosolattrianglesp4
  integer(4) :: gmfhosolatquadrilateralsq4
  integer(4) :: gmfhosolattetrahedrap4
  integer(4) :: gmfhosolatpyramidsp4
  integer(4) :: gmfhosolatprismsp4
  integer(4) :: gmfhosolathexahedraq4
  integer(4) :: gmfhosolatedgesp1nodespositions
  integer(4) :: gmfhosolatedgesp2nodespositions
  integer(4) :: gmfhosolatedgesp3nodespositions
  integer(4) :: gmfhosolatedgesp4nodespositions
  integer(4) :: gmfhosolattrianglesp1nodespositions
  integer(4) :: gmfhosolattrianglesp2nodespositions
  integer(4) :: gmfhosolattrianglesp3nodespositions
  integer(4) :: gmfhosolattrianglesp4nodespositions
  integer(4) :: gmfhosolatquadrilateralsq1nodespositions
  integer(4) :: gmfhosolatquadrilateralsq2nodespositions
  integer(4) :: gmfhosolatquadrilateralsq3nodespositions
  integer(4) :: gmfhosolatquadrilateralsq4nodespositions
  integer(4) :: gmfhosolattetrahedrap1nodespositions
  integer(4) :: gmfhosolattetrahedrap2nodespositions
  integer(4) :: gmfhosolattetrahedrap3nodespositions
  integer(4) :: gmfhosolattetrahedrap4nodespositions
  integer(4) :: gmfhosolatpyramidsp1nodespositions
  integer(4) :: gmfhosolatpyramidsp2nodespositions
  integer(4) :: gmfhosolatpyramidsp3nodespositions
  integer(4) :: gmfhosolatpyramidsp4nodespositions
  integer(4) :: gmfhosolatprismsp1nodespositions
  integer(4) :: gmfhosolatprismsp2nodespositions
  integer(4) :: gmfhosolatprismsp3nodespositions
  integer(4) :: gmfhosolatprismsp4nodespositions
  integer(4) :: gmfhosolathexahedraq1nodespositions
  integer(4) :: gmfhosolathexahedraq2nodespositions
  integer(4) :: gmfhosolathexahedraq3nodespositions
  integer(4) :: gmfhosolathexahedraq4nodespositions
  integer(4) :: gmfedgesreferenceelement     
  integer(4) :: gmftrianglereferenceelement  
  integer(4) :: gmfquadrilateralreferenceelement 
  integer(4) :: gmftetrahedronreferenceelement   
  integer(4) :: gmfpyramidreferenceelement   
  integer(4) :: gmfprismreferenceelement     
  integer(4) :: gmfhexahedronreferenceelement    
  integer(4) :: gmfboundarylayers
  integer(4) :: gmfreferencestrings
  integer(4) :: gmfprisms9
  integer(4) :: gmfhexahedra12

  parameter (gmfmeshversionformatted=1)
  parameter (gmfdimension=3)
  parameter (gmfvertices=4)
  parameter (gmfedges=5)
  parameter (gmftriangles=6)
  parameter (gmfquadrilaterals=7)
  parameter (gmftetrahedra=8)
  parameter (gmfprisms=9)
  parameter (gmfhexahedra=10)
  parameter (gmfiterationsall=11)
  parameter (gmftimesall=12)
  parameter (gmfcorners=13)
  parameter (gmfridges=14)
  parameter (gmfrequiredvertices=15)
  parameter (gmfrequirededges=16)
  parameter (gmfrequiredtriangles=17)
  parameter (gmfrequiredquadrilaterals=18)
  parameter (gmftangentatedgevertices=19)
  parameter (gmfnormalatvertices=20)
  parameter (gmfnormalattrianglevertices=21)
  parameter (gmfnormalatquadrilateralvertices=22)
  parameter (gmfangleofcornerbound=23)
  parameter (gmftrianglesp2=24)
  parameter (gmfedgesp2=25)
  parameter (gmfsolatpyramids=26)
  parameter (gmfquadrilateralsq2=27)
  parameter (gmfisolatpyramids=28)
  parameter (gmfsubdomainfromgeom=29)
  parameter (gmftetrahedrap2=30)
  parameter (gmffault_neartri=31)
  parameter (gmffault_inter=32)
  parameter (gmfhexahedraq2=33)
  parameter (gmfextraverticesatedges=34)
  parameter (gmfextraverticesattriangles=35)
  parameter (gmfextraverticesatquadrilaterals=36)
  parameter (gmfextraverticesattetrahedra=37)
  parameter (gmfextraverticesatprisms=38)
  parameter (gmfextraverticesathexahedra=39)
  parameter (gmfverticesongeometricvertices=40)
  parameter (gmfverticesongeometricedges=41)
  parameter (gmfverticesongeometrictriangles=42)
  parameter (gmfverticesongeometricquadrilaterals=43)
  parameter (gmfedgesongeometricedges=44)
  parameter (gmffault_freeedge=45)
  parameter (gmfpolyhedra=46)
  parameter (gmfpolygons=47)
  parameter (gmffault_overlap=48)
  parameter (gmfpyramids=49)
  parameter (gmfboundingbox=50)
  parameter (gmfbody=51)
  parameter (gmfprivatetable=52)
  parameter (gmffault_badshape=53)
  parameter (gmfend=54)
  parameter (gmftrianglesongeometrictriangles=55)
  parameter (gmftrianglesongeometricquadrilaterals=56)
  parameter (gmfquadrilateralsongeometrictriangles=57)
  parameter (gmfquadrilateralsongeometricquadrilaterals=58)
  parameter (gmftangents=59)
  parameter (gmfnormals=60)
  parameter (gmftangentatvertices=61)
  parameter (gmfsolatvertices=62)
  parameter (gmfsolatedges=63)
  parameter (gmfsolattriangles=64)
  parameter (gmfsolatquadrilaterals=65)
  parameter (gmfsolattetrahedra=66)
  parameter (gmfsolatprisms=67)
  parameter (gmfsolathexahedra=68)
  parameter (gmfdsolatvertices=69)
  parameter (gmfisolatvertices=70)
  parameter (gmfisolatedges=71)
  parameter (gmfisolattriangles=72)
  parameter (gmfisolatquadrilaterals=73)
  parameter (gmfisolattetrahedra=74)
  parameter (gmfisolatprisms=75)
  parameter (gmfisolathexahedra=76)
  parameter (gmfiterations=77)
  parameter (gmftime=78)
  parameter (gmffault_smalltri=79)
  parameter (gmfcoarsehexahedra=80)
  parameter (gmfcomments=81)
  parameter (gmfperiodicvertices=82)
  parameter (gmfperiodicedges=83)
  parameter (gmfperiodictriangles=84)
  parameter (gmfperiodicquadrilaterals=85)
  parameter (gmfprismsp2=86)
  parameter (gmfpyramidsp2=87)
  parameter (gmfquadrilateralsq3=88)
  parameter (gmfquadrilateralsq4=89)
  parameter (gmftrianglesp3=90)
  parameter (gmftrianglesp4=91)
  parameter (gmfedgesp3=92)
  parameter (gmfedgesp4=93)
  parameter (gmfirefgroups=94)
  parameter (gmfdrefgroups=95)
  parameter (gmftetrahedrap3=96)
  parameter (gmftetrahedrap4=97)
  parameter (gmfhexahedraq3=98)
  parameter (gmfhexahedraq4=99)
  parameter (gmfpyramidsp3=100)
  parameter (gmfpyramidsp4=101)
  parameter (gmfprismsp3=102)
  parameter (gmfprismsp4=103)
  parameter (gmfhosolatedgesp1=104)
  parameter (gmfhosolatedgesp2=105)
  parameter (gmfhosolatedgesp3=106)
  parameter (gmfhosolattrianglesp1=107)
  parameter (gmfhosolattrianglesp2=108)
  parameter (gmfhosolattrianglesp3=109)
  parameter (gmfhosolatquadrilateralsq1=110)
  parameter (gmfhosolatquadrilateralsq2=111)
  parameter (gmfhosolatquadrilateralsq3=112)
  parameter (gmfhosolattetrahedrap1=113)
  parameter (gmfhosolattetrahedrap2=114)
  parameter (gmfhosolattetrahedrap3=115)
  parameter (gmfhosolatpyramidsp1=116)
  parameter (gmfhosolatpyramidsp2=117)
  parameter (gmfhosolatpyramidsp3=118)
  parameter (gmfhosolatprismsp1=119)
  parameter (gmfhosolatprismsp2=120)
  parameter (gmfhosolatprismsp3=121)
  parameter (gmfhosolathexahedraq1=122)
  parameter (gmfhosolathexahedraq2=123)
  parameter (gmfhosolathexahedraq3=124)
  parameter (gmfbezierbasis=125)
  parameter (gmfbyteflow=126)
  parameter (gmfedgesp2ordering=127)
  parameter (gmfedgesp3ordering=128)
  parameter (gmftrianglesp2ordering=129)
  parameter (gmftrianglesp3ordering=130)
  parameter (gmfquadrilateralsq2ordering=131)
  parameter (gmfquadrilateralsq3ordering=132)
  parameter (gmftetrahedrap2ordering=133)
  parameter (gmftetrahedrap3ordering=134)
  parameter (gmfpyramidsp2ordering=135)
  parameter (gmfpyramidsp3ordering=136)
  parameter (gmfprismsp2ordering=137)
  parameter (gmfprismsp3ordering=138)
  parameter (gmfhexahedraq2ordering=139)
  parameter (gmfhexahedraq3ordering=140)
  parameter (gmfedgesp1ordering=141)
  parameter (gmfedgesp4ordering=142)
  parameter (gmftrianglesp1ordering=143)
  parameter (gmftrianglesp4ordering=144)
  parameter (gmfquadrilateralsq1ordering=145)
  parameter (gmfquadrilateralsq4ordering=146)
  parameter (gmftetrahedrap1ordering=147)
  parameter (gmftetrahedrap4ordering=148)
  parameter (gmfpyramidsp1ordering=149)
  parameter (gmfpyramidsp4ordering=150)
  parameter (gmfprismsp1ordering=151)
  parameter (gmfprismsp4ordering=152)
  parameter (gmfhexahedraq1ordering=153)
  parameter (gmfhexahedraq4ordering=154)
  parameter (gmffloatingpointprecision=155)
  parameter (gmfhosolatedgesp4=156)
  parameter (gmfhosolattrianglesp4=157)
  parameter (gmfhosolatquadrilateralsq4=158)
  parameter (gmfhosolattetrahedrap4=159)
  parameter (gmfhosolatpyramidsp4=160)
  parameter (gmfhosolatprismsp4=161)
  parameter (gmfhosolathexahedraq4=162)
  parameter (gmfhosolatedgesp1nodespositions=163)
  parameter (gmfhosolatedgesp2nodespositions=164)
  parameter (gmfhosolatedgesp3nodespositions=165)
  parameter (gmfhosolatedgesp4nodespositions=166)
  parameter (gmfhosolattrianglesp1nodespositions=167)
  parameter (gmfhosolattrianglesp2nodespositions=168)
  parameter (gmfhosolattrianglesp3nodespositions=169)
  parameter (gmfhosolattrianglesp4nodespositions=170)
  parameter (gmfhosolatquadrilateralsq1nodespositions=171)
  parameter (gmfhosolatquadrilateralsq2nodespositions=172)
  parameter (gmfhosolatquadrilateralsq3nodespositions=173)
  parameter (gmfhosolatquadrilateralsq4nodespositions=174)
  parameter (gmfhosolattetrahedrap1nodespositions=175)
  parameter (gmfhosolattetrahedrap2nodespositions=176)
  parameter (gmfhosolattetrahedrap3nodespositions=177)
  parameter (gmfhosolattetrahedrap4nodespositions=178)
  parameter (gmfhosolatpyramidsp1nodespositions=179)
  parameter (gmfhosolatpyramidsp2nodespositions=180)
  parameter (gmfhosolatpyramidsp3nodespositions=181)
  parameter (gmfhosolatpyramidsp4nodespositions=182)
  parameter (gmfhosolatprismsp1nodespositions=183)
  parameter (gmfhosolatprismsp2nodespositions=184)
  parameter (gmfhosolatprismsp3nodespositions=185)
  parameter (gmfhosolatprismsp4nodespositions=186)
  parameter (gmfhosolathexahedraq1nodespositions=187)
  parameter (gmfhosolathexahedraq2nodespositions=188)
  parameter (gmfhosolathexahedraq3nodespositions=189)
  parameter (gmfhosolathexahedraq4nodespositions=190)
  parameter (gmfedgesreferenceelement=191)
  parameter (gmftrianglereferenceelement=192)
  parameter (gmfquadrilateralreferenceelement=193)
  parameter (gmftetrahedronreferenceelement=194)
  parameter (gmfpyramidreferenceelement=195)
  parameter (gmfprismreferenceelement=196)
  parameter (gmfhexahedronreferenceelement=197)
  parameter (gmfboundarylayers=198)
  parameter (gmfreferencestrings=199)
  parameter (gmfprisms9=200)
  parameter (gmfhexahedra12=201)

!   !> interface GmfSetHONodesOrdering_c  
!   interface
!     function GmfSetHONodesOrdering_c(InpMsh, GmfKey, BasOrd, FilOrd) result(iErr) bind(c, name="GmfSetHONodesOrdering")
!       !>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
!       import c_long,c_int,c_ptr
!       !<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
!       !>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
!       integer(c_long)     , intent(in)        :: InpMsh
!       integer(c_int)      , intent(in)        :: GmfKey
!      !integer(c_int)      , intent(in)        :: BasOrd(:,:)
!      !integer(c_int)      , intent(in)        :: FilOrd(:,:)
!       type(c_ptr)         , intent(in)        :: BasOrd
!       type(c_ptr)         , intent(in)        :: FilOrd
!       integer(c_int)                          :: iErr
!       !<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
!     end function GmfSetHONodesOrdering_c    
!     
!     function GmfCloseMesh_c(InpMsh) result(iErr) bind(c, name="GmfCloseMesh")
!       !>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
!       import c_long,c_int,c_ptr
!       !<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
!       !>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
!       integer(c_long)     , intent(in)        :: InpMsh
!       integer(c_int)                          :: iErr
!       !<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
!     end function GmfCloseMesh_c
!     
!   end interface
!   
!   
!   public :: GmfSetHONodesOrdering_f90
!   public :: GmfOpenMesh_f90
!   public :: GmfCloseMesh_f90
  
  
  !> les lignes suivantes sont en conflit avec la variable integer(4) :: gmfsethonodesordering
  !interface GmfSetHONodesOrdering
  !  module procedure GmfSetHONodesOrdering_f90
  !  module procedure GmfSetHONodesOrdering_c
  !end interface
  
contains
    
!   subroutine GmfSetHONodesOrdering_f90(unit, GmfKey, BasOrd, FilOrd)
!     !>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
!     use, intrinsic :: iso_c_binding, only: c_loc,c_int,c_long,c_ptr
!     !<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<  
!     !>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
!     integer(8), intent(in)          :: unit
!     integer(4), intent(in)          :: GmfKey
!     integer(4), intent(in), pointer :: BasOrd(:,:)
!     integer(4), intent(in), pointer :: FilOrd(:,:)
!     !>
!     integer(c_int)                  :: iErr
!     !<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
!     !>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
!     !> Broker
!     iErr=GmfSetHONodesOrdering_c(            & 
!     &    InpMsh=int(unit,kind=c_long)       ,&
!     &    GmfKey=int(GmfKey,kind=c_int)      ,&
!     &    BasOrd=c_loc(BasOrd)               ,&
!     &    FilOrd=c_loc(FilOrd)                )
!     !<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
!     
!     return
!   end subroutine GmfSetHONodesOrdering_f90
!   
!   subroutine GmfOpenMesh_f90(unit, GmfKey, BasOrd, FilOrd)
!     !>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
!     use, intrinsic :: iso_c_binding, only: c_loc,c_int,c_long
!     !<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<  
!     !>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
!     integej1r(8), intent(in)          :: unit
!     !>
!     integer(c_int)                  :: iErr
!     !<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
!     !>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
!     !> Broker
!     iErr=GmfOpenMesh_c(InpMsh=int(unit,kind=c_long)  )
!     !<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
!     return
!   end subroutine GmfOpenMesh_f90
!   
!   subroutine GmfCloseMesh_f90(unit, GmfKey, BasOrd, FilOrd)
!     !>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
!     use, intrinsic :: iso_c_binding, only: c_loc,c_int,c_long
!     !<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<  
!     !>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
!     integer(8), intent(in)          :: unit
!     !>
!     integer(c_int)                  :: iErr
!     !<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
!     !>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
!     !> Broker
!     iErr=GmfCloseMesh_c(InpMsh=int(unit,kind=c_long)  )
!     !<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
!     return
!   end subroutine GmfCloseMesh_f90
  
   
  
  
end module libmeshb7
