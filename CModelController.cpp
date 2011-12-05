/* 
	CModelController.cpp

	Author:			Tom Naughton
	Description:	<describe the CModelController class here>
*/


#include <glut.h>

#include "CModelController.h"
#include "AppConstants.h"
#include "CPakRatApp.h"
#include "CPreferences.h"
#include "utilities.h"

CModelController::CModelController(CModelInstance *inModel)
{
	_model = inModel;
	_animating = true;
	_animations = new vector<Sequence*>;
	_animationSequence = nil;
	init();
}

CModelController::~CModelController()
{
	if (_animations)
		delete _animations;
}

void CModelController::Reset()
{
}

Boolean CModelController::init()
{
	// find all the animation sequences
	UInt32 frames = _model->modelClass()->frameCount();
	Sequence *currSequence = nil;
	
	
	for (int i = 0; i < frames; i++) {
		string name = _model->modelClass()->frameName(i);
		
		// index can be one or two digits
		int digits = 1;
		if (name[name.length()-2] >= '0' && name[name.length()-2] <= '9')
			digits = 2;
		string sequence = name.substr(0,name.length()-digits );
		string index = name.substr(name.length()-digits ,name.length());
		
		// continue or end
		if (currSequence != nil) {
			if (sequence != currSequence->name) {
				_animations->insert(_animations->end(), currSequence);
				currSequence = nil;
			} else {
				currSequence->endIndex++;
			}
		}
		
		// start new
		if (currSequence == nil) {
			currSequence = new Sequence();
			currSequence->name = sequence;
			currSequence->startIndex = i;
			currSequence->endIndex = i;
		}
	}
	
	// make sure the last one gets in the vector
	if (currSequence != nil) {
		_animations->insert(_animations->end(), currSequence);
	}

	for(int i = 0; i < _animations->size(); i++) {
		currSequence = (*_animations)[i];
		dprintf("%s %d %d\n", currSequence->name.c_str(), currSequence->startIndex, currSequence->endIndex);
	}
	
	StartSequenceAtIndex(0);
	return true;
}

void CModelController::appendAnimationNames(StringList *result)
{
	if (_animations) {
		vector<Sequence*>::iterator e = _animations->begin();
		
		while (e != _animations->end()) {
			 result->push_back((*e)->name);
		 	e++;
		}
	}
}

int CModelController::animationCount()
{
	return _animations->size();
}


void CModelController::Draw(renderingAttributes_t *renderingAttributes)
{
	if (renderingAttributes->_wireframe) 
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); 
	else 
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        
	if (renderingAttributes->_lighting)
		glEnable( GL_LIGHTING );
	else
		glDisable( GL_LIGHTING );

    if (renderingAttributes->_textured)	
		glEnable( GL_TEXTURE_2D );
	else
		glDisable( GL_TEXTURE_2D );


	glColor4f(1,1,1,1);

	_model->Draw(renderingAttributes);
	_needsRedraw = false;
	
	// draw links after the blend phase
	if (renderingAttributes->_pickTag && renderingAttributes->_renderBlend) {
		_model->DrawLinks(renderingAttributes);
	}
}

void CModelController::BeginSequence(Sequence *sequence)
{
	_theTime = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0;
	
	if (_model) {
		_animationSequence = sequence;
		if (_animationQueue.queueLength > 1)
			_animationQueue.queueLength = 1;
		_animationQueue.startTime = _theTime;
		_animationQueue.PushSequence(sequence->startIndex, sequence->endIndex - sequence->startIndex + 1);			
	}
}


const char *CModelController::DisplayString()
{	
	static string message;
	message = _model->modelClass()->frameName(_animationQueue.currentFrame);
	return message.c_str();
}


void CModelController::NextFrame(Sequence *sequence)
{
	float elapsedTime = _theTime - _animationQueue.startTime;
	float frame = elapsedTime * 10; // fps
	SInt16 intFrame = (SInt16)frame;
	
	// haven't been here in a while, just continue from where we left off
	if (intFrame > ANIMATION_MAX_LOST_FRAMES) {
		frame = intFrame = 0;
		_animationQueue.startTime = _theTime;
	}

	// loop animation
	int loopCount = 0;
	while (_animationQueue.queueLength < intFrame + 2 && loopCount < 50) {
		_animationQueue.PushSequence(sequence->startIndex, sequence->endIndex - sequence->startIndex + 1);			
		loopCount++;
	}
	
	// skip to current frame
	if (intFrame) {
		_animationQueue.PopFrames(intFrame);
		_animationQueue.startTime = _theTime;
	}
	
		
	_animationQueue.currentFrame = _animationQueue.PeekFrame(0); 
	_animationQueue.nextFrame =  _animationQueue.PeekFrame(1); 
	_animationQueue.interpolation = frame - intFrame;
	
	if (!gApplication->preferences()->booleanForKey("useInterpolation"))
		_animationQueue.interpolation = 0.0;
		
	if (intFrame != 0 || _animationQueue.interpolation != 0.0)
		_needsRedraw = true;
		
	_model->setFrame(
		_animationQueue.currentFrame, 
		_animationQueue.nextFrame, 
		_animationQueue.interpolation);
}

Boolean CModelController::Animate()
{
	_theTime = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0;
	if (_animating && _animationSequence) {
		NextFrame(_animationSequence);
	}
	return _needsRedraw;
}

void CModelController::StartSequenceAtIndex(int index)
{
	if (_animations->size() > 0) {
		Sequence *sequence = (*_animations)[index];
		if (sequence) {
			_animating = true;
			BeginSequence(sequence);
		}
	}
}


// ---------------------------------------------------------------------------
//	¥ HandleKeyPress												  [public]
// ---------------------------------------------------------------------------
//	Tab switches the Target to the next item in the TabGroup.
//	Shift-Tab to the previous item.
//	All other keystrokes (and Tabs with modifiers other than Shift)
//		get passed up.

Boolean CModelController::HandleKeyPress(const EventRecord	&inKeyEvent)
{
	CPreferences *prefs = gApplication->preferences();
	string command = prefs->commandForKeyEvent(inKeyEvent);
	SInt16 currentFrame, nextFrame;
	float interpolation;
	
	_needsRedraw = true;
	int key = animationIndexForKeyEvent(inKeyEvent);
	if (key >= 0 && key < (*_animations).size()) {
		StartSequenceAtIndex(key);
		return true;
	}

	// the key mapped to anything?
	if (command.length() == 0)
		return false;

	if (command == "toggleAnimation") {
		_animating = !_animating;
		return true;
	} else if (command == "toggleInterpolation") {
		Boolean bonebox = prefs->booleanForKey("useInterpolation");
		prefs->setBooleanForKey(!bonebox, "useInterpolation");
		return true;
	} else if (command == "nextSkin") {
		int index = _model->getTextureIndex();
		if (index < _model->modelClass()->textureCount()-1)
			index++;
		else
			index = 0;
		_model->setTextureIndex(index);
		return true;
	} else if (command == "previousSkin") {
		int index = _model->getTextureIndex();
		if (index > 0)
			index--;
		else
			index = _model->modelClass()->textureCount()-1;
		_model->setTextureIndex(index);
		return true;
	}

	// animation commands
	if (command == "incrementLower") {
		_animating = false;
		_model->getFrame(currentFrame, nextFrame, interpolation);
		_model->setFrame(currentFrame + 1, currentFrame, 0.0);
	} else if (command == "decrementLower") {
		_animating = false;
		_model->getFrame(currentFrame, nextFrame, interpolation);
		_model->setFrame(currentFrame - 1, currentFrame, 0.0);
	}	
	return false;
}

int CModelController::animationIndexForKeyEvent(const EventRecord	&inKeyEvent)
{
	// FIXME - non querty keyboards?
	UInt16	keyLookup[] = {
			0xc00, 0xd00, 0xe00, 0xf00, 0x1100, 0x1000, 0x2000, 0x2200, 0x1f00, 0x2300,		// qwertyuiop
			0x0, 0x100, 0x200, 0x300, 0x500, 0x400, 0x2600, 0x2800, 0x2500,					// asdfghjkl
			0x600, 0x700, 0x800, 0x900, 0xb00, 0x2d00,	0x2e00, 							// zxcvbnm
			0xFFFF, // Sentinel 
			};
			
	UInt8	theChar = (UInt16) (inKeyEvent.message & charCodeMask);
	UInt16	theKey = (UInt16) (inKeyEvent.message & keyCodeMask);
	Boolean commandDown = (inKeyEvent.modifiers & cmdKey) != 0;
	Boolean controlDown = (inKeyEvent.modifiers & controlKey) != 0;
	Boolean shiftDown = (inKeyEvent.modifiers & shiftKey) != 0;
	Boolean optionDown = (inKeyEvent.modifiers & optionKey) != 0;
	int index = 0;
	dprintf("theChar 0x%x '%c' theKey 0x%x\n", theChar, theChar, theKey);


	while (keyLookup[index] != 0xFFFF) {
		if (keyLookup[index] == theKey) {
		
			if (shiftDown)
				index += 26;
			if (controlDown)
				index += 26*2;
			if (optionDown)
				index += 26*4;
		//	if (commandDown)
		//		index += 26*7;
				
			return index;
		}
		index++;
	}
	
	return -1;
}