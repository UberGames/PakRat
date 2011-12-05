/*
  Copyright (C) 1998
  All rights reserved Matthew Baranowski Matthew_Baranowski@Brown.Edu
  You can redistribute it and/or modify the program for personal and NON-COMMERCIAL 
  purposes. If you do so please give credit to the author. This program is distributed 
  in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the 
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  matrix functions
*/

#include "mathlib.h"

#ifdef MATHLIB_INLINE
        #ifdef MATHLIB_INCLUDE
                #define ACTIVE_MATRIX_CODE
        #else
                #define ACTIVE_GLOBALDEF
        #endif
#else
        #define ACTIVE_MATRIX_CODE
        #define ACTIVE_GLOBALDEF
#endif  

#ifdef ACTIVE_GLOBALDEF

/*
global special matrices
*/

Mat4 identity( 1,0,0,0, 
                            0,1,0,0,
                                0,0,1,0,
                                0,0,0,1  );

Mat4 zero(     0,0,0,0,
                                0,0,0,0,
                                0,0,0,0,
                                0,0,0,0  );

Mat4 unit(     1,1,1,1,
                                1,1,1,1,
                                1,1,1,1,
                                1,1,1,1  );
#endif

#ifdef ACTIVE_MATRIX_CODE

INLINE Mat4::Mat4( ) {
        vec *p = m;
        p[0] = 0; p[4] = 0; p[8] =  0; p[12] = 0;
        p[1] = 0; p[5] = 0; p[9] =  0; p[13] = 0;
        p[2] = 0; p[6] = 0; p[10] = 0; p[14] = 0;
        p[3] = 0; p[7] = 0; p[11] = 0; p[15] = 0;
}

INLINE Mat4::Mat4( const vec m0,  const vec m1,  const vec m2,  const vec m3, 
                                     const vec m4,  const vec m5,  const vec m6,  const vec m7,
                                     const vec m8,  const vec m9,  const vec m10, const vec m11, 
                                     const vec m12, const vec m13, const vec m14, const vec m15 )                       
{
        set( m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15 );    
}

INLINE Mat4::Mat4( vec *t )
{
        set( t );
}

INLINE Mat4::Mat4(const Mat4& n)
{
        set( n );
}

INLINE Mat4::Mat4( const Vec3 v1, const vec rw1, 
                                         const Vec3 v2, const vec rw2, 
                                         const Vec3 v3, const vec rw3, 
                                         const vec w0, const vec w1, const vec w2, const vec w3 )
{
        set( v1(X), v1(Y), v1(Z), rw1, 
                 v2(X), v2(Y), v2(Z), rw2, 
                 v3(X), v3(Y), v3(Z), rw3,
                    w0,    w1,    w2, w3 );
}

//
// set functions
//

INLINE void Mat4::set( const vec a,  const vec b, const vec c, const vec d,
                                                const vec e,  const vec f, const vec g, const vec h,
                                                const vec i,  const vec j, const vec k, const vec l,
                                                const vec md, const vec n, const vec o, const vec p) {

        MAT(m, 0, 0) = a; MAT(m, 0, 1) = b; MAT(m, 0, 2) = c; MAT(m, 0, 3) = d;
        MAT(m, 1, 0) = e; MAT(m, 1, 1) = f; MAT(m, 1, 2) = g; MAT(m, 1, 3) = h;
        MAT(m, 2, 0) = i; MAT(m, 2, 1) = j; MAT(m, 2, 2) = k; MAT(m, 2, 3) = l;
        MAT(m, 3, 0) = md; MAT(m, 3, 1) = n; MAT(m, 3, 2) = o; MAT(m, 3, 3) = p;
}

INLINE void Mat4::set( vec *t )
{
        vec *p = m;
        p[0] = t[0]; p[4] = t[4]; p[8] =   t[8]; p[12] = t[12]; 
        p[1] = t[1]; p[5] = t[5]; p[9] =   t[9]; p[13] = t[13]; 
        p[2] = t[2]; p[6] = t[6]; p[10] = t[10]; p[14] = t[14]; 
        p[3] = t[3]; p[7] = t[7]; p[11] = t[11]; p[15] = t[15]; 
}

INLINE void Mat4::set(const Mat4& n)
{
        vec *p = m;
        vec *t = (vec *)(n.m);
        p[0] = t[0]; p[4] = t[4]; p[8] =   t[8]; p[12] = t[12]; 
        p[1] = t[1]; p[5] = t[5]; p[9] =   t[9]; p[13] = t[13]; 
        p[2] = t[2]; p[6] = t[6]; p[10] = t[10]; p[14] = t[14]; 
        p[3] = t[3]; p[7] = t[7]; p[11] = t[11]; p[15] = t[15]; 
}

INLINE vec *Mat4::raw()
{
        return m;
}


INLINE Mat4 Mat4::operator*(const Mat4 &n) const
{
          vec *a = (vec *)m;
          vec *b = (vec *)n.m;

          return    Mat4( MAT(a,0,0)*MAT(b,0,0) + MAT(a,0,1)*MAT(b,1,0) + MAT(a,0,2)*MAT(b,2,0) + MAT(a,0,3)*MAT(b,3,0), 
                                           MAT(a,0,0)*MAT(b,0,1) + MAT(a,0,1)*MAT(b,1,1) + MAT(a,0,2)*MAT(b,2,1) + MAT(a,0,3)*MAT(b,3,1), 
                                           MAT(a,0,0)*MAT(b,0,2) + MAT(a,0,1)*MAT(b,1,2) + MAT(a,0,2)*MAT(b,2,2) + MAT(a,0,3)*MAT(b,3,2),
                                           MAT(a,0,0)*MAT(b,0,3) + MAT(a,0,1)*MAT(b,1,3) + MAT(a,0,2)*MAT(b,2,3) + MAT(a,0,3)*MAT(b,3,3),

                                           MAT(a,1,0)*MAT(b,0,0) + MAT(a,1,1)*MAT(b,1,0) + MAT(a,1,2)*MAT(b,2,0) + MAT(a,1,3)*MAT(b,3,0), 
                                           MAT(a,1,0)*MAT(b,0,1) + MAT(a,1,1)*MAT(b,1,1) + MAT(a,1,2)*MAT(b,2,1) + MAT(a,1,3)*MAT(b,3,1), 
                                           MAT(a,1,0)*MAT(b,0,2) + MAT(a,1,1)*MAT(b,1,2) + MAT(a,1,2)*MAT(b,2,2) + MAT(a,1,3)*MAT(b,3,2),
                                           MAT(a,1,0)*MAT(b,0,3) + MAT(a,1,1)*MAT(b,1,3) + MAT(a,1,2)*MAT(b,2,3) + MAT(a,1,3)*MAT(b,3,3),

                                           MAT(a,2,0)*MAT(b,0,0) + MAT(a,2,1)*MAT(b,1,0) + MAT(a,2,2)*MAT(b,2,0) + MAT(a,2,3)*MAT(b,3,0), 
                                           MAT(a,2,0)*MAT(b,0,1) + MAT(a,2,1)*MAT(b,1,1) + MAT(a,2,2)*MAT(b,2,1) + MAT(a,2,3)*MAT(b,3,1), 
                                           MAT(a,2,0)*MAT(b,0,2) + MAT(a,2,1)*MAT(b,1,2) + MAT(a,2,2)*MAT(b,2,2) + MAT(a,2,3)*MAT(b,3,2),
                                           MAT(a,2,0)*MAT(b,0,3) + MAT(a,2,1)*MAT(b,1,3) + MAT(a,2,2)*MAT(b,2,3) + MAT(a,2,3)*MAT(b,3,3),
                   
                                           MAT(a,3,0)*MAT(b,0,0) + MAT(a,3,1)*MAT(b,1,0) + MAT(a,3,2)*MAT(b,2,0) + MAT(a,3,3)*MAT(b,3,0), 
                                           MAT(a,3,0)*MAT(b,0,1) + MAT(a,3,1)*MAT(b,1,1) + MAT(a,3,2)*MAT(b,2,1) + MAT(a,3,3)*MAT(b,3,1), 
                                           MAT(a,3,0)*MAT(b,0,2) + MAT(a,3,1)*MAT(b,1,2) + MAT(a,3,2)*MAT(b,2,2) + MAT(a,3,3)*MAT(b,3,2),
                                           MAT(a,3,0)*MAT(b,0,3) + MAT(a,3,1)*MAT(b,1,3) + MAT(a,3,2)*MAT(b,2,3) + MAT(a,3,3)*MAT(b,3,3)  );

}

INLINE Mat4& Mat4::operator*=(const Mat4& n)
{
          vec *a = (vec *)m;
          vec *b = (vec *)n.m;

          set( MAT(a,0,0)*MAT(b,0,0) + MAT(a,0,1)*MAT(b,1,0) + MAT(a,0,2)*MAT(b,2,0) + MAT(a,0,3)*MAT(b,3,0), 
                   MAT(a,0,0)*MAT(b,0,1) + MAT(a,0,1)*MAT(b,1,1) + MAT(a,0,2)*MAT(b,2,1) + MAT(a,0,3)*MAT(b,3,1), 
                   MAT(a,0,0)*MAT(b,0,2) + MAT(a,0,1)*MAT(b,1,2) + MAT(a,0,2)*MAT(b,2,2) + MAT(a,0,3)*MAT(b,3,2),
                   MAT(a,0,0)*MAT(b,0,3) + MAT(a,0,1)*MAT(b,1,3) + MAT(a,0,2)*MAT(b,2,3) + MAT(a,0,3)*MAT(b,3,3),

                   MAT(a,1,0)*MAT(b,0,0) + MAT(a,1,1)*MAT(b,1,0) + MAT(a,1,2)*MAT(b,2,0) + MAT(a,1,3)*MAT(b,3,0), 
                   MAT(a,1,0)*MAT(b,0,1) + MAT(a,1,1)*MAT(b,1,1) + MAT(a,1,2)*MAT(b,2,1) + MAT(a,1,3)*MAT(b,3,1), 
                   MAT(a,1,0)*MAT(b,0,2) + MAT(a,1,1)*MAT(b,1,2) + MAT(a,1,2)*MAT(b,2,2) + MAT(a,1,3)*MAT(b,3,2),
                   MAT(a,1,0)*MAT(b,0,3) + MAT(a,1,1)*MAT(b,1,3) + MAT(a,1,2)*MAT(b,2,3) + MAT(a,1,3)*MAT(b,3,3),

                   MAT(a,2,0)*MAT(b,0,0) + MAT(a,2,1)*MAT(b,1,0) + MAT(a,2,2)*MAT(b,2,0) + MAT(a,2,3)*MAT(b,3,0), 
                   MAT(a,2,0)*MAT(b,0,1) + MAT(a,2,1)*MAT(b,1,1) + MAT(a,2,2)*MAT(b,2,1) + MAT(a,2,3)*MAT(b,3,1), 
                   MAT(a,2,0)*MAT(b,0,2) + MAT(a,2,1)*MAT(b,1,2) + MAT(a,2,2)*MAT(b,2,2) + MAT(a,2,3)*MAT(b,3,2),
                   MAT(a,2,0)*MAT(b,0,3) + MAT(a,2,1)*MAT(b,1,3) + MAT(a,2,2)*MAT(b,2,3) + MAT(a,2,3)*MAT(b,3,3),

                   MAT(a,3,0)*MAT(b,0,0) + MAT(a,3,1)*MAT(b,1,0) + MAT(a,3,2)*MAT(b,2,0) + MAT(a,3,3)*MAT(b,3,0), 
                   MAT(a,3,0)*MAT(b,0,1) + MAT(a,3,1)*MAT(b,1,1) + MAT(a,3,2)*MAT(b,2,1) + MAT(a,3,3)*MAT(b,3,1), 
                   MAT(a,3,0)*MAT(b,0,2) + MAT(a,3,1)*MAT(b,1,2) + MAT(a,3,2)*MAT(b,2,2) + MAT(a,3,3)*MAT(b,3,2),
                   MAT(a,3,0)*MAT(b,0,3) + MAT(a,3,1)*MAT(b,1,3) + MAT(a,3,2)*MAT(b,2,3) + MAT(a,3,3)*MAT(b,3,3)  );

        return *this;
}


INLINE Vec4 Mat4::operator*(const Vec4 &n) const
{       
        vec *p = (vec *)n.v;

        return Vec4(   p[0]*MAT(m,0,0) + p[1]*MAT(m,0,1) + p[2]*MAT(m,0,2) + p[3]*MAT(m,0,3), 
                                        p[0]*MAT(m,1,0) + p[1]*MAT(m,1,1) + p[2]*MAT(m,1,2) + p[3]*MAT(m,1,3), 
                                        p[0]*MAT(m,2,0) + p[1]*MAT(m,2,1) + p[2]*MAT(m,2,2) + p[3]*MAT(m,2,3),
                                        p[0]*MAT(m,3,0) + p[1]*MAT(m,3,1) + p[2]*MAT(m,3,2) + p[3]*MAT(m,3,3) );
};

INLINE Vec3 Mat4::operator*(const Vec3 &n) const
{       
        vec *v = (vec *)n.v;

        return    Vec3( v[0]*MAT(m,0,0) + v[1]*MAT(m,0,1) + v[2]*MAT(m,0,2) + MAT(m,0,3) , 
                                     v[0]*MAT(m,1,0) + v[1]*MAT(m,1,1) + v[2]*MAT(m,1,2) + MAT(m,1,3), 
                                     v[0]*MAT(m,2,0) + v[1]*MAT(m,2,1) + v[2]*MAT(m,2,2) + MAT(m,2,3) );
}



INLINE Mat4 Mat4::operator-(const Mat4& n) const
{
        const vec *a = &m[0];
        const vec *b = &n.m[0];

        Mat4 c( MAT(a,0,0)-MAT(b,0,0), MAT(a,0,1)-MAT(b,0,1), 
                         MAT(a,0,2)-MAT(b,0,2), MAT(a,0,3)-MAT(b,0,3), 
                        
                         MAT(a,1,0)-MAT(b,1,0), MAT(a,1,1)-MAT(b,1,1), 
                         MAT(a,1,2)-MAT(b,1,2), MAT(a,1,3)-MAT(b,1,3), 

                         MAT(a,2,0)-MAT(b,2,0), MAT(a,2,1)-MAT(b,2,1), 
                         MAT(a,2,2)-MAT(b,2,2), MAT(a,2,3)-MAT(b,2,3), 

                         MAT(a,3,0)-MAT(b,3,0), MAT(a,3,1)-MAT(b,3,1), 
                         MAT(a,3,2)-MAT(b,3,2), MAT(a,3,3)-MAT(b,3,3)  );
                
        return c;
}

INLINE Mat4 Mat4::operator+(const Mat4& n) const
{
        const vec *a = &m[0];
        const vec *b = &n.m[0];

        Mat4 c( MAT(a,0,0)+MAT(b,0,0), MAT(a,0,1)+MAT(b,0,1), 
                         MAT(a,0,2)+MAT(b,0,2), MAT(a,0,3)+MAT(b,0,3), 
                        
                         MAT(a,1,0)+MAT(b,1,0), MAT(a,1,1)+MAT(b,1,1), 
                         MAT(a,1,2)+MAT(b,1,2), MAT(a,1,3)+MAT(b,1,3), 

                         MAT(a,2,0)+MAT(b,2,0), MAT(a,2,1)+MAT(b,2,1), 
                         MAT(a,2,2)+MAT(b,2,2), MAT(a,2,3)+MAT(b,2,3), 

                         MAT(a,3,0)+MAT(b,3,0), MAT(a,3,1)+MAT(b,3,1), 
                         MAT(a,3,2)+MAT(b,3,2), MAT(a,3,3)+MAT(b,3,3)  );
                
        return c;
}

INLINE Mat4 Mat4::operator*(const vec s) const
{
        vec *p = (vec *)&m[0];
    return Mat4( MAT(p,0,0)*s, MAT(p,0,1)*s, MAT(p,0,2)*s, MAT(p,0,3)*s,
                                  MAT(p,1,0)*s, MAT(p,1,1)*s, MAT(p,1,2)*s, MAT(p,1,3)*s,
                                  MAT(p,2,0)*s, MAT(p,2,1)*s, MAT(p,2,2)*s, MAT(p,2,3)*s,
                                  MAT(p,3,0)*s, MAT(p,3,1)*s, MAT(p,3,2)*s, MAT(p,3,3)*s  );
}

INLINE Mat4 Mat4::transpose()
{
    vec *p;
    p = (vec *)&m[0];
    
    return Mat4( MAT(p,0,0), MAT(p,1,0), MAT(p,2,0), MAT(p,3,0),
                              MAT(p,0,1), MAT(p,1,1), MAT(p,2,1), MAT(p,3,1),
                              MAT(p,0,2), MAT(p,1,2), MAT(p,2,2), MAT(p,3,2),
                              MAT(p,0,3), MAT(p,1,3), MAT(p,2,3), MAT(p,3,3)  );
}


INLINE Mat4 Mat4::matrixFromQuat( Vec4 &quat )
{
	   vec wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;
	   Mat4 matrix;
	   vec *mv = matrix.raw();

	   // calculate coefficients
	   x2 = quat[X] + quat[X]; 
	   y2 = quat[Y] + quat[Y]; 
	   z2 = quat[Z] + quat[Z];
	   xx = quat[X] * x2;   xy = quat[X] * y2;   xz = quat[X] * z2;
	   yy = quat[Y] * y2;   yz = quat[Y] * z2;   zz = quat[Z] * z2;
	   wx = quat[W] * x2;   wy = quat[W] * y2;   wz = quat[W] * z2;

	   MAT(mv,0,0) = 1.0 - (yy + zz);    MAT(mv,0,1) = xy - wz;
	   MAT(mv,0,2) = xz + wy;            MAT(mv,0,3) = 0.0;
  
	   MAT(mv,1,0) = xy + wz;            MAT(mv,1,1) = 1.0 - (xx + zz);
	   MAT(mv,1,2) = yz - wx;            MAT(mv,1,3) = 0.0;

	   MAT(mv,2,0) = xz - wy;            MAT(mv,2,1) = yz + wx;
	   MAT(mv,2,2) = 1.0 - (xx + yy);    MAT(mv,2,3) = 0.0;

	   MAT(mv,3,0) = 0;                  MAT(mv,3,1) = 0;
	   MAT(mv,3,2) = 0;                  MAT(mv,3,3) = 1;         

	   return matrix;
}

INLINE Mat4  Mat4::matrixFromQuatPos( Vec4 &quat, Vec3 &pos )
{
	   vec wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;
	   Mat4 matrix;
	   vec *mv = matrix.raw();

	   // calculate coefficients
	   x2 = quat[X] + quat[X]; 
	   y2 = quat[Y] + quat[Y]; 
	   z2 = quat[Z] + quat[Z];
	   xx = quat[X] * x2;   xy = quat[X] * y2;   xz = quat[X] * z2;
	   yy = quat[Y] * y2;   yz = quat[Y] * z2;   zz = quat[Z] * z2;
	   wx = quat[W] * x2;   wy = quat[W] * y2;   wz = quat[W] * z2;

	   MAT(mv,0,0) = 1.0-(yy+zz); MAT(mv,0,1) = xy - wz;    MAT(mv,0,2) = xz + wy;        MAT(mv,0,3) = pos[0];
	   MAT(mv,1,0) = xy + wz;     MAT(mv,1,1) = 1.0-(xx+zz);MAT(mv,1,2) = yz - wx;        MAT(mv,1,3) = pos[1];
	   MAT(mv,2,0) = xz - wy;     MAT(mv,2,1) = yz + wx;    MAT(mv,2,2) = 1.0 - (xx + yy);MAT(mv,2,3) = pos[2];
	   MAT(mv,3,0) = 0;           MAT(mv,3,1) = 0;          MAT(mv,3,2) = 0;              MAT(mv,3,3) = 1;         

	   return matrix;
}

#ifdef MATHLIB_PRINT_IOSTREAM
#include <iostream.h>
INLINE void Mat4::print()
{
  cout << MAT(m,0,0) << " " << MAT(m,0,1) << " " << MAT(m,0,2) << " " << MAT(m,0,3) << endl;
  cout << MAT(m,1,0) << " " << MAT(m,1,1) << " " << MAT(m,1,2) << " " << MAT(m,1,3) << endl;
  cout << MAT(m,2,0) << " " << MAT(m,2,1) << " " << MAT(m,2,2) << " " << MAT(m,2,3) << endl;
  cout << MAT(m,3,0) << " " << MAT(m,3,1) << " " << MAT(m,3,2) << " " << MAT(m,3,3) << endl;
}
#endif

#ifdef MATHLIB_PRINT_STDIO
#include <stdio.h>
INLINE void Mat4::print()
{
  printf( "%f %f %f %f\n", MAT(m,0,0), MAT(m,0,1), MAT(m,0,2), MAT(m,0,3) );
  printf( "%f %f %f %f\n", MAT(m,1,0), MAT(m,1,1), MAT(m,1,2), MAT(m,1,3) );
  printf( "%f %f %f %f\n", MAT(m,2,0), MAT(m,2,1), MAT(m,2,2), MAT(m,2,3) );
  printf( "%f %f %f %f\n", MAT(m,3,0), MAT(m,3,1), MAT(m,3,2), MAT(m,3,3) );
}
#endif



/*! returns a scaling transformation matrix, which will scale by the vector v */
INLINE Mat4 Mat4::scale(const Vec3& v) 
{
  return Mat4( v[0] ,   0  ,   0  ,  0 ,
				 0  , v[1] ,   0  ,  0 ,
				 0  ,   0  , v[2] ,  0 ,
				 0  ,   0  ,   0  ,  1 );
}

/*! Returns a translaion matrix, which will translate by the vector v */
INLINE Mat4 Mat4::translate(const Vec3& v) 
{
  return Mat4(   1  ,   0  ,   0  , v[0] ,
				 0  ,   1  ,   0  , v[1] ,
		         0  ,   0  ,   1  , v[2] ,
		         0  ,   0  ,   0  ,   1  );
}

/*! returns a rotation matrix effecting a rotation around the X axis by specified radians */
INLINE Mat4 Mat4::rotX(const vec a) 
{

  return Mat4(    1    ,    0    ,    0    ,    0    ,
		          0    ,  cos(a) , -sin(a) ,    0    ,
		          0    ,  sin(a) ,  cos(a) ,    0    ,
		          0    ,    0    ,    0    ,    1    );
}

/*! Returns a rotation matrix effecting rotation around the Y axis */
INLINE Mat4 Mat4::rotY(const vec a) 
{
	return Mat4(  cos(a) ,    0    ,  sin(a) ,    0    ,
			   	       0 ,    1    ,    0    ,    0    ,
				 -sin(a) ,    0    ,  cos(a) ,    0    ,
					   0 ,    0    ,    0    ,    1    );
}

/*! Returns a rotation matrix effecting rotation around the Z axis */
INLINE Mat4 Mat4::rotZ(const vec a) 
{
  return Mat4(  cos(a) , -sin(a) ,    0    ,    0    ,
   		        sin(a) ,  cos(a) ,    0    ,    0    ,
 		          0    ,    0    ,    1    ,    0    ,
 		          0    ,    0    ,    0    ,    1    );
}

/*! Returns a rotation matrix effecting a rotation around the given vector and point, by the specified number of radians.*/
INLINE Mat4 Mat4::rot(const Vec3 &p, const Vec3 &v, const vec a)
{
  vec ax,ay;
  vec angle = a;
  vec atanv;

  if(v[2] == 0){

	atanv = atan(v[0]/v[1]);
    return
      translate(-p) * 
	  rotZ( -atanv ) * 
	  rotY(angle) * 
	  rotZ( atanv ) *
      translate(p);

  }else{

    ax = atan(v[0]/v[2]);
    ay = -asin(v[1]/ SQRT(v[X]*v[X]+v[Y]*v[Y]+v[Z]*v[Z]) );
    
    if(v[2]<0) ax = ax+M_PI;
    
    return 
      translate(-p)*
      rotY(ax)*
      rotX(ay)*
      rotZ(angle)*
      rotX(-ay)*
      rotY(-ax)*
      translate(p);

  }  

}

// Returns the inverse matrix of scale_mat()
INLINE Mat4 Mat4::inv_scale(const Vec3 &v) 
{
  return Mat4( 1/v[0] ,   0    ,   0    ,  0 ,
      	         0    , 1/v[1] ,   0    ,  0 ,
		         0    ,   0    , 1/v[2] ,  0 ,
		         0    ,   0    ,   0    ,  1 );
}

// Returns the inverse matrix of trans_may()
INLINE Mat4 Mat4::inv_translate(const Vec3 &v) 
{
  return Mat4(   1  ,   0  ,   0  , -v[0] ,
				 0  ,   1  ,   0  , -v[1] ,
			     0  ,   0  ,   1  , -v[2] ,
				 0  ,   0  ,   0  ,   1   );
}

// Returns the inverse matrix of rotX_mat()
INLINE Mat4 Mat4::inv_rotX(const vec a)
{
  return Mat4(    1    ,    0    ,    0    ,    0    ,
				  0    ,  cos(a) ,  sin(a) ,    0    ,
				  0    , -sin(a) ,  cos(a) ,    0    ,
		          0    ,    0    ,    0    ,    1    );
}

// Returns the inverse matrix of rotY_mat()
INLINE Mat4 Mat4::inv_rotY(const vec a)
{
  return Mat4(  cos(a) ,    0    , -sin(a) ,    0    ,
				  0    ,    1    ,    0    ,    0    ,
				sin(a) ,    0    ,  cos(a) ,    0    ,
				  0    ,    0    ,    0    ,    1    );
}

// Returns the inverse matrix of rotZ_mat()
INLINE Mat4 Mat4::inv_rotZ(const vec a)
{
  return Mat4(       cos(a) ,  sin(a) ,    0    ,    0    ,
 					-sin(a) ,  cos(a) ,    0    ,    0    ,
 					   0    ,    0    ,    1    ,    0    ,
 					   0    ,    0    ,    0    ,    1    );
}

// Returns the inverse matrix of rot_mat()
INLINE Mat4 Mat4::inv_rot(const Vec3 &p, const Vec3 &v, const vec a){
  double ax,ay;
  double angle = a;
  double atanv;

  if(v[2] == 0){
	atanv = atan(v[0]/v[1]);

    return
      translate(-p)*
      rotZ(-atanv)*
      rotY(angle)*
      rotZ( atanv)*
      translate(p);

  }else{

    ax = atan(v[0]/v[2]);
    ay = -asin(v[1]/ SQRT(v[X]*v[X]+v[Y]*v[Y]+v[Z]*v[Z]) );
    
    if(v[2]<0){
      ax = ax+M_PI;
    }

    return 
      translate(-p)*
      rotY(ax)*
      rotX(ay)*
      rotZ(-angle)*
      rotX(-ay)*
      rotY(-ax)*
      translate(p);
  }  
}

#endif



#ifdef ACTIVE_GLOBALDEF
        #undef ACTIVE_GLOBALDEF
#endif
#ifdef ACTIVE_MATRIX_CODE
        #undef ACTIVE_MATRIX_CODE
#endif
        
        



