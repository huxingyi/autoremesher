
c     libmesh6 example : transform a quadrilateral mesh into a triangular one

      include 'libmesh6.ins'

	  integer n
	  parameter (n=2000)
      integer i, NmbVer, NmbQad, InpMsh, OutMsh, ver, dim
     +, RefTab(n), TriTab(4,2*n), QadTab(5,n), res
      real*8 VerTab(3,n)


c     --------------------------------------------
c     Open the quadrilateral mesh file for reading
c     --------------------------------------------

      InpMsh = gmfopenmesh('quad.mesh',GmfRead,ver,dim)
      print*, 'input mesh  :', InpMsh,'version :',ver,'dim :',dim

      if(InpMsh.eq.0) STOP ' InpMsh = 0'
      if(ver.ne.GmfDouble) STOP ' version <> GmfDouble'
      if(dim.ne.3) STOP ' dimension <> 3'

c     Check memory bounds
      NmbVer = gmfstatkwd(InpMsh, GmfVertices)
	  if(NmbVer.gt.n) STOP 'Too many vertices'

      NmbQad = gmfstatkwd(InpMsh, GmfQuadrilaterals)
	  if(NmbQad.gt.n) STOP 'Too many quads'

      print*, 'input mesh  : ',NmbVer,' vertices,',NmbQad,'quads'

c     Read the vertices
      res = gmfgotokwd(InpMsh, GmfVertices)
	  res = gmfgetblock(InpMsh, GmfVertices,
     +                  GmfDouble, VerTab(1,1), VerTab(1,2),
     +                  GmfDouble, VerTab(2,1), VerTab(2,2),
     +                  GmfDouble, VerTab(3,1), VerTab(3,2),
     +                  GmfInt,    RefTab(1),   RefTab(2))

c     Read the quads
      res = gmfgotokwd(InpMsh, GmfQuadrilaterals)
	  res = gmfgetblock(InpMsh, GmfQuadrilaterals,
     +                  GmfInt, QadTab(1,1), QadTab(1,2),
     +                  GmfInt, QadTab(2,1), QadTab(2,2),
     +                  GmfInt, QadTab(3,1), QadTab(3,2),
     +                  GmfInt, QadTab(4,1), QadTab(4,2),
     +                  GmfInt, QadTab(5,1), QadTab(5,2))

c     Close the quadrilateral mesh
      res = gmfclosemesh(InpMsh)


c     ------------------------
c     Create a triangular mesh
c     ------------------------

      OutMsh = gmfopenmesh('tri.mesh', GmfWrite, ver, dim)
      if(OutMsh.eq.0) STOP ' OutMsh = 0'

c     Set the number of vertices
	  res = gmfsetkwd(OutMsh, GmfVertices, NmbVer, 0 , 0)

c     Then write them down
	  res = gmfsetblock(OutMsh, GmfVertices, 
     +                  GmfDouble, VerTab(1,1), VerTab(1,2),
     +                  GmfDouble, VerTab(2,1), VerTab(2,2),
     +                  GmfDouble, VerTab(3,1), VerTab(3,2),
     +                  GmfInt,    RefTab(1),   RefTab(2))

c     Build two triangles out of each quad
      do i = 1, NmbQad
		  TriTab(1,i*2-1) = QadTab(1,i)
		  TriTab(2,i*2-1) = QadTab(2,i)
		  TriTab(3,i*2-1) = QadTab(3,i)
		  TriTab(4,i*2-1) = QadTab(5,i)

		  TriTab(1,i*2) = QadTab(1,i)
		  TriTab(2,i*2) = QadTab(3,i)
		  TriTab(3,i*2) = QadTab(4,i)
		  TriTab(4,i*2) = QadTab(5,i)
	  end do

c     Write the triangles
	  res = gmfsetkwd(OutMsh, GmfTriangles, 2*NmbQad, 0, 0)
	  res = gmfsetblock(OutMsh, GmfTriangles,
     +                  GmfInt, TriTab(1,1), TriTab(1,2),
     +                  GmfInt, TriTab(2,1), TriTab(2,2),
     +                  GmfInt, TriTab(3,1), TriTab(3,2),
     +                  GmfInt, TriTab(4,1), TriTab(4,2))

c     Don't forget to close the file
      res = gmfclosemesh(OutMsh)

      print*, 'output mesh : ',NmbVer,' vertices,',
     +         2*NmbQad,'triangles'

      end      
