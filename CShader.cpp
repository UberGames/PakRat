/*
	CShader.cpp

	Author:			Tom Naughton
	Description:	based on AfterShock
 
 * Aftershock 3D rendering engine
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

#include <agl.h>
#include <glut.h>
#include <math.h>
#include <stdlib.h>

#include "CPakRatApp.h"
#include "CFileArchive.h"
#include "CGLImage.h"
#include "CTextures.h"
#include "CShader.h"
#include "CResourceManager.h"
#include "AppConstants.h"

#include "mathdefs.h"
#include "utilities.h"

extern "C" {
	#include "matrix.h"
}

typedef struct texmat_s {
   float m00, m01;
   float m10, m11;
   float t0,  t1;
} texmat_t;

const texmat_t identity_texmat = {
   1, 0,
   0, 1,
   0, 0
};

texmat_t shader_texmat = {
   1, 0,
   0, 1,
   0, 0
};


#define MAX_TEXTURES 500
#define CLAMP_COORD(x) (((long)(x * 10000.0)) % 20000) / 10000.0;
double g_frametime;
double g_starttime;
arrays_t gShaderArrays;
#define CLAMP(min, val, max) ((val < min)?(min):((val > max)?(max):(val)))
Boolean CShader::_animateShaders;



vec_t VectorNormalize( vec3_t v );

void R_ConcatenateTexMatrix(texmat_t* m1, texmat_t* m2, texmat_t* out);
void R_ApplyTexMatrixOnce(texmat_t* m, float os, float ot, float* s, float* t);
void R_MakeScaleTexMatrix(texmat_t* m, float s, float t);
void R_MakeRotateTexMatrix(texmat_t* m, float a);
void R_MakeTranslateTexMatrix(texmat_t* m, float s, float t);



//reference_t transform_ref;



CShader::CShader(CResourceManager *inResources)
{
	_type = 0;
	_texture = nil;
	_resources = inResources;
	_glName = nextGLName();
	strcpy(filename, "");
	strcpy(name, "");
    flags = 0;
    numpasses = 0;
	startIndex = endIndex = 0;
}

CShader::CShader(CResourceManager *inResources, const char* inName, int type, CGLImage *image)
{	
    char 	fname[128];
    
	_texture = image;
	_type = type;
	// Append tga to get file name 
	strcpy(name, inName);
	strcpy(filename, "");
	strcpy(fname, inName);
	strcat(fname, ".tga");
	
	sortkey = 0;
	flush = 0;
	flags = 0;
	numpasses = 0;
	sort  = 0;
	numdeforms = 0;
	skyheight = 512.0f;
	flush = SHADER_FLUSH_GENERIC;
	cull = SHADER_CULL_FRONT;
	startIndex = endIndex = 0;

	for (int i = 0; i < SHADERPASS_MAX; i++) {
		memset (&pass[i],0,sizeof(shaderpass_t));
	}

    // load image if needed
    if (!image) {
		switch (type ) {
			case SHADER_2D :
				image = inResources->textureWithName(fname, SHADER_NOMIPMAPS | SHADER_NOPICMIP); //shader->_resources->textureWithName (name ,SHADER_NOMIPMAPS | SHADER_NOPICMIP);
				break;
			case SHADER_BSP :
				image = inResources->textureWithName(fname, 0); 
				break;
			case SHADER_MD3 :
			default:
				image = inResources->textureWithName(fname, 0);
				break;
		}
    }

	_resources = inResources;
	_glName = nextGLName();
		switch (type )
		{
		case SHADER_2D :
			flags =  SHADER_NOPICMIP;
			numpasses = 1;
			pass[0].flags =  SHADER_BLEND ;
			pass[0].blendsrc=GL_SRC_ALPHA;
			pass[0].blenddst=GL_ONE_MINUS_SRC_ALPHA;
			pass[0].texref = image;
			pass[0].depthfunc = GL_ALWAYS;
			pass[0].rgbgen = RGB_GEN_VERTEX;
			sort = SHADER_SORT_ADDITIVE;
			numdeforms=0;
			flush=SHADER_FLUSH_GENERIC;
			cull=SHADER_CULL_FRONT;
			break;

		case SHADER_BSP :
			 flags =SHADER_DEPTHWRITE;
			 numpasses = 2;
			 
			pass[0].flags = SHADER_LIGHTMAP | SHADER_DEPTHWRITE;
			pass[0].texref = 0; 
			pass[0].depthfunc = GL_LEQUAL;
			pass[0].rgbgen = RGB_GEN_VERTEX;	 

			pass[1].flags = SHADER_BLEND;
			pass[1].texref = image;
			pass[1].blendsrc = GL_DST_COLOR;
			pass[1].blenddst = GL_ZERO;
			pass[1].depthfunc = GL_LEQUAL;
			pass[1].rgbgen = RGB_GEN_IDENTITY;
			 
			 sort = SHADER_SORT_OPAQUE;
			 numdeforms=0;
			 flush=SHADER_FLUSH_GENERIC;
			 cull=SHADER_CULL_FRONT;
			break;
		case SHADER_MD3 :
		default:
			flags = SHADER_DEPTHWRITE;
			numpasses = 1;
			pass[0].flags = SHADER_DEPTHWRITE;
			pass[0].texref = image;
			pass[0].depthfunc = GL_LEQUAL;
			pass[0].rgbgen = RGB_GEN_IDENTITY;
			sort = SHADER_SORT_OPAQUE;
			numdeforms=0;
			flush=SHADER_FLUSH_GENERIC;
			cull=SHADER_CULL_FRONT;
			break;
		}

	Finish();
}

void CShader::Finish ()
{
	UInt8 sort;

/*
	// check if we can use Flush_Multitexture_Combine :
	// TODO :
	// check if we can use Flush_Multitexture_Lightmapped :

	if (r_allowExtensions->integer && gl_ext_info._ARB_Multitexture)
	{
		if (s->numpasses == 2 )
		{
			if ( !(s->pass[0].flags & SHADER_BLEND ) && s->pass[1].flags & SHADER_BLEND )
			{
				if (s->pass[0].flags & SHADER_LIGHTMAP && ((s->pass[1].blendsrc == GL_DST_COLOR && s->pass[1].blenddst== GL_ZERO) || 
					(s->pass[1].blendsrc == GL_ZERO && s->pass[1].blenddst == GL_SRC_COLOR)))
				{
					if (! (s->pass[1].flags & SHADER_LIGHTMAP ))
					{
						// OK !
						s->flush=SHADER_FLUSH_MULTITEXTURE_LIGHTMAP;
					}
				}
			}
		}
	}
*/

	// If all passes are blends, it's transparent
	Boolean allblends = true;
	for (int i= 0; allblends && i < this->numpasses; i++) {
		allblends = allblends 
			&& (0 //this->pass[i].flags & SHADER_ALPHAFUNC 
				||((this->pass[i].flags & SHADER_BLEND) 
					&& (this->pass[i].blenddst == GL_ONE 
						// FIXME: not sure if I need all these
						|| this->pass[i].blenddst == GL_ONE_MINUS_SRC_ALPHA
						|| this->pass[i].blenddst == GL_ONE_MINUS_DST_COLOR
						|| this->pass[i].blenddst == GL_ONE_MINUS_SRC_COLOR
						|| this->pass[i].blenddst == GL_ONE_MINUS_DST_ALPHA
						)));
	}
	if (allblends && !(this->flags & SHADER_SKY))// && !(this->sort & SHADER_SORT_OPAQUE)) 
		this->flags |= SHADER_TRANSPARENT;

	// Explicit depth write for first pass 
	if (! (this->flags & SHADER_DEPTHWRITE) &&
	    ! (this->flags & SHADER_TRANSPARENT) &&
	    ! (this->flags & SHADER_SKY) &&
	    this->numpasses > 0){
	    	this->pass[0].flags |= SHADER_DEPTHWRITE;
	}


	sort = (this->flags & SHADER_POLYGONOFFSET) + (this->cull << 1 ) + ( this->flush << 4 );
	this->sortkey =sort;
}


CShader::~CShader()
{
	if (_texture)
		delete _texture;
}

void CShader::setTexture(CGLImage *image)
{
	if (_texture)
		delete _texture;
    _texture = image;
    
	sortkey = 0;
	flush = 0;
	flags = 0;
	numpasses = 0;
	sort  = 0;
	numdeforms = 0;
	skyheight = 512.0f;
	flush = SHADER_FLUSH_GENERIC;
	cull = SHADER_CULL_FRONT;
	startIndex = endIndex = 0;

	for (int i = 0; i < SHADERPASS_MAX; i++) {
		memset (&pass[i],0,sizeof(shaderpass_t));
	}


	switch (_type )
	{
	case SHADER_2D :
		flags =  SHADER_NOPICMIP;
		numpasses = 1;
		pass[0].flags =  SHADER_BLEND ;
		pass[0].blendsrc=GL_SRC_ALPHA;
		pass[0].blenddst=GL_ONE_MINUS_SRC_ALPHA;
		pass[0].texref = image;
		pass[0].depthfunc = GL_ALWAYS;
		pass[0].rgbgen = RGB_GEN_VERTEX;
		sort = SHADER_SORT_ADDITIVE;
		numdeforms=0;
		flush=SHADER_FLUSH_GENERIC;
		cull=SHADER_CULL_FRONT;
		break;

	case SHADER_BSP :
		 flags =SHADER_DEPTHWRITE;
		 numpasses = 2;
		 
		pass[0].flags = SHADER_LIGHTMAP | SHADER_DEPTHWRITE;
		pass[0].texref = 0; 
		pass[0].depthfunc = GL_LEQUAL;
		pass[0].rgbgen = RGB_GEN_VERTEX;	 

		pass[1].flags = SHADER_BLEND;
		pass[1].texref = image;
		pass[1].blendsrc = GL_DST_COLOR;
		pass[1].blenddst = GL_ZERO;
		pass[1].depthfunc = GL_LEQUAL;
		pass[1].rgbgen = RGB_GEN_IDENTITY;
		 
		 sort = SHADER_SORT_OPAQUE;
		 numdeforms=0;
		 flush=SHADER_FLUSH_GENERIC;
		 cull=SHADER_CULL_FRONT;
		break;
	case SHADER_MD3 :
	default:
		flags = SHADER_DEPTHWRITE;
		numpasses = 1;
		pass[0].flags = SHADER_DEPTHWRITE;
		pass[0].texref = image;
		pass[0].depthfunc = GL_LEQUAL;
		pass[0].rgbgen = RGB_GEN_IDENTITY;
		sort = SHADER_SORT_OPAQUE;
		numdeforms=0;
		flush=SHADER_FLUSH_GENERIC;
		cull=SHADER_CULL_FRONT;
		break;
	}

	Finish ( );
}


void CShader::Make_Vertices ( )
{
	vec3_t v;
	float startoff, off, wavesize;
	

	if (flags & SHADER_DEFORMVERTS ) {
		for(int def = 0; def < numdeforms; def++) {
		
		    // Setup wave function 
			double base = deform_vertices[def].deformv_wavefunc.args[0];
			double amplitude = deform_vertices[def].deformv_wavefunc.args[1]; 
			double phase  = deform_vertices[def].deformv_wavefunc.args[2];
			double freq  = deform_vertices[def].deformv_wavefunc.args[3];
		    wavesize = deform_vertices[def].deform_params[0];
		    startoff = deform_vertices[def].deformv_wavefunc.args[2];
		
			switch (deform_vertices[def].type) {
				case DEFORMV_WAVE:

					for (int i=0;i<gShaderArrays.numverts;i++) {
						off = (gShaderArrays.verts[i][0] + gShaderArrays.verts[i][1] + gShaderArrays.verts[i][2]) /  wavesize;
						phase = startoff + off;
						float deflect = (amplitude * WaveForm(SHADER_FUNC_SIN, freq * (g_frametime + phase))) + base;
						
						VectorCopy(gShaderArrays.norms[i], v);
						VectorScale(v, deflect, v);
						VectorAdd(v, gShaderArrays.verts[i], gShaderArrays.verts[i]);
					}
					break;
					
					/*
				case DEFORMV_NONE:
					break;
				case DEFORMV_NORMAL:
					break;
				case DEFORMV_BULGE:
					break;
				case DEFORMV_MOVE:
					break;
				case DEFORMV_AUTOSPRITE:
					break;
				case DEFORMV_AUTOSPRITE2:
					break;
					*/

				default :
					break;
			}
		}
	}

}

float * CShader::Make_TexCoords (renderingAttributes_t *rendering, shaderpass_t * inPass ,int stage )
{
	vec2_t * in = gShaderArrays.tex_st;
	vec2_t * out ;

	switch (inPass->tc_gen)
	{
	case TC_GEN_BASE :

		in =gShaderArrays.tex_st;
		break;
		
	case TC_GEN_LIGHTMAP:

		in = gShaderArrays.lm_st;
		break;

	case TC_GEN_ENVIRONMENT:
		{
			Vec3 pos ,v,n;
			Vec3 dir;
			int j;
			in= gShaderArrays.stage_tex_st[stage];

			pos[0] = rendering->_xPos;
			pos[1] = rendering->_yPos;
			pos[2] = rendering->_zPos;
		
			Mat4 transform = identity;
			transform *= identity.rotX( rendering->_rotAngleX * DEGTORAD_CONST);
			transform *= identity.rotY( rendering->_rotAngleY * DEGTORAD_CONST);
			transform *= identity.rotZ( rendering->_rotAngleZ * DEGTORAD_CONST);

			transform *= identity.rotX( rendering->_rotCorrectionX * DEGTORAD_CONST);
			transform *= identity.rotY( rendering->_rotCorrectionY * DEGTORAD_CONST);
 			transform *= identity.rotZ( rendering->_rotCorrectionZ * DEGTORAD_CONST);
			
			/*
			Mat4 transform = rendering->_environmentTransform;
			vec *mv = transform.raw();
			pos[0] = MAT(mv,0,3) / 64.0;
			pos[1] = MAT(mv,1,3) / 64.0;
			pos[2] = MAT(mv,2,3) / 64.0;
 			*/

			for(j=0; j<gShaderArrays.numverts; j++) {
			
				VectorCopy (gShaderArrays.verts[j],v);
				Vec3 tempVec2 = transform * v;

				VectorSubtract(tempVec2, pos, dir);
				VectorNormalize((float*)&dir);

				VectorCopy (gShaderArrays.norms[j],n);
				Vec3 tempVec3 = transform * n;
				
				dir[0]+=tempVec3[0];
				dir[1]+=tempVec3[1];
				
				
				in[j][0]= dir[0]  * 0.5;
				in[j][1]= dir[1]  * 0.5;
			//	in[j][0]= 1.5 - (dir[0] * 0.5) ;
			//	in[j][1]= 1.5 - (dir[1] * 0.5) ;
			}
		}
		break;

	case TC_GEN_VECTOR :
		{
			int j;
			in=gShaderArrays.stage_tex_st[stage];
			for (j=0;j<gShaderArrays.numverts;j++)
			{
				in[j][0]=DotProduct(inPass->tc_gen_s,gShaderArrays.verts[j]);
				in[j][1]=DotProduct(inPass->tc_gen_t,gShaderArrays.verts[j]);
			}
			break;
		}
	default :
		in=gShaderArrays.tex_st;
	}

	if (inPass->num_tc_mod >0) {
		for (int n=0;n<inPass->num_tc_mod;n++) {
			out=gShaderArrays.stage_tex_st[stage];
	      	shader_texmat = identity_texmat;
			switch (inPass->tc_mod[n].type)
			{
				case SHADER_TCMOD_ROTATE:
				{
				//  tcMod rotate <degrees per per second>
					   texmat_t translate1 = identity_texmat;
					   texmat_t translate2 = identity_texmat;
					   texmat_t rotate = identity_texmat;
					   float s_half, t_half;

					   s_half = 0.5 / sqrt(
					       (shader_texmat.m00 * shader_texmat.m00) +
					       (shader_texmat.m01 * shader_texmat.m01));

					   t_half = 0.5 / sqrt(
					       (shader_texmat.m10 * shader_texmat.m10) +
					       (shader_texmat.m11 * shader_texmat.m11));

					   R_MakeTranslateTexMatrix(&translate1,  s_half,  t_half);
					   R_MakeRotateTexMatrix(&rotate, inPass->tc_mod[n].args[0] * g_frametime);
					   R_MakeTranslateTexMatrix(&translate2, -s_half, -t_half);

					   R_ConcatenateTexMatrix(&shader_texmat, &translate1,  &shader_texmat);
					   R_ConcatenateTexMatrix(&shader_texmat, &rotate,      &shader_texmat);
					   R_ConcatenateTexMatrix(&shader_texmat, &translate2,  &shader_texmat);
				
				}
				break;
	
				case SHADER_TCMOD_SCALE:
				{
					//   tcMod scale <sScale> <tScale>
				   texmat_t scale = identity_texmat;


				   R_MakeScaleTexMatrix(&scale,  inPass->tc_mod[n].args[0], inPass->tc_mod[n].args[1]);
				   R_MakeScaleTexMatrix(&scale,  inPass->tc_mod[n].args[0],  inPass->tc_mod[n].args[1]);
				   R_ConcatenateTexMatrix(&shader_texmat, &scale, &shader_texmat);

				}
				break;
				case SHADER_TCMOD_TURB:
				{
					//    tcMod turb <base> <amplitude> <phase> <freq>
				   texmat_t scale = identity_texmat;
				   float sy, ty;


				   sy = (inPass->tc_mod[n].args[1] * WaveForm(SHADER_FUNC_SIN, inPass->tc_mod[n].args[3] * (g_frametime + inPass->tc_mod[n].args[2]))) + inPass->tc_mod[n].args[0];
				   ty = (inPass->tc_mod[n].args[1] * WaveForm(SHADER_FUNC_SIN, inPass->tc_mod[n].args[3] * (g_frametime + inPass->tc_mod[n].args[2] + 0.5))) + inPass->tc_mod[n].args[0];

				   R_MakeScaleTexMatrix(&scale, 1 + sy, 1 + ty);
				   R_ConcatenateTexMatrix(&shader_texmat, &scale, &shader_texmat);
				}
				break;

				case SHADER_TCMOD_STRETCH:
				{
				//    tcMod stretch <func> <base> <amplitude> <phase> <frequency>

				   texmat_t translate1 = identity_texmat;
				   texmat_t translate2 = identity_texmat;
				   texmat_t scale = identity_texmat;
				   float s_half, t_half;
				   float y;

					int form = inPass->tc_mod_stretch.func;
					float base = inPass->tc_mod_stretch.args[0];
					float amplitude = inPass->tc_mod_stretch.args[1];
					float phase = inPass->tc_mod_stretch.args[2];
					float frequency = inPass->tc_mod_stretch.args[3];

				   y = 1.0f/((amplitude * WaveForm(form, frequency * (g_frametime + phase))) + base);

				   s_half = 0.5 / sqrt(
				       (shader_texmat.m00 * shader_texmat.m00) +
				       (shader_texmat.m01 * shader_texmat.m01));

				   t_half = 0.5 / sqrt(
				       (shader_texmat.m10 * shader_texmat.m10) +
				       (shader_texmat.m11 * shader_texmat.m11));

				   R_MakeTranslateTexMatrix(&translate1,  s_half,  t_half);
				   R_MakeScaleTexMatrix(&scale, y, y);
				   R_MakeTranslateTexMatrix(&translate2, -s_half, -t_half);

				   R_ConcatenateTexMatrix(&shader_texmat, &translate1,  &shader_texmat);
				   R_ConcatenateTexMatrix(&shader_texmat, &scale,       &shader_texmat);
				   R_ConcatenateTexMatrix(&shader_texmat, &translate2,  &shader_texmat);
		
				}
				break;
				case SHADER_TCMOD_SCROLL:
				{
				//    tcMod scroll <sSpeed> <tSpeed>

			   texmat_t translate = identity_texmat;
			   texmat_t recip = identity_texmat;
			   float  s,  t;
			   float ls, lt;

			   // TODO: The shader docs mention that a 'mod' is used to save
			   //       precision.  I do not know if it is actually necessary,
			   //       but I will leave it out because some shaders work
			   //       better without it and I do not know how to implement
			   //       it correctly.
				
					float sSpeed = inPass->tc_mod[n].args[0];
					float tSpeed = inPass->tc_mod[n].args[1];

			   ls = 1.0 / sqrt((shader_texmat.m00 * shader_texmat.m00) +
			                   (shader_texmat.m01 * shader_texmat.m01));

			   lt = 1.0 / sqrt((shader_texmat.m10 * shader_texmat.m10) +
			                   (shader_texmat.m11 * shader_texmat.m11));

			   ls *= ls;
			   lt *= lt;

			   recip.m00 = shader_texmat.m00 * ls;
			   recip.m01 = shader_texmat.m01 * ls;

			   recip.m10 = shader_texmat.m10 * lt;
			   recip.m11 = shader_texmat.m11 * lt;

			   s = g_frametime * ((sSpeed * recip.m00) + (tSpeed * recip.m10));
			   t = g_frametime * ((sSpeed * recip.m01) + (tSpeed * recip.m11));

			   R_MakeTranslateTexMatrix(&translate, s, t);
			   R_ConcatenateTexMatrix(&shader_texmat, &translate, &shader_texmat);

				}
				break;
				case SHADER_TCMOD_NONE:
				default:
					break;
			}

			vec2_t * in_p = in;
			for (int i = 0; i < gShaderArrays.numverts; i++) {
				float in_s = (*in_p)[0];
				float in_t = (*in_p)[1];
				(*out)[0] = (in_s * shader_texmat.m00) + (in_t * shader_texmat.m10) + shader_texmat.t0;
				(*out)[1] = (in_s * shader_texmat.m01) + (in_t * shader_texmat.m11) + shader_texmat.t1;
				in_p++;
				out++;
			}
			
			in=gShaderArrays.stage_tex_st[stage];

		}

		out=gShaderArrays.stage_tex_st[stage];

	} else  {
		// FIXME: out = in?
		out = gShaderArrays.stage_tex_st[stage];
		memcpy (out,in,gShaderArrays.numverts * sizeof ( vec2_t ));
	}

	return *(float**)&out;
}


// under construction!
UInt8 * CShader::Make_Colors ( renderingAttributes_t *rendering, shaderpass_t * inPass )
{
#pragma unused (rendering)
	colour_t * col=NULL;
	unsigned char colors[4];


	switch (inPass->rgbgen)
	{

		case RGB_GEN_WAVE :
		
			double base = inPass->rgbgen_func.args[0];
			double amplitude = inPass->rgbgen_func.args[1]; 
			double phase  = inPass->rgbgen_func.args[2];
			double freq  = inPass->rgbgen_func.args[3];

			double y = 255 * (amplitude * WaveForm(inPass->rgbgen_func.func, freq * (g_frametime + phase))) + base;
			y = CLAMP(0, y, 255);

		   colors[0] = CLAMP(0, y, 255);
		   colors[1] = CLAMP(0, y, 255);
		   colors[2] = CLAMP(0, y, 255);
		   colors[3] = 0;
		
			glColor4ubv(colors);
			break;
		
		case RGB_GEN_IDENTITY:
		default:
			glColor4f(1,1,1,1);
			break;

	#if 0
		case RGB_GEN_IDENTITY_LIGHTING :
			memset (gShaderArrays.mod_colour,/*r_overBrightBits->integer ? 128 :*/ 255 ,gShaderArrays.numverts *4 );
			col= gShaderArrays.mod_colour;
			break;
		case RGB_GEN_ENTITY :
			col= gShaderArrays.entity_colour;
			break;
		case RGB_GEN_ONE_MINUS_ENTITY:
			for (i=0;i<gShaderArrays.numverts;i++)
			{
				gShaderArrays.mod_colour[i][0] = 255 - gShaderArrays.entity_colour[i][0];
				gShaderArrays.mod_colour[i][1] = 255 - gShaderArrays.entity_colour[i][1];
				gShaderArrays.mod_colour[i][2] = 255 - gShaderArrays.entity_colour[i][2];
				gShaderArrays.mod_colour[i][2] = 255 ;
			}
			break;
		case RGB_GEN_VERTEX:
			col= gShaderArrays.colour;
			break;
		case RGB_GEN_ONE_MINUS_VERTEX:
			for (i=0;i<gShaderArrays.numverts;i++)
			{
				gShaderArrays.mod_colour[i][0] = 255 - gShaderArrays.colour[i][0];
				gShaderArrays.mod_colour[i][1] = 255 - gShaderArrays.colour[i][1];
				gShaderArrays.mod_colour[i][2] = 255 - gShaderArrays.colour[i][2];
			}
			col= gShaderArrays.mod_colour;
			break;
		case RGB_GEN_LIGHTING_DIFFUSE :
			// TODO 
			memset (gShaderArrays.mod_colour,255 ,gShaderArrays.numverts *4 );
			col= gShaderArrays.mod_colour;
			break;



		default :
			col= gShaderArrays.colour;
	#endif

			break;

	}


		// TODO !!!!
	switch (inPass->alpha_gen)
	{

		case  ALPHA_GEN_PORTAL :
	/*		
			for (i=0;i<gShaderArrays.numverts;i++)
			{
				// TODO 
				int alpha;
				alpha = 255.0 * 1.0 /Distance (r_eyepos,gShaderArrays.verts[i]);	
				
				if (alpha>255 )
					alpha = 255;

				col[i][3] = alpha;
			}
			// TODO
		case ALPHA_GEN_DEFAULT :
		case ALPHA_GEN_VERTEX :
		case ALPHA_GEN_ENTITY:
		case ALPHA_GEN_LIGHTINGSPECULAR:
		default :
			for (i=0;i<gShaderArrays.numverts;i++)
				col[i][3]=gShaderArrays.colour[i][3];
			
	*/
		break;

	}


	return *(UInt8**)&col;

}


void CShader::wireframeFlush()
{
	glEnable(GL_CULL_FACE);
	glVertexPointer(3, GL_FLOAT, 0, gShaderArrays.verts);
    glEnableClientState(GL_VERTEX_ARRAY);
    
	glCullFace(GL_BACK);
	glColor3f(0.5,0.5,0.5);				
	glDrawElements(GL_TRIANGLES, gShaderArrays.numelems, GL_UNSIGNED_INT, gShaderArrays.elems);
	glCullFace(GL_FRONT);
	glColor3f( 1,1,1);				
	glDrawElements(GL_TRIANGLES, gShaderArrays.numelems, GL_UNSIGNED_INT, gShaderArrays.elems);

}

void CShader::renderFlush (renderingAttributes_t *rendering, int lmtex )
{

	shaderpass_t *thePass;
	int i;
	CGLImage *texture;
	float *texCoords;
	Boolean drawingSelectedShader = false;
	int passCount = numpasses;


    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   	if(gShaderArrays.numcolours)
		glEnableClientState(GL_COLOR_ARRAY);

	switch (cull )
	{
	case SHADER_CULL_DISABLE:
		glDisable (GL_CULL_FACE);
		break;

	case SHADER_CULL_FRONT:
		glEnable (GL_CULL_FACE);
		glCullFace (GL_FRONT);
		break;

	case SHADER_CULL_BACK:
		glEnable (GL_CULL_FACE);
		glCullFace (GL_BACK);
		break;
		
	default:
		glEnable (GL_CULL_FACE);
		glCullFace (GL_FRONT);
	}


	if (flags & SHADER_POLYGONOFFSET) {
		glEnable (GL_POLYGON_OFFSET_POINT);
	} else {
		glDisable (GL_POLYGON_OFFSET_POINT);
	}

	Make_Vertices();
	glVertexPointer(3, GL_FLOAT, 0, gShaderArrays.verts);
	
	
	if (rendering->_pickShader && rendering->_pickedName == _glName) {
		CShader *selectionShader = _resources->selectionShader();
		if (selectionShader) {
			_resources->setSelectedShader(this);
			passCount += _resources->selectionShader()->numpasses;
			_resources->selectionShader()->Make_Vertices();
		}
	}
	
	if (!(rendering->_renderTextureCoordinates || rendering->_renderTexturePreview) && rendering->_pickShader && _glName)
		glPushName (_glName);

	drawingSelectedShader = _resources->selectedShader() == this;

	Boolean keepRendering = true;
	for (i=0; keepRendering && i<passCount;i++)
	{
		Boolean draw = false;
		thePass=&pass[i];
		
		// use selection shader 
		if (i >= numpasses) {
			CShader *selectionShader = _resources->selectionShader();
			if (selectionShader) {
				thePass = &selectionShader->pass[i - numpasses];
			}
		}

		// Set the Texture Coord Array :
		texCoords = Make_TexCoords (rendering, thePass,0 );
		glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
	//	glTexCoordPointer(2, GL_FLOAT, 0, gShaderArrays.tex_st);
		
		// Set the Colors :
		Make_Colors(rendering, thePass);
	//	glColorPointer(4, GL_UNSIGNED_BYTE, 0, Make_Colors(rendering, thePass));
	   	if(gShaderArrays.numcolours)
			glColorPointer(4, GL_UNSIGNED_BYTE, 0, gShaderArrays.colour);
	
		// Set the Texture  :
		if (thePass->flags & SHADER_LIGHTMAP && _resources->r_lightmaptex && rendering->_lightmap) {
		
			// Select lightmap texture 
			glTexCoordPointer(2, GL_FLOAT, 0, gShaderArrays.lm_st);
			glEnable (GL_TEXTURE );
			glBindTexture(GL_TEXTURE_2D, _resources->r_lightmaptex[lmtex]);
			draw = true;
		
			// don't render textures after the lightmap
			if (!rendering->_textured && rendering->_lightmap)
				keepRendering = false;
		} 
		
		if (rendering->_textured || rendering->_lightmap) {
		
			if (thePass->flags & SHADER_ANIMMAP ) {
				int frame ;

				if (!thePass->anim_numframes || thePass->anim_numframes > 8) return ;
				frame = (int)(g_frametime * thePass->anim_fps) % thePass->anim_numframes;
				
				texture =thePass->anim_frames[frame];
			} else {
				texture =thePass->texref;
			}

			if (texture) {
				glEnable (GL_TEXTURE );
				texture->bindTexture();
				draw = true;
			}

			// Blending :
			if (thePass->flags & SHADER_BLEND ) {
				glEnable (GL_BLEND );
				glBlendFunc(thePass->blendsrc, thePass->blenddst);	
			} else {
				glDisable(GL_BLEND);
			}

			// Alphafunc :
			if (thePass->flags & SHADER_ALPHAFUNC) {
				glEnable(GL_ALPHA_TEST);
				glAlphaFunc(thePass->alphafunc, thePass->alphafuncref);
			} else {
				glDisable(GL_ALPHA_TEST);
			}

			
			// Depth :
			glDepthFunc(thePass->depthfunc);
			if (thePass->flags & SHADER_DEPTHWRITE)
				glDepthMask(GL_TRUE);
			else
				glDepthMask(GL_FALSE);
				
			// handle unlit lightmaps...
			if (i == 1 && pass[0].flags & SHADER_LIGHTMAP && !rendering->_lightmap) {
				glDepthMask(GL_TRUE);
				glDisable(GL_BLEND);
			}


			// Draw it :
			if (draw) {
				if (texture && drawingSelectedShader
					&& ((rendering->_renderTextureCoordinates && i == 0)
						||(rendering->_renderTexturePreview && i < numpasses)))
						DrawTextureMesh(rendering, texture);
				else if (!rendering->_renderTextureCoordinates && !rendering->_renderTexturePreview)
					glDrawElements(GL_TRIANGLES, gShaderArrays.numelems, GL_UNSIGNED_INT, gShaderArrays.elems);
			}

		} else if (!rendering->_renderTextureCoordinates && !rendering->_renderTexturePreview) {
			glDisable( GL_TEXTURE_2D );
			wireframeFlush();	
		}
		
	}
	
	if ( !(rendering->_renderTextureCoordinates || rendering->_renderTexturePreview) && rendering->_pickShader && _glName)
		glPopName ();

	gShaderArrays.numverts=gShaderArrays.numelems=0;

}


void CShader::DrawTextureMesh (renderingAttributes_t *rendering, CGLImage *texture)
{
	float x,y,z;
	int *elems = (int*)gShaderArrays.elems;
	float *texvecs = (float*)gShaderArrays.tex_st;
    int elem = 0;
    Rect bounds = texture->bounds();
    
	glPushMatrix ();
	x= 0.5;
	y= 0.5;
	z= 0.0;

	if (rendering->_renderTexturePreview) {
		glDisable( GL_LIGHTING );
		glEnable( GL_TEXTURE_2D );
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); 
		glVertex3f(-x, y, z);
		glTexCoord2f(1.0f, 0.0f); 
		glVertex3f( x, y, z);
		glTexCoord2f(1.0f, 1.0f); 
		glVertex3f( x, -y, z);
		glTexCoord2f(0.0f, 1.0f); 
		glVertex3f(-x, -y, z);
		glEnd();	
		if (rendering->_textureMap) {
			if (rendering->_textureMap->NextSize(bounds.right - bounds.left, bounds.bottom - bounds.top))
				rendering->_pickedImage = texture;
			
		}
	}	

	if (rendering->_renderTextureCoordinates) {
		glDisable( GL_LIGHTING );
		glDisable( GL_TEXTURE_2D );
	 	glEnable(GL_LINE_SMOOTH);
	 	glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
		glColor3f(1,1,1);
		
		if (rendering->_textureMap)
			rendering->_textureMap->BeginMap();
	  	
	    while (elem < gShaderArrays.numelems) {
	    
			if (rendering->_textureMap)
				rendering->_textureMap->BeginTriangle();

		  	glBegin( GL_LINE_LOOP );
	    	for (int v = 0 ; v < 3; v++) {
	    		int index = elems[elem + v] * 2;
		     	x = CLAMP_COORD(texvecs[index++])
				y = CLAMP_COORD(texvecs[index]);

				if (rendering->_textureMap)
					rendering->_textureMap->AddVertex(x,y);

		     	x = x - 0.5;
				y = -y + 0.5;
				z = 0.0;
	   			glVertex3f(x ,y ,z);
	   			
	    	}
			elem += 3;
			glEnd();

			if (rendering->_textureMap)
				rendering->_textureMap->EndTriangle();

	    }
    }
    
		
	glPopMatrix ();
	glColor4f(1,1,1,1);

}

void CShader::Flush_Multitexture_Lightmapped (int lmtex )
{
#pragma unused (lmtex)
}

void CShader::Flush_Multitexture_Combine (int lmtex )
{
#pragma unused (lmtex)
}

void CShader::Flush_Vertex_Lit (int lmtex )
{
#pragma unused (lmtex)
}

#if 0
colour_t *CShader::R_Make_Rgba (shaderpass_t * inPass )
{
	int i;
	UInt8 rgb;
	colour_t * col;
	switch (inPass->rgbgen)
	{
	case RGB_GEN_NONE:  // should not happen !!
		break;

	case RGB_GEN_IDENTITY_LIGHTING :
		memset (gShaderArrays.mod_colour,/*r_overBrightBits->integer ? 128 :*/ 255 ,gShaderArrays.numverts *4 );
		col= gShaderArrays.mod_colour;
		break;
	case RGB_GEN_IDENTITY:
		memset (gShaderArrays.mod_colour,255 ,gShaderArrays.numverts *4 );
		col= gShaderArrays.mod_colour;
		break;
	case RGB_GEN_WAVE :
		rgb = (UInt8)255 * (float)render_func_eval(inPass->rgbgen_func.func,
					    inPass->rgbgen_func.args);
		memset (gShaderArrays.mod_colour,rgb,gShaderArrays.numverts * 4 );
		for (i=0;i< gShaderArrays.numverts;i++)
			gShaderArrays.mod_colour[i][3] =255;

		col= gShaderArrays.mod_colour;
		break;
	case RGB_GEN_ENTITY :
		col= gShaderArrays.entity_colour;
		break;
	case RGB_GEN_ONE_MINUS_ENTITY:
		for (i=0;i<gShaderArrays.numverts;i++)
		{
			gShaderArrays.mod_colour[i][0] = 255 - gShaderArrays.entity_colour[i][0];
			gShaderArrays.mod_colour[i][1] = 255 - gShaderArrays.entity_colour[i][1];
			gShaderArrays.mod_colour[i][2] = 255 - gShaderArrays.entity_colour[i][2];
			gShaderArrays.mod_colour[i][2] = 255 ;
		}
		break;
	case RGB_GEN_VERTEX:
		col= gShaderArrays.colour;
		break;
	case RGB_GEN_ONE_MINUS_VERTEX:
		for (i=0;i<gShaderArrays.numverts;i++)
		{
			gShaderArrays.mod_colour[i][0] = 255 - gShaderArrays.colour[i][0];
			gShaderArrays.mod_colour[i][1] = 255 - gShaderArrays.colour[i][1];
			gShaderArrays.mod_colour[i][2] = 255 - gShaderArrays.colour[i][2];
		}
		col= gShaderArrays.mod_colour;
		break;
	case RGB_GEN_LIGHTING_DIFFUSE :
		// TODO 
		memset (gShaderArrays.mod_colour,255 ,gShaderArrays.numverts *4 );
		col= gShaderArrays.mod_colour;
		break;




	default :
		col= gShaderArrays.colour;

		break;

	}


		// TODO !!!!
	switch (inPass->alpha_gen)
	{

	case  ALPHA_GEN_PORTAL :
		/*
		for (i=0;i<gShaderArrays.numverts;i++)
		{
			// TODO 
			int alpha;
			alpha = 255.0 * 1.0 /Distance (r_eyepos,gShaderArrays.verts[i]);	
			
			if (alpha>255 )
				alpha = 255;

			col[i][3] = alpha;
		}
		// TODO
		*/
	case ALPHA_GEN_DEFAULT :
	case ALPHA_GEN_VERTEX :
	case ALPHA_GEN_ENTITY:
	case ALPHA_GEN_LIGHTINGSPECULAR:
	default :
		for (i=0;i<gShaderArrays.numverts;i++)
			col[i][3]=gShaderArrays.colour[i][3];
		
	break;

	}


	return col;

}
#endif

#if 0
double CShader::render_func_eval(UInt32 func, float *args)
{
    double x, y;
	double base = args[0], amplitude  = args[1], phase  = args[2], freq  = args[3];
	
    /* Evaluate a number of time based periodic functions */
    /* y = base + amplitude * func( (time + freq) * phase ) */
    
    x = (g_frametime + phase) * freq;
    x -= floor(x);

    switch (func)
    {
	case SHADER_FUNC_SIN:
	    y = sin(x * TWOPI);
	    break;
	    
	case SHADER_FUNC_TRIANGLE:
	
	    if (x < 0.5)
		y = 2.0 * x - 1.0;
	    else
		y = -2.0 * x + 2.0;
	    break;
	    
	case SHADER_FUNC_SQUARE:
	    if (x < 0.5)
		y = 1.0;
	    else
		y = -1.0;
	    break;
	    
	case SHADER_FUNC_SAWTOOTH:
	    y = x;
	    break;
	    
	case SHADER_FUNC_INVERSESAWTOOTH:
	    y = 1.0 - x;
	    break;
    case SHADER_FUNC_RANDOM:
    case SHADER_FUNC_NOISE:
         // random noise
         y = ((float)rand() * (1.0f / (float)(RAND_MAX >> 1))) - 1;
   }

    return y * amplitude + base;
}
#endif


vec_t VectorNormalize( vec3_t v ) 
{
	float	length, ilength;

	length = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
	length = sqrt (length);

	if ( length ) {
		ilength = 1/length;
		v[0] *= ilength;
		v[1] *= ilength;
		v[2] *= ilength;
	}
		
	return length;
}

#pragma mark -

void R_backend_init(void)
{
    
    gShaderArrays.verts = (vec3_t*)malloc(MAX_ARRAYS_VERTS * sizeof(vec3_t));
	gShaderArrays.norms = (vec3_t*)malloc(MAX_ARRAYS_VERTS * sizeof(vec3_t));
    gShaderArrays.tex_st = (texcoord_t*)malloc(MAX_ARRAYS_VERTS * sizeof(texcoord_t));
	gShaderArrays.lm_st = (texcoord_t*)malloc(MAX_ARRAYS_VERTS * sizeof(texcoord_t));
	
	gShaderArrays.elems = (int*)malloc(MAX_ARRAYS_ELEMS * sizeof(int));
    gShaderArrays.colour = (colour_t*)malloc(MAX_ARRAYS_VERTS * sizeof(colour_t));
	gShaderArrays.mod_colour= (colour_t*)malloc(MAX_ARRAYS_VERTS * sizeof(colour_t));
	gShaderArrays.entity_colour = 	(colour_t*)malloc(MAX_ARRAYS_VERTS * sizeof(colour_t));	

	if (0 /*!r_allowExtensions->integer*/)
	{

		gShaderArrays.stage_tex_st = (vec2_t ** ) malloc (1 * sizeof(vec2_t*));
		gShaderArrays.stage_tex_st[0] =  (vec2_t * ) malloc (MAX_ARRAYS_VERTS * sizeof(vec2_t));
	}
	else 
	{
		int i;
		gShaderArrays.stage_tex_st = (vec2_t ** ) malloc (MAX_TEXTURES * sizeof(vec2_t*));
		
		for (i=0;i<SHADERPASS_MAX;i++)
		{	
			gShaderArrays.stage_tex_st[i] =  (vec2_t * ) malloc (MAX_ARRAYS_VERTS * sizeof(vec2_t));
		}

	}



}

void R_backend_shutdown(void )
{

	free(gShaderArrays.verts);
    free(gShaderArrays.tex_st);
    free(gShaderArrays.lm_st);
	free (gShaderArrays.norms);
    free(gShaderArrays.elems);
    free(gShaderArrays.colour);
	free (gShaderArrays.mod_colour);


	if (1 /*!r_allowExtensions->integer*/)
	{
		free (gShaderArrays.stage_tex_st[0]);
	}
	else 
	{
		int i;
		for (i=0;i<MAX_TEXTURES;i++)
		{
			free (gShaderArrays.stage_tex_st[i]);

		}

	}

	free (gShaderArrays.stage_tex_st);


}

#pragma mark -


void R_ConcatenateTexMatrix(texmat_t* m1, texmat_t* m2, texmat_t* out)
{
   texmat_t tmp;

   tmp.m00 = (m2->m00 * m1->m00) + (m2->m01 * m1->m10);
   tmp.m01 = (m2->m00 * m1->m01) + (m2->m01 * m1->m11);
   tmp.m10 = (m2->m10 * m1->m00) + (m2->m11 * m1->m10);
   tmp.m11 = (m2->m10 * m1->m01) + (m2->m11 * m1->m11);

   tmp.t0 = (m2->t0 * m1->m00) + (m2->t1 * m1->m10) + m1->t0;
   tmp.t1 = (m2->t0 * m1->m01) + (m2->t1 * m1->m11) + m1->t1;

   *out = tmp;
}



void R_ApplyTexMatrixOnce(texmat_t* m, float os, float ot, float* s, float* t)
{
   *s = (m->m00 * os) + (m->m10 * ot) + m->t0;
   *t = (m->m01 * os) + (m->m11 * ot) + m->t1;
}





void R_MakeScaleTexMatrix(texmat_t* m, float s, float t)
{
   m->m00 = s;
   m->m01 = 0;

   m->m10 = 0;
   m->m11 = t;

   m->t0  = 0;
   m->t1  = 0;
}



void R_MakeRotateTexMatrix(texmat_t* m, float a)
{
   float c = cos(a * -(M_PI / 180.0));
   float s = sin(a * -(M_PI / 180.0));

   m->m00 =  c;
   m->m01 =  s;

   m->m10 = -s;
   m->m11 =  c;

   m->t0 = 0;
   m->t1 = 0;
}



void R_MakeTranslateTexMatrix(texmat_t* m, float s, float t)
{
   m->m00 = 1;
   m->m01 = 0;

   m->m10 = 0;
   m->m11 = 1;

   m->t0 = s;
   m->t1 = t;
}

double WaveForm(int func, double ox)
{
   double x, y;

   x = ox - floor(ox);

   switch (func) {
      case SHADER_FUNC_SIN:
         // a simple sin wave
         // TODO: this could be a table
         // look up, but speed is not
         // as important here

         y = sin(2 * M_PI * x);

         break;

      case SHADER_FUNC_TRIANGLE:
         // mirrors a sin wave,
         // but is linear.

         if (x < 0.25) {
            y = (2 * x) + 0.5;
         }
         else if (x < 0.75) {
            y = 1 - (2 * (x - 0.25));
         }
         else {
            y = 2 * (x - 0.75);
         }

         break;

      case SHADER_FUNC_SQUARE:
         // mirrors the sin wave,
         // but is either completely
         // positive or negative

         if (x < 0.5) {
            y =  1;
         }
         else {
            y = -1;
         }
         
         break;

      case SHADER_FUNC_SAWTOOTH:
         // raises linearly and falls
         // back suddenly at the beginning
         // of the next period

         y = x;

         break;

      case SHADER_FUNC_INVERSESAWTOOTH:
         // falls linearly and raises
         // back suddenly at the beginning
         // of the next period

         y = 1 - x;

         break;

      case SHADER_FUNC_RANDOM:
      case SHADER_FUNC_NOISE:
         // random noise
         y = ((float)rand() * (1.0f / (float)(RAND_MAX >> 1))) - 1;

         break;

      default:
         // to catch any errors, this is
         // simply a flat-line.

		dprintf("error: unknown shader waveform\n");
         y = 0;

         break;
   }

   return y;
}


