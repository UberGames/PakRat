/* 
	CModelInstance.cpp

	Author:			Tom Naughton
	Description:	<describe the CModelInstance class here>
*/

#include <agl.h>
#include <glut.h>


#include "CModelInstance.h"
#include "CFileArchive.h"
#include "CPakStream.h"
#include "CShader.h"
#include "CTokenizer.h"
#include "CGLImage.h"
#include "CResourceManager.h"
#include "CMd3.h"
#include "utilities.h"



CModelInstance::CModelInstance(CResourceManager *inResources)
{
#pragma unused (inResources)
	_textureIndex = 0;
	_currentFrame = 0;
	_nextFrame = 0;
	_interpolationFraction = 0.0f ;// between 0 and 1
	_parent = nil;
	_tagNum = 0;
	_frameCount = 0;
	_modelType = unknown_model;
	_meshNum = 0;
	_links = 0; 
}


CModelInstance::~CModelInstance()
{
	// throw away linked models
	if (_links) {
		for (int j=0; j<_tagNum; j++) {
			if (_links[j]) {
				delete _links[j];
				_links[j] = 0;
			}
		}
		CMemoryTracker::safeFree(_links);
	}
	
	// holds references to shaders that will be disposed by 
	// resource manager
	if (_meshTextures)
		CMemoryTracker::safeFree(_meshTextures);
}

OSErr CModelInstance::init(C3DModel *inModelClass)
{

	OSErr err = noErr;
	_modelClass = inModelClass;
	
	_tagNum = _modelClass->tagNum();
	_frameCount = _modelClass->frameCount();
	_modelType = _modelClass->modelType();
	_meshNum = _modelClass->meshNum();
	_glName = nextGLName();
	
	// create link array
	if (_tagNum) {
		_links = (CModelInstance**) CMemoryTracker::safeAlloc (_tagNum, sizeof(CModelInstance*), "model link array");
		if (!_links)
			goto fail;
		for (int j=0; j<_tagNum; j++) {
			_links[j] = 0;
			nextGLName(); // waste some names
		}
	}
	
	// create mesh texture index array
	_meshTextures = 0;
	if (_meshNum) {
		_meshTextures = (CShader**) CMemoryTracker::safeAlloc (_meshNum, sizeof(CShader*), "mesh texture array");
		if (!_meshTextures)
			goto fail;
		for (int j=0; j<_meshNum; j++) {
			_meshTextures[j] = 0;
		}
	}
		

fail:
	return err;	

}

void CModelInstance::setFrame(SInt16 currentFrame, SInt16 nextFrame, float interp)
{
	if (currentFrame >= _frameCount)
		currentFrame = _frameCount-1;
	if (currentFrame < 0)
		currentFrame = 0;
	if (nextFrame >= _frameCount)
		nextFrame = _frameCount-1;
	if (nextFrame < 0)
		nextFrame = 0;

	_currentFrame = currentFrame;
	_nextFrame = nextFrame;
	_interpolationFraction = interp;
}

void CModelInstance::getFrame(SInt16 &currentFrame, SInt16 &nextFrame, float &interp)
{
	currentFrame = _currentFrame;
	nextFrame = _nextFrame;
	interp = _interpolationFraction;
}


SInt16 CModelInstance::getTextureIndex()
{
	return _textureIndex;
}

void CModelInstance::setTextureIndex(SInt16 index)
{
	_textureIndex = index;
}

void CModelInstance::Draw(renderingAttributes_t *renderingAttributes)
{
	_modelClass->Draw(this, renderingAttributes);
	
	// draw linked models
	for (int j=0; j< _tagNum ; j++) {
		CModelInstance *child = _links[j];
		Mat4 oldTransform;
		
      	if (child) {
      		if (!(renderingAttributes->_renderTextureCoordinates || renderingAttributes->_renderTexturePreview)) {
      			oldTransform = renderingAttributes->_environmentTransform;
				renderingAttributes->_environmentTransform *= 
					_modelClass->PushTagTransformMatrix(this, j);
			}
			
			child->Draw(renderingAttributes);
			
      		if (!(renderingAttributes->_renderTextureCoordinates || renderingAttributes->_renderTexturePreview)) {
				glPopMatrix();		
      			renderingAttributes->_environmentTransform = oldTransform;
			}
		}
	}
}

void CModelInstance::DrawLinks(renderingAttributes_t *renderingAttributes)
{
	_modelClass->DrawLinks(this, renderingAttributes);
	
	// draw child links
	for (int j=0; j< _tagNum ; j++) {
		CModelInstance *child = _links[j];
		
      	if (child) {
			_modelClass->PushTagTransformMatrix(this, j);
			child->DrawLinks(renderingAttributes);
			glPopMatrix();		
		}
	}
}

CModelInstance *CModelInstance::rootModel()
{
	if (!_parent)
		return this;
	else
		return _parent->rootModel();
}

void CModelInstance::setFrameForModelType(SInt16 currentFrame, SInt16 nextFrame, float interp, ModelType type)
{
	if (_modelType == type)
		setFrame(currentFrame, nextFrame, interp);
		
	for (int j=0; j< _tagNum ; j++) {
		CModelInstance *child = _links[j];
		if (child)
			child->setFrameForModelType( currentFrame,  nextFrame,  interp,  type);
	}	
}

#pragma mark -

Boolean CModelInstance::applySkin(CFileArchive *pak, string path)
{
	dprintf("applySkin %s \n", path.c_str());
	CPakStream *skinFile = nil;
	char *skindata = nil;
	Boolean result = false;
	
	if (!pak)
		goto exit;
	skinFile = pak->itemWithPathName(path.c_str());
	if (!skinFile)
		goto exit;
	long size = skinFile->getSize();
	skindata = (char*) skinFile->getData("md3 skinfile");
	if (!skindata )
		goto exit;
		
	parseSkin(skindata, size);
	result = true;

exit:
	if (skinFile)
		delete skinFile;
	if (skindata)
		CMemoryTracker::safeFree(skindata);
	return result;
}

void CModelInstance::applySkinNamed(CFileArchive *pak, const char *package, const char *skinName, Boolean childrentoo)
{
	dprintf("applySkinNamed %s %s\n", package, skinName);
	CPakStream *skinFile = nil;
	string partName, texture = "";
	
	switch(_modelClass->modelType()){
		case upper_model:
			partName = "upper_";
			break;
		case lower_model:
			partName = "lower_";
			break;
		case head_model:
			partName = "head_";
			break;
	}

	string skinFileName = package + partName + skinName + ".skin";
	applySkin(pak, skinFileName);
	
	if (childrentoo) {
		for (int j=0; j< _tagNum ; j++) {
			CModelInstance *child = _links[j];
			if (child)
				child->applySkinNamed(pak, package, skinName, childrentoo);
		}	
	}
}

void CModelInstance::parseSkin(char *skindata, long size)
{
	char *p = skindata;
	
	char *e = p + size;
	int lineno = 1;
	static string delimiters =  " ,\t";
	
	while (p < e) {
		string line = nextLine(p,e);
		
		CTokenizer tokenizer(line, delimiters);
		string token;

		// ignore lines with more or less than two tokens
		if (tokenizer.countTokens() == 2) {

			string meshName = tokenizer.nextToken();
			string textureName = lowerString(fixSlashes(stripExtension(tokenizer.nextToken())));
			dprintf("skinfile - meshName: %s texture: %s\n",meshName.c_str(), textureName.c_str());
			// load the texture or use the cached one
			CShader *texture = resources()->shaderWithName(textureName.c_str());
			
			// (re)associate the texture with the tag
			if (texture) {
				setTextureNameForMesh(textureName, meshName);
			}
		}
	}
	dumpMeshToTextureMap();
}

void CModelInstance::setTextureNameForMesh(string texturename, string mesh)  
{ 
	_meshToTextureMap[mesh] = texturename; 
	
	// clear out the lookup cache
	for (int i = 0; i < _meshNum; i++)
		_meshTextures[i] = 0;
	
	// set it in all the children too
	for (int j=0; j< _tagNum ; j++) {
		CModelInstance *child = _links[j];
		if (child) {
			child->setTextureNameForMesh(texturename, mesh);
		}
	}
}

string CModelInstance::textureNameForMesh(string mesh)
{ 
	string result;
	
	// Lower LOD meshes have names like 'h_head_2'
	// which needs to match the mesh name 'h_head' 
	// from the skin file
	CModelInstance::mesh_iterator e = _meshToTextureMap.begin();
	while (e != _meshToTextureMap.end()) {
		if (mesh.find(e->first) == 0)
			result = e->second;
		e++;
	}	
	
	//dprintf("textureName %s ForMesh %s\n", result.c_str(), mesh.c_str());
	return result; 
} 

void CModelInstance::dumpMeshToTextureMap()
{
	CModelInstance::mesh_iterator e = _meshToTextureMap.begin();
		
	dprintf("texture Cache:\n");
	while (e != _meshToTextureMap.end()) {
		dprintf("	%s : %s\n", e->first.c_str(), e->second.c_str());
		e++;
	}	
}


#pragma mark -

void CModelInstance::addLinkedModel(CModelInstance *model, const char *tagName)
{
	int tagIndex = _modelClass->tagIndexWithName(tagName);
	if (tagIndex >= 0 && tagIndex < _tagNum)  {
		_links[tagIndex] = model;
		model->_parent = this;
	}
}

CModelInstance *CModelInstance::modelAtTag(const char *tagName)
{
	int tagIndex = _modelClass->tagIndexWithName(tagName);
	if (tagIndex >= 0)  {
		return _links[tagIndex];
	}
	return nil;
}

void CModelInstance::autoAssemble(CFileArchive *pak, const char *name)
{

	int tagIndex;
	string filename;
	string upperfilename;
	string headfilename;
	string package, file, extension;
	CModelInstance *torso = nil;
	CModelInstance *head = nil;
	CModelInstance *weapon = nil;
	CModelInstance *barrel = nil;
	CModelInstance *flash = nil;
 	CModelInstance *model = this;
 	string playerName = playerNameFromPath(name);

 	// load skins after attaching to supermodel 
 	// they share textures which are all stored in the supermodel's cache
 	 	
 	
  	// attach barrel
	decomposeEntryName(name,  package, file, extension);
  	tagIndex = model->_modelClass->tagIndexWithName("tag_barrel");
  	if (tagIndex >= 0) {
  		string name = package + getLODPrefix(file) + "_barrel" + getLODPostfix(file) + "." +  extension;
 		barrel = model->attachModel("tag_barrel",  pak, name.c_str()); 
 		if (!barrel)
 			return;
		model = barrel;
  	}

// this makes detaching weapons difficult...
#if 0
  	// attach flash
	decomposeEntryName(name,  package, file, extension);
  	tagIndex = model->_modelClass->tagIndexWithName("tag_flash");
  	if (tagIndex >= 0) {
  		string name = package + getLODPrefix(file) + "_flash" + getLODPostfix(file) + "." +  extension;
 		flash = model->attachModel("tag_flash",  pak, name.c_str()); 
 		if (!flash)
 			return;
		model = flash;
  	}
#endif

  	// attach torso
	decomposeEntryName(name,  package, file, extension);
  	tagIndex = model->_modelClass->tagIndexWithName("tag_torso");
  	if (tagIndex >= 0 && stringStartsWith(lowerString(file), string("lower"))) {
  		string name = package + "upper" + getLODPostfix(file) + "." +  extension;
 		torso = model->attachModel("tag_torso",  pak, name.c_str()); 
 		if (!torso)
 			return;
		model = torso;
  	}

	// attach head
	decomposeEntryName(name,  package, file, extension);
  	tagIndex = model->_modelClass->tagIndexWithName("tag_head");
  	if (tagIndex >= 0 && stringStartsWith(lowerString(file), "upper")) {  			
		string name = package + "head" + getLODPostfix(file) + ".md3"; // hack alert! heads are always .md3
 		head = model->attachModel("tag_head", pak, name.c_str()); 
 		if (!head)
 			return;
 		model = head;
 	}  	
}

CModelInstance *CModelInstance::attachModel(const char *tagName, CFileArchive *pak, const char *name)
{
#pragma unused (pak)
	CModelInstance *child = resources()->modelInstanceWithClassName(name);
	if (child)
		this->addLinkedModel(child, tagName);
	
	return child;
}

void CModelInstance::detachModelAtTag(string tagName)
{
	for (int j=0; j< _tagNum ; j++) {
		CModelInstance *child = _links[j];
		string tag = _modelClass->tagNameForFrameAtIndex(_currentFrame, j);
		
		if (tag == tagName) {
			if (child) 
				delete child;
			_links[j] = 0;
		}
	}
}

string CModelInstance::selectedModelTag(renderingAttributes_t *rendering, CModelInstance *&model)
{
	string tagname = "";
	CMd3 *md3 = dynamic_cast<CMd3 *> (_modelClass);

	// search self
	for (int j=0; j< _tagNum ; j++) {
		CModelInstance *child = _links[j];
		md3_tag_t *tag = md3->tagForFrameAtIndex(_currentFrame, j);
		
		if(rendering->_pickedName == tag->info.glName + glName()) {
			model = this;
			return tag->name;
		}
	}
	
	// search attached children
	for (int j=0; j< _tagNum ; j++) {
		CModelInstance *child = _links[j];
		model = this;
		if (child) {
			tagname = child->selectedModelTag(rendering, model);
			if (tagname.length() > 0)
				return tagname;
		}
	}
exit:	
	return tagname;
}

#pragma mark -

void CModelInstance::exportFormatToFile(ExportFormatT format, FSSpec spec)
{
	SInt32 length;
	
	LFileStream *file = new LFileStream(spec);
	try {
		file->CreateNewDataFile('????', 'TEXT', smSystemScript);
	} catch (...) {
	}
	
	file->OpenDataFork(fsWrPerm);
	
	switch (format) {
	
		case WAVEFRONT_OBJ_FORMAT:
			this->objExportVertexData(file, identity);
			int vertexCount, meshCount;
			vertexCount = meshCount = 0;
			this->objExportElementData(file, vertexCount, meshCount);
			break;
			
		case AUTOCAD_DXF_FORMAT:
		    char *header = "0\r\nSECTION\r\n2\r\nENTITIES\r\n";
		    length = strlen(header); file->PutBytes(header,length);
			this->dxfExport(file, identity);
			char *footer = "0\r\nENDSEC\r\n0\r\nEOF";
		    length = strlen(footer); file->PutBytes(footer,length);
			break;
	}

	file->CloseDataFork();
}



void CModelInstance::dxfExport(LFileStream *file, Mat4 inTransform)
{

	for (int i = 0; i < _meshNum; ++i) {
		_modelClass->dxfExportMesh(this, file, inTransform, i);
	}
				
	// linked models
	for (int j=0; j< _tagNum ; j++) {
		CModelInstance *child = _links[j];
		
		if (child) {
			md3_tag_t *tag = _modelClass->tagForFrameAtIndex(_currentFrame, j);
			if (tag) {
				Mat4 transform = inTransform * inTransform.translate(tag->Position) * Mat4::matrixFromQuat(tag->Rotation);
				child->dxfExport(file, transform);
			}
		}
	}
} 

void CModelInstance::objExportElementData(LFileStream *file, int &vertexCount, int &meshCount)
{

	for (int i = 0; i < _meshNum; ++i) {
		_modelClass->objExportMeshElementData(file, i, vertexCount, meshCount);
	}
		
	// linked models
	for (int j=0; j< _tagNum ; j++) {
		CModelInstance *child = _links[j];
		
		if (child) {
			child->objExportElementData(file, vertexCount, meshCount);
		}
	}
} 


void CModelInstance::objExportVertexData(LFileStream *file, Mat4 inTransform)
{

	for (int i = 0; i < _meshNum; ++i) {
		_modelClass->objExportMeshVertexData(this, file, inTransform, i);
	}
		
	// linked models
	for (int j=0; j< _tagNum ; j++) {
		CModelInstance *child = _links[j];
		
		if (child) {
			md3_tag_t *tag = _modelClass->tagForFrameAtIndex(_currentFrame, j);
			if (tag) {
				Mat4 transform = inTransform * inTransform.translate(tag->Position) * Mat4::matrixFromQuat(tag->Rotation);
				child->objExportVertexData(file, transform);
			}
		}
	}
} 

