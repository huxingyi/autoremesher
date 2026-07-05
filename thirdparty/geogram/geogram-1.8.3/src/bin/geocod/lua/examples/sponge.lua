require("pixel")

N=3*3

function sponge(x,y,z,d)
  if d < 1 then
     pix3d(x,y,z)
  else
     sponge(x,y,z,d/3)
     sponge(x+d,y,z,d/3)
     sponge(x+2*d,y,z,d/3)
     sponge(x,y+d,z,d/3)
     sponge(x+2*d,y+d,z,d/3)
     sponge(x,y+2*d,z,d/3)
     sponge(x+d,y+2*d,z,d/3)
     sponge(x+2*d,y+2*d,z,d/3)
     sponge(x,y,z+d,d/3)
     sponge(x+2*d,y,z+d,d/3)
     sponge(x,y+2*d,z+d,d/3)
     sponge(x+2*d,y+2*d,z+d,d/3)     
     sponge(x,y,z+2*d,d/3)
     sponge(x+d,y,z+2*d,d/3)
     sponge(x+2*d,y,z+2*d,d/3)
     sponge(x,y+d,z+2*d,d/3)
     sponge(x+2*d,y+d,z+2*d,d/3)
     sponge(x,y+2*d,z+2*d,d/3)
     sponge(x+d,y+2*d,z+2*d,d/3)
     sponge(x+2*d,y+2*d,z+2*d,d/3)
  end
end

function GLUP.init_graphics()
  GLUP.SetRegionOfInterest(1,1,1,N*3,N*3,N*3)
end

function GLUP.draw_scene()
  GLUP.Enable(GLUP.DRAW_MESH)
  pixBegin()
  col("yellow")
  sponge(1,1,1,N)
  pixEnd()
end
