/* 
	CMd2.h

	Author:			Tom Naughton
	Description:	<describe the CMd2 class here>
*/

#ifndef CMd2_H
#define CMd2_H

#include <iostream.h>
#include "C3DModel.h"
#include "CMdl.h"

#define	MAX_SKINNAME	64

typedef struct MDL2HEADER
{
	UInt32 ident;
	UInt32 version;
	UInt32 skinwidth;
	UInt32 skinheight;
	UInt32 framesize;		// byte size of each frame
	UInt32 num_skins;
	UInt32 num_xyz;
	UInt32 num_st;			//greater than num_xyz for seams
	UInt32 num_tris;
	UInt32 num_glcmds;		//dwords in strip/fan command list
	UInt32 num_frames;
	UInt32 ofs_skins;		//each skin is a MAX_SKINNAME string
	UInt32 ofs_st;			//byte offset from start for stverts
	UInt32 ofs_tris;		//offset for dtriangles
	UInt32 ofs_frames;		//offset for first frame
	UInt32 ofs_glcmds;
	UInt32 ofs_end;			//end of file
} mdl2_header_t;


typedef struct MDL2FRAME
{
	float scale[3]; 		// multiply byte verts by this
	float translate[3]; 	// then add this
	char name[16]; 			// frame name from grabbing
	trivertx_t verts[1]; 	// variable sized
} frame_t;

typedef struct 
{
	short index_xyz[3];
	short index_st[3];
} dtriangle_t;

typedef struct
{
	short s;
	short t;
} dstvert_t;

typedef struct MD2MODEL
{
	mdl2_header_t *header;
	long *glcmds;
	dtriangle_t *triangles;
	dstvert_t *texCoord;
	char *frames;
} md2_t;

typedef struct VERTEX
{
	float x,y,z;
	float u,v;
} vert_t;



class CPakStream;

class CMd2 : public C3DModel
{
public:

	CMd2(CResourceManager *resources);
	virtual ~CMd2();
	
	virtual Boolean 	init(CPakStream *inItem);
	virtual void 		Draw(CModelInstance *instance, renderingAttributes_t *renderingAttributes);
	virtual void 		DrawTextureMesh (renderingAttributes_t *rendering, CGLImage *texture);
	virtual void 		Draw2(CModelInstance *instance, renderingAttributes_t *renderingAttributes);
	virtual void 		Draw3(CModelInstance *instance, renderingAttributes_t *renderingAttributes);

	virtual SInt16 		frameCount();
	virtual string 		frameName(SInt16 frame);
	virtual UInt32 		meshNum();
	
	// exporting 
	virtual Boolean 	canExportFormat(ExportFormatT format); 
	virtual void 		dxfExportMesh(CModelInstance *instance, LFileStream *file, Mat4 transform, int meshnum);
	virtual void 		objExportMeshVertexData(CModelInstance *instance, LFileStream *file, Mat4 transform, int meshnum) ;
	virtual void 		objExportMeshElementData(LFileStream *file, int meshnum, int &vertexCount, int &meshCount);
		
protected:
	
	void 				SwapGLCommands();
	CGLImage			*loadSkin(CPakStream *inItem, char *p);

	UInt32				_glName;
	md2_t 				*md2;
	char				*_modelData;
};

#endif	// CMd2_H
