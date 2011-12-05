// ===========================================================================
//	CPakRatApp.h		©1994-1999 Metrowerks Inc. All rights reserved.
// ===========================================================================

#ifndef _H_CDocumentApp
#define _H_CDocumentApp
#pragma once

#include <ios.h>
#include <iostream.h>
#include <map>
#include <string>
#include <LDocApplication.h>
#include <LMenuBar.h>

class CFileArchive;
class CPakStream;
class LDebugMenuAttachment;
class CPreferences;
class CWav;
class CAnimationMenu;
class CWindowMenu;
class CRecentFilesMenu;
class CTypeRegistry;

using std::string;
using std::map;

class CPakRatApp : public LDocApplication, public LPeriodical {

public:
							CPakRatApp();
	virtual					~CPakRatApp();

	virtual Boolean			ObeyCommand(
								CommandT			inCommand,
								void*				ioParam = nil);	

	Boolean 				HandleKeyPress(const EventRecord &inKeyEvent);
	
	virtual void			FindCommandStatus(
								CommandT			inCommand,
								Boolean&			outEnabled,
								Boolean&			outUsesMark,
								UInt16&				outMark,
								Str255				outName);

	virtual void			OpenDocument(
								FSSpec*				inMacFSSpec);
									
	virtual LModelObject*	MakeNewDocument();
	virtual void			ChooseDocument();

	virtual void			PrintDocument(
								FSSpec*				inMacFSSpec);

	void 					OpenEditorForItem(CFileArchive *pak, const char *path, const char *itemname = "", int modifers = 0);
	void 					OpenControls();
	void 					MemoryIsLow();
	Boolean 				openDocumentWithPath(const char *path);
	Boolean 				ApplySkin(CFileArchive *pak, const char *path);
	Boolean 				updateSkin(const char  *path, char *skindata, UInt32 size);

	#if debS
		LDebugMenuAttachment	*mDebugAttachment;
	#endif
	
	CPakStream 				*itemWithPathName(CFileArchive *pak, const char  *path);
	CWav 					*soundWithPathName(CFileArchive *pak, const char *path);
	void					SpendTime(const EventRecord	&inMacEvent);
	CPreferences			*preferences() { return _preferences; }
	
	double 					realtime;

protected:
	virtual void		PutOnDuty(
								LCommander*			inNewTarget);
								
	virtual void		TakeOffDuty();

	virtual void			MakeMenuBar();	
	virtual void			Initialize();
	virtual void			StartUp();
	virtual	void			DoReopenApp();
	void 					FindQuake(); 
	
			void			RegisterClasses();

	CPreferences			*_preferences;
	Boolean					_showMemoryWarning;
//	CFileArchive			*_quakePak;
};

class	CPakRatMenuBar : LMenuBar {
public:
						CPakRatMenuBar(
								ResIDT				inMBARid);

	virtual CommandT	FindKeyCommand(
								const EventRecord&	inKeyEvent,
								SInt32&				outMenuChoice) const;
};

#define PALETTE_SIZE	256

// gloabls
extern CPakRatApp			*gApplication;
extern CTypeRegistry		*gRegistry;
extern CWindowMenu			*gWindowMenu;	
extern CRecentFilesMenu 	*gRecentFilesMenu;	
extern CAnimationMenu		*gAnimationMenu;
extern Byte hexenPalette[PALETTE_SIZE*3];
extern Byte quakePalette[PALETTE_SIZE*3];
extern Byte quake2Palette[PALETTE_SIZE*3];

#endif // _H_CDocumentApp