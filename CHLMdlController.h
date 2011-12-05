/* 
	CHLMdlController.h

	Author:			
	Description:	<describe the CHLMdlController class here>
*/

#ifndef CHLMdlController_H
#define CHLMdlController_H

#include "CModelController.h"


class CHLMdlController: public CModelController
{
public:
	CHLMdlController(CModelInstance *inModel);
	virtual ~CHLMdlController();
	
	virtual void 		Reset();
	virtual Boolean 	init();
	virtual Boolean		Animate();
	virtual const char 	*DisplayString();
	virtual void		StartSequenceAtIndex(int index);
	virtual void		appendAnimationNames(StringList *result);
	virtual int			animationCount();

protected:

	void				NextFrame(Sequence *sequence);
	void 				BeginSequence(Sequence *sequence);

	CModelInstance 		*_model;
	Boolean				_animating;
};

#endif	// CHLMdlController_H
