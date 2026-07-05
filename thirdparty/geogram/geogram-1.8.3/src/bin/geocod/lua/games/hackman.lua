require("pixel") 
 
GLUP.ArcadeStyle() 
 
joueurx=13 
joueury=10 
vx=0 
vy=0 
newvx=0 
newvy=0 
lasttime=0 
lastfantomtime=0 
 
gumtime=0 
gummode=false 
eatentime=0 
eatenmode=false 
frozen=true 
gameover=false 
 
fantomx=13 
fantomy=15 
fantomvx=0 
fantomvy=1 
 
 
score=0 
level=1 
lives=3 
 
niveau = { 
"*************************", 
"*...........*...........*", 
"*.****.****.*.****.****.*", 
"*O*  *.*  *.*.*  *.*  *O*", 
"*.****.****.*.****.****.*", 
"*.......................*", 
"*.****.*.*******.*.****.*", 
"*......*....*....*......*", 
"******.**** * ****.******", 
"     *.*         *.*     ", 
"******.* ******* *.******", 
"      .  *     *  .      ", 
"******.* *     * *.******", 
"     *.* ******* *.*     ", 
"     *.*         *.*     ", 
"******.* ******* *.******", 
"*...........*...........*", 
"*.****.****.*.****.****.*", 
"*O...*.............*...O*", 
"****.*.*.*******.*.*.****", 
"*......*....*....*......*", 
"*.*********.*.*********.*", 
"*.......................*", 
"*************************" 
} 
 
niveauH=#niveau 
niveauL=#niveau[1] 
 
function youwin() 
    niveau = { 
    "O.O.O.O.O.O.O.O.O.O.O.O.O", 
    ".                       .", 
    "O  *   *    ***   *  *  O", 
    ".  *   *   *   *  *  *  .", 
    "O  *   *   *   *  *  *  O", 
    ".   * *    *   *  *  *  .", 
    "O    *     *   *  *  *  O", 
    ".    *     *   *  *  *  .", 
    "O    *     *   *  *  *  O", 
    ".    *      ***    **   .", 
    "O                       O", 
    ".                       .", 
    "O                       O", 
    ".  * * *  ***  **    *  .", 
    "O  * * *   *   * *   *  O", 
    ".  * * *   *   * *   *  .", 
    "O  * * *   *   *  *  *  O", 
    ".  * * *   *   *  *  *  .", 
    "O  * * *   *   *   * *  O", 
    ".  * * *   *   *   * *  .", 
    "O   ***   ***  *    **  O", 
    ".                       .", 
    "O.O.O.O.O.O.O.O.O.O.O.O.O" 
    } 
    niveauH=#niveau 
    niveauL=#niveau[1] 
    gameover=true 
end 
 
function youlose() 
    niveau = { 
    "O.O.O.O.O.O.O.O.O.O.O.O.O", 
    ".                       .", 
    "O       *********       O", 
    ".      * ... ... *      .", 
    "O     *   O   O   *     O", 
    ".    *             *    .", 
    "O    *  .       .  *    O", 
    ".    *   .......   *    .", 
    "O    *             *    O", 
    ".    * **  **  **  *    .", 
    "O    **  **  **  ***    O", 
    ".                       .", 
    "O                       O", 
    ".  ****    ***    ***   .", 
    "O  *   *  *   *  *   *  O", 
    ".  *   *  *   *  *   *  .", 
    "O  ****   *   *  *   *  O", 
    ".  *   *  *   *  *   *  .", 
    "O  *   *  *   *  *   *  O", 
    ".  *   *  *   *  *   *  .", 
    "O  ****    ***    ***   O", 
    ".                       .", 
    "O.O.O.O.O.O.O.O.O.O.O.O.O" 
    } 
    niveauH=#niveau 
    niveauL=#niveau[1] 
    gameover=true 
end 
 
 
function getniveau(x,y) 
   return niveau[#niveau-y+1]:sub(x,x) 
end 
 
function setniveau(x,y,c) 
    local s = niveau[#niveau-y+1] 
    s = s:sub(1,x-1) .. c .. s:sub(x+1) 
    niveau[#niveau-y+1] = s 
end 
 
function nbballs() 
    local result = 0 
    for y=1,niveauH,1 do 
	for x=1,niveauL,1 do 
	    if getniveau(x,y) == "." then 
		result = result + 1 
	    end 
	end 
    end 
    return result 
end 
 
niveauBalls = nbballs() 
 
function terrain() 
   pixBegin() 
   col("blue") 
   for y=1,niveauH,1 do 
      for x=1,niveauL,1 do 
	  if getniveau(x,y) == "*" then 
	      pix(x,y)	 
	  end 
      end 
   end 
   pixEnd() 
   GLUP.SetColor(GLUP.FRONT_COLOR,"white") 
   local Rgum = ((GLUP.ElapsedTime()*5)%2)*0.2+0.1 
   GLUP.Begin(GLUP.SPHERES) 
   for y=1,niveauH,1 do 
      for x=1,niveauL,1 do 
          local c = getniveau(x,y) 
	  if c == "." then 
	      GLUP.Vertex(x+0.5,y+0.5,0.5,0.2) 
	  elseif c == "O" then 
	      GLUP.Vertex(x+0.5,y+0.5,0.5,Rgum)	 
	  end 
      end 
   end 
   GLUP.End() 
end 
 
function GLUP.init_graphics() 
   GLUP.SetRegionOfInterest(1,1,1,niveauH,niveauH,1) 
end 
 
function imgui.on_key_pressed(key) 
   frozen = false 
   if key == "left" then 
      newvx=-1 
      newvy=0 
   elseif key == "right" then 
      newvx=1 
      newvy=0 
   elseif key == "up" then 
      newvx=0 
      newvy=1 
   elseif key == "down" then 
      newvx=0 
      newvy=-1 
   end 
end 
 
function dessinjoueur(x,y,vx,vy) 
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
 
function dessinfantome(x,y,vx,vy,c) 
    GLUP.Enable(GLUP.VERTEX_COLORS) 
    GLUP.Begin(GLUP.SPHERES) 
    if not eatenmode then 
        if gummode then 
	   col("blue") 
	else 
	   col(c) 
        end 
        GLUP.Vertex(x+0.5, y+0.5, 0.5, 0.5) 
        for xx=x+0.2,x+0.8,0.3 do 
            GLUP.Vertex(xx,y+0.1,0.5,0.3) 
        end 
    end 
    col("white") 
    GLUP.Vertex(x+0.35+0.3*vx, y+0.5+0.3*vy, 1.0, 0.15) 
    GLUP.Vertex(x+0.65+0.3*vx, y+0.5+0.3*vy, 1.0, 0.15) 
    if eatenmode or not gummode then 
        col("black") 
    	GLUP.Vertex(x+0.35+0.32*vx, y+0.5+0.32*vy, 1.1, 0.1) 
        GLUP.Vertex(x+0.65+0.32*vx, y+0.5+0.32*vy, 1.1, 0.1) 
    end 
    GLUP.End() 
    GLUP.Disable(GLUP.VERTEX_COLORS) 
end 
 
function killed() 
   frozen=true 
   joueurx=13 
   joueury=10 
   vx=0 
   vy=0 
   newvx=0 
   newvy=0 
   lasttime=0 
   lastfantomtime=0 
 
   gumtime=0 
   gummode=false 
   eatentime=0 
   eatenmode=false 
 
   fantomx=13 
   fantomy=15 
   fantomvx=0 
   fantomvy=1 
   lives = lives - 1 
   if lives == 0 then 
       youlose() 
   end 
end 
 
function collision(time) 
   if joueurx == fantomx and joueury == fantomy then 
      if gummode then 
          score = score + 100 
          eatenmode = true 
          eatentime = time 
       elseif not eatenmode then 
          killed() 
       end 
   end 
end 
 
function joueur() 
 
    local time = GLUP.ElapsedTime() 
 
    if gummode then 
	if (time - gumtime) > 5 then 
	    gummode = false 
	end 
    end 
 
    if not frozen and time-lasttime > 0.075 then 
	lasttime = time 
	local newx,newy 
	newx = joueurx+newvx 
	newy = joueury+newvy 
	if newx < 1 then 
	    newx = niveauL 
        elseif newx > niveauL then 
	    newx = 1 
	end 
	if newy < 1 then 
	    newy = niveauH 
        elseif newy > niveauH then 
	    newy = 1 
	end 
	local c = getniveau(newx,newy) 
	if c == " " or c == "." or c == "O" then 
	    joueurx = newx 
	    joueury = newy 
	    vx = newvx 
	    vy = newvy 
        else 
	    newx = joueurx + vx 
	    newy = joueury + vy 
	    c = getniveau(newx,newy) 
	    if c == " " or c == "." or c == "O" then 
	        joueurx = newx 
		joueury = newy 
	    end 
	end 
	if c == "O" then 
	    gumtime = time 
	    gummode = true 
	    fantomvx = -fantomvx 
	    fantomvy = -fantomvy 
	elseif c == "." then 
	    score = score + 10 
	    niveauBalls = niveauBalls - 1 
	    if niveauBalls == 0 then 
		youwin() 
	    end 
	end 
	setniveau(joueurx, joueury, " ") 
    end 
    collision(time) 
    dessinjoueur(joueurx,joueury,vx,vy) 
end 
 
function fantome() 
    local time = GLUP.ElapsedTime() 
    local delay = 0.1 
    if gummode then 
       delay = 0.2 
    end 
    if eatenmode and (time - eatentime) > 5 then 
       eatenmode = false 
    end 
    if not frozen and time-lastfantomtime > delay then 
	lastfantomtime = time 
        while getniveau(fantomx + fantomvx, fantomy + fantomvy) ~= " " do 
	   local v = math.random(1,4) 
	   if v == 1 then 
	      fantomvx = -1 
	      fantomvy = 0 
	   elseif v == 2 then 
	      fantomvx = 1 
	      fantomvy = 0 
	   elseif v == 3 then 
	      fantomvx = 0 
	      fantomvy = -1 
	   elseif v == 4 then 
	      fantomvx = 0 
	      fantomvy = 1 
	   end 
	end 
	fantomx = fantomx + fantomvx 
        fantomy = fantomy + fantomvy 
    end 
    collision(time) 
    dessinfantome(fantomx,fantomy,fantomvx,fantomvy,"pink") 
end 
 
function GLUP.draw_scene() 
   GLUP.Enable(GLUP.DRAW_MESH) 
   GLUP.SetColor(GLUP.MESH_COLOR, 0,0,0.5) 
   GLUP.SetMeshWidth(2) 
   terrain() 
   if not gameover then 
       joueur() 
       fantome() 
   end 
end 
 
function imgui.draw_object_properties() 
   imgui.Text("Level: " .. level) 
   imgui.Text("Score: " .. score) 
   imgui.Text("Lives: " .. lives) 
end 
