require("turtle")


function Sierpinski(a, level)
   if level==0 then
    pd()
	for i=1,3,1 do
      	 fd(a)
      	 tl(120)
	end
   pu()
   else
	Sierpinski(a/2, level - 1)
	pu()
	fd(a/2)
	pd()
	Sierpinski(a/2, level - 1)
	pu()
	tl(120)
	fd(a/2)
	tr(120)
	pd()
	Sierpinski(a/2, level - 1)
    	pu()
	-- We should return home!
	pd()
	tl(60)
	bk(a/2)
	tr(60)
	pu()
   end
end


function GLUP.draw_scene()
    home()
    pwidth(0.2)
    GLUP.Enable(GLUP.VERTEX_COLORS)
    GLUP.Disable(GLUP.LIGHTING)
    pcol("gray")
    Sierpinski(100,5)
end
