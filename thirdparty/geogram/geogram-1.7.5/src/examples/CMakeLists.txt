
add_subdirectory(geogram)

if(GEOGRAM_WITH_GRAPHICS)
   add_subdirectory(graphics)
endif() 

if(GEOGRAM_WITH_EXPLORAGRAM)
   add_subdirectory(exploragram)
endif()   

if(GEOGRAM_WITH_VORPALINE)
   add_subdirectory(vorpaline)
endif()
