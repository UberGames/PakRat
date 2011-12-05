extern "C" {
	#include <assert.h>
	#include <math.h>
	#include <stdio.h>
	#include <stdarg.h>
	#include <string.h>
	#include <stdlib.h>
	#include <time.h>
	#include <ctype.h>

	//#include "a_shared.h"
	#include "matrix.h"
}

/* Stolen from Mesa:matrix.c */
#define A(row,col)  a[(col<<2)+row]
#define B(row,col)  b[(col<<2)+row]
#define P(row,col)  product[(col<<2)+row]

void Matrix3_Multiply_Vec3 (vec3_t a[3],vec3_t b,vec3_t product);


void Matrix4_Identity(mat4_t mat )
{
	memset(mat,0,16 *sizeof (float));

	mat[0]=mat[5]=mat[10]=mat[15]=1.0;

}

void Matrix3_Identity (vec3_t mat [3])
{
	memset(mat,0,9 *sizeof (float));
	mat[0][0]=mat[1][1]=mat[2][2]=1.0;
}


void Matrix4_Multiply(mat4_t a, mat4_t b, mat4_t product)
{
   int i;
   for (i = 0; i < 4; i++)
   {
      float ai0=A(i,0),  ai1=A(i,1),  ai2=A(i,2),  ai3=A(i,3);
      P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
      P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
      P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
      P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
   }

}

void Matrix3_Multiply (vec3_t in1[3], vec3_t in2[3], vec3_t out[3])
{
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +
				in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +
				in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +
				in1[0][2] * in2[2][2];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +
				in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +
				in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +
				in1[1][2] * in2[2][2];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +
				in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +
				in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +
				in1[2][2] * in2[2][2];
}

void Matrix3_Multiply_Vec3 (vec3_t a[3],vec3_t b,vec3_t product)
{
	float b0=b[0], b1=b[1], b2=b[2];

	product[0] = a[0][0]*b0 + a[1][0]*b1 + a[2][0]*b2 ;
	product[1] = a[0][1]*b0 + a[1][1]*b1 + a[2][1]*b2 ;
	product[2] = a[0][2]*b0 + a[1][2]*b1 + a[2][2]*b2 ;


}

// This can be used to calc the inverse of a rotation matrix 
void Matrix3_Transponse (vec3_t in [3] ,vec3_t out [3])
{

	vec3_t tmp [3];

	memcpy (tmp,in,9* sizeof (float ));

	out [0][0] = tmp [0][0];
	out [1][1] = tmp [1][1];
	out [2][2] = tmp [2][2];

	out [0][1] = tmp [1][0];
	out [1][0] = tmp [0][1];

	out [0][2] = tmp [2][0];
	out [2][0] = tmp [0][2];

}


void Matrix_Multiply_Vec4 (mat4_t a, vec4_t b, vec4_t product)
{

    float b0=b[0], b1=b[1], b2=b[2], b3=b[3];

	product[0] = a[0]*b0 + a[4]*b1 + a[8]*b2 + a[12]*b3;
	product[1] = a[1]*b0 + a[5]*b1 + a[9]*b2 + a[13]*b3;
	product[2] = a[2]*b0 + a[6]*b1 + a[10]*b2 + a[14]*b3;
	product[3] = a[3]*b0 + a[7]*b1 + a[11]*b2 + a[15]*b3;

}

void Matrix_Multiply_Vec3 (mat4_t a, vec3_t b, vec3_t product)
{

    float b0=b[0], b1=b[1], b2=b[2], b3=b[3];

	product[0] = a[0]*b0 + a[4]*b1 + a[8]*b2 + a[12]*b3;
	product[1] = a[1]*b0 + a[5]*b1 + a[9]*b2 + a[13]*b3;
	product[2] = a[2]*b0 + a[6]*b1 + a[10]*b2 + a[14]*b3;

}

void Matrix_Multiply_Vec2 (mat4_t a, vec2_t b, vec2_t product)
{
	float b0=b[0], b1=b[1];
  
	product[0] = A(0,0)*b0 + A(0,1)*b1 + A(0,2)+ A(0,3);
	product[1] = A(1,0)*b0 + A(1,1)*b1 + A(1,2)+ A(1,3);
    


}


static float Matrix3_Det (float *mat)
{
	float det;

    det = mat[0] * ( mat[4]*mat[8] - mat[7]*mat[5] )
         - mat[1] * ( mat[3]*mat[8] - mat[6]*mat[5] )
         + mat[2] * ( mat[3]*mat[7] - mat[6]*mat[4] );

    return( det );
      

}



static void Matrix3_Inverse( float *mr, float *ma )
{
     float det =Matrix3_Det ( ma );
#warning removed
//     if ( fabs( det ) < 0.0005 )
//      {
//			Matrix3_Identity( (vec3_t)ma );
//			return;
//       }

     mr[0] =    ma[4]*ma[8] - ma[5]*ma[7]   / det;
     mr[1] = -( ma[1]*ma[8] - ma[7]*ma[2] ) / det;
     mr[2] =    ma[1]*ma[5] - ma[4]*ma[2]   / det;

     mr[3] = -( ma[3]*ma[8] - ma[5]*ma[6] ) / det;
     mr[4] =    ma[0]*ma[8] - ma[6]*ma[2]   / det;
     mr[5] = -( ma[0]*ma[5] - ma[3]*ma[2] ) / det;

     mr[6] =    ma[3]*ma[7] - ma[6]*ma[4]   / det;
     mr[7] = -( ma[0]*ma[7] - ma[6]*ma[1] ) / det;
     mr[8] =    ma[0]*ma[4] - ma[1]*ma[3]   / det;
}

static void Matrix4_Submat( mat4_t mr, float * mb, int i, int j )
{
    int ti, tj, idst, jdst;

    for ( ti = 0; ti < 4; ti++ )
        {
			if ( ti < i )
				idst = ti;
			else
			if ( ti > i )
				idst = ti-1;

			for ( tj = 0; tj < 4; tj++ )
			{
				if ( tj < j )
					jdst = tj;
				else
				if ( tj > j )
					jdst = tj-1;

				if ( ti != i && tj != j )
					 mb[idst*3 + jdst] = mr[ti*4 + tj ];
			}
        }
}

static float Matrix4_Det( mat4_t mr )
{
     float   det, result = 0, i = 1;
     float msub3[9];
     int     n;

     for ( n = 0; n < 4; n++, i *= -1 )
        {
			Matrix4_Submat( mr, msub3, 0, n );

			det     = Matrix3_Det( msub3 );
			result += mr[n] * det * i;
        }

      return( result );
}



int Matrix4_Inverse( mat4_t mr, mat4_t ma )
{
    float  mdet = Matrix4_Det( ma );
    float  mtemp[9];
      int     i, j, sign;

      if ( fabs( mdet ) < 0.0005 )
        return( 0 );

      for ( i = 0; i < 4; i++ )
			for ( j = 0; j < 4; j++ )
			{
				sign = 1 - ( (i +j) % 2 ) * 2;

				Matrix4_Submat( ma, mtemp, i, j );

				mr[i+j*4] = ( Matrix3_Det( mtemp ) * sign ) / mdet;
			}

      return( 1 );
}




