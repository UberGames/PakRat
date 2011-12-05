/* 
	CMd3Controller.cpp

	Author:			Tom Naughton
	Description:	<describe the CMd3Controller class here>
*/



#include <glut.h>

#include "CMd3Controller.h"
#include "AppConstants.h"
#include "CPakRatApp.h"
#include "CResourceManager.h"
#include "CPreferences.h"
#include "CMd3.h"
#include "CMd3AnimationInfo.h"
#include "CWav.h"

CMd3Controller::CMd3Controller(CModelInstance *inModel) : CModelController(inModel)
{
	_animating = true;
	_currentSound = nil;
	_theTime = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0;
	_nextWeapon = nil;
	_animationInfo = _model->modelClass()->animationInfo();
	memset(&_upperAnimation, 0, sizeof(CMd3Animation));
	memset(&_lowerAnimation, 0, sizeof(CMd3Animation));


	if (_animationInfo) {
		UInt16 id;
		id = _animationInfo->animationIDWithName("Stand With Weapon");
		BeginSequence(upper_model,_animationInfo->animationWithID(id));
		id = _animationInfo->animationIDWithName("Stand Idle");
		BeginSequence(lower_model,_animationInfo->animationWithID(id));
	}
	
	// setup initial frame
	Animate();
}


CMd3Controller::~CMd3Controller()
{
}

void CMd3Controller::appendAnimationNames(StringList *result)
{
	if (_animationInfo) {
		vector<CMd3Animation*>::iterator e = _animationInfo->_animations->begin();
		
		while (e != _animationInfo->_animations->end()) {
			 result->push_back((*e)->_name);
		 	e++;
		}
	}
}

int CMd3Controller::animationCount()
{
	if (_animationInfo) {
		return _animationInfo->size();
	} else {
		return 0;
	}
}



void CMd3Controller::Reset()
{
	CMd3AnimationInfo 	*_oldInfo = _animationInfo;
	_animationInfo = _model->modelClass()->animationInfo();
	
	// bring newly attached models into sync
	for(int i = 0; i < 2; i++)
		if (_animationInfo && _animating) {
			AnimateModel(upper_model, _upperAnimation);
			AnimateModel(lower_model, _lowerAnimation);
		}
}

const char *CMd3Controller::DisplayString()
{
	static string s;
	
	s = "";
	if (strlen(_upperAnimation._name)) {
		s += _upperAnimation._name;
		s += " ";
		s += integerString(_animationQueue[upper_model].currentFrame);
	}
	if (strlen(_lowerAnimation._name)) {
		if (s.length())
			s += " / ";
		s += _lowerAnimation._name;
		s += " ";
		s += integerString(_animationQueue[lower_model].currentFrame);
	}
	return s.c_str();
}

void CMd3Controller::PlayAnimationSound(CMd3Animation &newAnimation)
{
	if (!gApplication->preferences()->booleanForKey("playSounds"))
		return;

	// handle sounds
	if (gApplication->preferences()->booleanForKey("playSounds")) {
		if (_currentSound == newAnimation._sound && _loopSound) {
			// don't restart looping sounds
		} else {
			// stop looping sounds, let others finish
			if (_currentSound && _loopSound)
				_currentSound->stop();
			_currentSound = newAnimation._sound;
			_loopSound = newAnimation._loopSound;
			if (_currentSound) {
			
				// looping doesn't sound very good and its annoying
				// to I took this out, we still need to handle looping
				// animations that play sounds though.
				#if LOOPSOUNDS
					if (_loopSound)
						_currentSound->loop(true);
				#endif
				_currentSound->play();
			}
		}
	}
}

void CMd3Controller::BeginSequence(ModelType modelType, CMd3Animation newAnimation)
{
	_theTime = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0;
	CAnimationQueue *animationQueue = &_animationQueue[modelType];	
	PlayAnimationSound(newAnimation);
		
	// begin upper
	if (modelType == upper_model) {
		if (newAnimation._type == animation_TORSO 
			|| newAnimation._type == animation_BOTH
			|| newAnimation._type == animation_ALL) {
			
			_upperAnimation = newAnimation;
			if (animationQueue->queueLength > 1)
				animationQueue->queueLength = 1;
			animationQueue->startTime = _theTime;
			animationQueue->PushSequence(_upperAnimation._first, _upperAnimation._num);
		}
	}
	
	// begin lower
	if (modelType == lower_model) {
		if (newAnimation._type == animation_LEGS 
			|| newAnimation._type == animation_BOTH
			|| newAnimation._type == animation_ALL) {
			
			_lowerAnimation = newAnimation;
			if (animationQueue->queueLength > 1)
				animationQueue->queueLength = 1;
			animationQueue->startTime = _theTime;
			animationQueue->PushSequence(_lowerAnimation._first, _lowerAnimation._num);			
		}
	}
	
}



void CMd3Controller::NextFrame(ModelType modelType, CMd3Animation &animation)
{
	CAnimationQueue *animationQueue = &_animationQueue[modelType];
	float elapsedTime = _theTime - animationQueue->startTime;
	float frame = elapsedTime * animation._fps;
	SInt16 intFrame = (SInt16)frame;
	
	// haven't been here in a while, just continue from where we left off
	if (intFrame > ANIMATION_MAX_LOST_FRAMES) {
		frame = intFrame = 0;
		animationQueue->startTime = _theTime;
	}
	
	int loopCount = 0;
	while (animationQueue->queueLength < intFrame + 2 && loopCount < 50) {

		if (!gApplication->preferences()->booleanForKey("repeatAnimations") 
			&& gApplication->preferences()->booleanForKey("useNextSequence") 
			&& _animationInfo->hasAnimationNamed(animation._nextSequence)) {
			
			// next sequence
			UInt16 id = _animationInfo->animationIDWithName(animation._nextSequence);
			animation = _animationInfo->animationWithID(id);
			PlayAnimationSound(animation);
			animationQueue->PushSequence(animation._first, animation._num);
			
			// change weapon
			if(_nextWeapon && (strcmp(animation._name, "Raise Weapon") == 0)) {
				CModelInstance *torso = _model->modelAtTag("tag_torso");
				if (torso) {
					torso->detachModelAtTag("tag_weapon");
					torso->addLinkedModel(_nextWeapon, "tag_weapon");
					_nextWeapon = nil;
				}
			}
			
		} else if (gApplication->preferences()->booleanForKey("repeatAnimations")) {
		
			// repeat
			animation = _animationInfo->animationWithID(animation._id);
			PlayAnimationSound(animation);
			animationQueue->PushSequence(animation._first, animation._num);
			
		} else if (animation._looping == 0) {
		
			// loop all
			animation = _animationInfo->animationWithID(animation._id);
			animationQueue->PushSequence(animation._first, animation._num);
			
		} else if (animation._looping > 0) {
		
			// loop some
			animation = _animationInfo->animationWithID(animation._id);
			animationQueue->PushSequence(animation._first + animation._num - animation._looping, animation._looping);
				
		} else {
		
			// idle
			int lastFrame = animation._first + animation._num - 1;
			animationQueue->PushSequence(lastFrame > 0 ? lastFrame : 0, 1);
		}
		loopCount++;
	}
	
	// skip to current frame
	if (intFrame) {
		animationQueue->PopFrames(intFrame);
		animationQueue->startTime = _theTime;
	}
		
	animationQueue->currentFrame = animationQueue->PeekFrame(0); 
	animationQueue->nextFrame =  animationQueue->PeekFrame(1); 
	animationQueue->interpolation = frame - intFrame;
	
	if (!gApplication->preferences()->booleanForKey("useInterpolation"))
		animationQueue->interpolation = 0.0;
		
	if (intFrame != 0 || animationQueue->interpolation != 0.0)
		_needsRedraw = true;
		
	_model->setFrameForModelType(
		animationQueue->currentFrame, 
		animationQueue->nextFrame, 
		animationQueue->interpolation, 
		modelType);
}

void CMd3Controller::AnimateModel(ModelType modelType, CMd3Animation &animation)
{
	NextFrame(modelType, animation);
	if (!gApplication->preferences()->booleanForKey("useInterpolation")) {
		_animationQueue[modelType].interpolation = 0.0f;
	}
}

Boolean CMd3Controller::Animate()
{
	_theTime = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0;
	if (_animationInfo && _animating) {
		AnimateModel(upper_model, _upperAnimation);
		AnimateModel(lower_model, _lowerAnimation);
	}
	return _needsRedraw;
}

void CMd3Controller::StartSequenceAtIndex(int index)
{
	if(_animationInfo && index < (*_animationInfo).size()) {
		CMd3Animation newAnimation = _animationInfo->animationWithID(index);
		_animating = true;
		BeginSequence(upper_model, newAnimation);
		BeginSequence(lower_model, newAnimation);
	}
}

// ---------------------------------------------------------------------------
//	¥ HandleKeyPress												  [public]
// ---------------------------------------------------------------------------
//	Tab switches the Target to the next item in the TabGroup.
//	Shift-Tab to the previous item.
//	All other keystrokes (and Tabs with modifiers other than Shift)
//		get passed up.

Boolean CMd3Controller::HandleKeyPress(const EventRecord	&inKeyEvent)
{
	CPreferences *prefs = gApplication->preferences();
	string command = prefs->commandForKeyEvent(inKeyEvent);
	_needsRedraw = true;

	if (_animationInfo) {
		if (_animationInfo->_ravenFormat) {
		
			// alphabetic key?
			int key  = animationIndexForKeyEvent(inKeyEvent);
			if (key > 0) {
				StartSequenceAtIndex(key);
				return true;
			}
		} else {
		
			// animation we've heard of?
			if (_animationInfo->hasAnimationNamed(command)) {
				UInt16 id = _animationInfo->animationIDWithName(command);
				dprintf("start sequence %s\n", command.c_str());
				CMd3Animation newAnimation = _animationInfo->animationWithID(id);
				_animating = true;
				BeginSequence(upper_model, newAnimation);
				BeginSequence(lower_model, newAnimation);
				return true;
			}
		}
	}


	// animation commands
	if (command == "toggleAnimation") {
		_animating = !_animating;
		return true;
	} else if (command == "incrementLower") {
		_animating = false;
		_animationQueue[lower_model].currentFrame++;
		_model->setFrameForModelType(_animationQueue[lower_model].currentFrame, 
		_animationQueue[lower_model].currentFrame, 0.0, lower_model);
		return true;
	} else if (command == "decrementLower") {
		_animating = false;
		_animationQueue[lower_model].currentFrame--;
		_model->setFrameForModelType(_animationQueue[lower_model].currentFrame, 
		_animationQueue[lower_model].currentFrame, 0.0, lower_model);
		return true;
	} else if (command == "incrementUpper") {
		_animating = false;
		_animationQueue[upper_model].currentFrame++;
		_model->setFrameForModelType(_animationQueue[upper_model].currentFrame, 
		_animationQueue[upper_model].currentFrame, 0.0, upper_model);
		return true;
	} else if (command == "decrementUpper") {
		_animating = false;
		_animationQueue[upper_model].currentFrame--;
		_model->setFrameForModelType(_animationQueue[upper_model].currentFrame, 
		_animationQueue[upper_model].currentFrame, 0.0, upper_model);
		return true;
	} else if (command == "weapon 1") {
		if (!SwitchToWeapon("models/weapons2/gauntlet/gauntlet.md3"))
			SwitchToWeapon("models/weapons2/phaser/phaser_w.md3");
		return true;
	} else if (command == "weapon 2") {
		if (!SwitchToWeapon("models/weapons2/machinegun/machinegun.md3"))
			SwitchToWeapon("models/weapons2/prifle/prifle_w.md3");
		return true;
	} else if (command == "weapon 3") {
		if (!SwitchToWeapon("models/weapons2/shotgun/shotgun.md3"))
			SwitchToWeapon("models/weapons2/imod/imod2_w.md3");
		return true;
	} else if (command == "weapon 4") {
		if (!SwitchToWeapon("models/weapons2/grenadel/grenadel.md3"))
			SwitchToWeapon("models/weapons2/scavenger/scavenger_w.md3");
		return true;
	} else if (command == "weapon 5") {
		if (!SwitchToWeapon("models/weapons2/rocketl/rocketl.md3"))
			SwitchToWeapon("models/weapons2/tpd/tpd_w.md3");
		return true;
	} else if (command == "weapon 6") {
		if (!SwitchToWeapon("models/weapons2/lightning/lightning.md3"))
			SwitchToWeapon("models/weapons2/tricorder/tricorder.md3");
		return true;
	} else if (command == "weapon 7") {
		if (!SwitchToWeapon("models/weapons2/railgun/railgun.md3"))
			SwitchToWeapon("models/weapons2/tricorder/padd.md3");
		return true;
	} else if (command == "weapon 8") {
		if (!SwitchToWeapon("models/weapons2/plasma/plasma.md3"))
			SwitchToWeapon("models/weapons2/tricorder/hand_tool.md3");
		return true;
	} else if (command == "weapon 9") {
		if (!SwitchToWeapon("models/weapons2/bfg/bfg.md3"))
			SwitchToWeapon("models/weapons2/tricorder/klingon_knife.md3");
		return true;
	} else if (command == "weapon 10") {
		if (!SwitchToWeapon("models/weapons2/grapple/grapple.md3"))
			;
		return true;
	}
			
	return CModelController::HandleKeyPress(inKeyEvent);
}

CModelInstance *CMd3Controller::SwitchToWeapon(const char *weapon)
{
	// load weapon
	_nextWeapon = _model->resources()->modelInstanceWithClassName(weapon);
	_model->resources()->parseShaders();

	// pick it up on next "Raise Weapon"
	if (_animationInfo) {
		UInt16 id = _animationInfo->animationIDWithName("Drop Weapon");
		CMd3Animation newAnimation = _animationInfo->animationWithID(id);
		BeginSequence(upper_model, newAnimation);
	}
	
	return _nextWeapon;
}