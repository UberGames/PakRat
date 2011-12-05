/*
  mathlib.h
  
  defines vec a floating point scalar, element of vectors
  defines classes Vec2, Vec3, Vec4 which are 2, 3 and 4 element vectors with 
  standard math operation defined on them. 
  ex, VecX * VecX is a dot product resulting in a vec scalar.
  ex. VecX ^ VecX is a cross product resulting in a VecX vector

  VecX classes can be defined as a copy of a previous vector, by giving immediate
  scalar values, or by supplying a pointer to the data. when a pointer is given in a 
  constructor the data is not copied into the VecX object, it uses the reference. 
  The desctructor frees memory, in case of using a reference to ouside memory this can 
  cause problems unless the pointer is set to null before deallocation.
  defines Mat4, a 4x4 row major matrix with standard matrix operations defined

*/


#ifndef _MATHLIB_H_
#define _MATHLIB_H_

#include <math.h>
#include "mathdefs.h"


/*===================================================================================
Vec2 
=====================================================================================*/
// TODO: fix so it conforms to array standards

class Vec2
{
        friend class Vec2;
        friend class Vec3;
        friend class Vec4;
        friend class Mat4;

		private:
            vec v[2];

        public: 
            Vec2( )                  { set( 0, 0 ); }
            Vec2( vec x, vec y ) { set( x, y);  } 
            Vec2( const Vec2 &v2 )  { set( &v2.v[0] );  } 
            Vec2( vec *vP )        { set( vP );   }
                
            INLINE void set( vec x, vec y );
                        INLINE void set( const vec *vP );
                        
            inline vec  operator()(int i) const  { return v[i]; }
            inline vec& operator()(int i)        { return v[i]; }                
            inline vec& operator[](int i)        { return v[i]; }
            inline vec  operator[](int i) const  { return v[i]; }                
            inline vec  *raw()                   { return v;    }                
            
            INLINE MBoolean operator==(const Vec2& ) const;
            INLINE MBoolean operator!=(const Vec2& ) const;
     
            inline Vec2& operator=( const Vec2& v2 ) { set(&v2.v[0]); return *this; }
            INLINE Vec2& operator+=( const Vec2& ); 
            INLINE Vec2& operator-=( const Vec2& );
            INLINE Vec2& operator*=( vec );
            INLINE Vec2& operator/=( vec );
            INLINE Vec2  operator+( const Vec2& ) const;
            INLINE Vec2  operator-( const Vec2& ) const;
            INLINE Vec2  operator-( ) const;
            INLINE Vec2  operator*( vec s ) const;
            INLINE Vec2  operator/( vec s ) const;
            INLINE vec  operator*( const Vec2& ) const;
            INLINE Vec2  operator^( const Vec2& ) const;

            INLINE vec  length( );
            INLINE void   normalize( );
            INLINE void   inverse( );                              
};

/*===================================================================================

Vec3

=====================================================================================*/

class Vec3
{
        friend class Vec2;
        friend class Vec3;
        friend class Vec4;
        friend class Mat4;

        private:
            vec v[3];

        protected:                              

        public:
            Vec3( )						{ set( 0, 0, 0);   }
            Vec3( vec x, vec y, vec z ) { set( x, y, z );  }
            Vec3( vec x, vec y )        { set( x, y, 0 );  }
            Vec3( vec *vP  )            { set( vP[0], vP[1], vP[2] ); }                 
            Vec3( const Vec3 &v3 )      { set( v3.v ); }                        
            Vec3( const Vec2 &v2 )      { set( v2.v[X], v2.v[Y], 0 ); }                 

            INLINE void   set( vec x, vec y, vec z );
            INLINE void   set( const vec *vP );
            inline vec  *raw() { return v; }                

            inline vec  operator()( int i ) const   { return v[i]; }
            inline vec& operator()( int i )         { return v[i]; }                
            inline vec  operator[]( int i ) const   { return v[i]; }                
            inline vec& operator[]( int i )         { return v[i]; }
                              
            INLINE MBoolean   operator==( const Vec3& ) const;
            INLINE MBoolean   operator!=( const Vec3& ) const;    
            inline Vec3& operator=( const Vec3& v3 ) { set(v3.v); return *this; }
            inline Vec3& operator=( const Vec2& v2 ) { set(v2.v); return *this; }

            INLINE Vec3& operator+=( const Vec3& ); 
            INLINE Vec3& operator-=( const Vec3& );
            INLINE Vec3& operator*=( vec );
            INLINE Vec3& operator/=( vec );
            INLINE Vec3  operator+( const Vec3& ) const;
            INLINE Vec3  operator-( const Vec3& ) const;
            INLINE Vec3  operator-( ) const;
            INLINE Vec3  operator*( vec s ) const;
            INLINE Vec3  operator/( vec s ) const;
            INLINE vec  operator*( const Vec3& ) const;
            INLINE Vec3  operator^( const Vec3& ) const;

            INLINE vec  length( );
            INLINE void   normalize( );
            INLINE void   inverse( );

#ifdef MATHLIB_PRINT_IOSTREAM
			INLINE void   print( ostream& );
#endif

#ifdef MATHLIB_PRINT_STDIO
			INLINE void   print();
#endif
};


/*===================================================================================
Vec4
=====================================================================================*/

class Vec4
{
        friend class Vec2;
        friend class Vec3;
        friend class Vec4;
        friend class Mat4;


       private:
                vec v[4];
 
        protected:                      

        public:
            Vec4( )  { set( 0, 0, 0, 1 ); }
            Vec4( vec x, vec y, vec z, vec w ) { set( x, y, z, w ); }
            Vec4( vec x, vec y, vec z) { set( x, y, z, 1 ); }
            Vec4( vec x, vec y )  { set( x, y, 0, 1 ); }
            Vec4( const Vec4 &v4 ) { set( v4.v[X], v4.v[Y], v4.v[Z], v4.v[W] ); }
            Vec4( const Vec3 &v3, vec w ) { set( v3.v[X], v3.v[Y], v3.v[Z], w );       }
            Vec4( const Vec3 &v3 ) { set( v3.v[X], v3.v[Y], v3.v[Z], 1 );       }
            Vec4( vec *vP ) { set( vP ); }                  


            INLINE void   set( vec x, vec y, vec z, vec w );
            INLINE void   set( const vec *vP );
            INLINE void   set( const Vec4 &v4 ) { set( v4.v ); }
                           
            inline vec  operator()(int i) const  { return v[i]; }
            inline vec& operator()(int i)        { return v[i]; }                
            inline vec& operator[](int i)        { return v[i]; }
            inline vec  operator[](int i) const  { return v[i]; }                
            inline vec  *raw()                   { return v;    }                

            INLINE MBoolean   operator==(const Vec4& ) const;
            INLINE MBoolean   operator!=(const Vec4& ) const;
                                  
            inline Vec4& operator=( const Vec4& v4 ) { set(v4.v); return *this; }
            inline Vec4& operator=( const Vec3& v3 ) { set(v3.v); return *this; }

            INLINE Vec4& operator+=( const Vec4& ); 
            INLINE Vec4& operator-=( const Vec4& );
            INLINE Vec4& operator*=( vec );
            INLINE Vec4& operator/=( vec );
            INLINE Vec4  operator+( const Vec4& ) const;
            INLINE Vec4  operator+( const Vec3& ) const;
            INLINE Vec4  operator-( const Vec4& ) const;
            INLINE Vec4  operator-( ) const;
            INLINE Vec4  operator*( vec s ) const;
            INLINE Vec4  operator/( vec s ) const;
            INLINE vec  operator*( const Vec4& ) const;
               
            INLINE vec  length( );
            INLINE void normalize( );
            INLINE void inverse( );

			static INLINE Vec4 quatFromMatrix( Mat4& );
			static INLINE Vec4 quatSlerp( Vec4 &from, Vec4 &to, vec frac );

#ifdef MATHLIB_PRINT_IOSTREAM
			INLINE void   print( ostream& );
#endif

#ifdef MATHLIB_PRINT_STDIO
			INLINE void   print();
#endif

};


/*===================================================================================
Mat4 - 4x4 matrix object
=====================================================================================*/

class Mat4
{
        friend class Vec2;
        friend class Vec3;
        friend class Vec4;
        friend class Mat4;

public:
		vec m[16];

        INLINE Mat4( );

        INLINE Mat4( const vec m0,  const vec m1,  const vec m2,  const vec m3, 
                     const vec m4,  const vec m5,  const vec m6,  const vec m7,
                     const vec m8,  const vec m9,  const vec m10, const vec m11, 
                     const vec m12, const vec m13, const vec m14, const vec m15 );                 

        INLINE Mat4( vec *t );    
		INLINE Mat4(const Mat4& t);

        INLINE Mat4( const Vec3 v1, const vec rw1, 
					 const Vec3 v2, const vec rw2, 
                     const Vec3 v3, const vec rw3, 
                     const vec w0, const vec w1, const vec w2, const vec w3 );

        INLINE void set( const vec m0,  const vec m1,  const vec m2,  const vec m3, 
                         const vec m4,  const vec m5,  const vec m6,  const vec m7,
                         const vec m8,  const vec m9,  const vec m10, const vec m11, 
                         const vec m12, const vec m13, const vec m14, const vec m15 );                      

        INLINE void set( vec *t );
        INLINE void set(const Vec4& r0,const Vec4& r1,const Vec4& r2,const Vec4& r3);
	    INLINE void set(const Mat4& t);

        INLINE void set( const Vec3 v1, const vec rw1, 
						 const Vec3 v2, const vec rw2, 
                         const Vec3 v3, const vec rw3, 
                         const vec w0, const vec w1, const vec w2, const vec w3 );

        INLINE vec *raw();

        INLINE Mat4 operator*(const Mat4 &n) const;
        INLINE Vec4 operator*(const Vec4 &n) const;
        INLINE Vec3 operator*(const Vec3 &n) const;
        INLINE Mat4 operator*(const vec s) const;

        INLINE Mat4 operator-(const Mat4& n) const;
        INLINE Mat4 operator+(const Mat4& n) const;

        INLINE Mat4& operator*=(const Mat4& m);
        INLINE Mat4 transpose();


#ifdef MATHLIB_PRINT_IOSTREAM
			INLINE void   print( ostream& );
#endif

#ifdef MATHLIB_PRINT_STDIO
			INLINE void   print();
#endif

		static INLINE Mat4  matrixFromQuat( Vec4 &quat );
		static INLINE Mat4  matrixFromQuatPos( Vec4 &quat, Vec3 &pos );

		static INLINE Mat4  scale( const Vec3& );
		static INLINE Mat4  translate( const Vec3& );
		static INLINE Mat4  rotX( const vec a );
		static INLINE Mat4  rotY( const vec a );
		static INLINE Mat4  rotZ( const vec a );
		static INLINE Mat4  rot(const Vec3  &p, const Vec3 &v, const vec a);

		static INLINE Mat4  inv_scale( const Vec3& );
		static INLINE Mat4  inv_translate( const Vec3& );
		static INLINE Mat4  inv_rotX( const vec a );
		static INLINE Mat4  inv_rotY( const vec a );
		static INLINE Mat4  inv_rotZ( const vec a );
		static INLINE Mat4  inv_rot(const Vec3  &p, const Vec3 &v, const vec a);
};
        
#ifdef MATHLIB_INLINE
#define MATHLIB_INCLUDE
#include "matrixlib.C"
#include "veclib.C"
#undef  MATHLIB_INCLUDE
#endif


extern Mat4 identity;
extern Mat4 zero;
extern Mat4 unit;

#endif
