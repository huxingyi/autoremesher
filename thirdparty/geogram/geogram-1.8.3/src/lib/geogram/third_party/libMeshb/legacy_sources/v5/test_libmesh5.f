
c     libmesh5 example : transform a quadrilateral mesh into a triangular one

      include 'libmesh5_api.ins'
 
      integer i, NmbVer, NmbQad, InpMsh, OutMsh, ver, dim
     +, RefTab(10000), QadTab(10000,5), ParIdx
      real*8 VerTab(3,10000)


c     --------------------------------------------
c     Open the quadrilateral mesh file for reading
c     --------------------------------------------

      InpMsh = GmfOpenMeshF77('quad.mesh',GmfRead,ver,dim)
      print*, 'InpMsh = ', InpMsh, ' version = ', ver, ' dim = ', dim

      if(InpMsh.eq.0) STOP ' InpMsh = 0'
      if(ver.ne.GmfDouble) STOP ' version <> GmfDouble'
      if(dim.ne.3) STOP ' dimension <> 3'

c     Check memory bounds
      NmbVer = GmfStatKwdF77(InpMsh, GmfVertices)
      print*, 'NmbVer = ', NmbVer
	  if(NmbVer.gt.10000) STOP ' NmbVer > 10000'

      NmbQad = GmfStatKwdF77(InpMsh, GmfQuadrilaterals)
      print*, 'NmbQad = ', NmbQad
	  if(NmbQad.gt.10000) STOP ' NmbQad > 10000'

c     Read the vertices
      res = GmfGotoKwdF77(InpMsh, GmfVertices)

      do i = 1, NmbVer
          res = GmfGetVertex3dr8(InpMsh, VerTab(1,i), VerTab(2,i)
     +, VerTab(3,i), RefTab(i))
      end do

c     Read the quads
      res = GmfGotoKwdF77(InpMsh, GmfQuadrilaterals)

      do i = 1, NmbQad
          res = GmfGetQuadrilateral(InpMsh, QadTab(i,1), QadTab(i,2)
     +,                          QadTab(i,3), QadTab(i,4), QadTab(i,5))
      end do

c     Close the quadrilateral mesh
      res = GmfCloseMeshF77(InpMsh)


c     ------------------------
c     Create a triangular mesh
c     ------------------------

      OutMsh = GmfOpenMeshf77('tri.mesh', GmfWrite, ver, dim)
      if(OutMsh.eq.0) STOP ' OutMsh = 0'

c     Set the number of vertices
	  res = GmfSetKwdF77(OutMsh, GmfVertices, NmbVer, 0 , 0)

c     Then write them down
      do i = 1, NmbVer
		  res = GmfSetVertex3dr8(OutMsh, VerTab(1,i), VerTab(2,i)
     +,                           VerTab(3,i), RefTab(i))
      end do

c     Build two triangles from each quad
	  res = GmfSetKwdF77(OutMsh, GmfTriangles, 2*NmbQad, 0, 0)

      do i = 1, NmbQad
          res = GmfSetTriangle(OutMsh, QadTab(i,1)
     +                     , QadTab(i,2), QadTab(i,3), QadTab(i,5))
          res = GmfSetTriangle(OutMsh, QadTab(i,1)
     +                     , QadTab(i,3), QadTab(i,4), QadTab(i,5))
      end do

c     Don't forget to close the file
      res = GmfCloseMeshf77(OutMsh)

      end      
