/* 
	CMd3Controller.h

	Author:			Tom Naughton
	Description:	<describe the CMd3Controller class here>
*/

#ifndef CMd3Controller_H
#define CMd3Controller_H

#include "CMd3.h"
#include "CModelController.h"
#include "CAnimationQueue.h"
#include "CMd3AnimationInfo.h"


class CMd3Controller : public CModelController
{

public:

	CMd3Controller(CModelInstance *inModel);
	virtual ~CMd3Controller();
	
	virtual void 		Reset();
	virtual Boolean		Animate();
	virtual Boolean		HandleKeyPress(const EventRecord&	inKeyEvent);
	virtual void		appendAnimationNames(StringList *result);
	virtual void		StartSequenceAtIndex(int index);
	virtual const char 	*DisplayString();
	virtual int			animationCount();
	
protected:

	void			BeginSequence(ModelType modelType, CMd3Animation newAnimation);
	CModelInstance	*SwitchToWeapon(const char *weapon);
	void 			AnimateModel(ModelType modelType, CMd3Animation &animation);
	void 			NextFrame(ModelType modelType, CMd3Animation &animation);
	void 			PlayAnimationSound(CMd3Animation &newAnimation);

	CMd3AnimationInfo 	*_animationInfo;
	
	// there should really be one sound per model
	// to handle footsteps, but I don't handle footsteps
	CWav				*_currentSound;
	Boolean				_loopSound;
	float				_theTime;
	CMd3Animation		_upperAnimation;
	CMd3Animation		_lowerAnimation;
	CModelInstance 		*_nextWeapon;
	CAnimationQueue		_animationQueue[other_model];
	
};

#endif	// CMd3Controller_H
