/* 
	CMdl.h

	Author:			Tom Naughton
	Description:	<describe the CMdl class here>
*/

#ifndef CMdl_H
#define CMdl_H

#include <vector>

#include "C3DModel.h"
#include "CGLImage.h"
#include "mathdefs.h"
#include "mathlib.h"

#define MDL_VERSION	6		// current mdl file version
#define MAX_VERTS	1024
#define PALETTE_SIZE	256
#define FRAME_NAME_LENGTH		16	// fame names are no longer than this
#define HEADER_MDL	'IDPO'	// 0x4F504449 = "IDPO" for IDPOLYGON
#define MDL_VERSION	6		// current mdl file version

// Here is the format of the .MDL file header: 
typedef float scalar_t;

typedef struct MDLHEADER
{
	UInt32 tag;                    // 0x4F504449 = "IDPO" for IDPOLYGON
	UInt32 version;                // Version = 6
	Vec3 scale;                // Model scale factors.
	Vec3 origin;               // Model origin.
	scalar_t radius;             // Model bounding radius.
	Vec3 offsets;              // Eye position (useless?)
	UInt32 numskins ;              // the number of skin textures
	UInt32 skinwidth;              // Width of skin texture
	                             //           must be multiple of 8
	UInt32 skinheight;             // Height of skin texture
	                             //           must be multiple of 8
	UInt32 numverts;               // Number of vertices
	UInt32 numtris;                // Number of triangles surfaces
	UInt32 numframes;              // Number of frames
	UInt32 synctype;               // 0= synchron, 1= random
	UInt32 flags;                  // 0 (see Alias models)
	scalar_t size;               // average size of triangles
} mdl_header_t;


//The list of skin vertices is made of these structures: 

typedef struct
{
	UInt32 onseam;                 // 0 or 0x20
	UInt32 s;                      // position, horizontally
	                             //  in range [0,skinwidth[
	UInt32 t;                      // position, vertically
	                             //  in range [0,skinheight[
} stvert_t;

// stvert_t vertices[numverts];

/*
	skinsizes is the sum of the size of all skin pictures. 

      If they are all simple skins, then skinsize = (4 + skinwidth * skinheight) * numskins.
      If there is a mix of simple skin and group skin, you have to decode it to find out where the vertex begin. 
*/

// Here is the structure of triangles: 

typedef struct
{
	UInt32 facesfront;             // boolean
	UInt32 vertices[3];            // Index of 3 triangle vertices
	                             // in range [0,numverts[
} itriangle_t;

// At offset baseTri = baseVerts + numverts * sizeof(stvert_t) in the .MDL file, you will find: 

// itriangle_t triangles[numtris];

// When the triangle is on the back skin, then any skin vertex that is on the skin seam (as indicated by onseam=1) must have it's
// s coordinate increased by skinwidth/2.

/*
for(j=0; j < numtris; j++)
{
	for(i=0; i < 3 ; i++)
	{ 
		vertex = triangles[j].vertices[i]
		s = vertices[vertex].s;
		t = vertices[vertex].t;
		if( (vertices[vertex].onseam) && (!triangle[j].facesfront))
		{ 
			s += skinwidth / 2;
		}
		// use s and t as the coordinates of the vertex
	}
}
*/

/*
	This frame structure is rather complex to figure out, because: 

      Frames can come standalone or in groups 
      vertex position, in frames, are packed to save space. 
      vertex normals are indicated by an index in a table. 
*/

// The frame vertices
// Each frame vertex is defined by a 3D position and a normal for each of the vertices in the model. 
typedef struct
{
	UInt8 packedposition[3];    // X,Y,Z coordinate, packed on 0-255
	UInt8 lightnormalindex;     // index of the vertex normal
} trivertx_t;

// The formula for calculating positions is: 
// vec3_t position[i] = ( scale[i] *  packedposition[i] ) + origin[i]

// The simple frames can come standalone or in groups (see below). They always have the same structure: 
typedef struct
{
	UInt32 frametype;             
	trivertx_t min;              // minimum values of X,Y,Z
	trivertx_t max;              // maximum values of X,Y,Z
	char name[FRAME_NAME_LENGTH];  // name of frame
	trivertx_t vertex[1];  // array of vertices
} simpleframe_t;




extern float	avertexnormals[NUMVERTEXNORMALS][3];

typedef struct MDLMODEL
{
	mdl_header_t	*header;		// header of the mdl file
	itriangle_t		*triangles;		// the triangle array
	simpleframe_t	*frames;		// the frames array
	stvert_t		*texCoord;		// texture coordinates
	UInt32			frameSize;
	
} mdl_t;

using std::vector;


class CMdl:  public C3DModel
{
public:
	
	CMdl(CResourceManager *resources);
	virtual ~CMdl();
	
	virtual Boolean 	init(CPakStream *inItem);
	virtual void 		Draw(CModelInstance *instance, renderingAttributes_t *renderingAttributes);
	virtual void 		DrawTextureMesh (renderingAttributes_t *rendering, CGLImage *texture);
	static CGLImage 	*CreateTextureFromSkin(unsigned char *skin, RGB *palette, int skinWidth, int skinHeight);

	virtual SInt16 		frameCount();
	virtual string 		frameName(SInt16 frame);
	virtual UInt32 		meshNum();

	// exporting 
	virtual Boolean 	canExportFormat(ExportFormatT format); 
	virtual void 		dxfExportMesh(CModelInstance *instance, LFileStream *file, Mat4 transform, int meshnum);
	virtual void 		objExportMeshVertexData(CModelInstance *instance, LFileStream *file, Mat4 transform, int meshnum) ;
	virtual void 		objExportMeshElementData(LFileStream *file, int meshnum, int &vertexCount, int &meshCount);

/*
	// exporting 
	virtual Boolean 	canExportFormat(ExportFormatT format); 
	virtual void 		dxfExport(CModelInstance *instance, LFileStream *file, Mat4 inTransform);

	void				objExportElementData(LFileStream *file);
	void				objExportVertexData(CModelInstance *instance, LFileStream *file);
*/

protected:

	CGLImage			*loadSkin(char *p, UInt32 dataSize);
	//void 				loadPalette();
	
	mdl_t				*_mdl;
	char 				*_data;
	UInt32 				_dataSize;
	UInt32				_glName;
	
};

#endif	// CMdl_H
