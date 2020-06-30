#ifndef AUTO_REMESHER_VECTOR2_H
#define AUTO_REMESHER_VECTOR2_H
#include <AutoRemesher/Double>
#include <Eigen/Dense>

namespace AutoRemesher
{
    
class Vector2;

inline Vector2 operator*(double number, const Vector2 &v);
inline Vector2 operator*(const Vector2 &v, double number);
inline Vector2 operator+(const Vector2 &a, const Vector2 &b);
inline Vector2 operator-(const Vector2 &a, const Vector2 &b);

class Vector2
{
public:
    inline Vector2() = default;
    
    inline Vector2(double x, double y) :
        m_data {x, y}
    {
    }

    inline const double &x() const
    {
        return m_data[0];
    }
    
    inline const double &y() const
    {
        return m_data[1];
    }
    
    inline void setX(double x)
    {
        m_data[0] = x;
    }
    
    inline void setY(double y)
    {
        m_data[1] = y;
    }
    
    inline const double *data() const
    {
        return &m_data[0];
    }
    
    inline void setData(double x, double y)
    {
        m_data[0] = x;
        m_data[1] = y;
    }
    
    inline double lengthSquared() const
    {
        return x() * x() + y() * y();
    }
    
    inline double length() const
    {
        double length2 = x() * x() + y() * y();
        return std::sqrt(length2);
    }
    
    inline Vector2 normalized() const
    {
        double length2 = x() * x() + y() * y();
    
        double length = std::sqrt(length2);
        if (Double::isZero(length))
            return Vector2();
        
        return Vector2(x() / length, y() / length);
    }
    
    inline void normalize()
    {
        double length2 = x() * x() + y() * y();
    
        double length = std::sqrt(length2);
        if (Double::isZero(length))
            return;
        
        m_data[0] = x() / length;
        m_data[1] = y() / length;
    }
    
    inline static bool isLocallyDelaunay(const Vector2 &a, const Vector2 &b, const Vector2 &c, 
        const Vector2 &point)
    {
        Eigen::Matrix4d m;
        m << a.x(), a.y(), a.x() * a.x() + a.y() * a.y(), 1,
            b.x(), b.y(), b.x() * b.x() + b.y() * b.y(), 1,
            c.x(), c.y(), c.x() * c.x() + c.y() * c.y(), 1,
            point.x(), point.y(), point.x() * point.x() + point.y() * point.y(), 1;
        return m.determinant() > 0;
    }
    
private:
    double m_data[2] = {0.0};
};

inline Vector2 operator*(double number, const Vector2 &v)
{
    return Vector2(number * v.x(), number * v.y());
}

inline Vector2 operator*(const Vector2 &v, double number)
{
    return Vector2(number * v.x(), number * v.y());
}

inline Vector2 operator+(const Vector2 &a, const Vector2 &b)
{
    return Vector2(a.x() + b.x(), a.y() + b.y());
}

inline Vector2 operator-(const Vector2 &a, const Vector2 &b)
{
    return Vector2(a.x() - b.x(), a.y() - b.y());
}

}

#endif
