1.1.0
- Replace Geogram with a native quad parameterizer
- Add anisotropic quad sizing and the `--anisotropy` CLI option
- Improve UV parameterization and quad extraction
- Smooth and project extracted quads onto the input mesh
- Split six-edge faces during quad extraction
- Add source, isotropic, parameterization, and remeshed-model previews
- Show singular vertices and extracted connections in previews
- Fix remeshing of non-manifold meshes
- Fix a resampling data race
- Print headless progress as a percentage
- Add headless remeshing reports
- Enable high-DPI support with Qt 5
- Use Accelerate/BLAS on macOS
- Support Linux CPUs without AVX2
- Update Windows CI and build instructions
- Add a Contributors dialog

1.0.0
- Relicense from GPLv3 to MIT (reimplemented MIT-incompatible dependencies)
- Improve parameterizer, isotropic remesher, and quad extraction algorithms
- Add adaptivity parameter
- Add sharp edge parameter
- Add smooth normal parameter for low-poly mesh
- Replace density with target quads parameter
- Add command-line interface
- Refine main window, theme, and graphics widgets
- Replace app icon

1.0.0-beta.3
- Remesh isolated meshes separately   
- Improve quad extractor  
- Add edge scaling setting for generating low poly  
- Add rough progress reporting (Windows only)  
- Generate quad dominated mesh    
- Improve parameterization for thin surfaces  

1.0.0-beta.2
- Fix holes  
- Replace Poly budget with density setting  
- Remove laplacian smooth in preprocess  

1.0.0-beta.1
- Replace MIQ with QuadCover  
- Implement simple quad extractor  
- Remove libQEx  
- Add OpenVDB for uniform remeshing  

1.0.0-alpha.4
- Add constrained option: Better Edge Flow/Less Distortion  
- Fix libQEx access violation  
- Fix OpenMesh crash  
- Limit singularities to 320  
- Improve wireframe render  

1.0.0-alpha.3
- Support mesh with holes  
- Generate better edge flow by increase the default constraint ratio from 0.4 to 0.5  
- Alleviate spiral pattern by up-sampling  
- Speed up on complex mesh by reducing singularities   
- Add debug dialog  
