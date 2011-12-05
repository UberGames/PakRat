/* 
	CHLMdl.h

	Author:			Tom Naughton
	Description:	<describe the CHLMdl class here>
*/

#ifndef CHLMdl_H
#define CHLMdl_H

#include "C3DModel.h"
#include "CGLImage.h"
#include "hlmathlib.h"

/***
*
*	Copyright (c) 1998, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/


enum // render modes
{
	RM_WIREFRAME,
	RM_FLATSHADED,
	RM_SMOOTHSHADED,
	RM_TEXTURED
};



typedef struct
{
	// model 
	float rot[3];
	float trans[3];

	// render
	int renderMode;
	float transparency;
	bool showBackground;
	bool showGround;
	bool showHitBoxes;
	bool showBones;
	bool showTexture;
	bool showAttachments;
	int texture;
	float textureScale;
	int skin;
	bool mirror;
	bool useStencil;	// if 3dfx fullscreen set false

	// animation
	int sequence;
	float speedScale;

	// bodyparts and bonecontrollers
	int submodels[32];
	float controllers[8];

	// fullscreen
	int width, height;
	bool use3dfx;
	bool cds;

	// colors
	float bgColor[4];
	float lColor[4];
	float gColor[4];

	// misc
	int textureLimit;
	bool pause;

	// only used for fullscreen mode
	char modelFile[256];
	char backgroundTexFile[256];
	char groundTexFile[256];
} ViewerSettings;



/*
==============================================================================

STUDIO MODELS

Studio models are position independent, so the cache manager can move them.
==============================================================================
*/


#define MAXSTUDIOTRIANGLES	20000	// TODO: tune this
#define MAXSTUDIOVERTS		2048	// TODO: tune this
#define MAXSTUDIOSEQUENCES	256		// total animation sequences
#define MAXSTUDIOSKINS		100		// total textures
#define MAXSTUDIOSRCBONES	512		// bones allowed at source movement
#define MAXSTUDIOBONES		128		// total bones actually used
#define MAXSTUDIOMODELS		32		// sub-models per model
#define MAXSTUDIOBODYPARTS	32
#define MAXSTUDIOGROUPS		4
#define MAXSTUDIOANIMATIONS	512		// per sequence
#define MAXSTUDIOMESHES		256
#define MAXSTUDIOEVENTS		1024
#define MAXSTUDIOPIVOTS		256
#define MAXSTUDIOCONTROLLERS 8

typedef struct 
{
	int					id;
	int					version;

	char				name[64];
	int					length;

	vec3_t				eyeposition;	// ideal eye position
	vec3_t				min;			// ideal movement hull size
	vec3_t				max;			

	vec3_t				bbmin;			// clipping bounding box
	vec3_t				bbmax;		

	int					flags;

	int					numbones;			// bones
	int					boneindex;

	int					numbonecontrollers;		// bone controllers
	int					bonecontrollerindex;

	int					numhitboxes;			// complex bounding boxes
	int					hitboxindex;			
	
	int					numseq;				// animation sequences
	int					seqindex;

	int					numseqgroups;		// demand loaded sequences
	int					seqgroupindex;

	int					numtextures;		// raw textures
	int					textureindex;
	int					texturedataindex;

	int					numskinref;			// replaceable textures
	int					numskinfamilies;
	int					skinindex;

	int					numbodyparts;		
	int					bodypartindex;

	int					numattachments;		// queryable attachable points
	int					attachmentindex;

	int					soundtable;
	int					soundindex;
	int					soundgroups;
	int					soundgroupindex;

	int					numtransitions;		// animation node to animation node transition graph
	int					transitionindex;
} studiohdr_t;

// header for demand loaded sequence group data
typedef struct 
{
	int					id;
	int					version;

	char				name[64];
	int					length;
} studioseqhdr_t;

// bones
typedef struct 
{
	char				name[32];	// bone name for symbolic links
	int		 			parent;		// parent bone
	int					flags;		// ??
	int					bonecontroller[6];	// bone controller index, -1 == none
	float				value[6];	// default DoF values
	float				scale[6];   // scale for delta DoF values
} mstudiobone_t;


// bone controllers
typedef struct 
{
	int					bone;	// -1 == 0
	int					type;	// X, Y, Z, XR, YR, ZR, M
	float				start;
	float				end;
	int					rest;	// byte index value at rest
	int					index;	// 0-3 user set controller, 4 mouth
} mstudiobonecontroller_t;

// intersection boxes
typedef struct
{
	int					bone;
	int					group;			// intersection group
	vec3_t				bbmin;		// bounding box
	vec3_t				bbmax;		
} mstudiobbox_t;

typedef void *cache_user_t;

// demand loaded sequence groups
typedef struct
{
	char				label[32];	// textual name
	char				name[64];	// file name
	cache_user_t		cache;		// cache index pointer
	int					data;		// hack for group 0
} mstudioseqgroup_t;

// sequence descriptions
typedef struct
{
	char				label[32];	// sequence label

	float				fps;		// frames per second	
	int					flags;		// looping/non-looping flags

	int					activity;
	int					actweight;

	int					numevents;
	int					eventindex;

	int					numframes;	// number of frames per sequence

	int					numpivots;	// number of foot pivots
	int					pivotindex;

	int					motiontype;	
	int					motionbone;
	vec3_t				linearmovement;
	int					automoveposindex;
	int					automoveangleindex;

	vec3_t				bbmin;		// per sequence bounding box
	vec3_t				bbmax;		

	int					numblends;
	int					animindex;		// mstudioanim_t pointer relative to start of sequence group data
										// [blend][bone][X, Y, Z, XR, YR, ZR]

	int					blendtype[2];	// X, Y, Z, XR, YR, ZR
	float				blendstart[2];	// starting value
	float				blendend[2];	// ending value
	int					blendparent;

	int					seqgroup;		// sequence group for demand loading

	int					entrynode;		// transition node at entry
	int					exitnode;		// transition node at exit
	int					nodeflags;		// transition rules
	
	int					nextseq;		// auto advancing sequences
} mstudioseqdesc_t;

// events
typedef struct 
{
	int 				frame;
	int					event;
	int					type;
	char				options[64];
} mstudioevent_t;


// pivots
typedef struct 
{
	vec3_t				org;	// pivot point
	int					start;
	int					end;
} mstudiopivot_t;

// attachment
typedef struct 
{
	char				name[32];
	int					type;
	int					bone;
	vec3_t				org;	// attachment point
	vec3_t				vectors[3];
} mstudioattachment_t;

typedef struct
{
	unsigned short	offset[6];
} mstudioanim_t;

// animation frames
typedef union 
{
	struct {
		byte	valid;
		byte	total;
	} num;
	short		value;
} mstudioanimvalue_t;



// body part index
typedef struct
{
	char				name[64];
	int					nummodels;
	int					base;
	int					modelindex; // index into models array
} mstudiobodyparts_t;



// skin info
typedef struct
{
	char					name[64];
	int						flags;
	int						width;
	int						height;
	int						index;
} mstudiotexture_t;


// skin families
// short	index[skinfamilies][skinref]

// studio models
typedef struct
{
	char				name[64];

	int					type;

	float				boundingradius;

	int					nummesh;
	int					meshindex;

	int					numverts;		// number of unique vertices
	int					vertinfoindex;	// vertex bone info
	int					vertindex;		// vertex vec3_t
	int					numnorms;		// number of unique surface normals
	int					norminfoindex;	// normal bone info
	int					normindex;		// normal vec3_t

	int					numgroups;		// deformation groups
	int					groupindex;
} mstudiomodel_t;


// vec3_t	boundingbox[model][bone][2];	// complex intersection info


// meshes
typedef struct 
{
	int					numtris;
	int					triindex;
	int					skinref;
	int					numnorms;		// per mesh normals
	int					normindex;		// normal vec3_t
} mstudiomesh_t;

// triangles
typedef struct 
{
	short				vertindex;		// index into vertex array
	short				normindex;		// index into normal array
	short				s,t;			// s,t position on skin
} mstudiotrivert_t;


// lighting options
#define STUDIO_NF_FLATSHADE		0x0001
#define STUDIO_NF_CHROME		0x0002
#define STUDIO_NF_FULLBRIGHT	0x0004

// motion flags
#define STUDIO_X		0x0001
#define STUDIO_Y		0x0002	
#define STUDIO_Z		0x0004
#define STUDIO_XR		0x0008
#define STUDIO_YR		0x0010
#define STUDIO_ZR		0x0020
#define STUDIO_LX		0x0040
#define STUDIO_LY		0x0080
#define STUDIO_LZ		0x0100
#define STUDIO_AX		0x0200
#define STUDIO_AY		0x0400
#define STUDIO_AZ		0x0800
#define STUDIO_AXR		0x1000
#define STUDIO_AYR		0x2000
#define STUDIO_AZR		0x4000
#define STUDIO_TYPES	0x7FFF
#define STUDIO_RLOOP	0x8000	// controller that wraps shortest distance

// sequence flags
#define STUDIO_LOOPING	0x0001

// bone flags
#define STUDIO_HAS_NORMALS	0x0001
#define STUDIO_HAS_VERTICES 0x0002
#define STUDIO_HAS_BBOX		0x0004
#define STUDIO_HAS_CHROME	0x0008	// if any of the textures have chrome on them

#define RAD_TO_STUDIO		(32768.0/M_PI)
#define STUDIO_TO_RAD		(M_PI/32768.0)


class CHLMdl:  public C3DModel
{
public:
	
	CHLMdl(CResourceManager *resources);
	virtual ~CHLMdl();
	
	virtual Boolean 	init(CPakStream *inItem);
	virtual void 		Draw(CModelInstance *instance, renderingAttributes_t *renderingAttributes);

	virtual SInt16 		frameCount();
	virtual string 		frameName(SInt16 frame);
	virtual UInt32 		meshNum();

	// exporting 
	virtual Boolean 	canExportFormat(ExportFormatT format); 
	virtual void 		dxfExportMesh(CModelInstance *instance, LFileStream *file, Mat4 transform, int meshnum);
	virtual void 		objExportMeshVertexData(CModelInstance *instance, LFileStream *file, Mat4 transform, int meshnum) ;
	virtual void 		objExportMeshElementData(LFileStream *file, int meshnum, int &vertexCount, int &meshCount);

	studiohdr_t				*getStudioHeader () const { return m_pstudiohdr; }
	studiohdr_t				*getTextureHeader () const { return m_ptexturehdr; }
	studiohdr_t				*getAnimHeader (int i) const { return m_panimhdr[i]; }

	void					UploadTexture( mstudiotexture_t *ptexture, byte *data, byte *pal, int name );
	void					FreeModel ();
	studiohdr_t				*LoadModel( char *modelname );
	bool					PostLoadModel ( char *modelname );
	bool					SaveModel ( char *modelname );
	void					AdvanceFrame( float dt );
	int						SetFrame (int nFrame);

	void					ExtractBbox( float *mins, float *maxs );

	int						SetSequence( int iSequence );
	int						GetSequence( void );
	void					GetSequenceInfo( float *pflFrameRate, float *pflGroundSpeed );

	float					SetController( int iController, float flValue );
	float					SetMouth( float flValue );
	float					SetBlending( int iBlender, float flValue );
	int						SetBodygroup( int iGroup, int iValue );
	int						SetSkin( int iValue );

	void					scaleMeshes (float scale);
	void					scaleBones (float scale);


protected:

	// entity settings
//	vec3_t					m_origin;
//	vec3_t					m_angles;	
	int						m_sequence;			// sequence index
	float					m_frame;			// frame
	int						m_bodynum;			// bodypart selection	
	int						m_skinnum;			// skin group selection
	byte					m_controller[4];	// bone controllers
	byte					m_blending[2];		// animation blending
	byte					m_mouth;			// mouth position
	bool					m_owntexmodel;		// do we have a modelT.mdl ?

	// internal data
	studiohdr_t				*m_pstudiohdr;
	mstudiomodel_t			*m_pmodel;

	studiohdr_t				*m_ptexturehdr;
	studiohdr_t				*m_panimhdr[32];

	vec4_t					m_adj;				// FIX: non persistant, make static

	void					CalcBoneAdj( void );
	void					CalcBoneQuaternion( int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *q );
	void					CalcBonePosition( int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *pos );
	void					CalcRotations ( vec3_t *pos, vec4_t *q, mstudioseqdesc_t *pseqdesc, mstudioanim_t *panim, float f );
	mstudioanim_t			*GetAnim( mstudioseqdesc_t *pseqdesc );
	void					SlerpBones( vec4_t q1[], vec3_t pos1[], vec4_t q2[], vec3_t pos2[], float s );
	void					SetUpBones ( void );

	void					DrawPoints( void );

	void					Lighting (float *lv, int bone, int flags, vec3_t normal);
	void					Chrome (int *chrome, int bone, vec3_t normal);

	void					SetupLighting( void );

	void					SetupModel ( int bodypart );

private:

	UInt32 				_dataSize;
	mstudiomodel_t		*_mdl;
	char 				*_data;

	
};

extern vec3_t g_vright;		// needs to be set to viewer's right in order for chrome to work
extern float g_lambert;		// modifier for pseudo-hemispherical lighting


#endif	// CHLMdl_H
