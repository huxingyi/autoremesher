c     Exemple d'utilisation de la librairie GMF : Recopie d'un fichier mesh dans un autre


c     Inclusion des definitions d'etiquettes de la librairie GMF
      include 'libmesh4.ins'
 
c     Le tableau KwdTab permet de stocker un mot-clef par ligne
c     La colonne 1 donne le nombre de lignes de ce mot-clefs presents dans le fichier
c     Les colonnes suivantes ne sont utilisees que par les champs de solutions
      integer KwdTab(GmfMaxSol + 2, GmfMaxKwd)

      integer i, j, InsIdx, OutIdx, res,dim, NmbVer, NmbTri
      real CrdTab(3, 600000)
      integer RefTab(600000)
      integer TriTab(4,1200000)


c     Ouverture du fichier .meshb. Les quatre parametres a fournir sont :
c     1 : le nom du fichier avec l'extension .mesh ou .meshb
c     2 : le mode d'ouverture GmfRead ou GmfWrite
c     3 : un entier qui indique la dimension en cas de creation d'un maillage
c         ou qui recevra la dimension en cas de lecture.
c     4 : le tableau des mot-clefs qui sera rempli des l'ouverture du maillage en lecture
c         ce qui permet de preparer ses allocations memoire avant la lecture des champs
c         proprement dit.
c     L'ouverture retourne une etiquette unique permettant d'identifier les differents
c     fichiers ouverts. Elle servira d'argument a toutes les autres commandes de la librairie.
      InsIdx = GmfOpenMeshF77('in.meshb',GmfRead,dim,KwdTab)
      print*, 'InsIdx = ', InsIdx
      if(InsIdx.le.0) STOP ' InsIdx==0'

c     La colonne 1 de la ligne GmfVertices contient le nombre de vertices du fichier
      NmbVer = KwdTab(1,GmfVertices)
      print*, 'NmbVer = ', NmbVer

c     La colonne 1 de la ligne GmfTriangles contient le nombre de triangles du fichier
      NmbTri = KwdTab(1,GmfTriangles)
      print*, 'NmbTri = ', NmbTri

c     Ouverture du fichier destination en ecriture. Il est inutile de fournir un tableau de mot-clefs
      OutIdx = GmfOpenMeshf77('out.meshb',GmfWrite,dim,0)
      print*,'Outidx = ',OutIdx
      if(OutIdx.le.0) STOP ' OutIdx==0'

c     Lecture des vertices,  les quatres parametres a fournir sont :
c     1 : l'etiquette du fichier a lire
c     2 : l'etiquette du mot-clef a lire
c     3 : un tableau suffisamment grand pour contenir tous les nombres reels retournes par ce mot-clefs
c     4 : un tableau suffisamment grand pour contenir tous les nombres entiers retournes par ce mot-clefs
c     Dans le cas de lecture de vertices en 3d le tableau de reels doit contenir 3 * le nombre de noeuds
c     (pour les coordonnees) et le tableau d'entiers une fois ce nombre (pour les references)
      res = GmfReadFieldF77(InsIdx, GmfVertices, CrdTab, RefTab)
      print*,'read vertices = ',res

c     L'ecriture du mot-clefs fonctionne sur le meme principe mais 3 arguments sont ajoutes :
c     3 : le nombre de lignes a ecrire
c     4 : le nombre de type de solutions (dans le cas d'un champ de solutions)
c     5 : un tableau contenant tous les type de solutions.
      res = GmfWriteFieldf77(OutIdx,GmfVertices,NmbVer,0,0,CrdTab,
     +RefTab)
      print*,'Write vertices = ',res

c     Dans le cas des triangles qui n'utilisent que des entiers, il est inutile de fournir un tableau de reels
      res = GmfReadFieldF77(InsIdx, GmfTriangles, 0, TriTab)
      print*,'read triangles = ',res

      res = GmfWriteFieldf77(OutIdx,GmfTriangles,NmbTri,0,0,0,TriTab)
      print*,'Write triangles = ',res

c     La fermeture des fichiers est obligatoire, faute de quoi les donnees ne seront pas proprement ecrites.
      res = GmfCloseMeshF77(InsIdx)
      print*,'close read = ',res

      res = GmfCloseMeshf77(OutIdx)
      print*,'close write = ',res


      end      
