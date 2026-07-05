require("pixel") 
 
function GLUP.init_graphics() 
   GLUP.SetRegionOfInterest(1,1,1,31,31,1) 
end 


function carre(x1,y1,x2,y2,z)
    for x=x1,x2 do
       pix3d(x,y1,z)
       pix3d(x,y2,z)
    end
    for y=y1+1,y2-1 do
       pix3d(x1,y,z)
       pix3d(x2,y,z)
    end
end

function colonne(x,y,h)
   col("white")
   carre(x-1,y-1,x+1,y+1,0)
   carre(x-1,y-1,x+1,y+1,h)
   col("pink")
   for z=1,h-1 do
      pix3d(x,y,z)
   end
end


function arbre(x,y,h)
   col("brown")
   for z=0,h-2 do
      pix3d(x,y,z)
   end
   col("green")
   for z=h-2,h do
     carre(x-1,y-1,x+1,y+1,z)
   end
end

 
function GLUP.draw_scene() 
 
    GLUP.Enable(GLUP.DRAW_MESH) 
    GLUP.SetCellsShrink(0.1) 
    pixGrid() 
 
    pixBegin() 
    col("blue") 
    for z=0,5 do
      carre(2,2,10,10,z)
    end 
    col("red")
    for z=5,10 do
      d = z-5
      carre(1+d,1+d,11-d,11-d,z)
    end
    for x=1,30,5 do
       colonne(x,15,3)
    end
    for x=1,30,5 do
       z = math.sin(x/3+GLUP.ElapsedTime()*3)*4+10
       arbre(x,20,z)
    end
    pixEnd() 
end 
