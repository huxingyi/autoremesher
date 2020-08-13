class BaseExporter
{
public:

  virtual void update() = 0;
  
  virtual PVertexIter const_vertices_begin()   = 0;
  virtual PVertexIter const_vertices_end()     = 0;
  
  virtual PTexCoordIter const_texcoords_begin() = 0;
  virtual PTexCoordIter const_texcoords_end()   = 0;

  virtual PIdxFaceIter const_idx_faces_begin() = 0;
  virtual PIdxFaceIter const_idx_faces_end()   = 0;

  virtual PFaceIter const_set_faces_begin() = 0;
  virtual PFaceIter const_set_faces_end()   = 0;

  virtual unsigned int n_faces()     = 0;
  virtual unsigned int n_vertices()  = 0;
  virtual unsigned int n_texcoords() = 0;
};
