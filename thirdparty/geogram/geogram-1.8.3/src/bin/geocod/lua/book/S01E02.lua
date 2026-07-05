-- FR Fais decoller la fusee de Shift et Tab
-- EN Launch Shift and Tab's rocket

require("pixel")

function GLUP.draw_scene()

   -- FR Pour afficher les aretes des pixels
   -- EN To display the edges of the pixels
   GLUP.Enable(GLUP.DRAW_MESH)

   -- FR Rend les pixels un peu plus petits
   -- EN Makes the pixels a little bit smaller
   GLUP.SetCellsShrink(0.1)

   -- FR Affiche une grille pour mieux reperer les pixels
   -- EN Displays a grid to help locating pixels
   pixGrid()
   
   pixBegin()
   col("black")
   pix(1,1)
   pix(1,2)
   pix(3,1)
   pix(3,2)
   pix(5,1)
   pix(5,2)

   col("white")
   pix(2,2)
   pix(4,2)

   col("red")
   pix(2,3)
   pix(4,3)
   pix(3,4)
   pix(2,5)
   pix(4,5)

   col("white")
   pix(3,3)
   pix(2,4)
   pix(4,4)
   pix(3,5)
   pix(2,6)
   pix(3,6)
   pix(4,6)
   pix(3,7)
   pix(3,8)
   
   pixEnd()
end
