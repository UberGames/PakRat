/* 
	CMd3AnimationInfo.cpp

	Author:			Tom Naughton
	Description:	<describe the CMd3AnimationInfo class here>
*/

#include "CMd3AnimationInfo.h"
#include "CTokenizer.h"
#include "CWav.h"
#include "CPakRatApp.h"
#include "CPreferences.h"
#include "CResourceManager.h"
#include "AppConstants.h"



// CMd3AnimationInfo

CMd3AnimationInfo::CMd3AnimationInfo(CResourceManager *resources, CPakStream *animationFile)
{
	_animations = nil;
	_nameToIndexMap = nil;
	_ravenFormat = false;
	_soundpath = "";
	_footsteps = "";
	
	long size = animationFile->getSize();
	char *animationdata = (char*) animationFile->getData("md3 skinfile");
	if (animationdata )
		parseAnimationInfo(resources, animationFile->pathName().c_str(), animationdata, size);
	CMemoryTracker::safeFree(animationdata);
}

CMd3AnimationInfo::~CMd3AnimationInfo()
{
	if (_animations) {
		vector<CMd3Animation*>::iterator e = _animations->begin();
		while (e != _animations->end()) {
			CMd3Animation *animation = *e;
			if (animation) 
				delete animation;
			e++; 
		}	
		delete _animations;
	}
	if (_nameToIndexMap) 
		delete _nameToIndexMap;
}

void CMd3AnimationInfo::parseAnimationInfo(CResourceManager *resources, const char *pathname, char *animationdata, long size)
{
	string delimiters =  " ,\t";
	char *p = animationdata;
	char *e = p + size;
	int lineno = 0;
	int i = 0;
	
	if (_animations)
		delete _animations;
	if (_nameToIndexMap)
		delete _nameToIndexMap;
			
	_animations = new vector<CMd3Animation*>;
	_nameToIndexMap = new map<string, UInt16>;
		
	// while there are lines
	while (p < e) {
		string line = nextLine(p,e);
		
		// parse a line
		CTokenizer tokenizer(line, delimiters);
		int tokenCount = tokenizer.countTokens();
		string token = tokenizer.nextToken();
		if (token.length() > 0) {
		
			if (lowerString(token) == "sex") {
				_sex = tokenizer.nextToken();
			} else if (lowerString(token) == "soundpath") {
				_soundpath = tokenizer.nextToken();
			} else if (lowerString(token) == "headoffset") {
				_headoffset.x = tokenizer.nextInt();
				_headoffset.y = tokenizer.nextInt();
				_headoffset.z = tokenizer.nextInt();
			} else if (lowerString(token) == "footsteps") {
				_footsteps = tokenizer.nextToken();
			} else {
				CMd3Animation *animation = new CMd3Animation();
				
				// get name from Raven animation.cfg
				if (tokenCount > 4) {
					
					_ravenFormat = true;
					if (token.find("BOTH_") != -1) animation->_type = animation_BOTH;
					else if (token.find("TORSO_") != -1) animation->_type = animation_TORSO;
					else if (token.find("LEGS_") != -1) animation->_type = animation_LEGS;
					
					strncpy(animation->_name, (const char *)token.c_str(), ANIMATION_NAME_LENGTH);
					token = tokenizer.nextToken();
					tokenCount--;
				}
				
				animation->_id = _animations->size();
				animation->_first = tokenizer.integerValueForToken(token);
				animation->_num = tokenizer.nextInt();
				animation->_looping = tokenizer.nextInt();
				animation->_fps = tokenizer.nextInt();				
				animation->_sound = nil;
				
				// FIXME: what does this mean?
				if (animation->_fps < 0)
					animation->_fps *= -1;
				
				_animations->insert(_animations->end(), animation);
				i++;
			}
		}
	}
	if (!_ravenFormat)
		parseExtendedAnimationInfo(resources, pathname);
fail:
	return;
}

void CMd3AnimationInfo::parseExtendedAnimationInfo(CResourceManager *resources, const char *pathname)
{
	// figure out the player name
	string package, file, extension;
	decomposeEntryName(pathname,  package, file, extension);
	string playerName = lastPathComponent(package);
	
//	CFileArchive *pak = animationFile->rootArchive();
	Handle animationHandle = ::GetResource('TEXT', TEXT_Animation);
	if (animationHandle) {
		::HLock(animationHandle);
		long size = GetHandleSize(animationHandle);
		char *p = (char*) *animationHandle;
		char *e = p + size;
		int lineno = 1;
		string delimiters =  " ,\t";
		UInt16 animationIndex = 0;
	
		int torsoOffset = -1;
		int firstTorsoFrame = -1;

		while (p < e) {
			string line = nextLine(p,e);
			//dprintf("%d: %s\n", lineno++, line.c_str());
			
			CTokenizer tokenizer(line, delimiters);
			string token;
			
			token = tokenizer.nextToken();
			if (token.length()) {

				CMd3Animation *animation = (*_animations)[animationIndex];
				strncpy(animation->_name, (const char *)token.c_str(), ANIMATION_NAME_LENGTH);
				(*_nameToIndexMap)[animation->_name] = animationIndex;
				token = tokenizer.nextToken();
				if (token == "animation_BOTH") animation->_type = animation_BOTH;
				else if (token == "animation_TORSO") animation->_type = animation_TORSO;
				else if (token == "animation_LEGS") animation->_type = animation_LEGS;
				token = tokenizer.nextToken();
				strncpy(animation->_nextSequence, (const char *)token.c_str(), ANIMATION_NAME_LENGTH);
				
				// workaround for strange numbering in animation.cfg:
				// skip TORSO frames for LEGS animation lines
				if (animation->_type == animation_TORSO && firstTorsoFrame < 0) 
					firstTorsoFrame = animation->_first;
				if (animation->_type == animation_LEGS && torsoOffset < 0) 
					torsoOffset = animation->_first - firstTorsoFrame;
				if (animation->_type == animation_LEGS)
					animation->_first -= torsoOffset;				
				
				// load the sound
				animation->_sound = nil;
				string soundName = tokenizer.nextToken();
				if (1 || gApplication->preferences()->booleanForKey("playSounds")) {
					SInt16 index = soundName.find("*name*");
					if (index > 0)
						soundName.replace(index, 6, playerName);
					dprintf("sound %s\n", soundName.c_str());
					if (soundName != "none")
						animation->_sound = resources->soundWithName(soundName.c_str());
				}
				animation->_loopSound = (tokenizer.nextToken() == "loop");
				animationIndex++;
			}
		}
		::ReleaseResource(animationHandle);
	}
}

Boolean	 CMd3AnimationInfo::hasAnimationNamed(string name) 
{ 
	//dprintf("CMd3AnimationInfo::hasAnimationNamed %s\n", name.c_str());
	return _nameToIndexMap->find(name) != _nameToIndexMap->end(); 
}

