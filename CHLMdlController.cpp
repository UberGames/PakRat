/* 
	CHLMdlController.cpp

	Author:			
	Description:	<describe the CHLMdlController class here>
*/

#include "CHLMdlController.h"



CHLMdlController::CHLMdlController(CModelInstance *inModel) : CModelController(inModel)
{
	_model = inModel;
	_animating = true;
	init();
}


CHLMdlController::~CHLMdlController()
{
}

Boolean CHLMdlController::init()
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

void CHLMdlController::Reset()
{
}

Boolean CHLMdlController::Animate()
{
	return false;
}

const char 	*CHLMdlController::DisplayString()
{
	return nil;
}

void CHLMdlController::StartSequenceAtIndex(int index)
{
#pragma unused (index)
}

void CHLMdlController::appendAnimationNames(StringList *result)
{
#pragma unused (result)
}

int	CHLMdlController::animationCount()
{
	return 0;
}

void CHLMdlController::NextFrame(Sequence *sequence)
{
#pragma unused (sequence)
}

void CHLMdlController::BeginSequence(Sequence *sequence)
{
#pragma unused (sequence)
}


