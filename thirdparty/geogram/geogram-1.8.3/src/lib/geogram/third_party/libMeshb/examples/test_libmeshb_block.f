
c     libMeshb 7.5 example: transform a quadrilateral mesh into a triangular one
c     using fast block transfer and pipelined post processing

      include 'libmeshb7.ins'

      external qad2tri

      integer n
      parameter (n=4000)
      integer i, ver, dim, res
     +, RefTab(n), TriTab(4,2*n), QadTab(5,n)
      integer*8 InpMsh, OutMsh, NmbVer, NmbQad
      real*8 VerTab(3,n)


c     --------------------------------------------
c     Open the quadrilateral mesh file for reading
c     --------------------------------------------

      InpMsh = gmfopenmesh('../sample_meshes/quad.meshb'
     +,GmfRead,ver,dim)

      if(InpMsh.eq.0) STOP ' InpMsh = 0'
      if(dim.ne.3) STOP ' dimension <> 3'

c     Check memory bounds
      NmbVer = gmfstatkwd(InpMsh, GmfVertices)
      if(NmbVer.gt.n) STOP 'Too many vertices'

      NmbQad = gmfstatkwd(InpMsh, GmfQuadrilaterals)
      if(NmbQad.gt.n) STOP 'Too many quads'

c     Print some information on the open file
      print*, 'input mesh  :', InpMsh
      print*, 'version     :', ver
      print*, 'dimension   :', dim
      print*, 'vertices    :', NmbVer
      print*, 'quads       :', NmbQad

c     Read the vertices using a vector of 3 consecutive doubles
c     to store the coordinates
      res = gmfgetblock(InpMsh,GmfVertices,
     +        1_8, NmbVer, 0, %val(0), %val(0),
     +        GmfDoubleVec, 3, VerTab(1,1), VerTab(1,NmbVer),
     +        GmfInt,          RefTab(  1), RefTab(  NmbVer))

c     Read the quads using one single vector of 5 consecutive integers
      res = gmfgetblock(InpMsh,GmfQuadrilaterals,
     +         1_8, NmbQad, 0, %val(0), %val(0),
     +         GmfIntVec, 5, QadTab(1,1), QadTab(1,NmbQad))

c     Close the quadrilateral mesh
      res = gmfclosemesh(InpMsh)


c     -----------------------
c     Write a triangular mesh
c     -----------------------

      OutMsh = gmfopenmesh('tri.meshb', GmfWrite, ver, dim)
      if(OutMsh.eq.0) STOP ' OutMsh = 0'

c     Set the number of vertices
      res = gmfsetkwd(OutMsh, GmfVertices, NmbVer, 0, 0)

c     Write them down using separate pointers for each scalar entry
      res = gmfsetblock(OutMsh,GmfVertices,
     +         1_8, NmbVer, 0, %val(0), %val(0),
     +         GmfDouble, VerTab(1,1), VerTab(1,NmbVer),
     +         GmfDouble, VerTab(2,1), VerTab(2,NmbVer),
     +         GmfDouble, VerTab(3,1), VerTab(3,NmbVer),
     +         GmfInt,    RefTab(1),   RefTab(NmbVer))

c     Write the triangles using 4 independant set of arguments
c     for each scalar entry: node1, node2, node3 and reference
      res = gmfsetkwd(OutMsh, GmfTriangles, 2*NmbQad, 0, 0)
      res = gmfsetblock(OutMsh, GmfTriangles,
     +                  1_8, 2*NmbQad, 0, %val(0),
     +                  qad2tri, 2, QadTab, TriTab,
     +                  GmfInt, TriTab(1,1), TriTab(1,2*NmbQad),
     +                  GmfInt, TriTab(2,1), TriTab(2,2*NmbQad),
     +                  GmfInt, TriTab(3,1), TriTab(3,2*NmbQad),
     +                  GmfInt, TriTab(4,1), TriTab(4,2*NmbQad))

c     Don't forget to close the file
      res = gmfclosemesh(OutMsh)

      print*, 'output mesh :',NmbVer,' vertices,',
     +         2*NmbQad,'triangles'

      end      


c     A subroutine that reads quads ans splits them into triangles
c     it is executed concurently with the block writing
      subroutine qad2tri(BegIdx,EndIdx,QadTab,TriTab)

      integer*8 i,BegIdx,EndIdx
      integer TriTab(4,*),QadTab(5,*)

      do i = BegIdx,EndIdx
         if(mod(i,2) .EQ. 1) then
            TriTab(1,i) = QadTab(1,(i+1)/2)
            TriTab(2,i) = QadTab(2,(i+1)/2)
            TriTab(3,i) = QadTab(3,(i+1)/2)
            TriTab(4,i) = QadTab(5,(i+1)/2)
         else
            TriTab(1,i) = QadTab(1,(i+1)/2)
            TriTab(2,i) = QadTab(3,(i+1)/2)
            TriTab(3,i) = QadTab(4,(i+1)/2)
            TriTab(4,i) = QadTab(5,(i+1)/2)
         endif
      end do

      return
      end
