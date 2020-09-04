require("pixel")

function GLUP.init_graphics()
   GLUP.SetRegionOfInterest(1,1,1,11,11,1)
end    

function GLUP.draw_scene()

    GLUP.Enable(GLUP.DRAW_MESH)
    GLUP.SetCellsShrink(0.1)
    pixGrid()
    
    pixBegin()
    col("blue")
    pix(1,1)
    pixEnd()
end