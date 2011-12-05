/* 
	CModelInstance.h

	Author:			Tom Naughton
	Description:	<describe the CModelInstance class here>
*/

#ifndef CModelInstance_H
#define CModelInstance_H

#include <map>
#include <vector>
#include <string>
#include <list>

using std::string;
using std::map;
using std::vector;
using std::list;

#include "C3DModel.h"
#include "CShader.h"
#include "mathdefs.h"
#include "mathlib.h"

class CGLImage;
class CFileArchive;
class CShader;
class CResourceManager;

class CModelInstance
{
public:
	CModelInstance(CResourceManager *inResources);
	virtual ~CModelInstance();

	OSErr 				init(C3DModel *inModelClass);
	void 				Draw(renderingAttributes_t *renderingAttributes);
	void 				DrawLinks(renderingAttributes_t *rendering);
	void 				setFrame(SInt16 currentFrame, SInt16 nextFrame, float interp);
	C3DModel 			*modelClass() { return _modelClass; };
	void 				getFrame(SInt16 &currentFrame, SInt16 &nextFrame, float &interp);

	// simple textures
//	void 				addTexture(CGLImage* texture);
	SInt16 				getTextureIndex();
	void 				setTextureIndex(SInt16 index);

	// linking
	void				addLinkedModel(CModelInstance *model, const char *tagName);
	string 				selectedModelTag(renderingAttributes_t *rendering, CModelInstance *&model);
	CModelInstance 		*modelAtTag(const char *tagName);
	void 				autoAssemble(CFileArchive *pak, const char *name);
	CModelInstance 		*attachModel(const char *tagName, CFileArchive *pak, const char *name); 
	void 				detachModelAtTag(string tagName);

	// skinning
	void				applySkinNamed(CFileArchive *pak, const char *package, const char *skinName, Boolean childrentoo = true);
	Boolean				applySkin(CFileArchive *pak, string path);
	void 				parseSkin(char *skindata, long size);
	string 				textureNameForMesh(string tag); 
	void 				setTextureNameForMesh(string texturename, string tag);
	CShader 			*textureForMeshAtIndex(UInt32 index) { return _meshTextures[index]; };
	void				dumpMeshToTextureMap();
	
	// exporting 
	virtual void 		exportFormatToFile(ExportFormatT format, FSSpec spec); 
	virtual void 		dxfExport(LFileStream *file, Mat4 inTransform);
	virtual void 		objExportElementData(LFileStream *file, int &vertexCount, int &meshCount);
	virtual void 		objExportVertexData(LFileStream *file, Mat4 inTransform);

	// access
	CResourceManager	*resources() { return _modelClass->resources(); };
	CModelInstance 		*rootModel();
	CModelInstance		*parent() { return _parent; };

	void				setFrameForModelType(SInt16 currentFrame, SInt16 nextFrame, float interp, ModelType type);
	void				releaseUnusedTextures();
	
	UInt32				glName() { return _glName; };

	SInt16				_textureIndex;
	SInt16				_currentFrame;
	SInt16				_nextFrame;
	float 				_interpolationFraction; // between 0 and 1
	UInt32				_tagNum;
	UInt32				_meshNum;
	UInt32				_frameCount;
	ModelType			_modelType;
	UInt32				_glName;

	CModelInstance		**_links;
	CShader				**_meshTextures;

protected:


	typedef pair<string, string> mesh_pair_type;
	typedef map<string, string> mesh_map_type;
	typedef mesh_map_type::value_type mesh_entry_type;
	typedef mesh_map_type::iterator mesh_iterator;
	
	mesh_map_type _meshToTextureMap; // stores mesh to texture mappings for each (sub)model

	CModelInstance		*_parent;
	C3DModel 			*_modelClass;
};

#endif	// CModelInstance_H
