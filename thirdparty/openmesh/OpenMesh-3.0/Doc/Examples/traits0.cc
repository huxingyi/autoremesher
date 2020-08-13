struct DefaultTraits
{
  typedef Vec3f  Point;
  typedef Vec3f  Normal;
  typedef Vec2f  TexCoord;
  typedef Vec3uc Color;

  VertexTraits    {};
  HalfedgeTraits  {};
  EdgeTraits      {};
  FaceTraits      {};
  
  VertexAttributes(0);
  HalfedgeAttributes(Attributes::PrevHalfedge);
  EdgeAttributes(0);
  FaceAttributes(0);
};
