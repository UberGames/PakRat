/* 
	CMd3AnimationInfo.h

	Author:			Tom Naughton
	Description:	<describe the CMd3AnimationInfo class here>
*/

#ifndef CMd3AnimationInfo_H
#define CMd3AnimationInfo_H

#include <map>
#include <vector>
#include <string>
#include <string.h>

#include "CPakStream.h"
#include "CMd3.h"

using std::string;
using std::map;
using std::vector;

class CWav;

// animation

typedef enum animation_type { 
	animation_LEGS = 0, 
	animation_TORSO, 
	animation_BOTH, 
	animation_ALL 
} animation_part_type;

#define ANIMATION_NAME_LENGTH 64

class CMd3Animation
{
public:

	SInt16 			_id;
	SInt16 			_first;
	SInt16			_num;
	SInt16			_looping;
	SInt16			_fps;
	char 			_name[ANIMATION_NAME_LENGTH];                                                                                                           		
	char 			_nextSequence[ANIMATION_NAME_LENGTH];                                                                                                           		
	CWav 			*_sound;                                                                                                           		
	Boolean 		_loopSound;                                                                                                           		
	animation_part_type 	_type;
};

class CMd3AnimationInfo
{

public:

	CMd3AnimationInfo(CResourceManager *resources, CPakStream *animationFile);
	virtual ~CMd3AnimationInfo();
	
	string 					_sex;
	mdl_vertex_t 			_headoffset;
	string 					_footsteps;
	string 					_soundpath;
	vector<CMd3Animation*>	*_animations;
	map<string, UInt16> 	*_nameToIndexMap;
	Boolean					_ravenFormat;
	
	void 			parseAnimationInfo(CResourceManager *resources, const char *pathname, char *animationdata, long size);
	void			parseExtendedAnimationInfo(CResourceManager *resources, const char *pathname);
	Boolean			hasAnimationNamed(string name);
	CMd3Animation	animationWithID(UInt16 id) { return *((*_animations)[id]); }
	UInt16			size() { return _animations->size(); }
	
	// this will add unknown names to the map and assign them to 0. 
	// Use hasAnimationNamed first. Thanks STL.
	UInt16			animationIDWithName(string name) { return (*_nameToIndexMap)[name]; }
	
};


#endif	// CMd3AnimationInfo_H
