--             preamble.lua
-- FR bibliotheque interne, incluse par defaut
-- FR ce fichier est toujours execute avant votre programme
-- EN internal library, included by default
-- EN this file is always executed before your program

-- FR une fonction qui ne fait rien
-- EN a function that does nothing
function NOP()
end

-- EN by-default definition for functions called by GEOCOD
-- FR definition par defaut des fonctions appelees par GEOCOD
GLUP.draw_scene = NOP
imgui.draw_application_menus = NOP
imgui.draw_object_properties = NOP
imgui.on_key_pressed = NOP
imgui.on_key_released = NOP


function GLUP.init_graphics()
    GLUP.SetRegionOfInterest(0,0,0,1,1,1)
end

-- Commented-out (I want to keep user's view when
-- F5 is pushed).
-- GLUP.ResetViewer()


