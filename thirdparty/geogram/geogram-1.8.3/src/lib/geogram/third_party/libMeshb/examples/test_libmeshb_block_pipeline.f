
c     libmeshb example: transform a quadrilateral mesh into a triangular one
c     using fast block transfer and pipelined post processing

      include 'libmeshb7.ins'

      external qad2tri, movver

      integer n
      parameter (n=4000)
      integer i, ver, dim, res
     +, RefTab(n), TriTab(4,2*n), QadTab(5,n)
      integer*8 NmbVer, NmbQad, InpMsh, OutMsh
      real*8 VerTab(3,n)


c     --------------------------------------------
c     Open the quadrilateral mesh file for reading
c     --------------------------------------------

      InpMsh = gmfopenmesh('../sample_meshes/quad.meshb'
     +,GmfRead,ver,dim)
      print*, 'input mesh  :', InpMsh,'version :',ver,'dim :',dim

      if(InpMsh.eq.0) STOP ' InpMsh = 0'
      if(dim.ne.3) STOP ' dimension <> 3'

c     Check memory bounds
      NmbVer = gmfstatkwd(InpMsh, GmfVertices)
      if(NmbVer.gt.n) STOP 'Too many vertices'

      i = gmfstatkwd(InpMsh, GmfSolAtQuadrilaterals)
      print*, i,'GmfSolAtQuadrilaterals'

      NmbQad = gmfstatkwd(InpMsh, GmfQuadrilaterals)
      if(NmbQad.gt.n) STOP 'Too many quads'

      print*, 'input mesh  : ',NmbVer,' vertices,',NmbQad,'quads'

c     Read the vertices
      res = gmfgetblock(InpMsh, GmfVertices, 1_8, NmbVer,
     +                  0, %val(0), movver, 1, VerTab,
     +                  GmfDouble, VerTab(1,1), VerTab(1,2),
     +                  GmfDouble, VerTab(2,1), VerTab(2,2),
     +                  GmfDouble, VerTab(3,1), VerTab(3,2),
     +                  GmfInt,    RefTab(1),   RefTab(2))

c     Read the quads
      res = gmfgetblock(InpMsh, GmfQuadrilaterals, 1_8, NmbQad,
     +                  0, %val(0), %val(0),
     +                  GmfInt, QadTab(1,1), QadTab(1,2),
     +                  GmfInt, QadTab(2,1), QadTab(2,2),
     +                  GmfInt, QadTab(3,1), QadTab(3,2),
     +                  GmfInt, QadTab(4,1), QadTab(4,2),
     +                  GmfInt, QadTab(5,1), QadTab(5,2))

c     Close the quadrilateral mesh
      res = gmfclosemesh(InpMsh)
      print*, QadTab(1,1),QadTab(2,1),QadTab(3,1),QadTab(4,1)


c     ------------------------
c     Create a triangular mesh
c     ------------------------

      OutMsh = gmfopenmesh('tri.meshb', GmfWrite, ver, dim)
      if(OutMsh.eq.0) STOP ' OutMsh = 0'

c     Set the number of vertices
      res = gmfsetkwd(OutMsh, GmfVertices, NmbVer, 0 , 0)

c     Then write them down
      res = gmfsetblock(OutMsh, GmfVertices, 1_8, NmbVer,
     +                  0, %val(0) ,%val(0),
     +                  GmfDouble, VerTab(1,1), VerTab(1,2),
     +                  GmfDouble, VerTab(2,1), VerTab(2,2),
     +                  GmfDouble, VerTab(3,1), VerTab(3,2),
     +                  GmfInt,    RefTab(1),   RefTab(2))

c     Write the triangles
      res = gmfsetkwd(OutMsh, GmfTriangles, 2*NmbQad, 0, 0)
      res = gmfsetblock(OutMsh, GmfTriangles, 1_8, 2*NmbQad,
     +                  0,%val(0),
     +                  qad2tri, 2, QadTab, TriTab,
     +                  GmfInt, TriTab(1,1), TriTab(1,2),
     +                  GmfInt, TriTab(2,1), TriTab(2,2),
     +                  GmfInt, TriTab(3,1), TriTab(3,2),
     +                  GmfInt, TriTab(4,1), TriTab(4,2))

c     Don't forget to close the file
      res = gmfclosemesh(OutMsh)

      print*, 'output mesh : ',NmbVer,' vertices,',
     +         2*NmbQad,'triangles'

      end      



      subroutine qad2tri(BegIdx,EndIdx,QadTab,TriTab)

      integer*8 i,BegIdx,EndIdx
      integer TriTab(4,*),QadTab(5,*)
      print*, 'beg : ',BegIdx, 'end : ', EndIdx

      do i = BegIdx,EndIdx
          if(mod(i,2) .EQ. 1) then
              TriTab(1,i) = QadTab(1,i/2)
              TriTab(2,i) = QadTab(2,i/2)
              TriTab(3,i) = QadTab(3,i/2)
              TriTab(4,i) = QadTab(5,i/2)
          else
              TriTab(1,i) = QadTab(1,i/2)
              TriTab(2,i) = QadTab(3,i/2)
              TriTab(3,i) = QadTab(4,i/2)
              TriTab(4,i) = QadTab(5,i/2)
          endif
      end do

      return
      end


      subroutine movver(BegIdx,EndIdx,VerTab)

      integer*8 i,BegIdx,EndIdx
      real*8 VerTab(3,*)
      print*, 'beg : ',BegIdx, 'end : ', EndIdx

      do i = BegIdx,EndIdx
          VerTab(1,i) = VerTab(1,i)*2
      end do

      return
      end
