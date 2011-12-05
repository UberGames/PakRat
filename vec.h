/* Aftershock 3D rendering engine
 * Copyright (C) 1999 Stephen C. Taylor
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#ifndef __VEC_H__
#define __VEC_H__

#define DEG2RAD 0.0174532925199

typedef float vec2_t[2];  /* st */
typedef float vec3_t[3];  /* xyz */
typedef float vec4_t[4];  /* xyzw */
typedef float mat3_t[9];  /* 3x3 matrix */
typedef float mat4_t[16]; /* 4x4 matrix */

void vec_normalize(vec3_t a);

#define vec_copy(s,d) {d[0]=s[0];d[1]=s[1];d[2]=s[2];}
#define vec_clear(d) {d[0]=0.0f;d[1]=0.0f;d[2]=0.0f;}
#define vec_add(a,b,c) {c[0]=a[0]+b[0];c[1]=a[1]+b[1];c[2]=a[2]+b[2];}
#define vec_sub(a,b,c) {c[0]=a[0]-b[0];c[1]=a[1]-b[1];c[2]=a[2]-b[2];} 
#define vec_scale(a,b,c) {c[0]=b*a[0];c[1]=b*a[1];c[2]=b*a[2];}
#define vec_dot(a,b) (a[0]*b[0]+a[1]*b[1]+a[2]*b[2])
#define vec_avg(a,b,c) {c[0]=(a[0]+b[0])*0.5f;c[1]=(a[1]+b[1])*0.5f;c[2]=(a[2]+b[2])*0.5f;}
#define vec_cross(a,b,c) {c[0]=(a[1]*b[2])-(a[2]*b[1]);c[1]=(a[2]*b[0])-(a[0]*b[2]);c[2]=(a[0]*b[1])-(a[1]*b[0]);}
#define vec_cmp(a,b) (a[0]==b[0]&&a[1]==b[1]&&a[2]==b[2])

#define vec2_copy(s,d) {d[0]=s[0];d[1]=s[1];}
#define vec2_avg(a,b,c) {c[0]=(a[0]+b[0])*0.5f;c[1]=(a[1]+b[1])*0.5f;}

#define colour_copy(s,d) *(UInt32*)d = *(UInt32*)s
#define colour_avg(a,b,c) {c[0]=((int)a[0]+(int)b[0])/2;c[1]=((int)a[1]+(int)b[1])/2;c[2]=((int)a[2]+(int)b[2])/2;c[3]=((int)a[3]+(int)b[3])/2;}

void vec_point(vec3_t point, float az, float el);

void mat4_vmult(mat4_t a, vec4_t v, vec4_t product);
/* Note: (product == a) allowed */
void mat4_mmult(mat4_t a, mat4_t b, mat4_t product);

#endif /*__VEC_H__*/
