// =================================================================================
//	CAnimationMenu.cp					©1995-1998  Metrowerks Inc. All rights reserved.
// =================================================================================
//	CAnimationMenu.h	

#include <LCommander.h>
#include <LWindow.h>
#include <PP_Messages.h>
#include <UDesktop.h>

#include "CAnimationMenu.h"
#include "CModelController.h"

// ---------------------------------------------------------------------------------
//		¥ CAnimationMenu
// ---------------------------------------------------------------------------------

CAnimationMenu::CAnimationMenu()
	: mBaseItems( 0 )
{
	_controller  = nil;
}


// ---------------------------------------------------------------------------------
//		¥ CAnimationMenu
// ---------------------------------------------------------------------------------

CAnimationMenu::CAnimationMenu(
	ResIDT	inMenuID )
		: LMenu( inMenuID ), mBaseItems( 0 )
{
	_controller  = nil;
}


// ---------------------------------------------------------------------------------
//		¥ CAnimationMenu
// ---------------------------------------------------------------------------------

CAnimationMenu::CAnimationMenu(
	SInt16	inMenuID,
	Str255	inTitle )
		: LMenu( inMenuID, inTitle ), mBaseItems( 0 )
{
	_controller  = nil;
}


// ---------------------------------------------------------------------------------
//		¥ ~CAnimationMenu
// ---------------------------------------------------------------------------------

CAnimationMenu::~CAnimationMenu()
{
}

void CAnimationMenu::RemoveAll()
{
	_controller  = nil;
	while(::CountMenuItems( GetMacMenuH() )) {
		RemoveItem( ::CountMenuItems( GetMacMenuH() ));
	}
}

void CAnimationMenu::SetItems(StringList *names, CModelController *inController)
{
	int count = 0;

	RemoveAll();
	_controller = inController;
	StringList_iterator e = names->begin();
	while (e != names->end()) {
		Str255 theTitle;
		
		InsertCommand( "\p ", cmd_UseMenuItem, 21000 );
		c2pstrcpy((unsigned char*)theTitle, (const char*)e->c_str());
		count++;
		::SetMenuItemText( GetMacMenuH(), count, theTitle );
		e++;
	}
	ShowCommands();
}

void CAnimationMenu::ShowCommands()
{
	char *keys = "qwertyuiopasdfghjklzxcvbnm";
	int numKeys = strlen(keys);
	UInt8 inModifiers;	

	SInt16	menuLength = ::CountMenuItems(mMacMenuH);
	for(int count = 0; count < menuLength; count++) {
		char theCommand = keys[count % numKeys];
		
		inModifiers = kMenuNoCommandModifier;
		if ((count % (26*2)) >= 26)
			inModifiers |= kMenuShiftModifier;
		if ((count % (26*4)) >= 26*2)
			inModifiers |= kMenuControlModifier;
		if ((count % (26*8)) >= 26*4)
			inModifiers |= kMenuOptionModifier ;
			
		if ((count-1) < 26*8) {
		
		// This doesn't work, it causes lettered menu items to respond 
		// as if the command key is held down. Unfortunate.
		
			::SetItemCmd( GetMacMenuH(), count+1, theCommand); 
			::SetMenuItemModifiers (GetMacMenuH(), count+1, inModifiers);		
		}
	
	}
}

void CAnimationMenu::HideCommands()
{
	SInt16	menuLength = ::CountMenuItems(mMacMenuH);
	for(int count = 0; count < menuLength; count++) {
		::SetItemCmd( GetMacMenuH(), count+1, 0); 
		::SetMenuItemModifiers (GetMacMenuH(), count+1, kMenuNoCommandModifier);		
	}
}

// =================================================================================
//	CAnimationMenuAttachment
// =================================================================================

// ---------------------------------------------------------------------------------
//		¥ CAnimationMenuAttachment
// ---------------------------------------------------------------------------------

CAnimationMenuAttachment::CAnimationMenuAttachment(
	CAnimationMenu	*inWindowMenu )
		: LAttachment( msg_AnyMessage, true ), mAnimationMenu( inWindowMenu )
{
}


// ---------------------------------------------------------------------------------
//		¥ ~CAnimationMenuAttachment
// ---------------------------------------------------------------------------------

CAnimationMenuAttachment::~CAnimationMenuAttachment()
{
}


// ---------------------------------------------------------------------------------
//		¥ ExecuteSelf
// ---------------------------------------------------------------------------------

void
CAnimationMenuAttachment::ExecuteSelf(
	MessageT	inMessage,
	void		*ioParam )
{
	// Turn on host execution by default.
	mExecuteHost = true;

	if ( inMessage == msg_CommandStatus ) {

		// Handle commander status just like in FindCommandStatus
		// even though this class isn't an LCommander.
		ResIDT			theMenuID;
		SInt16			theMenuItem;
		SCommandStatus	*theStatus = static_cast<SCommandStatus *> (ioParam);

		if ( LCommander::IsSyntheticCommand( theStatus->command, theMenuID,
			theMenuItem ) && theMenuID == mAnimationMenu->GetMenuID() ) {
			
			// Since the item corresponds to an entry in the window menu
			// we'll handle it's status here. Note: there may be other
			// items at the top of window menu that are the responsibility
			// of some commander.
			mExecuteHost = false;

			*theStatus->enabled = false;
			*theStatus->usesMark = false;
			*theStatus->mark = noMark;

			if (mAnimationMenu->_controller && mAnimationMenu->_controller->animationCount()) 
				*theStatus->enabled = true;
		}


	} else {

		// Handle commands similar to ObeyCommand even
		// though this class isn't an LCommander.
		ResIDT	theMenuID;
		SInt16	theMenuItem;

		if ( LCommander::IsSyntheticCommand( inMessage, theMenuID,
			theMenuItem ) && theMenuID == mAnimationMenu->GetMenuID() ) {

			if (mAnimationMenu->_controller) {
				mAnimationMenu->_controller->StartSequenceAtIndex(theMenuItem-1);
			}
	
		}
	
	}
}
