// =================================================================================
//	CAnimationMenu.h					©1995-1998  Metrowerks Inc. All rights reserved.
// =================================================================================
//	CAnimationMenu.cp	

#pragma once

#include <LMenu.h>
#include <LArray.h>
#include <PP_Types.h>

#include "utilities.h"

class LWindow;
class CModelController;

class CAnimationMenu : public LMenu {
public:
					CAnimationMenu();
					CAnimationMenu( ResIDT inMenuID );
					CAnimationMenu( SInt16 inMenuID, Str255 inTitle );
					~CAnimationMenu();
					
					void ShowCommands();
					void HideCommands();
					void RemoveAll();
					void SetItems(StringList *names, CModelController *inController);
					
					CModelController *_controller;
	

protected:

	SInt16			mBaseItems;
};


// =================================================================================
//	CAnimationMenuAttachment
// =================================================================================

#include <LAttachment.h>

class CAnimationMenuAttachment : public LAttachment {
public:
					CAnimationMenuAttachment( CAnimationMenu *inWindowMenu );
					~CAnimationMenuAttachment();
	
protected:
	CAnimationMenu *	mAnimationMenu;

	void			ExecuteSelf( MessageT inMessage, void *ioParam );
};
