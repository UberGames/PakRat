/*
 mathlibdefs.h

  Copyright (C) 1998
  All rights reserved Matthew Baranowski Matthew_Baranowski@Brown.Edu
  You can redistribute it and/or modify the program for personal and NON-COMMERCIAL 
  purposes. If you do so please give credit to the author. This program is distributed 
  in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the 
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 shared definition for the mathlib
*/
#ifndef _MATHLIBDEFS_H_
#define _MATHLIBDEFS_H_

/*
 mathlib preprocessor switches
*/

#define MATRIX_COLUMN_MAJOR                          // selects if the matrices are row or column major, opengl like column major
//#define MATHLIB_INLINE                             // selects if mathlib functions are inline or not. 

//#define MATHLIB_PRINT_IOSTREAM					 // selects which liblary the is for debug printing
#define MATHLIB_PRINT_STDIO

#define QUAT_SLERP_DELTA 0.01   // specified the amount of error until quaternion spherical interploation falls back on linear

/*
useful macros
*/
#ifndef M_PI
#define M_PI 3.14159265259
#endif

#define DEGTORAD_CONST 0.01745329252             // macro for converting degrees to radians, all of the lib functions use radians
#define DEGTORAD( a ) (vec)((vec)(a)*(vec)(DEGTORAD_CONST))

#ifdef MATRIX_COLUMN_MAJOR
#define MAT( p, row, col ) (p)[((col)<<2)+(row)]
#else 
#define MAT( p, row, col ) (p)[((row)<<2)+(col)]
#endif


#ifdef MATHLIB_INLINE
#define INLINE inline
#else
#define INLINE 
#endif

typedef int MBoolean;

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif


#define SQRT sqrt

/*
 basic math lib datatypes
*/

#ifndef vec
#define vec float
#endif

#ifndef byte 
#define byte unsigned char
#endif

#define X 0
#define Y 1
#define Z 2
#define W 3


#ifndef ERROR_EPSILON
#define ERROR_EPSILON 0.01
#endif

#ifndef ERROR_EPSILON2
#define ERROR_EPSILON2 0.0001
#endif

#define VEC_MAX 3.402823466e+38F
#define VEC_MIN 1.175494351e-38F
/*
 bits masks
*/

#define BIT_1   0x0001
#define BIT_2   0x0002
#define BIT_3   0x0004
#define BIT_4   0x0008
#define BIT_5   0x0010
#define BIT_6   0x0020
#define BIT_7   0x0040
#define BIT_8   0x0080
#define BIT_9   0x0100
#define BIT_10  0x0200
#define BIT_11  0x0400
#define BIT_12  0x0800
#define BIT_13  0x1000
#define BIT_14  0x2000
#define BIT_15  0x4000
#define BIT_16  0x8000

#define IBIT_1  0xFFFE
#define IBIT_2  0xFFFD
#define IBIT_3  0xFFFB
#define IBIT_4  0xFFF7
#define IBIT_5  0xFFEF
#define IBIT_6  0xFFDF
#define IBIT_7  0xFFBF
#define IBIT_8  0xFF7F
#define IBIT_9  0xFEFF
#define IBIT_10 0xFDFF
#define IBIT_11 0xFBFF
#define IBIT_12 0xF7FF
#define IBIT_13 0xEFFF
#define IBIT_14 0xDFFF
#define IBIT_15 0xBFFF
#define IBIT_16 0x7FFF

#endif
