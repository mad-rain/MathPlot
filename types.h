#ifndef _TYPES_H_INCLUDED
#define _TYPES_H_INCLUDED


#include <string.h>


typedef unsigned char TBYTE;
typedef unsigned short TWORD;
typedef unsigned int TDWORD;
typedef float TFloat;
typedef double TDouble;
typedef long double TLongDouble;


#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))


class TPoint2D {

public:

    TDouble x;
    TDouble y;

    TPoint2D() :
        x(0.0), y(0.0)
    {
    }

    TPoint2D(const TPoint2D&Other) :
        x(Other.x),
        y(Other.y)
    {
    }

    TPoint2D(TDouble _x, TDouble _y)
    {
        x = _x;
        y = _y;
    }

    TPoint2D(TFloat v[2])
    {
        x = v[0];
        y = v[1];
    }

    TPoint2D(TDouble v[2])
    {
        x = v[0];
        y = v[1];
    }

    TPoint2D operator-(const TPoint2D &p)
    {
        return TPoint2D(x - p.x, y - p.y);
    }

    TPoint2D operator+(const TPoint2D &p)
    {
        return TPoint2D(x + p.x, y + p.y);
    }

    TPoint2D operator*(TDouble Scale)
    {
        return TPoint2D(x * Scale, y * Scale);
    }

    TPoint2D operator/(TDouble Scale)
    {
        return TPoint2D(x / Scale, y / Scale);
    }

    TPoint2D operator-()
    {
        return TPoint2D(-x, -y);
    }

    void Min(const TPoint2D &, const TPoint2D &);
    void Max(const TPoint2D &, const TPoint2D &);
};

class TPoint3D {

public:

    TDouble x;
    TDouble y;
    TDouble z;

    TPoint3D() :
        x(0.0), y(0.0), z(0.0)
    {
    }

    TPoint3D(TFloat v[3])
    {
        x = v[0];
        y = v[1];
        z = v[2];
    }

    TPoint3D(TDouble v[3])
    {
        x = v[0];
        y = v[1];
        z = v[2];
    }

    TPoint3D(TDouble _x, TDouble _y, TDouble _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }
    
    TPoint3D operator*(TDouble Scale)
    {
        return TPoint3D(x * Scale, y * Scale, z * Scale);
    }

    TPoint3D operator-(const TPoint3D p)
    {
        return TPoint3D(x - p.x, y - p.y, z - p.z);
    }

    TPoint3D operator+(const TPoint3D p)
    {
        return TPoint3D(x + p.x, y + p.y, z + p.z);
    }
    
    TPoint3D operator=(const TPoint2D &p)
    {
        x = p.x;
        y = p.y;
        z = 0.0;

        return *this;
    }

    void Min(const TPoint3D &, const TPoint3D &);
    void Max(const TPoint3D &, const TPoint3D &);
};

class TBox2D {

public:

    TPoint2D Min;
    TPoint2D Max;

    TBox2D(const TPoint2D &Min, const TPoint2D &Max)
    {
        this->Min = Min;
        this->Max = Max;
    }
    
    void Intersection(const TBox2D &, const TBox2D &);
    void Union(const TBox2D &, const TBox2D &);
};

class TBox3D {

public:

    TPoint3D Min;
    TPoint3D Max;

    TBox3D(const TPoint3D &Min, const TPoint3D &Max)
    {
        this->Min = Min;
        this->Max = Max;
    }
    
    int Inside(const TPoint3D &);
    void Intersection(TBox3D &, TBox3D &);
    void Union(TBox3D &, TBox3D &);
};

class TMatrix {

public:

    TDouble m[4][4];

    void Rotate(TDouble, TDouble, TDouble);
    void Identity();
    void Transpose();
    void Translate(const TPoint3D &);

    TPoint3D operator*(const TPoint3D);
    TMatrix operator*(const TMatrix);

    TMatrix(double s[4][4])
    {
        memcpy(m, s, sizeof(TDouble[4][4]));
    }

    TMatrix() :
        m{}
    {
    }
};


#endif /* _TYPES_H_INDLUDED */
