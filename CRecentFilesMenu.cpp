/* 
	CRecentFilesMenu.cpp

	Author:			Tom Naughton
	Description:	<describe the CRecentFilesMenu class here>
*/

#include <LCommander.h>
#include <PP_Messages.h>
#include <LDocApplication.h>
#include <stdio.h>

#include "CPreferences.h"
#include "CRecentFilesMenu.h"
#include "AppConstants.h"



// ---------------------------------------------------------------------------------
//		¥ CRecentFilesMenu
// ---------------------------------------------------------------------------------

CRecentFilesMenu::CRecentFilesMenu(CPreferences *prefs)
	: _baseItems( 0 ), _preferences(prefs)
{
}


// ---------------------------------------------------------------------------------
//		¥ CRecentFilesMenu
// ---------------------------------------------------------------------------------

CRecentFilesMenu::CRecentFilesMenu(CPreferences *prefs,
	ResIDT	inMenuID )
		: LMenu( inMenuID ), _baseItems( 0 ), _preferences(prefs)
{
}


// ---------------------------------------------------------------------------------
//		¥ CRecentFilesMenu
// ---------------------------------------------------------------------------------

CRecentFilesMenu::CRecentFilesMenu(CPreferences *prefs,
	SInt16	inMenuID,
	Str255	inTitle )
		: LMenu( inMenuID, inTitle ), _baseItems( 0 ), _preferences(prefs)
{
}


// ---------------------------------------------------------------------------------
//		¥ ~CRecentFilesMenu
// ---------------------------------------------------------------------------------

CRecentFilesMenu::~CRecentFilesMenu()
{
}


// ---------------------------------------------------------------------------------
//		¥ addRecentFile
// ---------------------------------------------------------------------------------

void CRecentFilesMenu::addRecentFile(string path)
{
	_preferences->addRecentFile(path);
	populate();

}


void CRecentFilesMenu::populate()
{
	// add recent files from preferences
	CPreferences::recent_type *files = _preferences->recentFiles();
	CPreferences::recent_type_iterator e = files->begin();
	int index = 1;
		 
	// the menu never gets smaller, just add items as needed
	while (e != files->end()) {
		if(::CountMenuItems( GetMacMenuH()) < index)
			InsertCommand( "\p ", cmd_UseMenuItem, 16000 );
		string path = *e;
		//dprintf("addRecentFile %s\n", path.c_str());
		LStr255 theTitle = path.c_str();
		::SetMenuItemText( GetMacMenuH(), index, theTitle );
		e++; index++;
	}
}



// ---------------------------------------------------------------------------------
//		¥ MenuItemToWindow
// ---------------------------------------------------------------------------------

string
CRecentFilesMenu::MenuItemToPath(
	SInt16	inMenuItem )
{
	return _preferences->recentFileAtIndex( inMenuItem); // - _baseItems );
}



// =================================================================================
//	CRecentFilesMenuAttachment
// =================================================================================

// ---------------------------------------------------------------------------------
//		¥ CRecentFilesMenuAttachment
// ---------------------------------------------------------------------------------

CRecentFilesMenuAttachment::CRecentFilesMenuAttachment(
	CRecentFilesMenu *inWindowMenu, LCommander *inCommander )
		: LAttachment( msg_AnyMessage, true ), _recentFilesMenu( inWindowMenu ), _commander(inCommander)
{
}


// ---------------------------------------------------------------------------------
//		¥ ~CRecentFilesMenuAttachment
// ---------------------------------------------------------------------------------

CRecentFilesMenuAttachment::~CRecentFilesMenuAttachment()
{
}


// ---------------------------------------------------------------------------------
//		¥ ExecuteSelf
// ---------------------------------------------------------------------------------

void
CRecentFilesMenuAttachment::ExecuteSelf(
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
			theMenuItem ) && theMenuID == _recentFilesMenu->GetMenuID() ) {
			
			// All window menu items should be enabled and use a mark.
			mExecuteHost = false;
			*theStatus->enabled = true;
			*theStatus->usesMark = false;
			*theStatus->mark = noMark;
		}

	} else {

		// Handle commands similar to ObeyCommand even
		// though this class isn't an LCommander.
		ResIDT	theMenuID;
		SInt16	theMenuItem;

		if ( LCommander::IsSyntheticCommand( inMessage, theMenuID,
			theMenuItem ) && theMenuID == _recentFilesMenu->GetMenuID() ) {
			
			mExecuteHost = false;
			string path = _recentFilesMenu->MenuItemToPath( theMenuItem );
			if ( path.length() ) {
				_commander->ObeyCommand(cmd_openDocumentPath, (void*)path.c_str());
			}
	
		}
	
	}
}


