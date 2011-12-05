/* 
	CTypeRegistry.h

	Author:			Tom Naughton
	Description:	<describe the CTypeRegistry class here>
*/

#ifndef CTypeRegistry_H
#define CTypeRegistry_H

#include <iostream.h>
#include <map>
#include <vector>
#include <string>
#include <set>
#include "AppConstants.h"

using std::map;
using std::string;
using std::vector;
using std::set;
using std::pair;

typedef enum {

	none_type = 0,
	
	// images
	pcx_type,
	bmp_type,
	tga_type,
	jpg_type,
	wal_type,
	ftx_type,
	psd_type,
	gif_type,
	png_type,
	
	// models
	mdl_type,
	hl_mdl_type,// half life mdl
	md2_type, 
	md3_type,
	mdr_type, 
	mds_type, 
	mdc_type, 
	
	// sounds
	wav_type,
	mp3_type,
	midi_type,
	
	// maps
	bsp_type,
	
	// scripts
	shader_type,
	skin_type,
	animation_type,
	
	text_type,
	
	last_type
	
} resource_type_t;




typedef struct {
	resource_type_t 	resourcetype;
	ResIDT				iconID;
	OSType				qtcomponent;
	int	 				attributes;
} type_attribute_pair;

class CTypeRegistry
{

	enum {
		kNoneType 		= 0,
		kModelType		= (1 << 0),
		kMapType		= (1 << 1),
		kScriptType		= (1 << 2),
		kImageType		= (1 << 3),
		kTextType		= (1 << 4),
		kSoundType		= (1 << 5),
		kShaderType		= (1 << 6)
	} type_attributes_t;

public:

	CTypeRegistry();
	virtual ~CTypeRegistry();
	
	resource_type_t 	extensionToType(const char *extension);
	
	
	OSType				QTComponentType(resource_type_t aType)  { return type_registry[aType].qtcomponent; }; 
	
	ResIDT				iconID(resource_type_t aType) { return type_registry[aType].iconID; }; 
	Boolean				isTextType(resource_type_t aType) { return type_registry[aType].attributes & kTextType; }; 
	Boolean				isScriptType(resource_type_t aType) { return type_registry[aType].attributes & kScriptType; };
	Boolean				isModelType(resource_type_t aType) { return type_registry[aType].attributes & kModelType; };
	Boolean				isMapType(resource_type_t aType) { return type_registry[aType].attributes & kMapType; };
	Boolean				isImageType(resource_type_t aType) { return type_registry[aType].attributes & kImageType; };
	Boolean				isSoundType(resource_type_t aType) { return type_registry[aType].attributes & kSoundType; }; 
	Boolean				isShaderType(resource_type_t aType) { return type_registry[aType].attributes & kShaderType; };

	void				debugDump();
private:

	typedef pair<string, resource_type_t> resource_type_pair_t;
	typedef map<string, resource_type_t> resource_type_map_t;
	typedef resource_type_map_t::value_type resource_type_value_t;
	typedef resource_type_map_t::iterator resource_type_iterator;

	resource_type_map_t 			*_resource_type_map;
	static type_attribute_pair 		type_registry[];
};

#endif	// CTypeRegistry_H
