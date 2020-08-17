---------------------------------------------------------------

-- FR Un programme d'exemple qui affiche une fonction mathematique
-- EN An example program that plots a mathematical function

---------------------------------------------------------------

-- FR La fonction peut etre changee ici
-- EN The function can be changed here

function f(x,y)
   return 0.5+0.3*math.sin(x*5)*math.sin(y*5)
end

---------------------------------------------------------------

function GLUP.init_graphics() 
   GLUP.SetRegionOfInterest(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)
end 



nx = 10
ny = 10

function point(i,j)
    local x = i/(nx-1)
    local y = j/(ny-1)
    local z = f(x,y)
    return x,y,z
end

function GLUP.draw_scene() 
    GLUP.Enable(GLUP.DRAW_MESH)
    GLUP.SetPointSize(4.0)
    GLUP.SetColor(GLUP.FRONT_COLOR, 1.0, 1.0, 1.0)
    GLUP.Begin(GLUP.QUADS)
    for j=0,ny-2 do
	for i=0,nx-2 do
	    GLUP.Vertex(point(i,j))
	    GLUP.Vertex(point(i+1,j))
	    GLUP.Vertex(point(i+1,j+1))
	    GLUP.Vertex(point(i,j+1))
	end
    end
    GLUP.End()
end

function save()
    print("Saving plot to plot.obj")
    local file = io.open("plot.obj","w")
    for j=0,(ny-1) do
	for i=0,(nx-1) do
	   local x,y,z = point(i,j)
	   file:write("v "..x.." "..y.." "..z.."\n")
	end
    end
    for j=0,ny-2 do
       for i=0,nx-2 do
          local v00 = math.floor(j*nx+i)+1
	  local v10 = math.floor(j*nx+i+1)+1
	  local v01 = math.floor((j+1)*nx+i)+1
	  local v11 = math.floor((j+1)*nx+i+1)+1
	  file:write("f "..v00.." "..v10.." "..v11.."\n")
	  file:write("f "..v00.." "..v11.." "..v01.."\n")
       end
    end	
    file:close()
end

function imgui.draw_object_properties()
    local b
    b,nx = imgui.SliderInt("nx", nx, 2, 100, "%.0f")
    b,ny = imgui.SliderInt("ny", ny, 2, 100, "%.0f")
    imgui.Separator()
    if imgui.Button("Save as plot.obj") then
	save()
    end
end
