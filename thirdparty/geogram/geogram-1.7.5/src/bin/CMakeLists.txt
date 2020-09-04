
add_subdirectory(vorpastat)
add_subdirectory(vorpacomp)
add_subdirectory(geodump)

if(GEOGRAM_WITH_GRAPHICS)
  add_subdirectory(vorpaview)
  add_subdirectory(geobox)
  if(GEOGRAM_WITH_LUA)  
     add_subdirectory(geocod)
  endif()   
  add_subdirectory(geoshade)
endif()

if(GEOGRAM_WITH_VORPALINE) 
  add_subdirectory(vorpaline)
else()
  add_subdirectory(vorpalite)
endif()

if(GEOGRAM_WITH_FPG)
  add_subdirectory(fpg)
endif()
