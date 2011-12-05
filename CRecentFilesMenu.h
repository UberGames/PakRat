/* 
	CRecentFilesMenu.h

	Author:			Tom Naughton
	Description:	<describe the CRecentFilesMenu class here>
*/

#ifndef CRecentFilesMenu_H
#define CRecentFilesMenu_H

#include <vector>
#include <map>
#include <string>
#include <iostream.h>
#include <LMenu.h>
#include <LArray.h>
#include <PP_Types.h>

using std::string;
using std::map;
using std::vector;

class LWindow;
class CPreferences;

class CRecentFilesMenu : public LMenu {
public:
					CRecentFilesMenu(CPreferences *prefs);
					CRecentFilesMenu(CPreferences *prefs, ResIDT inMenuID );
					CRecentFilesMenu(CPreferences *prefs, SInt16 inMenuID, Str255 inTitle );
					~CRecentFilesMenu();
	
	void			InsertWindow( const LWindow *inWindow );
	void			RemoveWindow( const LWindow *inWindow );

	string			MenuItemToPath( SInt16 inMenuItem );
	void			addRecentFile(string path);
	void 			populate();

protected:

	CPreferences	*_preferences;
	SInt16			_baseItems;
	LArray			_recentFilesList;
};

// =================================================================================
//	CWindowMenuAttachment
// =================================================================================

#include <LAttachment.h>

class CRecentFilesMenuAttachment : public LAttachment {
public:
					CRecentFilesMenuAttachment( CRecentFilesMenu *inWindowMenu, LCommander *inCommander );
					~CRecentFilesMenuAttachment();
	
protected:
	LCommander 			*_commander;
	CRecentFilesMenu 	*_recentFilesMenu;
	void				ExecuteSelf( MessageT inMessage, void *ioParam );
};

#endif	// CRecentFilesMenu_H
