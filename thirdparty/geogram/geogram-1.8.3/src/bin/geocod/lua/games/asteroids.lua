
require("pixel") 

-- EN A simple scrolling shoot-em-up
-- EN Collect the yellow ones, avoid the red ones
-- FR Un jeu de tir a deplacement vertical
-- FR Prenez les jaunes, evitez les rouges.

niveau = {
     "...................", 
     ". ... .. ... . .. *", 
     " . .. .  . . .  .  ", 
     " .  .  .   .  .  . ", 
     "      .       .    ", 
     "   .   .   .   .   ", 
     "                   ", 
     "                   ", 
     "                   ", 
     "                   ", 
     "     -     -       ", 
     "   / |`  / |`      ", 
     "   | | | | | |     ", 
     "   | | | | | |     ", 
     "   | | | - | |     ", 
     "   | | | | | |     "
}  

game = true

niveauH=#niveau 
niveauL=#niveau[1] 

level=1
score=0
lives=1
delay=0.1
started = false
thrust = false
laser = false

function imgui.draw_object_properties() 
   imgui.Text("Level: " .. level) 
   imgui.Text("Score: " .. score) 
   imgui.Text("Lives: " .. lives) 
end 


function gameover() 
   game = false
   niveau = {
     "|||||||||||||||||||",
     " ... .. . .. ... . ",
     " OO         . ..   ",
     "O     OOO OOOOO OOO",
     "O OO  O O O O O OO ",
     "O  O  OOO O   O O  ",
     " OOO  O O O   O OOO",
     " .     . ... .     ", 
     "  OO .         . . ",
     " O..O O O OOO OOO  ",
     " O .O O O OO  O O  ",
     " O. O O O O . OO   ",
     "  OO   O .OOO O O  ",
     " .. . ... ... . .. ",
     "|||||||||||||||||||"
   }
   niveauH=#niveau 
   niveauL=#niveau[1] 
end



function avance()
   if not started then
      return
   end
   for y=niveauH-1,1,-1 do
      niveau[y+1] = niveau[y]
   end
   for x=1,niveauL do
      local rnd = math.random(1,niveauL)
      if rnd == 1 then
          setniveau(x,niveauH,'O')
      elseif rnd == 2 then
          setniveau(x,niveauH,'*')
      elseif rnd < 4 then
          setniveau(x,niveauH,'.')
      elseif rnd < 8 then
          setniveau(x,niveauH,'o')
      else
          setniveau(x,niveauH,' ')
      end
      score=score+1
   end
end

function getniveau(x,y) 
   return niveau[#niveau-y+1]:sub(x,x) 
end 
 
function setniveau(x,y,c) 
    local s = niveau[#niveau-y+1] 
    s = s:sub(1,x-1) .. c .. s:sub(x+1) 
    niveau[#niveau-y+1] = s 
end 

function etoile(x,y,dx,dy)
   GLUP.Color("white")
   GLUP.Vertex(x+0.5+dx, y+0.5+dy, 0.0, 0.1)
end

function asteroide(x,y,coul)
   GLUP.Color(coul)
   GLUP.Vertex(x+0.5, y+0.5, 0.0, 0.45)
   GLUP.Vertex(x+0.8, y+0.8, 0.0, 0.1)
   GLUP.Vertex(x+0.2, y+0.5, 0.3, 0.1)
   GLUP.Vertex(x+0.7, y+0.4, 0.3, 0.1)
end


function barre(x,y,c)
    GLUP.Color("green")
    if c == '|' then
       GLUP.Vertex(x+0.5, y,      0, 0.1)
       GLUP.Vertex(x+0.5, y+0.25, 0, 0.1)
       GLUP.Vertex(x+0.5, y+0.5,  0, 0.1)
       GLUP.Vertex(x+0.5, y+0.75, 0, 0.1)
       GLUP.Vertex(x+0.5, y+1   , 0, 0.1)
    elseif c == '-' then
       GLUP.Vertex(x,      y+0.5, 0, 0.1)
       GLUP.Vertex(x+0.25, y+0.5, 0, 0.1)
       GLUP.Vertex(x+0.5,  y+0.5, 0, 0.1)
       GLUP.Vertex(x+0.75, y+0.5, 0, 0.1)
       GLUP.Vertex(x+1,    y+0.5, 0, 0.1)
    elseif c == '/' then
       GLUP.Vertex(x+0.75,  y+0.25, 0, 0.1)
       GLUP.Vertex(x+1, y+0.5, 0, 0.1)
       GLUP.Vertex(x+1.25,    y+0.75, 0, 0.1)
       GLUP.Vertex(x+1.5, y+1, 0, 0.1)
       GLUP.Vertex(x+1.75,  y+1.25, 0, 0.1)
    elseif c == '`' then
       GLUP.Vertex(x+0.25, y+1.25, 0, 0.1)
       GLUP.Vertex(x+0.5,  y+1, 0, 0.1)
       GLUP.Vertex(x+0.75, y+0.75, 0, 0.1)
       GLUP.Vertex(x+1,    y+0.5, 0, 0.1)
       GLUP.Vertex(x+1.25, y+0.25, 0, 0.1)
    end
end

function dessine_niveau()
   GLUP.PushMatrix()
   if game and started then
      GLUP.Translate(0,-(GLUP.ElapsedTime()-t0)/delay,0)
   end
-- pixGrid()
   GLUP.Enable(GLUP.VERTEX_COLORS)
   GLUP.Begin(GLUP.SPHERES)
   for y = 1,niveauH do
      for x = 1,niveauL do
         local c = getniveau(x,y)
         if c == '.' then
            etoile(x,y, 0, 0)
         elseif c == 'o' then
            etoile(x,y, -0.1, 0.2)
         elseif c == 'O' then
            asteroide(x,y,"red")
         elseif c == '*' then
            asteroide(x,y,"yellow")
         else
            barre(x,y,c)
         end
      end
   end
   GLUP.Disable(GLUP.VERTEX_COLORS)
   GLUP.End()
   GLUP.PopMatrix() 
   pixBegin()
   col("blue")
   for x=1,niveauL do
      pix(x,0,0)
      pix(x,niveauH,0)
   end
   pixEnd()
end

function GLUP.init_graphics() 
   GLUP.ArcadeStyle()
   GLUP.SetRegionOfInterest(1,1,1,niveauL+1,niveauH+1,1) 
end 

function fusee(x,y)
   GLUP.PushMatrix()

   GLUP.Translate(x+0.5, y+0.5, 0)
   GLUP.Scale(1.0/5.0, 1.0/5.0, 1.0/5.0)
   if started then
      local alpha = (GLUP.ElapsedTime() - torigin) * 200.0 
      GLUP.Rotate(alpha, 0, 1, 0)
   end
   GLUP.Translate(-3.5, -4.5, -0.5)
   pixBegin() 
   col("black") 
   pix(1,1) 
   pix(1,2) 
   pix(3,1) 
   pix(3,2) 
   pix(5,1) 
   pix(5,2) 
  
   col("red") 
   pix(2,3) 
   pix(4,3) 
   pix(3,4) 
   pix(2,5) 
   pix(4,5) 
 
   col("white") 
   pix(2,2) 
   pix(4,2) 
   pix(3,3) 
   pix(2,4) 
   pix(4,4) 
   pix(3,5) 
   pix(2,6) 
   pix(3,6) 
   pix(4,6) 
   pix(3,7) 
   pix(3,8) 
 
   if thrust then
      col("yellow")
      pix(3,-1)
      pix(3,-2)
   end

   if laser then
      col("red")
      pix(3,10)
      pix(3,11)
      col("yellow")
      pix(3,12)
      col("white")
      pix(3,13)

      for y = 15,5*(niveauH-joueury+1),2 do
         pix(3,y)
      end
   end

   pixEnd() 
   GLUP.PopMatrix()
end

joueurx = math.floor(niveauL/2)
joueury = 1

function imgui.on_key_pressed(k)
   started = true
   if k == "left" and joueurx > 1 then
        joueurx = joueurx - 1
   end
   if k == "right" and joueurx < niveauL then
        joueurx = joueurx + 1
   end
   if k == "up" and joueury < niveauH-1 then
        joueury = joueury + 1
        thrust = true
   end
   if k == "down" and joueury > 1 then
        joueury = joueury - 1
   end
   if k == " " then
      laser = true
       for y=joueury,niveauH do
          setniveau(joueurx,y,' ')
       end
   end
end

function imgui.on_key_released()
   thrust = false
   laser = false
end

t0 = GLUP.ElapsedTime()
torigin = t0

function GLUP.draw_scene() 
-- pixGrid()
   GLUP.Enable(GLUP.LIGHTING) 
   GLUP.Disable(GLUP.DRAW_MESH)
   GLUP.SetCellsShrink(0)
   dessine_niveau()
   if game then
      fusee(joueurx,joueury)
      local c = getniveau(joueurx,joueury) 
      if c == 'O' or c == '|' or c == '-' or c == '/' or c == '`' then
         gameover()
      elseif getniveau(joueurx,joueury) == '*' then
         score = score + 100
         setniveau(joueurx, joueury, ' ')
      else
         local t = GLUP.ElapsedTime()
         if t - t0 > delay then
            t0 = t
            avance()
         end
      end
   end
end 
