// =================================================================================
//	CWindowMenu.h					©1995-1998  Metrowerks Inc. All rights reserved.
// =================================================================================
//	CWindowMenu.cp	

#pragma once

#include <LMenu.h>
#include <LArray.h>
#include <PP_Types.h>

class LWindow;

class CWindowMenu : public LMenu {
public:
					CWindowMenu();
					CWindowMenu( ResIDT inMenuID );
					CWindowMenu( SInt16 inMenuID, Str255 inTitle );
					~CWindowMenu();
	
	void			InsertWindow( const LWindow *inWindow );
	void			RemoveWindow( const LWindow *inWindow );

	LWindow *		MenuItemToWindow( SInt16 inMenuItem );
	SInt16			WindowToMenuItem( const LWindow *inWindow );
	void			SetCommandKeys();

protected:
	SInt16			mBaseItems;
	LArray			mWindowList;
};


// =================================================================================
//	CWindowMenuAttachment
// =================================================================================

#include <LAttachment.h>

class CWindowMenuAttachment : public LAttachment {
public:
					CWindowMenuAttachment( CWindowMenu *inWindowMenu );
					~CWindowMenuAttachment();
	
protected:
	CWindowMenu *	mWindowMenu;

	void			ExecuteSelf( MessageT inMessage, void *ioParam );
};
