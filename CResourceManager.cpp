/* 
	CResourceManager.cpp

	Author:			Tom Naughton
	Description:	<describe the CResourceManager class here>
*/


#include <glut.h>

#include <map>
#include <vector>
#include <string>
#include <list>

using std::string;
using std::map;
using std::vector;
using std::list;

#include "CPakRatApp.h"
#include "CPreferences.h"
#include "CResourceManager.h"
#include "CTextures.h"
#include "CGLImage.h"
#include "CModelInstance.h"
#include "C3DPane.h"
#include "CMdr.h"
#include "CMds.h"
#include "CMdc.h"
#include "CMd3.h"
#include "CMd2.h"
#include "CHLMdl.h"
#include "CMd3AnimationInfo.h"
#include "CWav.h"
#include "CShaderParser.h"
#include "utilities.h"


CResourceManager::resourceManagerSet CResourceManager::_resourceManagerSet;


CResourceManager::CResourceManager(C3DPane *inPane, CFileArchive *inPak)
{
	_pak = inPak;
	_pane = inPane;
	r_gamma = 1.7;
	r_brightness = 30;
	r_shaderrefs = 0;
	r_lodbias = 0;  // lowers image quality if > 0 (needs to be 2 or some textures won't display)
	r_lightmaptex = 0;
	
	_needShaderParse = false;
	_animation = new canimation_map_t;
	_sound = new csound_map_t;
	_shaders = new cshader_map_t;
	_textures = new cglimage_map_t;
	_modelClasses = new c3dmodel_map_t;
	_selectionShader = nil;
	_selectedShader = nil;
	_resourceManagerSet.insert(this);
	dprintf("CResourceManager\n");
}

CResourceManager::~CResourceManager()
{

	dprintf("~CResourceManager\n");
	// throw away all the model classes
	if (_modelClasses) {
		for (c3dmodel_iterator e3 = _modelClasses->begin(); e3 != _modelClasses->end(); e3++) {
			if (e3->second) delete e3->second;
		}	
		delete _modelClasses;
	}

	// throw away all the sounds
	if (_sound) {
		for (csound_iterator e1 = _sound->begin(); e1 != _sound->end(); e1++) {
			if (e1->second) { 
				//dprintf("deleting sound: %s\n", e1->first.c_str());
				delete e1->second;
			}
		}	
		delete _sound;
	}
	
	// throw away all the shaders
	if (_shaders) {
		for (cshader_iterator e1 = _shaders->begin(); e1 != _shaders->end(); e1++) {
			if (e1->second) { 
				//dprintf("deleting shader: %s\n", e1->first.c_str());
				delete e1->second;
			}
		}	
		delete _shaders;
	}

	// throw away all the textures
	if (_textures) {
		for (cglimage_iterator e2 = _textures->begin(); e2 != _textures->end(); e2++) {
			if (e2->second) {
				//dprintf("deleting texture: %s\n", e2->first.c_str());
				delete e2->second;
			}
		}	
		delete _textures;;
	}
	
	
	if (r_shaderrefs)
    	CMemoryTracker::safeFree(r_shaderrefs);
    	
    if (r_lightmaptex) {
	    glDeleteTextures(r_numlightmaptex, (unsigned long*)r_lightmaptex);
	    CMemoryTracker::safeFree(r_lightmaptex);
   	}
	_resourceManagerSet.erase(this);
}

OSErr CResourceManager::initShaders(UInt32 inNumShaders, UInt32 inExtraShaders, shaderref_t *shaderData)
{
	OSErr myErr = noErr;
	
    r_numshaders = inNumShaders;
    r_extrashaders = inExtraShaders;
    r_numtextures = 0;

	dprintf("CResouceManager initShaders\n");
    // Make additional room for shader refs to be added later 
    r_shaderrefs = (shaderref_t*)CMemoryTracker::safeAlloc((r_numshaders + r_extrashaders), sizeof(shaderref_t), "r_shaderrefs");
    if (!r_shaderrefs) goto fail;
    
    if (shaderData) {
    	memcpy(r_shaderrefs, shaderData, (r_numshaders + r_extrashaders) * sizeof(shaderref_t));
	    for (int i=0; i<r_numshaders; i++) 
	      	r_shaderrefs[i].shader.shader = shaderWithName(r_shaderrefs[i].name, true, true);
    }
    r_addshaderstart = r_numshaders;
   
fail:
	return myErr;
}


OSErr CResourceManager::initLightMaps(UInt8 *lightmapdata, long lightmapsize)
{
 	OSErr err = noErr;
    int i, texsize = (128*128*3);
    
    r_numlightmaptex = lightmapsize / texsize;
    r_lightmaptex = (UInt32*) CMemoryTracker::safeAlloc(r_numlightmaptex, sizeof(UInt32), "r_lightmaptex");
    if (!r_lightmaptex) goto fail;
    
    for (i=0; i < r_numlightmaptex; ++i)
    {
	if (r_gamma != 1.0 || r_brightness != 0)
	{
	    int j, val;
	    UInt8 *c;

		r_lightmaptex[i] = CGLImage::GenTexture();
	    c = &lightmapdata[i * texsize];
	    for (j=0; j < texsize; j++, c++)
	    {
		val = (*c + r_brightness) * r_gamma;
		if (val > 255) val = 255;
		*c = val;
	    }
	}
	
	glBindTexture(GL_TEXTURE_2D, r_lightmaptex[i]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_RGB,
		     GL_UNSIGNED_BYTE, &lightmapdata[i * texsize]);
    }

fail:
	return err;
}

#pragma mark -

CShader *CResourceManager::shaderAtIndex(UInt32 inIndex) 
{
	if (inIndex > r_numshaders)
		return nil;

	return r_shaderrefs[inIndex].shader.shader;
}

CShader	*CResourceManager::shaderWithName(const char *inName, Boolean add, Boolean lightmap)	
{
	
	// create shader used for selection	
	if (!_selectionShader) {
		string s;
		CPakStream *result = nil;
		
		s += "{ \n";
		s += "	{ \n";
		
		// choose a texture
		result = _pak->itemWithPathNameSearchAll("textures/effects/quadmap2.jpg");
		if (result) {
			s += "		map textures/effects/quadmap2.jpg \n";
		} else {
			result = _pak->itemWithPathNameSearchAll("textures/effects/quadmap.jpg");
			if (result) {
				s += "		map textures/effects/quadmap.jpg \n";
			}
		}
		if (result) 
			delete result;
		
		s += "		blendFunc add \n";
		s += "      tcmod rotate 360 \n";
		s += "		tcmod scroll 6 0 \n";
		s += "		rgbGen identity \n";
		s += "	} \n";
		s += "} \n";

		CShaderParser parser = CShaderParser();
		_selectionShader = parser.shaderWithString(this, (char*)s.c_str());
		_selectionShader->_glName = 0; // so it can't be picked
	}
	
	// get rid of the file extension
	string name = stripExtension(lowerString(fixSlashes(inName))); 
	// remove leading '/'
	if (name[0] == '/') 
		name = name.substr(1, name.length());	

	cshader_iterator e = _shaders->find(name);
	if(e == _shaders->end()) {
		if (add) {
			_needShaderParse = true;
			CShader *shader = new CShader(this, name.c_str(), lightmap ? SHADER_BSP : SHADER_MD3);
			(*_shaders)[name] = shader;
			return (*_shaders)[name];
		}  
		return nil;
	}
	return e->second;
}

void CResourceManager::parseShaders()
{
	if (!_needShaderParse)
		return;
		
	_needShaderParse = false;
	OSErr err = noErr;
	CPakStream *shaderFile = nil;
	char *shaderbuf = nil;
    printf("Parsing Shaders\n");
    
    // textures need to be loaded into the current GL context
    _pane->FocusDraw();

    // Get a list of shader script files 
    StringList *shaders = new StringList();
	_pak->appendPathNamesOfTypeSearchAll(shaders, "/scripts/", "shader");
	
	// iterate through them
	StringList_iterator e1 = shaders->begin();
	while (e1 != shaders->end()) {
		string s = "/scripts/";
		s += *e1;
		
		shaderFile = _pak->itemWithPathName(s.c_str());
		if (shaderFile) {

			// load in and setup buffer for parsing
			UInt32 len = shaderFile->getSize();
		   	shaderbuf = (char*)shaderFile->getData("shaderbuf");
		    if (!shaderbuf) 
		    	goto fail;
			
			// parse the shader
			dprintf("...parsing '%s'\n", shaderFile->pathName().c_str());
			CShaderParser parser;
			parser.parse(this, shaderFile->pathName().c_str(), shaderbuf, len);
    		CMemoryTracker::safeFree(shaderbuf);
    		shaderbuf = nil;
			delete shaderFile;
			shaderFile = nil;
		}
		e1++;
	}	
	
fail:
	if (shaders)
		delete shaders;
	if (shaderFile)
		delete  shaderFile;
	if (shaderbuf)
    	CMemoryTracker::safeFree(shaderbuf);
}


void CResourceManager::updateShaders(const char *s, UInt32 length)
{
	// check all archives
	resourceManagerSet_iterator e = _resourceManagerSet.begin();
	while (e != _resourceManagerSet.end()) {
	
		// textures need to be loaded into the right GL context
    	(*e)->_pane->FocusDraw();
		CShaderParser parser;
		parser.parse(*e, nil, s,length);
	 	e++;
	}

}

#pragma mark -

CMd3AnimationInfo *CResourceManager::animationInfoWithName(const char *inName, Boolean add)	
{
	string animationFileName = lowerString(fixSlashes(inName)); 
	canimation_iterator e = _animation->find(animationFileName);
	if (e == _animation->end()) {
		if (add) {
			CPakStream *animationFile = nil;
			CMd3AnimationInfo *info;
			animationFile = pak()->itemWithPathName(animationFileName.c_str());
			if (animationFile) {
				info = new CMd3AnimationInfo(this, animationFile);
				delete animationFile;
				(*_animation)[animationFileName] = info;
				return (*_animation)[animationFileName];
			} else {
				dprintf("could not load %s\n", animationFileName.c_str());	
			}
		}  
		return nil;
	}
	return e->second;
}

void CResourceManager::updateAnimationInfo(const char *name, const char *s, UInt32 length)
{
	// check all resource managers
	resourceManagerSet_iterator e = _resourceManagerSet.begin();
	while (e != _resourceManagerSet.end()) {
		CMd3AnimationInfo *info = (*e)->animationInfoWithName(name, false);
		if (info) 
			info->parseAnimationInfo(*e, name, (char*)s, length);
		e++;
	}
}

#pragma mark -

CGLImage *CResourceManager::textureWithName(const char *inName, UInt32 flags, Boolean add)	
{
	string name = lowerString(fixSlashes(inName)); 
	string flaggedname = name + "_" + integerString(flags); // different flags require unique textures: clampmap
	cglimage_iterator e = _textures->find(flaggedname);
	if (e == _textures->end()) {
		if (add) {
			CGLImage *image =  CTextures::loadTexture(_pak, name);
			if (image) 
				image->uploadGLImage(r_lodbias, flags);
			(*_textures)[flaggedname] = image;
			return image;
		}  
		return nil;
	}
	return e->second;
}

CWav *CResourceManager::soundWithName(const char *inName, Boolean add)	
{
	string name = lowerString(fixSlashes(inName)); 
	csound_iterator e = _sound->find(name);
	if (e == _sound->end()) {
		if (add) {
		
			CPakStream *soundData = nil;
			CWav *wav = nil;
			try {
				soundData = _pak->itemWithPathName(name.c_str());
				if (soundData) {
					wav = new CWav(soundData);
					delete soundData;
					(*_sound)[name] = wav;
					return (*_sound)[name];
				}
			} catch (...) {
				// no sound, too bad.
				if (wav) 
					delete wav;
				if (soundData) 
					delete soundData;
			}
		}  
		return nil;
	}
	return e->second;
}

C3DModel *CResourceManager::modelClassWithName(const char *inName,  Boolean add)
{
	C3DModel *modelClass = nil;
	string name = lowerString(fixSlashes(inName)); 
	c3dmodel_iterator e = _modelClasses->find(name);
	if(e == _modelClasses->end()) {
		if (add) {
			string path, file, extension;
			decomposeEntryName(name, path, file, extension);
			CPakStream *pakItem = nil;
			pakItem = _pak->itemWithPathName(name.c_str());
			dprintf("modelClassWithName %s \n", inName);
			
			if (pakItem) {
			
				if (extension == "mdr") {
					modelClass = new CMdr(this);	
				} else if (extension == "mds") {
					modelClass = new CMds(this);	
				} else if (extension == "mdc") {
					modelClass = new CMdc(this);	
				} else if (extension == "md3") {
					modelClass = new CMd3(this);	
				} else if (extension == "md2") {
					modelClass = new CMd2(this);	
				} else if (extension == "mdl") {
					modelClass = new CHLMdl(this);	
				//	modelClass = new CMdl(this);	
				}
				
				if (modelClass && modelClass->init(pakItem)) {
					(*_modelClasses)[name] = modelClass;
				} else {
					delete modelClass;
					modelClass = nil;
				}
				delete pakItem;
			}
		} 
		
		dprintf(modelClass ? "loaded.\n":"failed to load.\n");
		return modelClass;
	}
	return e->second;
}
	
CModelInstance *CResourceManager::modelInstanceWithClassName(const char *name)	
{
	CModelInstance *instance = nil;	
	
	C3DModel *modelClass = modelClassWithName(name);
	if (modelClass) {
	
		instance = new CModelInstance(this);
		if (instance->init(modelClass) == noErr) {

			Boolean assemble = gApplication->preferences()->booleanForKey("assembleModels");
			if (assemble)
				instance->autoAssemble(_pak, name);
			modelClass->loadMeshTextures(instance);
			instance->applySkinNamed(_pak, packageNameFromPath(name).c_str(), "default");

		} else {
			delete instance;
			instance = nil;
		}
	}
	return instance;
}

