/* 
	CResourceManager.h

	Author:			Tom Naughton
	Description:	<describe the CResourceManager class here>
*/


#ifndef CResourceManager_H
#define CResourceManager_H

#include <map>
#include <vector>
#include <string>
#include <set>

#include "CFileArchive.h"

using std::map;
using std::string;
using std::vector;
using std::set;


class CGLImage;
class C3DModel;
class C3DPane;
class CModelInstance;
class CShader;
class CMd3AnimationInfo;
class CWav;

/* Special texture loading requirements */
enum
{
    TEXFILE_NOMIPMAPS  = 1 << 0,
    TEXFILE_CLAMP      = 1 << 1
};

// Gathers texture file names prior to texture loading 
typedef struct
{
    UInt32 flags;
    char fname[256];
    CGLImage *image;
} texfile_t;

// Shader references (indexed from faces) 
typedef struct
{
    char name[64];
    union {
    	struct {
    		int unknown[2];
    	} unknown;
    	struct {
    		CShader *shader;
    	} shader;
    };
} shaderref_t;

class CResourceManager
{

public:

	CResourceManager(C3DPane *inPane, CFileArchive *inPak);
	virtual ~CResourceManager();
	
	OSErr 				initShaders(UInt32 inNumShaders, UInt32 inExtraShaders, shaderref_t *shaderData);
	OSErr 				initLightMaps(UInt8 *lightmapdata, long r_lightmapsize);

	CFileArchive 		*pak() { return _pak; };
	CGLImage			*textureWithName(const char *name, UInt32 flags = 0, Boolean add = true);	
	C3DModel			*modelClassWithName(const char *name, Boolean add = true);	
	CModelInstance		*modelInstanceWithClassName(const char *name);	
	CMd3AnimationInfo	*animationInfoWithName(const char *name, Boolean add = true);	
	static void 		updateAnimationInfo(const char *name, const char *s, UInt32 length);
	CWav				*soundWithName(const char *name, Boolean add = true);	
	
	CShader				*shaderWithName(const char *name, Boolean add = true, Boolean lightmap = false);	
	void 				parseShaders();
	static void 		updateShaders(const char *s, UInt32 length);
	CShader 			*shaderAtIndex(UInt32 inIndex);
	CShader				*selectionShader() { return _selectionShader; };
	CShader				*selectedShader() { return _selectedShader; };
	void				setSelectedShader(CShader *s) {  _selectedShader = s; };
	
	long r_numshaders;
	long r_nummodels;
	long r_extrashaders;
	int r_lodbias;
	long r_addshaderstart;
	shaderref_t *r_shaderrefs;

	UInt32 *r_textures;
	int r_numtextures;
	UInt32 *r_lightmaptex;
	int r_numlightmaptex;	
	float r_gamma;
	long r_brightness;

private:

	typedef pair<string, CWav*> csound_pair_t;
	typedef map<string, CWav*> csound_map_t;
	typedef csound_map_t::value_type csound_value_t;
	typedef csound_map_t::iterator csound_iterator;

	typedef pair<string, CMd3AnimationInfo*> canimation_pair_t;
	typedef map<string, CMd3AnimationInfo*> canimation_map_t;
	typedef canimation_map_t::value_type canimation_value_t;
	typedef canimation_map_t::iterator canimation_iterator;

	typedef pair<string, CShader*> cshader_pair_t;
	typedef map<string, CShader*> cshader_map_t;
	typedef cshader_map_t::value_type cshader_value_t;
	typedef cshader_map_t::iterator cshader_iterator;

	typedef pair<string, CGLImage*> cglimage_pair_t;
	typedef map<string, CGLImage*> cglimage_map_t;
	typedef cglimage_map_t::value_type cglimage_value_t;
	typedef cglimage_map_t::iterator cglimage_iterator;

	typedef pair<string, C3DModel*> c3dmodel_pair_t;
	typedef map<string, C3DModel*> c3dmodel_map_t;
	typedef c3dmodel_map_t::value_type c3dmodel_value_t;
	typedef c3dmodel_map_t::iterator c3dmodel_iterator;

	typedef set<CResourceManager*> resourceManagerSet;
	typedef resourceManagerSet::iterator resourceManagerSet_iterator;

	static resourceManagerSet _resourceManagerSet;
	
	Boolean					_needShaderParse;
	csound_map_t 			*_sound;
	canimation_map_t 		*_animation;
	cshader_map_t 			*_shaders;
	cglimage_map_t 			*_textures;
	c3dmodel_map_t 			*_modelClasses;

	CFileArchive 			*_pak;
	C3DPane 				*_pane;
	CShader 				*_selectionShader;
	CShader 				*_selectedShader;
};

#endif	// CResourceManager_H
