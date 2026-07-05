--             turtle.lua
-- FR bibliotheque interne, incluse quand on appelle require("turtle")
-- FR fournit des fonctions pour l'affichage graphique en mode "tortue"
-- EN internal library, included when one calls require("turtle")
-- EN defines functions for "turtle" graphics

turtle={}

function turtle.pendown()
   if not turtle.pen then
      turtle.pen=true
      turtle.first=true
      GLUP.Enable(GLUP.VERTEX_COLORS)
      GLUP.Begin(GLUP.QUADS)
   end
end

function turtle.penup()
   if turtle.pen then
      turtle.pen=false
      GLUP.End()
      GLUP.Disable(GLUP.VERTEX_COLORS)
   end
end

function turtle.forward(dist)
   local new_x = turtle.x + math.cos(turtle.alpha)*dist
   local new_y = turtle.y + math.sin(turtle.alpha)*dist
   if turtle.pen then
      local Nx = turtle.width * (turtle.y - new_y) / dist
      local Ny = turtle.width * (new_x - turtle.x) / dist
      GLUP.Vertex(turtle.x+Nx,turtle.y+Ny,0)
      GLUP.Vertex(turtle.x-Nx,turtle.y-Ny,0)      
      GLUP.Vertex(new_x-Nx, new_y-Ny,0)
      GLUP.Vertex(new_x+Nx, new_y+Ny,0)
   end
   turtle.x = new_x
   turtle.y = new_y
end

function turtle.backward(dist)
   turtle.alpha = turtle.alpha + math.pi
   turtle.forward(dist)
   turtle.alpha = turtle.alpha - math.pi   
end

function turtle.left(dalpha)
   turtle.alpha = turtle.alpha + dalpha * math.pi / 180
end

function turtle.right(dalpha)
   turtle.alpha = turtle.alpha - dalpha * math.pi / 180
end

function turtle.home()
   turtle.x=0
   turtle.y=0
   turtle.alpha=math.pi/2
   if turtle.pen then
      turtle.penup()
   end
   turtle.width=0.2
   turtle.first=true
end

function turtle.penwidth(w)
   turtle.width = w
end

turtle.pencolor = GLUP.Color

-- FR fonctions "raccourcis"
-- FR (ca va plus vite a taper !)
-- EN shorthands

pu = turtle.penup
pd = turtle.pendown
fd = turtle.forward
bk = turtle.backward
tl = turtle.left
tr = turtle.right
home = turtle.home
pcol = turtle.pencolor
pwidth = turtle.penwidth

turtle.pu = turtle.penup
turtle.pd = turtle.pendown
turtle.fd = turtle.forward
turtle.bk = turtle.backward
turtle.tl = turtle.left
turtle.tr = turtle.right
turtle.pcol = turtle.pencolor
turtle.pwidth = turtle.penwidth

function GLUP.init_graphics()
   GLUP.SetRegionOfInterest(-50,-50,1,50,50,1)
end

