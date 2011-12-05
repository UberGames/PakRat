/* 
	C3DModel.h

	Author:			Tom Naughton
	Description:	<describe the C3DModel class here>
*/

#ifndef C3DModel_H
#define C3DModel_H

#include <iostream.h>
#include <string>
#include <map>

#include "CShader.h" 
#include "CResourceManager.h" 
#include "mathdefs.h"
#include "mathlib.h"

class CPakStream;
class CModelInstance;
class CFileArchive;
class CMd3AnimationInfo;
class CGLImage;

using std::string;
using std::map;

const double FAR_GL_PLANE = 10000.0;
const double NEAR_GL_PLANE = 0.1d;
//const double FAR_GL_PLANE = 512.0d;
//const double NEAR_GL_PLANE = 0.0; // 10.0;

typedef enum
{
	INTERP_NONE = 0,
	INTERP_LINEAR,
	INTERP_CUBIC
} InterpMethodT;

typedef enum
{
	WAVEFRONT_OBJ_FORMAT = 0,
	AUTOCAD_DXF_FORMAT
} ExportFormatT;

typedef struct MDLVERTEX
{
	float x,y,z;
} mdl_vertex_t;

typedef struct MDLMATRIX
{
	float m[3][3];
} mdl_matrix_t;

typedef struct MD3TAG
{
	char 			name[12];
	char 			unknown[52];
	Vec3			Position;			// relative position of tag	
	Vec4            Rotation;           // rotation quaternion
	
	union {
		struct {
			UInt32 			glName;
		} info;
		struct {
			float			pad[5];
		} pad;
	};
} md3_tag_t;
// used for animation
// determined by presence of tags
typedef enum ModelType  {
	unknown_model = 0,
	upper_model,
	lower_model,
	head_model,
	weapon_model,
	barrel_model,
	flash_model,
	other_model
} ModelType;



#define NUMVERTEXNORMALS 162	// normals for MDLs and MD2s are taken from a lookup table
extern float avertexnormals[NUMVERTEXNORMALS][3];

class C3DModel
{
public:
	C3DModel();
	C3DModel(CResourceManager *resources);
	virtual ~C3DModel();

	virtual Boolean 	init(CPakStream *inItem) = 0;
	virtual void 		Draw(CModelInstance *instance, renderingAttributes_t *renderingAttributes) = 0;
	virtual void 		DrawLinks(CModelInstance *instance, renderingAttributes_t *rendering);
	virtual Mat4 		PushTagTransformMatrix(CModelInstance *instance, int tagindex);

	
	virtual SInt16 		frameCount();
	virtual string 		frameName(SInt16 frame);
	virtual UInt32 		tagNum();
	virtual UInt32 		meshNum();
	virtual ModelType	modelType();
	virtual Boolean		shouldDraw(renderingAttributes_t *rendering, CShader *shader);
	virtual Boolean		needsNormals(renderingAttributes_t *rendering, CShader *shader);
	virtual CMd3AnimationInfo *animationInfo();
	virtual int 		tagIndexWithName(const char *name);
	virtual string 		tagNameForFrameAtIndex(short frame, short index);
	virtual md3_tag_t 	*tagForFrameAtIndex(short frame, short index);

	
	virtual void		loadMeshTextures(CModelInstance *instance);
	CGLImage 			*textureWithName(string name, string skinnname="");
	void 				setTextureWithName(CGLImage *texture, string name);
	void 				addTexture(CGLImage* texture);
	void 				dumpTextureCache();
	
	string 				pathName() { return _pathname; };
	
	// exporting 
	virtual Boolean 	canExportFormat(ExportFormatT format) = 0; 
	virtual void 		dxfExportMesh(CModelInstance *instance, LFileStream *file, Mat4 transform, int meshnum) = 0;
	virtual void 		objExportMeshVertexData(CModelInstance *instance, LFileStream *file, Mat4 transform, int meshnum) = 0;
	virtual void 		objExportMeshElementData(LFileStream *file, int meshnum, int &vertexCount, int &meshCount) = 0;
	
	float				Interpolate(float p[], float u, InterpMethodT method);
	
	
	SInt16				textureCount() { return _textureCount; };
	CResourceManager	*resources() { return _resources; };
	CFileArchive		*pak() { return _resources->pak(); };
	vector<CGLImage*> 	*_textures ;

protected:
		
	typedef pair<string, CGLImage *> texture_pair_type;
	typedef map<string, CGLImage *> texture_map_type;
	typedef texture_map_type::value_type texture_entry_type;
	typedef texture_map_type::iterator texture_iterator;
	
	texture_map_type _textureMap; // stores all textures for model group (stored in root model)

	CResourceManager 	*_resources;
	SInt16				_textureCount;
	string _pathname;

};

#endif	// C3DModel_H
