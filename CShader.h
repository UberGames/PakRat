/* 
	CShader.h

	Author:			Tom Naughton
	Description:	<describe the CShader class here>
*/

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

#ifndef CShader_H
#define CShader_H

#include "CResourceManager.h"
#include "CTextureMap.h"
#include "mathlib.h"

extern "C" {
	#include "vec.h"
}

#define SHADERPASS_MAX 10
#define MAX_TC_MOD  8
#define SHADER_ANIM_FRAMES_MAX 10
#define TWOPI 6.28318530718
#define TURB_SCALE 0.2
#define MAX_ARRAYS_VERTS 8192
#define MAX_ARRAYS_ELEMS MAX_ARRAYS_VERTS * 3 

typedef float texcoord_t[2];  /* Texture s&t coordinates */
typedef UInt8 colour_t[4];   /* RGBA */

#define DotProduct(x,y)			((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])
#define VectorSubtract(a,b,c)	((c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1],(c)[2]=(a)[2]-(b)[2])
#define VectorAdd(a,b,c)		((c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1],(c)[2]=(a)[2]+(b)[2])
#define VectorCopy(a,b)			((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2])
#define	VectorScale(v, s, o)	((o)[0]=(v)[0]*(s),(o)[1]=(v)[1]*(s),(o)[2]=(v)[2]*(s))
#define	VectorMA(v, s, b, o)	((o)[0]=(v)[0]+(b)[0]*(s),(o)[1]=(v)[1]+(b)[1]*(s),(o)[2]=(v)[2]+(b)[2]*(s))


class CFileArchive;
class CShader;
class CGLImage;
class CResourceManager;
class CPakStream;



typedef struct RENDERINGATTRIBUTES
{
	// preferences
	Boolean _lighting;
	Boolean _lightmap;
	Boolean _textured;
	Boolean _wireframe;
	Boolean _showBoneFrameBox;
	Boolean _showNormals;
	Boolean _showMissingTextures;
	
	// rendering phases
	Boolean	_renderOpaque;
	Boolean	_renderBlend;
	Boolean _renderTexturePreview;
	Boolean _renderTextureCoordinates;
	
	// picking
	Boolean	_pickTag;
	Boolean	_pickShader;
	Boolean	_pickTextureMap;
	UInt32 _pickedName;
	CGLImage *_pickedImage;
//	CShader *_pickedShader;
	CTextureMap	*_textureMap;
	
	// model camera position
	float _xPos, _yPos; 		//pos in x-y plane, pans model
	float _zPos; 				//pos on Z axis, scales model
	float _rotAngleX;
	float _rotAngleY;
	float _rotAngleZ;
	float _rotCorrectionX;
	float _rotCorrectionY;
	float _rotCorrectionZ;
	
	// enviroment mapping transform
	Mat4 _environmentTransform;

	// map camera position
	vec3_t r_eyepos;
	vec3_t r_eyedir;
	float r_eye_az, r_eye_el;
	float r_eyefov;
	long r_eyecluster;
	long r_lockpvs;
	float r_eyesep;
	float r_focallength;
	

} renderingAttributes_t;


typedef struct
{
    vec3_t v_point;     /* World coords */
    vec2_t tex_st;  /* Texture coords */
	vec2_t lm_st;   /* Lightmap texture coords */
    vec3_t v_norm;      /* Normal */
    colour_t colour;    /* Colour used for vertex lighting ? */
} cvertex_t;


// shader

#define SHADER_DOCULL 1<<1
/* Shader flags */
enum
{
    SHADER_TRANSPARENT   = 1 << 1,
    SHADER_DEPTHWRITE    = 1 << 2,  /* Also used for pass flag */
    SHADER_SKY           = 1 << 3,
    SHADER_NOMIPMAPS     = 1 << 4,
    SHADER_DEFORMVERTS   = 1 << 6,
	SHADER_POLYGONOFFSET = 1 << 7,
	SHADER_FOG           = 1 << 8,
	SHADER_NOPICMIP      = 1 << 9,
	SHADER_CLAMP         = 1 << 10  // will just be used for texture loading 
};

/* Shaderpass flags */
enum
{
    SHADER_LIGHTMAP   = 1 << 0,
    SHADER_BLEND      = 1 << 1,
    SHADER_ALPHAFUNC  = 1 << 3,
    SHADER_TCMOD      = 1 << 4,
    SHADER_ANIMMAP    = 1 << 5
};	

/* Transform functions */
enum
{
    SHADER_FUNC_SIN             = 1,
    SHADER_FUNC_TRIANGLE        = 2,
    SHADER_FUNC_SQUARE          = 3,
    SHADER_FUNC_SAWTOOTH        = 4,
    SHADER_FUNC_INVERSESAWTOOTH = 5,
    SHADER_FUNC_RANDOM 			= 6,
    SHADER_FUNC_NOISE 			= 7

};

/* tcmod functions */
enum 
{
	SHADER_TCMOD_NONE ,
	SHADER_TCMOD_SCALE,
	SHADER_TCMOD_SCROLL,
	SHADER_TCMOD_ROTATE,
	SHADER_TCMOD_TRANSFORM,
	SHADER_TCMOD_TURB,
	SHADER_TCMOD_STRETCH
};


// SORTING 
enum {
	SHADER_SORT_NONE = 0,
	SHADER_SORT_PORTAL = 1,
	SHADER_SORT_SKY = 2,
	SHADER_SORT_OPAQUE = 3,
	SHADER_SORT_BANNER = 6,
	SHADER_SORT_UNDERWATER = 8,
	SHADER_SORT_ADDITIVE = 9,
	SHADER_SORT_NEAREST = 16
} ;


// RGB_Gen :

enum {
	RGB_GEN_NONE ,
	RGB_GEN_IDENTITY_LIGHTING ,
	RGB_GEN_IDENTITY,
	RGB_GEN_WAVE ,
	RGB_GEN_ENTITY ,
	RGB_GEN_ONE_MINUS_ENTITY,
	RGB_GEN_VERTEX,
	RGB_GEN_ONE_MINUS_VERTEX,
	RGB_GEN_LIGHTING_DIFFUSE ,
	RGB_GEN_EXACT_VERTEX 
};


// ALPHA_GEN :

enum {
	ALPHA_GEN_DEFAULT =0,
	ALPHA_GEN_PORTAL =1,
	ALPHA_GEN_VERTEX ,
	ALPHA_GEN_ENTITY,
	ALPHA_GEN_LIGHTINGSPECULAR,
	ALPHA_GEN_WAVE // TODO !
};


// TC_GEN :

enum {
	TC_GEN_BASE =0,
	TC_GEN_LIGHTMAP =1,
	TC_GEN_ENVIRONMENT =2,
	TC_GEN_VECTOR =3
};

// Deform_Vertices :

enum {
	DEFORMV_NONE,
	DEFORMV_WAVE,
	DEFORMV_NORMAL,
	DEFORMV_BULGE,
	DEFORMV_MOVE,
	DEFORMV_AUTOSPRITE,
	DEFORMV_AUTOSPRITE2
};


// The flushing functions :
enum {
	SHADER_FLUSH_GENERIC ,
	SHADER_FLUSH_MULTITEXTURE_LIGHTMAP ,
	SHADER_FLUSH_MULTITEXTURE_COMBINE ,
	SHADER_FLUSH_VERTEX_LIT
};


// Culling :
enum {
	SHADER_CULL_DISABLE,
	SHADER_CULL_FRONT,
	SHADER_CULL_BACK
};

#define SHADER_BSP 0
#define SHADER_MD3 1
#define SHADER_2D  2

/* Periodic functions */
typedef struct
{
    UInt32 func;     /* SHADER_FUNC enum */
    float args[4];   /* offset, amplitude, phase_offset, rate */
} shaderfunc_t;

typedef struct {
	int type ;
	float args[6];
} tc_mod_t;

typedef struct {
	int type ;
    shaderfunc_t deformv_wavefunc;
    float deform_params[4];
} deformvertexes_t;

/* Triangle arrays */
typedef struct
{
    int numverts;
    int numcolours;
    vec3_t *verts;
	vec3_t * norms ;
    colour_t *colour;
	colour_t *entity_colour;
    texcoord_t *tex_st;
    texcoord_t *lm_st;
	vec2_t ** stage_tex_st ;
	colour_t * mod_colour;
    int numelems;
    int *elems;
} arrays_t;


/* Per-pass rendering state information */
typedef struct
{
    UInt32 flags;
    UInt32 texflags;
    CGLImage *texref;                 /* Texture ref (if not lightmap) */
    UInt32  blendsrc, blenddst;  /* glBlend args */
    UInt32  depthfunc;           /* glDepthFunc arg */
    UInt32  alphafunc;           /* glAlphaFunc arg1 */
    float alphafuncref;         /* glAlphaFunc arg2 */
    int   rgbgen;             
    shaderfunc_t rgbgen_func;
	int tc_gen ;
	vec3_t tc_gen_s;
	vec3_t tc_gen_t;
    int  num_tc_mod;               
	tc_mod_t tc_mod [ MAX_TC_MOD];
	shaderfunc_t tc_mod_stretch;

	int alpha_gen ;

    float anim_fps;             /* Animation frames per sec */
    int anim_numframes;
    CGLImage *anim_frames[SHADER_ANIM_FRAMES_MAX];  /* Texture refs */
} shaderpass_t;


class CShaderIndexInfo
{
public:
	char name [65];
	long startIndex;
	long endIndex;
};

typedef list<CShaderIndexInfo> CShaderIndexList;
typedef CShaderIndexList::iterator CShaderIndexList_iterator;




class CShader
{

	friend class CShaderParser;

public:

	CShader(CResourceManager *inResources);
	CShader(CResourceManager *inResources, const char* name, int type, CGLImage *image = nil);
	virtual ~CShader();
	
	void 				Finish();
	void				renderFlush(renderingAttributes_t *rendering, int lmtex);
	static void			wireframeFlush();
	static double 		render_func_eval(UInt32 func, float *args);
	Boolean				needsNormals() { return 1 || numdeforms > 0; };
	Boolean				isTransparent() { return flags & SHADER_TRANSPARENT; };
	void 				setTexture(CGLImage *image);

	Boolean 			found;
	UInt32				_glName;
	
	
/* Shader info */
	char name [65];
	char filename [128];
	UInt8 sortkey ;  // a precalculated sortkey which is added to the shaderkey ; (TODO )
	UInt8 flush;      // FLUSH_ENUM
	UInt8 cull;
    UInt32  flags;
    int numpasses;
	int sort ;
	int numdeforms ;
	deformvertexes_t deform_vertices[ MAX_TC_MOD];
    shaderpass_t pass[SHADERPASS_MAX];
    float skyheight;          /* Height for skybox */
	float fog_params [4];

    CResourceManager 	*_resources;
	UInt32				startIndex, endIndex;
	
	static Boolean _animateShaders;

	
private:

	void Make_Vertices ( );
	float * Make_TexCoords (renderingAttributes_t *rendering, shaderpass_t * pass ,int stage );
	UInt8 * Make_Colors (renderingAttributes_t *rendering, shaderpass_t * pass );
	void Flush (int shadernum ,int lmtex );
	void Flush_Generic (renderingAttributes_t *rendering, int lmtex );
	void Flush_Multitexture_Lightmapped (int lmtex );
	void Flush_Multitexture_Combine (int lmtex );
	void Flush_Vertex_Lit (int lmtex );
	colour_t *R_Make_Rgba (shaderpass_t * pass );
	void DrawTextureMesh (renderingAttributes_t *rendering, CGLImage *texture );
	
	CGLImage 	*_texture;
	int			_type;
};

extern double g_frametime;
extern double g_starttime;
extern arrays_t gShaderArrays;

void R_backend_init(void);
void R_backend_shutdown(void );
//double render_func_eval(UInt32 func, float *args);
double WaveForm(int func, double ox);


#endif	// CShader_H
