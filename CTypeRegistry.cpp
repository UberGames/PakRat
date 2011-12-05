/* 
	CTypeRegistry.cpp

	Author:			Tom Naughton
	Description:	<describe the CTypeRegistry class here>
*/

#include <stdio.h>
#include <QuickTimeComponents.h>
#include "CTypeRegistry.h"
#include "OutlineTableConstants.h"

char *extension_map[] =
{

	// images
	"pcx", (char *) pcx_type, 
	"bmp", (char *) bmp_type, 
	"tga", (char *) tga_type, 
	"jpg", "jpeg", (char *) jpg_type, 
	"wal", (char *) wal_type, 
	"ftx", (char *) ftx_type, 
	"gif", (char *) gif_type, 
	"psd", (char *) psd_type, 
	"png", (char *) png_type, 

	// models
//	"mdl", (char *) mdl_type, 
	"mdl", (char *) hl_mdl_type,  // half life mdl
	"md2", (char *) md2_type, 
	"md3", (char *) md3_type, 
	"mdr", "md4", (char *) mdr_type, 
	"mds",  (char *) mds_type, 
	"mdc",  (char *) mdc_type, 
	
	// maps
	"bsp", (char *) bsp_type, 
	
	// sound
	"wav", (char *) wav_type, 
	"mp3", (char *) mp3_type, 
	
	// scripts
	"shader", "low", (char *) shader_type, 
	"skin", (char *) skin_type, 
	"animation.cfg", (char *) animation_type, 
	
	// text
	"config", "cfg", "pre", "txt", "obj", "dxf", 
	"java", "h", "c", "cp", "cpp", "st", "tik", "skl", (char *) text_type, 
	
	// sentinel
	0, 
	
};


type_attribute_pair CTypeRegistry::type_registry[] =
{
	{ none_type,		icon_Document,	kNoneType, },

	// images
	{ pcx_type,			icon_TARGA,		0,						kImageType, },
	{ bmp_type, 		icon_TARGA, 	kQTFileTypeBMP, 		kImageType, },
	{ tga_type, 		icon_TARGA, 	kQTFileTypeTargaImage, 	kImageType, },
	{ jpg_type, 		icon_JPEG, 		kQTFileTypeJPEG,		kImageType, },
	{ wal_type, 		icon_TARGA, 	0,						kImageType, },
	{ ftx_type, 		icon_TARGA, 	0,						kImageType, },
	{ gif_type, 		icon_TARGA, 	kQTFileTypeGIF,			kImageType, },
	{ psd_type, 		icon_TARGA, 	kQTFileTypePhotoShop,	kImageType, },
	{ png_type, 		icon_TARGA, 	kQTFileTypePNG,			kImageType, },
	

	// models
	{ mdl_type, 		icon_Model, 	0,						kModelType, },
	{ hl_mdl_type, 		icon_Model, 	0,						kModelType, },
	{ md2_type,	 		icon_Model, 	0,						kModelType, },
	{ md3_type, 		icon_Model, 	0,						kModelType, },
	{ mdr_type, 		icon_Model, 	0,						kModelType, },
	{ mdc_type, 		icon_Model, 	0,						kModelType, },
	{ mds_type, 		icon_Model, 	0,						kModelType, },
	
	// sounds
	{ wav_type, 		icon_Sound, 	kQTFileTypeWave,		kSoundType, },
	{ mp3_type, 		icon_Sound, 	0,						kSoundType, },
	{ midi_type, 		icon_Sound, 	0,						kSoundType, },
	

	// maps
	{ bsp_type, 		icon_Map, 		0,						kMapType, },
	
	// shaders
	{ shader_type, 		icon_Shader,	0,						kScriptType | kTextType | kShaderType, },
	
	// scripts
	{ skin_type, 		icon_Skin, 		0,						kScriptType | kTextType, },
	{ animation_type, 	icon_Text, 		0,						kScriptType | kTextType, },
	
	// text
	{ text_type, 		icon_Text, 		0,						kTextType, },
	
	
	
};



CTypeRegistry::CTypeRegistry() 
{
	int index = 0;
	int count = 0;
	
	_resource_type_map = new resource_type_map_t;
	while(extension_map[index]) {
		
		if (extension_map[index] < (char*)last_type) {
		
			// found a type, register its extensions
			while(count) {
				resource_type_t rtype = (resource_type_t) extension_map[index];
				char *name = extension_map[index-count];
				(*_resource_type_map)[name] = rtype;
				//dprintf("setting type: %s -> %d\n", name, rtype);
				count--;
			}
		} else {
			count++;
		}	
		index++;
	}
	debugDump();
}

CTypeRegistry::~CTypeRegistry()
{
}

resource_type_t CTypeRegistry::extensionToType(const char *extension) 
{ 
	resource_type_t result = (*_resource_type_map)[extension];
//	dprintf("extensionToType: %s -> %d\n", extension, result);
	return result; 
};


void CTypeRegistry::debugDump() 
{ 
	// write out all the types
	resource_type_iterator e1 = _resource_type_map->begin();
	while (e1 != _resource_type_map->end()) {
		dprintf("type: %s -> %d\n", e1->first.c_str(), e1->second);
		e1++;
	}	
};

