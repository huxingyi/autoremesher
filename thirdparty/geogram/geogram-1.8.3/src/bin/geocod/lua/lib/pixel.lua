--             pixel.lua 
-- FR bibliotheque interne, incluse quand on appelle require("pixel") 
-- FR fournit des fonctions simples pour afficher des (gros) pixels 
-- EN internal library, included when one calls require("pixel") 
-- EN defines easy-to-use functions for displaying (big) pixels 
 
function GLUP.Pixel3D(x,y,z) 
   GLUP.Vertex(x,y,z) 
   GLUP.Vertex(x+1,y,z) 
   GLUP.Vertex(x,y+1,z) 
   GLUP.Vertex(x+1,y+1,z) 
   GLUP.Vertex(x,y,z+1) 
   GLUP.Vertex(x+1,y,z+1) 
   GLUP.Vertex(x,y+1,z+1) 
   GLUP.Vertex(x+1,y+1,z+1) 
end 
 
function GLUP.Pixel2D(x,y) 
   GLUP.Pixel3D(x,y,0.0) 
end 
 
pix = GLUP.Pixel2D 
pix3d = GLUP.Pixel3D 
col = GLUP.Color 
 
function pixBegin() 
   GLUP.Enable(GLUP.VERTEX_COLORS) 
   GLUP.Begin(GLUP.HEXAHEDRA) 
end 
 
function pixEnd() 
   GLUP.End() 
   GLUP.Disable(GLUP.VERTEX_COLORS) 
end 


GLUP.numbersGeom = {
   {{1/6,1/8}, {2/6,1/8}},
   {{1/6,1/2}, {2/6,1/2}},
   {{1/6,7/8}, {2/6,7/8}},
   {{1/6,1/8}, {1/6,1/2}},
   {{2/6,1/8}, {2/6,1/2}},
   {{1/6,1/2}, {1/6,7/8}},
   {{2/6,1/2}, {2/6,7/8}}
}

GLUP.numbersSeg = {
   0x7D, 0x50, 0x4F, 0x57, 0xF2, 0x37, 0x3F, 0x54, 0x7F, 0x77
}

function GLUP.drawDigit(x,y,n)
   for i=0,6 do
       if (GLUP.numbersSeg[n+1] & (1 << i)) ~= 0 then
	   GLUP.Vertex(GLUP.numbersGeom[i+1][1][1]+x, GLUP.numbersGeom[i+1][1][2]+y)
	   GLUP.Vertex(GLUP.numbersGeom[i+1][2][1]+x, GLUP.numbersGeom[i+1][2][2]+y)
       end
   end
end

function GLUP.drawNumber(x,y,n)
    if n >= 100 then 
    else
        if n < 10 then
	   GLUP.drawDigit(x+0.25,y,math.floor(n))
	else
	   GLUP.drawDigit(x,y,math.floor(n/10))	
	   GLUP.drawDigit(x+0.5,y,math.floor(n%10))
	end
    end   
end

 
function pixGrid() 
   GLUP.SetColor(GLUP.FRONT_COLOR,0,0,0.5) 
   GLUP.Begin(GLUP.LINES) 
   local xm,ym,zm,xM,yM,zM 
   xm,ym,zm,xM,yM,zM = GLUP.GetRegionOfInterest() 
   for x=xm,xM,1 do 
      GLUP.Vertex(x,ym-0.5,0) 
      GLUP.Vertex(x,yM,0) 
   end 
   for y=ym,yM,1 do 
      GLUP.Vertex(xm-0.5,y,0) 
      GLUP.Vertex(xM,y,0) 
   end 
   GLUP.End()
   GLUP.SetColor(GLUP.FRONT_COLOR,0.2,0.2,0.2)
   GLUP.Begin(GLUP.LINES) 
   for x=xm,xM-1,1 do
       GLUP.drawNumber(x,ym-1.2,x)
   end
   for y=ym,yM-1,1 do
       GLUP.drawNumber(xm-1.2,y,y)
   end
   GLUP.End()   
end 
 
function GLUP.init_graphics() 
   GLUP.SetRegionOfInterest(1,1,1,21,21,1) 
end 

function GLUP.draw_scene()
   pixGrid()
end

