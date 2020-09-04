require("pixel") 

-- Petit jeu de type Labyrinthe 
-- Par Laszlo Schaffer & Nathan Levy

-- Definir la taille du terrain
function GLUP.init_graphics() 
   GLUP.SetRegionOfInterest(1,1,1,21,21,1) 
end 

-- Est-ce que le joueur a les clefs ?
joueurk=0
joueurb=0
joueurp=0
joueurr=0

-- Deplacement que le joueur veut faire
dx=0
dy=0

-- Dernier moment ou on a appuye sur une touche
last_time=0

-- Delai de repetition des touches
delta_time=0.1

-- Temps de demarrage du programme
init_time=GLUP.ElapsedTime()

niv=0
niveau = {}

function niveau1() 
  niveau = {
   "********************",
   "*              D  O*",
   "* *********b********",
   "* *     *   *****B**",
   "* * *** * *     * **",
   "* * *K* * ***** * **",
   "* *   * *     * * **",
   "* ***** * ***** * **",
   "*       * *       **",
   "*********r**********",
   "**                 *",
   "** *****************",
   "*                  *",
   "* **************** *",
   "*  R*   p  *       *",
   "***** **** *       *",
   "*        * ******  *",
   "* **** *** ****   **",
   "* *P   *        ****",
   "********************"
  }
end

function niveau2()
  niveau = {
   "********************",
   "*        b         *",
   "* **************** *",
   "* *       D      * *",
   "* * ************ * *",
   "* * *    r     * * *",
   "* * * ******** * *B*",
   "* * * *      * * * *",
   "* * * * xxxx * * * *",
   "* * * * xO x * * * *",
   "* * * *    x * * * *",
   "* * * ****** * * * *",
   "* * *        * * * *",
   "* * ********** * * *",
   "* *     R      * * *",
   "* ************** * *",
   "*      K         *p*",
   "****************** *",
   "* P                *",
   "********************"
   }
end

function niveau3()
  niveau = {
   "********************",
   "*O* * * * * * * * **",
   "*D            x    *",
   "*x*x*x*x*x*x* * * **",
   "*             x x  *",
   "*x* *x*x* *x* * * **",
   "*   x   x xBx xKx  *",
   "*b*x* * *x* * *x* **",
   "*     x     x      *",
   "*x*r*x*x*x*x*x*x*x**",
   "*                  *",
   "*x*x*x*p*x*x*x*x* **",
   "*P      x      Rx  *",
   "*x*x* *x* *x*x*x* **",
   "*     x x          *",
   "* * * * * * * * * **",
   "********************"
   }
end

function niveau4()
  niveau = {
   "********************",
   "*                  *",
   "*  E            E  *",
   "*                  *",
   "*                  *",
   "*             O    *",
   "*                  *",
   "*                  *",
   "*     M N Q S      *",
   "*                  *",
   "*                  *",
   "*                  *",
   "*                  *",
   "*  E            E  *",
   "*                  *",
   "*                  *",
   "********************"
   }
end

function niveau5()
  niveau = {
   "********************",
   "*                  *",
   "* xxxxxxxxxxxxxxxx *",
   "* x       x      x *",
   "* x xxxxxxx xxxx x *",
   "* x x     x x    x *",
   "* x   x x x x xxxx *",
   "* xxxxx x   x    x *",
   "* x      xxxxxxx x *",
   "* x****xOxxx   x x *",
   "*  *    x   *  * x *",
   "*  *  x * x ** * x *",
   "*  ** x * x * ** x *",
   "*  *  x * x *  * x *",
   "*    xx   x      x *",
   "*xxxxxxxxxxxxxxxxx *",
   "*                  *",
   "********************"
   }
end

function niveau6()
  niveau = {
   "********************",
   "*                  *",
   "*    ***  *****    *",
   "*    *      *      *",
   "*    **     *      *",
   "*    *      *      *",
   "*    ***    *      *",
   "*                * *",
   "* *  * *** *  *  * *",
   "* ** * * * ** *  * *",
   "* * ** * * * **    *",
   "* *  * *** *  *  * *",
   "*                  *",
   "*  *    ***  *     *",
   "*  *    * *  *     *",
   "*  ***  ***  ***   *",
   "********************"
   }
end

function niveausuivant()
   niv = niv+1
   if niv == 1 then
       niveau1()
   end

   if niv == 2 then
       niveau2()
   end

   if niv == 3 then
       niveau3()
   end

   if niv == 4 then
       niveau4()
   end
   if niv == 5 then
       niveau5()
   end
   if niv == 6 then
       niveau6()
   end

   joueurk=0
   joueurb=0
   joueurp=0
   joueurr=0
   joueurx=2
   joueury=2
   niveauH=#niveau 
   niveauL=#niveau[1] 
end

niveausuivant()
 
function getniveau(x,y) 
   return niveau[#niveau-y+1]:sub(x,x) 
end 
 
function setniveau(x,y,c) 
    local s = niveau[#niveau-y+1] 
    s = s:sub(1,x-1) .. c .. s:sub(x+1) 
    niveau[#niveau-y+1] = s 
end 

function line(x1,x2,y)
   for x=x1,x2 do
      pix3d(x,y,-1)
   end 

end

function dessinekey(x,y,couleur)
   GLUP.PushMatrix()
   GLUP.Translate(x+0.5,y+0.5,0.5)
   local alpha = (GLUP.ElapsedTime() - init_time)*200.0
   GLUP.Rotate(alpha,1,1,1)
   GLUP.Scale(0.7,0.7,0.7)
   GLUP.Translate(-0.5,-0.5,0)      
   GLUP.SetColor(GLUP.FRONT_COLOR,couleur)
   GLUP.Begin(GLUP.SPHERES)
   GLUP.Vertex(0.5, 0.65, 0, 0.25)
   GLUP.End()
   GLUP.SetColor(GLUP.MESH_COLOR,couleur)
   GLUP.Begin(GLUP.LINES)
   GLUP.Vertex(0.5,  0.5,  0)
   GLUP.Vertex(0.5,  0,    0)
   GLUP.Vertex(0.5,  0,    0)
   GLUP.Vertex(0.75, 0,    0)
   GLUP.Vertex(0.5,  0.25, 0)
   GLUP.Vertex(0.75, 0.25, 0)
   GLUP.End()
   GLUP.PopMatrix()
end

function dessineporte(x,y,couleur)
   pixBegin()
   col(couleur)
   pix(x,y)
   pixEnd() 
end

function dessineEpee(x,y)
   GLUP.PushMatrix()
   GLUP.Translate(x-0.1,y-0.1,0.5)
   GLUP.Scale(0.1,0.1,0.1)

   GLUP.Translate(0.5,0.5,0.5)
   local alpha = (GLUP.ElapsedTime() - init_time)*200.0   
   GLUP.Rotate(alpha,1,1,0)
   GLUP.Translate(-0.5,-0.5,-0.5)

   pixBegin()
   col("gray")
   for i=1,10 do
      pix(i,i)
   end
   col("white")
   for i=3,9 do
      pix(i,i+1)
   end
   col("black")
   for i=4,10 do
      pix(i,i-1)
   end
   pix(2,3)
   pix(1,4)
   pix(3,2)
   pix(4,1)
   pixEnd()
   GLUP.PopMatrix()
end

function dessineFantome(x,y,c)
    vx = (joueurx - x) / 10.0
    vy = (joueury - y) / 10.0
    GLUP.Enable(GLUP.VERTEX_COLORS)
    GLUP.Begin(GLUP.SPHERES) 
    col(c) 
    GLUP.Vertex(x+0.5, y+0.5, 0.5, 0.5) 
    for xx=x+0.2,x+0.8,0.3 do 
        GLUP.Vertex(xx,y+0.1,0.5,0.3) 
    end 
    col("white") 
    GLUP.Vertex(x+0.35+0.3*vx, y+0.5+0.3*vy, 1.0, 0.15) 
    GLUP.Vertex(x+0.65+0.3*vx, y+0.5+0.3*vy, 1.0, 0.15) 
    col("black")
    GLUP.Vertex(x+0.35+0.32*vx, y+0.5+0.32*vy, 1.1, 0.1) 
    GLUP.Vertex(x+0.65+0.32*vx, y+0.5+0.32*vy, 1.1, 0.1) 
    GLUP.End() 
    GLUP.Disable(GLUP.VERTEX_COLORS) 
end 


function dessineniveau()
    GLUP.Disable(GLUP.DRAW_MESH) 
    GLUP.SetCellsShrink(0)

    -- pixGrid()
    
    -- dessin du terrain et des blocs verts
    
    pixBegin() 
    col("green") 

    for y=1,niveauH do
       line(1,niveauL,y)
    end

   for y=1,niveauH do
      for x=1, niveauL do
         -- regarder ce qu'il y a dans la case x,y
         local c = getniveau(x,y)
         if c == '*' then
             GLUP.Color(0,0.5,0)
             pix(x,y)
         end

         if c == 'O' then
            col("yellow")
            pix(x,y)
         end
 
       end
   end

   pixEnd()

   -- dessins des autres objets (clefs et portes)

   for y=1,niveauH do
      for x=1, niveauH do
         -- regarder ce qu'il y a dans la case x,y
         local c = getniveau(x,y)
         if c == 'K' then
            dessinekey(x,y,"red")
         end

         if c == 'B' then
            dessinekey(x,y,"blue")
         end

         if c == 'P' then
            dessinekey(x,y,"black")
         end
         
         if c == 'R' then
            dessinekey(x,y,"pink")
         end

         if c == 'D' then
            dessineporte(x,y,"red")    
         end

         if c == 'b' then
            dessineporte(x,y,"blue")
         end

         if c == 'p' then
            dessineporte(x,y,"black")
         end

         if c == 'r' then
            dessineporte(x,y,"pink")
         end

         if c == 'E' then
	    dessineEpee(x,y)
	 end

         if c == 'M' then
	    dessineFantome(x,y,"red")
	 end

         if c == 'N' then
	    dessineFantome(x,y,"blue")
	 end

         if c == 'Q' then
	    dessineFantome(x,y,"pink")
	 end

         if c == 'S' then
	    dessineFantome(x,y,"yellow")	 
	 end


      end

   end
 
end

-- parametres: x,y = la ou on veut dessiner le "pacman"
--             vx,vy = direction vers ou le pacman regarde
function pacman(x,y,vx,vy)
    GLUP.Enable(GLUP.VERTEX_COLORS) 
    GLUP.Begin(GLUP.SPHERES) 
    col("yellow") 
    GLUP.Vertex(x+0.5, y+0.5, 0.5, 0.5) 
    col("white") 
    GLUP.Vertex(x+0.35+0.25*vx, y+0.5+0.25*vy, 1.0, 0.15) 
    GLUP.Vertex(x+0.65+0.25*vx, y+0.5+0.25*vy, 1.0, 0.15) 
    col("black") 
    GLUP.Vertex(x+0.35+0.27*vx, y+0.5+0.27*vy, 1.1, 0.1) 
    GLUP.Vertex(x+0.65+0.27*vx, y+0.5+0.27*vy, 1.1, 0.1) 
    GLUP.End() 
    GLUP.Disable(GLUP.VERTEX_COLORS)
end

function inventaire()
    pixBegin()
    col("gray")
    pix(1,-1)
    pix(2,-1)
    pix(0,-1)
    pix(0,-2)
    pix(2,-2)
    pix(2,-3)
    pix(0,-3)
    pix(1,-3)
    pix(3,-3)
    pix(4,-3)
    pix(4,-2)
    pix(4,-1)
    pix(3,-1)
    pix(2,-1)
    pix(5,-3)
    pix(6,-3)
    pix(6,-2)
    pix(6,-1)
    pix(5,-1)
    pix(7,-1)
    pix(8,-1)
    pix(8,-2)
    pix(8,-3)
    pix(7,-3)

    pixEnd()

   if joueurk == 1 then  
      dessinekey(7,-2,"red")  
   end

   if joueurb == 1 then
      dessinekey(5,-2,"blue")
   end

   if joueurp == 1 then
      dessinekey(1,-2,"black")
   end

   if joueurr == 1 then
      dessinekey(3,-2,"pink")
   end

end 

function imgui.on_key_pressed(k)
  last_time = GLUP.ElapsedTime() - delta_time
  dx = 0
  dy = 0

  if k == "left" then
     dx = -1
     dy = 0 
  end

  if k == "right" then
     dx = 1
     dy = 0        
  end

  if k == "up" then
     dx = 0
     dy = 1       
  end

  if k == "down" then
     dx = 0
     dy = -1             
  end

end

function imgui.on_key_released(k)
   dx = 0
   dy = 0
end

function player_move(dx, dy)
  -- regarder ce qu il y a la ou on veut aller
  local c = getniveau(joueurx+dx, joueury+dy)
  if c == " " then
     joueurx = joueurx + dx
     joueury = joueury + dy
  end

  if c == 'K' then
     joueurx = joueurx + dx
     joueury = joueury + dy
     setniveau(joueurx,joueury,' ')
     joueurk=1
  end

  if c == 'B' then
     joueurx = joueurx + dx
     joueury = joueury + dy
     setniveau(joueurx,joueury,' ')
     joueurb=1
  end

  if c == 'R' then
     joueurx = joueurx + dx
     joueury = joueury + dy
     setniveau(joueurx,joueury,' ')
     joueurr=1
  end

  if c == 'P' then
     joueurx = joueurx + dx
     joueury = joueury + dy
     setniveau(joueurx,joueury,' ')
     joueurp=1
  end

  if c == 'D' and joueurk == 1 then
     joueurx = joueurx + dx
     joueury = joueury + dy
     setniveau(joueurx,joueury,' ')
  end

  if c == 'b' and joueurb == 1 then
     joueurx = joueurx + dx
     joueury = joueury + dy
     setniveau(joueurx,joueury,' ')
  end

  if c == 'p' and joueurp == 1 then
     joueurx = joueurx + dx
     joueury = joueury + dy
     setniveau(joueurx,joueury,' ')
  end

  if c == 'r' and joueurr == 1 then
     joueurx = joueurx + dx
     joueury = joueury + dy
     setniveau(joueurx,joueury,' ')
  end

  if c == 'O' then
     niveausuivant()
  end

end

function player_update()
   local t = GLUP.ElapsedTime()
   if t - last_time > delta_time then
      last_time = t
      player_move(dx,dy)
   end
end

function GLUP.draw_scene() 
    dessineniveau()
    pacman(joueurx,joueury,dx,dy)
    player_update()
    inventaire()
end 
