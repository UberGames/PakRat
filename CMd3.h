/* 
	CMd3.h

	Author:			Tom Naughton
	Description:	<describe the CMd3 class here>
*/

#ifndef CMd3_H
#define CMd3_H

#include <map>
#include <vector>
#include <string>
#include <list>

#include "C3DModel.h"
 
 
class CMd3;
class CWav;
class CMd3AnimationInfo;
class CShader;

using std::string;
using std::map;
using std::vector;
using std::list;



// mesh structures
 
typedef struct MD3MESHHEADER
{
	UInt32		ident;
	char		name[65];

	UInt32 		meshFrameNum;
	UInt32 		textureNum;
	UInt32 		vertexNum;
	UInt32 		triangleNum;
	UInt32 		triangleStart;
	UInt32 		textureStart;
	UInt32 		texVecStart;
	UInt32 		vertexStart;
	UInt32 		meshSize;

} md3_mesh_header_t;

typedef struct MD3VERTEXNORMAL
{
	SInt16 	x,y,z;
	UInt8 	u,v;
} md3_vertex_normal_t;

typedef struct MD3TRIANGLE
{
	SInt32 index[3];
} md3_triangle_t;

typedef struct MD3TEXTURE
{
	char name[65];
} md3_texture_t;

typedef struct MD3TEXVEC
{
	float u,v;
} md3_texvec_t;

typedef struct MD3MESH
{	
	UInt32 				glName;
	md3_mesh_header_t 	*header;
	md3_triangle_t		*triangles;
	md3_texture_t		*textures;
	md3_texvec_t		*texvecs;
	md3_vertex_normal_t *vertices;
	
} md3_mesh_t;

// model structures

typedef struct MD3HEADER
{
	UInt32 ident;
	UInt32 version;
	char name[65];
	UInt32 boneFrameNum;
	UInt32 tagNum; 
	UInt32 meshNum;  
	UInt32 maxTextureNum; 
	UInt32 boneFrameStart; 
 	UInt32 tagStart; 
 	UInt32 meshStart;                     
	UInt32 fileSize; 

} md3_header_t;

typedef struct MD3BONEFRAME
{
	mdl_vertex_t	mins;
	mdl_vertex_t	maxs;
	mdl_vertex_t	position;
	float 			scale;
	char			creator[16];
} md3_boneFrame_t;



typedef struct MD3MODEL
{
	char 				*meshData;

	md3_header_t		*header;
	md3_boneFrame_t		*boneframes;
	md3_tag_t			*tags;	
	md3_mesh_t			**meshes;
//	CMd3				**links;
} md3_t;



class CMd3:  public C3DModel
{
public:


	CMd3(CResourceManager *resources);
	virtual ~CMd3();

	virtual Boolean 	init(CPakStream *inItem);
	virtual void 		Draw(CModelInstance *instance, renderingAttributes_t *renderingAttributes);
	virtual void 		DrawLinks(CModelInstance *instance, renderingAttributes_t *rendering);
	virtual void 		loadMeshTextures(CModelInstance *instance);
	virtual Mat4 		PushTagTransformMatrix(CModelInstance *instance, int tagindex);

	virtual SInt16		frameCount() ;
	virtual UInt32 		tagNum();
	virtual UInt32 		meshNum();
	virtual ModelType	modelType();
	virtual CMd3AnimationInfo	*animationInfo();
	virtual int 		tagIndexWithName(const char *name);
	virtual string 		tagNameForFrameAtIndex(short frame, short index);
	virtual md3_tag_t 	*tagForFrameAtIndex(short frame, short index);
	void 				parseAnimationFile(CPakStream *skinFile);

	// exporting 
	virtual Boolean 	canExportFormat(ExportFormatT format); 
	virtual void 		dxfExportMesh(CModelInstance *instance, LFileStream *file, Mat4 transform, int meshnum);
	virtual void 		objExportMeshVertexData(CModelInstance *instance, LFileStream *file, Mat4 transform, int meshnum) ;
	virtual void 		objExportMeshElementData(LFileStream *file, int meshnum, int &vertexCount, int &meshCount);
	
protected:
	
	md3_mesh_t 			*loadMesh(char *&p);

	void 				DrawMesh(CModelInstance *instance, renderingAttributes_t *rendering, UInt32 mesh);
	void	 			DrawBoneFrameBox(CModelInstance *instance, renderingAttributes_t *rendering);
	void 				DrawVertexNormals(renderingAttributes_t *renderingAttributes, int vertexNum, float *vertices, float *normals); 

	void 				deformVerts(CShader *shader, double *vertices, double *normals, int vertexNum);
	double 				*interpolateMeshFrame(md3_vertex_normal_t *currMeshFrame, md3_vertex_normal_t *nextMeshFrame, double frac, UInt32 vertexNum);
	md3_boneFrame_t 	*interpolateBoneFrame(md3_boneFrame_t *currBoneFrame, md3_boneFrame_t *nextBoneFrame, float frac);
	double 				*interpolateVertexNormals(md3_vertex_normal_t *currNormals, md3_vertex_normal_t *nextNormals, double frac, UInt32 vertexNum);
	Mat4 				interpolateTransformation(md3_tag_t *currFrameTag, md3_tag_t *nextFrameTag, float frac);

	md3_t				*_md3;
	char				*_md3Data;
	
	CMd3AnimationInfo	*_animationInfo;
	ModelType			_modelType;
};

#endif	// CMd3_H
