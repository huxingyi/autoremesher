require("turtle")

function VonKoch(level)
   if level==1 then
      fd(1)
   else
      VonKoch(level-1)                  
      tl(60)
      VonKoch(level-1)            
      tr(120)
      VonKoch(level-1)      
      tl(60)
      VonKoch(level-1)
   end
end

function Flake(level)
   pd()
   VonKoch(level)
   tr(120)
   VonKoch(level)
   tr(120)
   VonKoch(level)
   pu()
end

function GLUP.draw_scene()
    home()
    fd(40)
    tl(90)
    fd(40)
    tr(90)
    pwidth(0.1)
    tr(90)
    GLUP.Enable(GLUP.VERTEX_COLORS)
    GLUP.Disable(GLUP.LIGHTING)
    pcol("gray")
    Flake(5)
end
