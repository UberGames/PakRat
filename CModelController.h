/* 
	CModelController.h

	Author:			Tom Naughton
	Description:	<describe the CModelController class here>
*/

#ifndef CModelController_H
#define CModelController_H

#include "CModelInstance.h"
#include "CAnimationQueue.h"

class Sequence 
{
public:
	string name;
	UInt32 startIndex;
	UInt32 endIndex;
};

class CModelController 
{

public:

	CModelController(CModelInstance *inModel);
	virtual ~CModelController();
	
	virtual void 		Reset();
	virtual Boolean 	init();
	virtual Boolean		Animate();
	virtual void 		Draw(renderingAttributes_t *renderingAttributes);
	virtual Boolean		HandleKeyPress(const EventRecord&	inKeyEvent);
	virtual int			animationIndexForKeyEvent(const EventRecord	&inKeyEvent);
	virtual const char 	*DisplayString();
	virtual void		StartSequenceAtIndex(int index);
	virtual void		appendAnimationNames(StringList *result);
	virtual int			animationCount();
	
	
protected:

	void				NextFrame(Sequence *sequence);
	void 				BeginSequence(Sequence *sequence);

	CModelInstance 		*_model;
	Boolean				_animating;
	Boolean				_needsRedraw;
	Sequence* 			_animationSequence;
	CAnimationQueue		_animationQueue;
	float				_theTime;

	
	vector<Sequence*> 	*_animations;
	
};

#endif	// CModelController_H
