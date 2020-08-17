require("turtle")

a=30
level=5
r=0
lr=0.3
threed=false

function feuille()
    local h=30/math.exp(level*lr)
    pwidth(h/3)
    pcol(0,1,0)
    local a = math.random(-r,r)
    tr(a)
    fd(h)
    tr(180)
    pwidth(0.001)
    pcol(0,0,0)
    fd(h)
    tl(180)
    tl(a)
end

function arbre(i)
   local b = a + math.random(-r,r)
   local h = 30*math.exp(i*lr)/math.exp(level*lr)
   pwidth(h*0.05)
   pcol(0,i/level,1-i/level)   
   fd(h)
   if i>1 then
      tl(b)
      arbre(i-1)
      tr(b)
      tr(b)
      arbre(i-1)
      tl(b)
   else
      feuille()
   end
   tr(180)
   fd(h)
   tl(180)
end
 
function GLUP.draw_scene()
   if threed then
      GLUP.Enable(GLUP.LIGHTING)
   else
      GLUP.Disable(GLUP.LIGHTING)
   end
   math.randomseed(0)
   home()
   pcol("green")

   if threed then
       GLUP.MatrixMode(GLUP.MODELVIEW_MATRIX)
       GLUP.PushMatrix()
       GLUP.Rotate(os.clock()*50,0,1,0)
   end

   tr(180)
   fd(20)
   tr(180)
   bk(30)

   pd()
   arbre(level)
   pu()

   if threed then
       for i=1,3,1 do
	   GLUP.Rotate(120,0,1,0)
	   pd()
	   arbre(level)
	   pu()
       end
       GLUP.PopMatrix()
    end
end

function imgui.draw_object_properties()
   local b
   b,a = imgui.SliderFloat("angle", a, 0, 180, "%3f", 1.0)
   b,r = imgui.SliderInt("rnd", r, 0, 100, "%.0f")   
   b,level = imgui.SliderInt("level", level, 1, 15, "%.0f")
   b,lr = imgui.SliderFloat("ratio", lr, 0.1, 0.5, "%3f", 1.0)
   b,threed = imgui.Checkbox("3D",threed)
end
