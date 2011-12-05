#include "mathlib.h"

#ifdef MATHLIB_INLINE
        #ifdef MATHLIB_INCLUDE
                #define ACTIVE_VECTOR_CODE
        #endif
#else
        #define ACTIVE_VECTOR_CODE
#endif

#ifdef ACTIVE_VECTOR_CODE

/*===================================================================================
Vec2
=====================================================================================*/
INLINE void Vec2::set( vec x, vec y )
{
        v[X] = x;
        v[Y] = y;
}

INLINE void Vec2::set( const vec *vP )
{ 
        v[X] = vP[X];
        v[Y] = vP[Y];
}

INLINE MBoolean Vec2::operator==(const Vec2& v2) const
{
    vec d0=v2[0]-v[0],  d1=v2[1]-v[1];
    return (d0*d0 + d1*d1) < ERROR_EPSILON;
}

INLINE MBoolean Vec2::operator!=(const Vec2& v2) const
{
    vec d0=v2[0]-v[0],  d1=v2[1]-v[1];
    return (d0*d0 + d1*d1) > ERROR_EPSILON;
}

INLINE Vec2& Vec2::operator+=(const Vec2& v2)
{
    v[0] += v2[0];   v[1] += v2[1];
    return *this;
}

INLINE Vec2& Vec2::operator-=(const Vec2& v2)
{
    v[0] -= v2[0];   v[1] -= v2[1];
    return *this;
}

INLINE Vec2& Vec2::operator*=(vec s)
{
    v[0] *= s;   v[1] *= s;
    return *this;
}

INLINE Vec2& Vec2::operator/=(vec s)
{
    v[0] /= s;   v[1] /= s;
    return *this;
}


INLINE Vec2 Vec2::operator+(const Vec2& v2) const
{
    return Vec2(v2[0]+v[0], v2[1]+v[1]);
}

INLINE Vec2 Vec2::operator-(const Vec2& v2) const
{
    return Vec2(v[0]-v2[0], v[1]-v2[1]);
}

INLINE Vec2 Vec2::operator-() const
{
    return Vec2(-v[0], -v[1]);
}

INLINE Vec2 Vec2::operator*(vec s) const
{
    return Vec2(v[0]*s, v[1]*s);
}

INLINE Vec2 Vec2::operator/(vec s) const
{
    return Vec2(v[0]/s, v[1]/s);
}

INLINE vec Vec2::operator*(const Vec2& v2) const
{
    return v[0]*v2[0] + v[1]*v2[1];
}

INLINE Vec2 operator*(vec s, const Vec2& v2)
{
    return v2*s;
}

INLINE vec Vec2::length( )
{
    return (vec)SQRT( v[0]*v[0] + v[1]*v[1]);
}

INLINE void Vec2::normalize( )
{
    vec l = length();
    v[0] /= l;
    v[1] /= l;
}

INLINE void Vec2::inverse( )
{
    v[0] = -v[0];
    v[1] = -v[1];
}

/*===================================================================================
Vec3
=====================================================================================*/

INLINE void Vec3::set( vec x, vec y, vec z ) 
{ 
        v[0] = x;
        v[1] = y;
        v[2] = z; 
}
                           
INLINE void Vec3::set( const vec *vP)
{ 
        v[0] = vP[0]; 
        v[1] = vP[1];
        v[2] = vP[2];
}

INLINE MBoolean Vec3::operator==(const Vec3& v3) const
{
    vec dx=v3[X]-v[X],  dy=v3[Y]-v[Y],  dz=v3[Z]-v[Z];
    return (dx*dx + dy*dy + dz*dz) < ERROR_EPSILON;
}

INLINE MBoolean Vec3::operator!=(const Vec3& v3) const
{
    vec dx=v3[X]-v[X],  dy=v3[Y]-v[Y],  dz=v3[Z]-v[Z];
    return (dx*dx + dy*dy + dz*dz) > ERROR_EPSILON;
}

INLINE Vec3& Vec3::operator+=(const Vec3& v3)
{
    v[0] += v3[0];   v[1] += v3[1];   v[2] += v3[2];
    return *this;
}

INLINE Vec3& Vec3::operator-=(const Vec3& v3)
{
    v[0] -= v3[0];   v[1] -= v3[1];   v[2] -= v3[2];
    return *this;
}

INLINE Vec3& Vec3::operator*=(vec s)
{
    v[0] *= s;   v[1] *= s;   v[2] *= s;
    return *this;
}

INLINE Vec3& Vec3::operator/=(vec s)
{
    v[0] /= s;   v[1] /= s;   v[2] /= s;
    return *this;
}

INLINE Vec3 Vec3::operator+(const Vec3& v3) const
{
    return Vec3(v3[0]+v[0], v3[1]+v[1], v3[2]+v[2]);
}

INLINE Vec3 Vec3::operator-(const Vec3& v3) const
{       
    return Vec3(v[0]-v3[0], v[1]-v3[1], v[2]-v3[2]);
}

INLINE Vec3 Vec3::operator-() const
{
    return Vec3(-v[0], -v[1], -v[2]);
}

INLINE Vec3 Vec3::operator*(vec s) const
{
    return Vec3(v[0]*s, v[1]*s, v[2]*s);
}

INLINE Vec3 Vec3::operator/(vec s) const
{
    return Vec3(v[0]/s, v[1]/s, v[2]/s);
}

INLINE vec Vec3::operator*(const Vec3& v3) const
{
    return v[0]*v3[0] + v[1]*v3[1] + v[2]*v3[2];
}

INLINE Vec3 Vec3::operator^(const Vec3& v3) const
{
    Vec3 w( v[1]*v3[2] - v3[1]*v[2],
                  -v[0]*v3[2] + v3[0]*v[2],
                  v[0]*v3[1] - v3[0]*v[1] );
    return w;
}

INLINE Vec3 operator*(vec s, const Vec3& v3)
{
    return v3*s;
}

INLINE vec Vec3::length( )
{
    return (vec)SQRT( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
}

INLINE void Vec3::normalize( )
{
    vec l = length();
    v[0] /= l;
    v[1] /= l;
    v[2] /= l;
}

INLINE void Vec3::inverse( )
{
    v[0] = -v[0];
    v[1] = -v[1];
    v[2] = -v[2];
}

#ifdef MATHLIB_PRINT_IOSTREAM
#include <iostream.h>
INLINE void Vec3::print( ostream &ostr )
{
  ostr << "(" << v[X] << "," << v[Y] << "," << v[Z] << ")";
}
#endif

#ifdef MATHLIB_PRINT_STDIO
#include <stdio.h>
INLINE void Vec3::print()
{
  printf("(%f %f %f)\n", v[X], v[Y], v[Z] );
}
#endif


/*===================================================================================
Vec4
=====================================================================================*/

INLINE void Vec4::set( vec x, vec y, vec z, vec w ) 
{ 
        v[X] = x; 
        v[Y] = y; 
        v[Z] = z; 
        v[W] = w; 
}

INLINE void Vec4::set(  const vec *vP )
{ 
        v[X] = vP[X];
        v[Y] = vP[Y];
        v[Z] = vP[Z];
        v[W] = vP[W];
}

INLINE MBoolean Vec4::operator==(const Vec4& v3) const
{
    vec dx=v3[X]-v[X],  dy=v3[Y]-v[Y],  dz=v3[Z]-v[Z], dw = v3[W]-v[W];
    return (dx*dx + dy*dy + dz*dz + dw*dw) < ERROR_EPSILON;
}

INLINE MBoolean Vec4::operator!=(const Vec4& v3) const
{
    vec dx=v3[X]-v[X],  dy=v3[Y]-v[Y],  dz=v3[Z]-v[Z], dw = v3[W]-v[W];
    return (dx*dx + dy*dy + dz*dz + dw*dw) > ERROR_EPSILON;
}

INLINE Vec4& Vec4::operator+=(const Vec4& v3)
{
    v[0] += v3[0];   v[1] += v3[1];   v[2] += v3[2]; v[3] += v3[3];
    return *this;
}

INLINE Vec4& Vec4::operator-=(const Vec4& v3)
{
    v[0] -= v3[0];   v[1] -= v3[1];   v[2] -= v3[2]; v[3] -= v3[3];
    return *this;
}

INLINE Vec4& Vec4::operator*=(vec s)
{
    v[0] *= s;   v[1] *= s;   v[2] *= s; v[3] *= s;
    return *this;
}

INLINE Vec4& Vec4::operator/=(vec s)
{
    v[0] /= s;   v[1] /= s;   v[2] /= s; v[3] /= s;
    return *this;
}

INLINE Vec4 Vec4::operator+(const Vec4& v3) const
{
    return Vec4(v3[0]+v[0], v3[1]+v[1], v3[2]+v[2], v3[3]+v[3]);
}

INLINE Vec4 Vec4::operator+(const Vec3& v3) const
{
    return Vec4(v3[0]+v[0], v3[1]+v[1], v3[2]+v[2], v[3]);
}

INLINE Vec4 Vec4::operator-(const Vec4& v3) const
{
    return Vec4(v[0]-v3[0], v[1]-v3[1], v[2]-v3[2], v[3]-v3[3]);
}

INLINE Vec4 Vec4::operator-() const
{
    return Vec4(-v[0], -v[1], -v[2], -v[3]);
}

INLINE Vec4 Vec4::operator*(vec s) const
{
    return Vec4(v[0]*s, v[1]*s, v[2]*s, v[3]*s);
}

INLINE Vec4 Vec4::operator/(vec s) const
{
    return Vec4(v[0]/s, v[1]/s, v[2]/s, v[3]/s);
}

INLINE vec Vec4::operator*(const Vec4& v3) const
{
    return v[0]*v3[0] + v[1]*v3[1] + v[2]*v3[2] + v[3]*v3[3];
}

INLINE Vec4 operator*(vec s, const Vec4& v3)
{
    return v3*s;
}

INLINE vec Vec4::length( )
{
    return (vec)SQRT( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] + v[3]*v[3]);
}

INLINE void Vec4::normalize( )
{
    vec l = length();
    v[0] /= l;
    v[1] /= l;
    v[2] /= l;
    v[3] /= l;
}

INLINE void Vec4::inverse( )
{
    v[0] = -v[0];
    v[1] = -v[1];
    v[2] = -v[2];
    v[3] = -v[3];
}

#ifdef MATHLIB_PRINT_IOSTREAM
#include <iostream.h>
INLINE void Vec4::print( ostream &ostr )
{
  ostr << "(" << v[X] << "," << v[Y] << "," << v[Z] << "," << v[W] << ")";
}
#endif

#ifdef MATHLIB_PRINT_STDIO
#include <stdio.h>
INLINE void Vec4::print()
{
  printf("(%f %f %f %f)\n", v[X], v[Y], v[Z], v[W] );
}
#endif


INLINE Vec4 Vec4::quatFromMatrix( Mat4 &matrix )
{
  vec  tr, s;
  int    i, j, k;
  int    nxt[3] = {1, 2, 0};
  vec  *m = matrix.raw();
  Vec4 quat;

  tr = MAT(m, 0, 0) + MAT(m, 1, 1) + MAT(m, 2, 2);

  // check the diagonal
  if (tr > 0.0) {
    s = sqrt (tr + 1.0);
    quat[W] = s * 0.5;
    s = 0.5 / s;
    quat[X] = (MAT(m,1,2) - MAT(m,2,1)) * s;
    quat[Y] = (MAT(m,2,0) - MAT(m,0,2)) * s;
    quat[Z] = (MAT(m,0,1) - MAT(m,1,0)) * s;
  } else {                
    // diagonal is negative
    i = 0;
    if (MAT(m,1,1) > MAT(m,0,0)) i = 1;
    if (MAT(m,2,2) > MAT(m,i,i)) i = 2;
    j = nxt[i];
    k = nxt[j];

    s = sqrt ((MAT(m,i,i) - (MAT(m,j,j) + MAT(m,k,k))) + 1.0);
                       
    quat[i] = s * 0.5;
                             
    if (s != 0.0) s = 0.5 / s;

    quat[3] = (MAT(m,j,k) - MAT(m,k,j)) * s;
    quat[j] = (MAT(m,i,j) + MAT(m,j,i)) * s;
    quat[k] = (MAT(m,i,k) + MAT(m,k,i)) * s;
  }

  return quat;
}


INLINE Vec4 Vec4::quatSlerp(Vec4 &from, Vec4 &to, vec t)
{
  float        to1[4];
  float        omega, cosom, sinom, scale0, scale1;
  Vec4         res;

  // calc cosine
  cosom = from[X]*to[X] + 
		  from[Y]*to[Y] + 
		  from[Z]*to[Z] +
          from[W]*to[W];

  // adjust signs (if necessary)
  if ( cosom < 0.0 ) { 
		  cosom = -cosom; 
		  to1[0] = - to[X];
          to1[1] = - to[Y];
          to1[2] = - to[Z];
          to1[3] = - to[W];
  } else  {
          to1[0] = to[X];
          to1[1] = to[Y];
          to1[2] = to[Z];
          to1[3] = to[W];
  }

 // calculate coefficients
 if ( (1.0 - cosom) > QUAT_SLERP_DELTA ) {
          // standard case (slerp)
          omega = acos(cosom);
          sinom = sin(omega);
          scale0 = sin((1.0 - t) * omega) / sinom;
          scale1 = sin(t * omega) / sinom;

  } else {        
      // "from" and "to" quaternions are very close 
      //  ... so we can do a linear interpolation
          scale0 = 1.0 - t;
          scale1 = t;
  }
  // calculate final values
  res[X] = scale0 * from[X] + scale1 * to1[0];
  res[Y] = scale0 * from[Y] + scale1 * to1[1];
  res[Z] = scale0 * from[Z] + scale1 * to1[2];
  res[W] = scale0 * from[W] + scale1 * to1[3];

  return res;
}	

#endif

#ifdef ACTIVE_VECTOR_CODE
        #undef ACTIVE_VECTOR_CODE
#endif
