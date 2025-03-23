
#include <math.h>
#include "types.h"


void TPoint2D::Min(TPoint2D &p0, TPoint2D &p1)
{
    x = min(p0.x, p1.x);
    y = min(p0.y, p1.y);
}

void TPoint2D::Max(TPoint2D &p0, TPoint2D &p1)
{
    x = max(p0.x, p1.x);
    y = max(p0.y, p1.y);
}

void TBox2D::Intersection(TBox2D &b0, TBox2D &b1)
{
    Min.Max(b0.Min, b1.Min);
    Max.Min(b0.Max, b1.Max);
}

void TBox2D::Union(TBox2D &b0, TBox2D &b1)
{
    Min.Min(b0.Min, b1.Min);
    Max.Max(b0.Max, b1.Max);
}

void TPoint3D::Min(TPoint3D &p0, TPoint3D &p1)
{
    x = min(p0.x, p1.x);
    y = min(p0.y, p1.y);
    z = min(p0.z, p1.z);
}

void TPoint3D::Max(TPoint3D &p0, TPoint3D &p1)
{
    x = max(p0.x, p1.x);
    y = max(p0.y, p1.y);
    z = max(p0.z, p1.z);
}

void TBox3D::Union(TBox3D &b0, TBox3D &b1)
{
    Min.Min(b0.Min, b1.Min);
    Max.Max(b0.Max, b1.Max);
}

void TBox3D::Intersection(TBox3D &b0, TBox3D &b1)
{
    Min.Max(b0.Min, b1.Min);
    Max.Min(b0.Max, b1.Max);
}

int TBox3D::Inside(TPoint3D &p)
{
    return Min.x <= p.x && Max.x >= p.x &&
           Min.y <= p.y && Max.y >= p.y &&
           Min.z <= p.z && Max.z >= p.z;
}

#if 0
void TMatrix::Rotate(TDouble Alpha, TDouble Beta, TDouble Gamma)
{
    TDouble sx, sy, sz;
    TDouble cx, cy, cz;

    sx = sin(Alpha); cx = cos(Alpha);
    sy = sin(Beta ); cy = cos(Beta );
    sz = sin(Gamma); cz = cos(Gamma);

    TDouble sycz = sy * cz;

    m[0][0] = sx * sy * sz + cx * cz;
    m[0][1] = cy * sz;
    m[0][2] = sx * cz - cx * sy * sz;

    m[1][0] = sx * sycz - cx * sz;
    m[1][1] = cy * cz;
    m[1][2] = -cx * sycz - sx * sz;

    m[2][0] = sx * cy;
    m[2][1] = -sy;
    m[2][2] = -cx * cy;

    m[0][3] = m[1][3] = m[2][3] = 0.0;
    m[3][0] = m[3][1] = m[3][2] = 0.0;
    m[3][3] = 1.0;
}
#endif

void TMatrix::Rotate(TDouble x, TDouble y, TDouble z)
{         
    TDouble sx, sy, sz;
    TDouble cx, cy, cz;

    sx = sin(x); cx = cos(x);
    sy = sin(y); cy = cos(y);
    sz = sin(z); cz = cos(z);      
      
    TDouble sxsy = sx*sy;      
    TDouble sycx = cx*sy;

    m[0][0] = cy * cz;
    m[0][1] = -cy * sz;
    m[0][2] = -sy;
    m[1][0] = -sxsy * cz + cx * sz;
    m[1][1] = sxsy * sz + cx * cz;
    m[1][2] = -sx * cy;
    m[2][0] = sycx * cz + sx * sz;
    m[2][1] = -sycx * sz + sx * cz;
    m[2][2] = cx * cy;

    m[0][3] = m[1][3] = m[2][3] = 0.0;
    m[3][0] = m[3][1] = m[3][2] = 0.0;
    m[3][3] = 1.0;
}

TPoint3D TMatrix::operator*(TPoint3D p)
{
    return TPoint3D (m[0][0] * p.x + m[0][1] * p.y + m[0][2] * p.z + m[0][3],
                     m[1][0] * p.x + m[1][1] * p.y + m[1][2] * p.z + m[1][3],
                     m[2][0] * p.x + m[2][1] * p.y + m[2][2] * p.z + m[2][3]);
}

void TMatrix::Transpose()
{
    for (int i = 0; i < 4; i ++)
        for (int j = 0; j < i; j ++)
        {
            TDouble v = m[i][j];
            m[i][j] = m[j][i];
            m[j][i] = v;
        }
}

void TMatrix::Translate(TPoint3D &p)
{
    m[0][3] -= m[0][0] * p.x + m[0][1] * p.y + m[0][2] * p.z;
    m[1][3] -= m[1][0] * p.x + m[1][1] * p.y + m[1][2] * p.z;
    m[2][3] -= m[2][0] * p.x + m[2][1] * p.y + m[2][2] * p.z;
}

void TMatrix::Identity()
{
    m[0][0] = 1.0; m[0][1] = 0.0; m[0][2] = 0.0; m[0][3] = 0.0;
    m[1][0] = 0.0; m[1][1] = 1.0; m[1][2] = 0.0; m[1][3] = 0.0;
    m[2][0] = 0.0; m[2][1] = 0.0; m[2][2] = 1.0; m[2][3] = 0.0;
    m[3][0] = 0.0; m[3][1] = 0.0; m[3][2] = 0.0; m[3][3] = 1.0;
}

TMatrix TMatrix::operator*(TMatrix s)
{
    TDouble d[4][4];

    for (int i = 0; i < 4; i ++)
        for (int j = 0; j < 4; j ++)
            d[i][j] = m[i][0] * s.m[0][j] +
                      m[i][1] * s.m[1][j] +
                      m[i][2] * s.m[2][j] +
                      m[i][3] * s.m[3][j];
                      
    return TMatrix(d);
}
