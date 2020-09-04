require("pixel") 

-- Simple Space Invaders game
-- By Alicia Levy

function GLUP.init_graphics() 
   GLUP.SetRegionOfInterest(1,1,1,11,11,1) 
end 

oldtime=GLUP.ElapsedTime()

x=5
y=1
missile=false
missileX=x
missileY=y+1

alien=true
alienTime=0
alienX=5
alienY=10

extra=true
extraTime=0
extraX=5
extraY=9

function animate()
  
  alienTime=alienTime+1
   if alienTime==4 then
     alienTime=0
   end

  extraTime=extraTime+1
   if extraTime==2 then
     extraTime=0
   end

  if missile then
   missileY=missileY+1
  end

 if alienTime==0 then
    alienX=alienX+1
    if alienX==11 then
     alienX=1
    end
  end

 if extraTime==0 then
    extraX=extraX-1
    if extraX==0 then
     extraX=10
    end
  end
end

function imgui.on_key_pressed(k)
   if k=='right' then
      x=x+1
   end
   if k=='left' then
      x=x-1
   end
   if k=='up' and not missile then
      missileY=2
      missileX=x
      missile=true
     end
end

function Missile(x,y)
   GLUP.Begin(GLUP.SPHERES)
      GLUP.Vertex(x+0.5, y+0.5, 0.0, 0.1)
   GLUP.End()
end

function Rocket(x,y)
    GLUP.PushMatrix()
    GLUP.Translate(x+2.0/9.0,y,0)
    GLUP.Scale(1.0/9.0, 1.0/7.0, 0.3)
    x=0
    y=0
    pixBegin()
    col("black")
    pix(x,y)
    pix(x,y+1)  
    pix(x,y+2)
    pix(x+4,y)
    pix(x+4,y+1) 
    pix(x+4,y+2)
    pix(x+1,y+1)
    pix(x+2,y+1)
    pix(x+3,y+1)
    pix(x+1,y+2)
    pix(x+2,y+2)
    pix(x+3,y+2)
    pix(x+1,y+3)
    pix(x+2,y+3)
    pix(x+3,y+3)
    pix(x+1,y+4)
    pix(x+2,y+4)
    pix(x+3,y+4)
    pix(x+1,y+5)
    pix(x+2,y+5)
    pix(x+3,y+5)
    pix(x+2,y+6)
    pix(x+2,y+7)
    pixEnd()
    GLUP.PopMatrix()
end

function Alien(x,y,color)
    GLUP.PushMatrix()
    GLUP.Translate(x,y,0)
    GLUP.Scale(1.0/9.0, 1.0/7.0, 0.3)
    x=0
    y=0
    pixBegin()
    col(color) 
    pix(x,y) 
    pix(x,y+1) 
    pix(x+1,y+2) 
    pix(x+2,y+2) 
    pix(x+2,y+1)
    pix(x+3,y) 
    pix(x+5,y+2) 
    pix(x+4,y+2) 
    pix(x+3,y+2)
    pix(x+6,y+2)  
    pix(x+6,y+1) 
    pix(x+5,y) 
    pix(x+7,y+2) 
    pix(x+8,y+1) 
    pix(x+8,y)
    pix(x+1,y+3) 
    pix(x+2,y+4) 
    pix(x+3,y+4) 
    pix(x+4,y+4) 
    pix(x+5,y+4) 
    pix(x+6,y+4) 
    pix(x+7,y+3) 
    pix(x+3,y+3) 
    pix(x+4,y+3) 
    pix(x+5,y+3) 
    pix(x+2,y+5) 
    pix(x+1,y+6) 
    pix(x+6,y+5)
    pix(x+7,y+6)
    pixEnd()
    GLUP.PopMatrix()
end


function GLUP.draw_scene() 
 
    GLUP.Enable(GLUP.DRAW_MESH) 
    GLUP.SetCellsShrink(0.1) 
    pixGrid()
    
    Rocket(x,y)
    
    if alien==true then
       Alien(alienX,alienY,"blue")
    end
    
    if extra==true then
       Alien(extraX,extraY,"red")
    end
    
    if GLUP.ElapsedTime()-oldtime>0.025 then 
       oldtime=GLUP.ElapsedTime()
       animate()
    end
     
    if missileY>10 then
       missile=false
    end

    if missile then
       Missile(missileX,missileY)
    end
    
    if missileX==alienX and missileY==alienY then
       alien=false
    end

    if missileX==extraX and missileY==extraY then
       extra=false
    end
end
  
