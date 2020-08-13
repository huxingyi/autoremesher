contains( OPENFLIPPER , OpenFlipper ){
	include( $$TOPDIR/qmake/all.include )
} else {
	include( $$TOPDIR/OpenMesh/qmake/all.include )
}

Subdirs()

addSubdirs( src/OpenMesh/Core )
addSubdirs( src/OpenMesh/Tools , src/OpenMesh/Core )
addSubdirs( src/OpenMesh/Apps/commandlineDecimater , src/OpenMesh/Core src/OpenMesh/Tools)
addSubdirs( src/OpenMesh/Apps/Decimating/DecimaterGui , src/OpenMesh/Core src/OpenMesh/Tools)
addSubdirs( src/OpenMesh/Apps/mconvert , src/OpenMesh/Core src/OpenMesh/Tools)
addSubdirs( src/OpenMesh/Apps/QtViewer , src/OpenMesh/Core src/OpenMesh/Tools)
addSubdirs( src/OpenMesh/Apps/Smoothing , src/OpenMesh/Core src/OpenMesh/Tools)
addSubdirs( src/OpenMesh/Apps/Subdivider/commandlineSubdivider , src/OpenMesh/Core src/OpenMesh/Tools)
addSubdirs( src/OpenMesh/Apps/Subdivider/commandlineAdaptiveSubdivider , src/OpenMesh/Core src/OpenMesh/Tools)
addSubdirs( src/OpenMesh/Apps/Subdivider/SubdividerGui , src/OpenMesh/Core src/OpenMesh/Tools)
