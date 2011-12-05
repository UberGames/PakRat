/* 
	CMdc.h

	Author:			Tom Naughton
\*/

\
#ifndef CMdc_H
#define CMdc_H

// FIXME: should inherit from a common abstract base class.
#include "CMd3.h"


class CShader;

class MDC_Triangle
{
public:
	UInt32 indices[3];
	void loadFromStream(CPakStream *inItem);
};		

class MDC_Vertex
{
public:
	SInt16 position[3];
	UInt8 	u,v;
	
	void loadFromStream(CPakStream *inItem);
};		

class MDC_TexCoord
{
public:
	float stu[3];

	void loadFromStream(CPakStream *inItem);
	void debugDump();
};		

class MDC_Shader
{
public:
	char name[64];
	UInt32 flags;

	void loadFromStream(CPakStream *inItem);
	void debugDump();
};		

class MDC_TagName
{
public:
	char name[64];

	void loadFromStream(CPakStream *inItem);
	void debugDump();
};		

class MDC_TagFrame
{
public:
	SInt16 xyz[3];
	float angles[3];
	// matrix

	void loadFromStream(CPakStream *inItem);
};		

class MDC_Surface
{
	virtual ~MDC_Surface();
public:
	UInt32 ident;
	char name[64];
	UInt32 flags;
	UInt32 numCompFrames;
	UInt32 numBaseFrames;
	UInt32 numShaders;
	UInt32 numVertices;
	UInt32 numTriangles;
	UInt32 offsetTriangles;
	UInt32 offsetShaders;
	UInt32 offsetTexCoords;
	UInt32 offsetVertices;
	UInt32 offsetCompVerts;
	UInt32 offsetFrameBaseFrames;
	UInt32 offsetFrameCompFrames;
	UInt32 offsetEnd;

	MDC_Triangle 	*_triangles;
	MDC_Shader 		*_shaders;
	MDC_TexCoord 	*_texCoords;
	MDC_Vertex 		*_vertices;

	void loadFromStream(CPakStream *inItem);
	Boolean isValid();
	void debugDump();
};	

class MDC_Frame
{
public:
	float bboxMin[3];
	float bboxMax[3];
	float localOrigin[3];
	float radius;
	char name[16];		// often creator

	void loadFromStream(CPakStream *inItem);
	void debugDump();
};		

class MDC_Header
{
public:
	UInt32 ident;
	UInt32 version;
	char name[64];
	UInt32 flags;
	UInt32 numFrames;
	UInt32 numTags;
	UInt32 numSurfaces;
	UInt32 numSkins;
	UInt32 offsetFrames;
	UInt32 offsetTagNames;
	UInt32 offsetTags;
	UInt32 offsetSurfaces;
	UInt32 offsetEnd;

	void loadFromStream(CPakStream *inItem);
	Boolean isValid();
	void debugDump();
};		


class CMdc: public CMd3
{
public:
	CMdc(CResourceManager *resources);
	virtual ~CMdc();

	virtual Boolean 	init(CPakStream *inItem);
	virtual void 		Draw(CModelInstance *instance, renderingAttributes_t *renderingAttributes);
	virtual void 		DrawLinks(CModelInstance *instance, renderingAttributes_t *rendering);
	virtual void 		loadMeshTextures(CModelInstance *instance);

	virtual SInt16		frameCount() ;
	virtual UInt32 		tagNum();
	virtual UInt32 		meshNum();
	virtual ModelType	modelType();
	virtual int 		tagIndexWithName(const char *name);
	virtual string 		tagNameForFrameAtIndex(short frame, short index);
	virtual md3_tag_t 	*tagForFrameAtIndex(short frame, short index);

	virtual Boolean 	canExportFormat(ExportFormatT format); 

protected:


	MDC_Header 	*_header;
	MDC_Frame 	*_frames;
	MDC_TagName *_tagNames;
	MDC_TagFrame *_tagFrames;
	MDC_Surface *_surfaces;
	
	typedef vector<md3_tag_t*> tagArray;
	tagArray	*_convertedTags;

	void DrawSurface(CModelInstance *instance, renderingAttributes_t *rendering , MDC_Surface *surface);
	
};

#endif	// CMdc_H
