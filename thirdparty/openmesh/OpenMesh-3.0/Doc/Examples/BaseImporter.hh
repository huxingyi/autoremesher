class BaseImporter
{
public:

  virtual void add_vertex  (const OpenMesh::Vec3f&) {};
  virtual void add_normal  (const OpenMesh::Vec3f&) {};
  virtual void add_texture (const OpenMesh::Vec2f&) {};
  virtual void add_face    (const FaceType&)        {};
};
