//#include "a_shared.h"


#ifndef MATRIX_H__
#define MATRIX_H__

typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef vec_t vec5_t[5];

typedef float texcoord_t [2];


typedef float mat4_t[16]; /* 4x4 matrix */
typedef vec3_t  mat3x3_t[3];

typedef	int	fixed4_t;
typedef	int	fixed8_t;
typedef	int	fixed16_t;




typedef float mat4_t[16]; /* 4x4 matrix */
typedef vec3_t  mat3x3_t[3];

typedef	int	fixed4_t;
typedef	int	fixed8_t;
typedef	int	fixed16_t;


typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef vec_t vec5_t[5];

typedef float texcoord_t [2];


typedef float mat4_t[16]; /* 4x4 matrix */
typedef vec3_t  mat3x3_t[3];

typedef	int	fixed4_t;
typedef	int	fixed8_t;
typedef	int	fixed16_t;

void Matrix4_Identity(mat4_t mat );
void Matrix4_Multiply(mat4_t a, mat4_t b, mat4_t product);
void Matrix_Multiply_Vec4 (mat4_t a, vec4_t b, vec4_t product);
void Matrix_Multiply_Vec2 (mat4_t a, vec2_t b, vec2_t product);

void Matrix3_Identity (vec3_t mat [3]);
void Matrix3_Multiply (vec3_t in1[3], vec3_t in2[3], vec3_t out[3]);
void Matrix3_Transponse (vec3_t in [3] ,vec3_t out [3]);

void Matrix_Multiply_Vec3 (mat4_t a, vec3_t b, vec3_t product);

int Matrix4_Inverse( mat4_t mr, mat4_t ma );










#endif 