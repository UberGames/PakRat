/* 
	CMds.h

	Author:			Tom Naughton
	Description:	Based on code from Michael Bristol
*/

/*
 ******************************************
 * Wrapper of id software's MDS file format
 *  Michael Bristol
 *  mbristol@bellatlantic.net
 *
 *  MDS file format property of id software
 *  This was adapted from the MD4 format released with FAKK2 
 *  - code Copyright (C) 1999 by Ritual Entertainment, Inc
 *    Provided as a portion of the FAKK2 toolkit
 **************************************/


#ifndef _ID_MDS_H_
#define _ID_MDS_H_

#include "CMd3.h"


#define ALIAS_VERSION2	8
#define ALIAS_VERSION3	15

#define ID_HEADER_MDS		'WSDM' //'IDP4'	
#define	MAX_QPATH			64
#define MDS_VERSION			4
#define	MDS_MAX_BONES		128

// ------------------------- MDS file structures -------------------------------
class MDSHeader
{
public:


	UInt32  ident;          // "MDSW"
	UInt32  version;        // 0x0004
	char    name[MAX_QPATH];       // Model name eg. "body.mds"

	float   lodScale;       // LOD Scale
	float   lodBias;        // LOD Bias

	UInt32  numFrames;      // Number of animation frames
	UInt32  numBones;       // Number of bones
	UInt32  offsetFrames;   // Offset of animation frames (see MDSFrame)
	UInt32  offsetBones;    // Offset of bones (see MDSBone)

	UInt32  torsoParent;    // Index of torso parent bone

	UInt32  numSurfaces;    // Number of surfaces
	UInt32  offsetSurfaces; // Offset of surfaces (numSurfaces * MDS_Surface)

	UInt32  numTags;        // Number of tags
	UInt32  offsetTags;     // Offset of tags (numTags * MDS_Tag)

	UInt32  offsetEnd;      // Offset of end of file


	void loadFromStream(CPakStream *inItem);
	Boolean isValid();
	void debugDump();
} ;

class MDSWeight
{
public:


	UInt32  boneIndex;  // Index of bone in the main bone list
	float   weight;     // Bone weighting (from 0.0 to 1.0)
	float  	xyz[3];        // xyz bone space position of vertex

	void loadFromStream(CPakStream *inItem);
} ;

class MDSVertex
{
public:



	float  normal[3];      // Vertex normal vector. In base pose/model space I presume.
	float  texCoords[2];   // Texture (s,t) co-ordinates
	UInt32    numWeights;  // Number of bone weights affecting this vertex

	UInt32    fixedParent; // Stay equidistant from this parent
	float     fixedDist;   // Fixed distance from parent

	void loadFromStream(CPakStream *inItem);
	
	MDSWeight *_weights;
} ;

class MDSPoly
{
public:

	UInt32			vind[3];		// Vertex indices
	void loadFromStream(CPakStream *inItem);
} ;

class MDSBoneRef
{
public:

	UInt32		Ref;
	void loadFromStream(CPakStream *inItem);
} ;


class MDSSurface
{
public:


	UInt32  ident;           // Always 0x0008
	char    name[64];        // Name of surface
	char    shader[64];      // Name of shader
	UInt32  shaderIndex;     // Used in game only

	SInt32  minLod;          // Minimum Lod level
	SInt32  offsetHeader;    // Offset of this surface header, always negative

	UInt32  numVerts;        // Number of vertices
	UInt32  offsetVerts;     // Offset of vertices (MDSVertex * numVerts)

	UInt32  numTris;         // Number of triangles
	UInt32  offsetTris;      // Offset of triangles (MDSTriangle * numTris)

	UInt32  offsetCollapseMap; // Offset of the collapse map (UInt32 * numVerts in size)

	UInt32  numBoneRefs;     // Number of bone references (bones that influence this surface)
	UInt32  offsetBoneRefs;  // Offset of bone refs (UInt32 * numBones)

	UInt32  offsetEnd;       // Offset of the end of this surface, from start of MDSSurface header.
	                       // The next surface (if there are more) can be found at this offset.	
	MDSVertex *_vertices;
	MDSPoly *_triangles;
	MDSBoneRef *_bonerefs;
	

	void loadFromStream(CPakStream *inItem);
	void debugDump();
} ;


// 80 bytes
class MDSBone
{
public:


	char   name[64];    // Bone name
	SInt32 parentIndex; // Bone parent index (-1 if root)
	float  torsoWeight; // 0.0 to 1.0
	float  parentDist;  // Distance from parent bone to this bone's pivot point
	UInt32 flags;       // Bit 0 is set if bone is a tag

	void loadFromStream(CPakStream *inItem);
	void debugDump();
} ;

// 12 bytes
class MDSBoneFrame
{
public:

//	unsigned char	data[12];
	short			Orientation[3];
	short			Position[3];		// /64 on input

	void loadFromStream(CPakStream *inItem);
	void debugDump();
} ;

class MDSCompressedBoneFrame
{
	SInt16  angles[4];        // Defines the bone orientation
	SInt16  offsetAngles[2];  // Defines the direction of the bone pivot from this bone's parent

	void loadFromStream(CPakStream *inItem);
};


// 52 bytes
class MDSFrame
{
public:

	float  bboxMin[3];      // Bounding box min
	float  bboxMax[3];      // Bounding box max
	float  localOrigin[3];  // Local origin
	float     radius;       // Radius of bounding sphere (used for runtime culling)
	float  parentOffset[3]; // Offset of parent bone from origin

	// Following this structure is a list of compressed bone frames
	MDSCompressedBoneFrame  bones; // [numBones]
  
	void loadFromStream(CPakStream *inItem);
	void debugDump();
} ;

// I believe this just advertises which bones to call 'tags' ... in the old sense
class MDSTag
{
public:


	char   name[64];    // Name of tag
	float  torsoWeight; // 0.0, 0.3
	UInt32 boneIndex;   // Index of bone this tag is attached to
  
	void loadFromStream(CPakStream *inItem);
} ;




class MDSTriangle
{
	UInt32 vertexIndices[3];  // 3 indices into the surface's vertex list

	void loadFromStream(CPakStream *inItem);
};




class CMds: public CMd3
{
public:
	CMds(CResourceManager *resources);
	virtual ~CMds();

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

	void DrawSurface(CModelInstance *instance, renderingAttributes_t *rendering , MDSSurface *surface);

	MDSHeader			*_header;
	MDSBone 			*_bones;
	MDSFrame 			*_frames;
	MDSBoneFrame 		*_boneFrames;
	MDSSurface			*_surfaces; 
	MDSTag 				*_tags;

	typedef vector<md3_tag_t*> tagArray;
	tagArray	*_convertedTags;
	
};


#endif // _ID_MDS_H_
