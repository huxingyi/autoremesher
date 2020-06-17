#include <OpenMesh/Core/Utils/GenProg.hh>

// draw a face normal if we have one
void drawFaceNormal(const MyMesh::Face& _f) { 
  drawFaceNormal(_f, GenProg::Bool2Type<OM_Check_Attrib(MyMesh::Face, Normal)>()); 
}

// normal exists -> use it
void drawFaceNormal(const MyMesh::Face& _f, GenProg::Bool2Type<true>) { 
  glNormal3fv(_f.normal());
}

// empty dummy (no normals)
void drawFaceNormal(const MyMesh::Face& _f, GenProg::Bool2Type<false>){}
