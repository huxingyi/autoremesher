
c     libMeshb 7.2 basic example: 
c     read a quad mesh, split it into triangles and write the result back

      include 'libmeshb7.ins'

      integer n
      parameter (n=4000)
      integer i,NmbVer,NmbQad,ver,dim,res,RefTab(n),QadTab(5,n)
      integer*8 InpMsh, OutMsh
      real*8 VerTab(3,n)


c     --------------------------------------------
c     Open the quadrilateral mesh file for reading
c     --------------------------------------------

c     Open the mesh file and check the version and dimension
      InpMsh = gmfopenmesh('../sample_meshes/quad.mesh ',
     +GmfRead,ver,dim)
      print*, 'input mesh :', InpMsh,'version:',ver,'dim:',dim
      if(InpMsh.eq.0) STOP ' InpMsh = 0'
      if(ver.le.1) STOP ' version <= 1'
      if(dim.ne.3) STOP ' dimension <> 3'

c     Check memory bounds
      NmbVer = gmfstatkwd(InpMsh, GmfVertices)
      if(NmbVer.gt.n) STOP 'Too many vertices'
      NmbQad = gmfstatkwd(InpMsh, GmfQuadrilaterals)
      if(NmbQad.gt.n) STOP 'Too many quads'
      print*, 'input mesh : ',NmbVer,' vertices,',NmbQad,'quads'

c     Read the vertices
      res = gmfgotokwd(InpMsh, GmfVertices)
      do i = 1, NmbVer
          res = gmfgetlin(InpMsh, GmfVertices
     +, VerTab(1,i), VerTab(2,i), VerTab(3,i), RefTab(i))
      end do

c     Read the quads
      res = gmfgotokwd(InpMsh, GmfQuadrilaterals)
      do i = 1, NmbQad
          res = gmfgetlin(InpMsh, GmfQuadrilaterals
     +, QadTab(1,i),QadTab(2,i),QadTab(3,i),QadTab(4,i),QadTab(5,i))
      end do

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
      do i = 1, NmbVer
          res = gmfsetlin(InpMsh, GmfVertices
     +, VerTab(1,i), VerTab(2,i), VerTab(3,i), RefTab(i))
      end do

c     Write the triangles
      res = gmfsetkwd(OutMsh, GmfTriangles, 2*NmbQad, 0, 0)
      do i=1,NmbQad
          res = gmfsetlin(InpMsh, GmfTriangles,
     +    QadTab(1,i),QadTab(2,i),QadTab(3,i),QadTab(5,i))
          res = gmfsetlin(InpMsh, GmfTriangles,
     +    QadTab(1,i),QadTab(3,i),QadTab(4,i),QadTab(5,i))
      end do

c     Don't forget to close the file
      res = gmfclosemesh(OutMsh)

      print*, 'output mesh: ',NmbVer,' vertices,',
     +         2*NmbQad,'triangles'

      end      
