// ===========================================================================
//	CPakRatApp.cp 		©1994-1999 Metrowerks Inc. All rights reserved.
// ===========================================================================
//	This file contains the starter code for a Document PowerPlant project


#include <PP_Debug.h>
#include <PP_DebugConstants.h>
#include <PP_DebugMacros.h>

#include <LGrowZone.h>
#include <PP_Messages.h>
#include <PP_Resources.h>
#include <UDrawingState.h>
#include <UMemoryMgr.h>
#include <URegistrar.h>
#include <UEnvironment.h>
#include <UAttachments.h>
#include <LDocument.h>
#include <LUndoer.h>

#include <UStandardDialogs.h>
#if PP_StdDialogs_Option == PP_StdDialogs_Conditional
	#include <UConditionalDialogs.h>
#endif

#include <UControlRegistry.h>
#include <LWindow.h>
#include <LPrintout.h>
#include <LPlaceHolder.h>
#include <Appearance.h>
#include <LSIOUXAttachment.h>
#include <LActiveScroller.h>
#include <LMultiPanelView.h>
#include <LPageController.h>
#include <LListBox.h>
#include <UDebugUtils.h>
#include <LDebugMenuAttachment.h>
#include <stdio.h>

#include "CPakRatApp.h"
#include "GLPane.h"
#include "CResourceDocument.h"
#include "CMemoryTable.h"
#include "CPakStream.h"
#include "CViewer.h"
#include "CGWorldView.h"
#include "C3DModelPane.h"
#include "C3DMapPane.h"
#include "C3DShaderPane.h"
#include "CControlPanel.h"
#include "CScrollWheelAttachment.h"
#include "CLiveResizeAttachment.h"
#include "CDirtyText.h"
#include "WTextView.h"
#include "CEditTable.h"
#include "COutlineTable.h"
#include "CPreferences.h"
#include "CWav.h"
#include "CShader.h"
#include "CModelInstance.h"
#include "CWindowMenu.h"
#include "CAnimationMenu.h"
#include "CRecentFilesMenu.h"
#include "AppConstants.h"
#include "utilities.h"
//#include "cDirectoryPopupAttachment.h"
#include "CShaderMenu.h"
#include "CTypeRegistry.h"

extern "C" {
	#include "MoreFiles.h"
	#include "MoreFilesExtras.h"
}

void DoAboutBox(Boolean simpleAbout);

CPakRatApp 			*gApplication;
CWindowMenu			*gWindowMenu;
CAnimationMenu		*gAnimationMenu;
CRecentFilesMenu 	*gRecentFilesMenu;	
CTypeRegistry		*gRegistry;

Byte  hexenPalette[PALETTE_SIZE*3] = 
	{
			#include "hexenpalette.h"		
	};

Byte  quakePalette[PALETTE_SIZE*3] = 
	{
			#include "quakepalette.h"		
	};

Byte  quake2Palette[PALETTE_SIZE*3] = 
	{
			#include "quake2palette.h"		
	};


// ===========================================================================
//	¥ main
// ===========================================================================

int main()
 {	
		// Set Debugging options
	SetDebugThrow_(debugAction_Alert);
	SetDebugSignal_(debugAction_Alert);

		// Initialize Memory Manager. Parameter is the number of
		// master pointer blocks to allocate
	InitializeHeap(3);
	
		// Initialize standard Toolbox managers
#if PP_Target_Carbon
	UQDGlobals::InitializeToolbox();
#else
	UQDGlobals::InitializeToolbox(&qd);
#endif
	#if debS
		//UDebugUtils::CheckEnvrionment();
	#endif	
	
	// outline tables support drag and drop, so it's good to ensure
	// it's present before we go any further
	
	if (!LDropArea::DragAndDropIsPresent()) {
		::SysBeep(3);
		::StopAlert(ALRT_NoDragAndDrop, nil);
		::ExitToShell();
	}
	
	// check for OpenGL
	if (!CFM_AddressIsResolved_(::aglChoosePixelFormat)) {
		::SysBeep(3);
		::StopAlert(ALRT_NoOpenGL, nil);
		::ExitToShell();
	}
	
		// Install a GrowZone to catch low-memory situations	
	new LGrowZone(500000);

		// Create the application object and run
	CPakRatApp	theApp;
	gApplication = &theApp;
	theApp.Run();
	
	return 0;
}


// ---------------------------------------------------------------------------
//	¥ CPakRatApp									[public]
// ---------------------------------------------------------------------------
//	Application object constructor

CPakRatApp::CPakRatApp()
{
	// Register ourselves with the Appearance Manager
	if (UEnvironment::HasFeature(env_HasAppearance)) {
		::RegisterAppearanceClient();
	}

	// Register the Appearance Manager/GA classes
	
	UControlRegistry::RegisterClasses();
	RegisterClasses();
	
		// Preload facilities for the Standard Dialogs
	PP_StandardDialogs::Load();
	
		// Require at least Navigation Services 1.1. See comments
		// above SetTryNavServices in UConditionalDialogs.cp for why
		// you might wish to do this.
#if PP_StdDialogs_Option == PP_StdDialogs_Conditional
	UConditionalDialogs::SetTryNavServices(0x01108000);
#endif

#if debS
	AddAttachment(new LSIOUXAttachment);	// *** Use SIOUX Attachment
#endif

	_showMemoryWarning = false;
	dprintf("startup\n");
	_preferences = new CPreferences();
	
	
}


// ---------------------------------------------------------------------------
//	¥ ~CPakRatApp									[public, virtual]
// ---------------------------------------------------------------------------
//	Application object destructor

CPakRatApp::~CPakRatApp()
{
//	StopIdling();
	R_backend_shutdown( );
		// Clean up after Standard Dialogs
	PP_StandardDialogs::Unload();
}

// ---------------------------------------------------------------------------
//		¥ÊMakeMenuBar
// ---------------------------------------------------------------------------

void CPakRatApp::MakeMenuBar()
{
	CPakRatMenuBar*	theBar = nil;
	theBar = new CPakRatMenuBar(128);
}


// ---------------------------------------------------------------------------
//	¥ Initialize												   [protected]
// ---------------------------------------------------------------------------
//	Last chance to initialize Application before processing events

void CPakRatApp::Initialize()
{
	
	LMenuBar	*theMBar = LMenuBar::GetCurrentMenuBar();
	ThrowIfNil_( theMBar );


	// Install scroll wheel attachment.
	AddAttachment( new CScrollWheelAttachment );

	// Install the Animation menu
	gAnimationMenu = new CAnimationMenu( menu_animation );
	ThrowIfNil_( gAnimationMenu );
	theMBar->InstallMenu( gAnimationMenu, 0 );
	
	// Install the Animation menu attachment.
	CAnimationMenuAttachment *theAnimationAttachment;
	theAnimationAttachment = new CAnimationMenuAttachment( gAnimationMenu );
	AddAttachment( theAnimationAttachment , nil, true );

	// Install the Window menu.
	gWindowMenu = new CWindowMenu( menu_window );
	ThrowIfNil_( gWindowMenu );
	theMBar->InstallMenu( gWindowMenu, 0 );

	// Install the Window menu attachment.
	CWindowMenuAttachment *theAttachment;
	theAttachment = new CWindowMenuAttachment( gWindowMenu );
	AddAttachment( theAttachment , nil, true );
	
	// install the directory popup attachment
//	AddAttachment(new cDirectoryPopupAttachment(kDirectoryPopMenuID));
	
	// Install the recent files menu.
	gRecentFilesMenu = new CRecentFilesMenu( _preferences, menu_recentfiles );
	ThrowIfNil_( gRecentFilesMenu );
	theMBar->InstallMenu( gRecentFilesMenu, hierMenu );

	ResIDT		outMENUid;
	MenuHandle	outMenuHandle;
	SInt16		outItem;

	theMBar->FindMenuItem(
		cmd_recentmenu,
		outMENUid,
		outMenuHandle,
		outItem);
		
	::SetItemCmd(outMenuHandle, outItem, hMenuCmd);
	::SetItemMark(outMenuHandle, outItem, menu_recentfiles);
	gRecentFilesMenu->populate();
	CShader::_animateShaders = _preferences->booleanForKey("animateShaders");
	
	// Install the recent files menu attachment.
	CRecentFilesMenuAttachment *theRecentFilesAttachment;
	theRecentFilesAttachment = new CRecentFilesMenuAttachment( gRecentFilesMenu, this );
	AddAttachment( theRecentFilesAttachment , nil, true );
	
	
	// create the type registry
	gRegistry = new CTypeRegistry();
 	UQuickTime::Initialize();
	OpenControls();
	R_backend_init();
}


// ---------------------------------------------------------------------------
//	¥ StartUp										[protected, virtual]
// ---------------------------------------------------------------------------
//	Perform an action in response to the Open Application AppleEvent.
//	Here, issue the New command to open a window.

void
CPakRatApp::StartUp()
{
	CPreferences *prefs = preferences();
	int recent = intValue(prefs->valueForKey("openRecentFiles"));
	for (int i=0; i < recent; i++)
		openDocumentWithPath(prefs->recentFileAtIndex(recent).c_str());
}

// ---------------------------------------------------------------------------
//	¥ RegisterClasses								[protected]
// ---------------------------------------------------------------------------
//	To reduce clutter within the Application object's constructor, class
//	registrations appear here in this seperate function for ease of use.

void
CPakRatApp::RegisterClasses()
{
	bool hasAM = UEnvironment::HasFeature(env_HasAppearance101);
	if ( hasAM ) {
		::RegisterAppearanceClient();
		UControlRegistry::RegisterAMClasses();
	}

		// Register core PowerPlant classes.
	RegisterClass_(LWindow);
	RegisterClass_(LCaption);
	RegisterClass_(LPrintout);
	RegisterClass_(LPlaceHolder);
	RegisterClass_(LKeyScrollAttachment);
	RegisterClass_(LColorEraseAttachment);
	RegisterClass_(LUndoer);
	RegisterClass_(LScroller);
	RegisterClass_(LActiveScroller);
	RegisterClass_(LView);
	RegisterClass_(LPageController);
	RegisterClass_(LListBox);
	RegisterClass_(LTextEditView);
	

		// Register the Appearance Manager/GA classes. You may want
		// to remove this use of UControlRegistry and instead perform
		// a "manual" registration of the classes. This cuts down on
		// extra code being linked in and streamlines your app and
		// project. However, use UControlRegistry as a reference/index
		// for your work, and ensure to check UControlRegistry against
		// your registrations each PowerPlant release in case
		// any mappings might have changed.
		
	UControlRegistry::RegisterClasses();

		// Register custom classes
	RegisterClass_(GLPane);
	RegisterClass_(CMemoryTable);
	RegisterClass_(CGWorldViewTimed);
	RegisterClass_(CGWorldViewListener);
	RegisterClass_(C3DModelPane);
	RegisterClass_(C3DShaderPane);
	RegisterClass_(C3DMapPane);
	RegisterClass_(CShaderMenu);

	RegisterClass_(COutlineTable);
	RegisterClass_(CDirtyText);
	RegisterClass_(WTextView);
	RegisterClass_(CLiveResizeAttachment);
	
	
}

#pragma mark -

// ---------------------------------------------------------------------------
// SpendTime
// ---------------------------------------------------------------------------
void CPakRatApp::SpendTime(const EventRecord	&/*inMacEvent*/)
{
	// using TickCount makes the textures look grainy 
	// for some reason.
	//realtime = ::TickCount() * .032123123;
	realtime += .025123123;
	
	// FIXME - this causes a crash after model windows are closed
	//MoviesTask(nil, 100);
	
	/*
	if (_showMemoryWarning) {
		// Display alert to warn user that memory is low. We
		// purposely don't deactivate the desktop or have an
		// event filter since we don't want to do more things
		// that use memory.

		if (::GetResource(FOUR_CHAR_CODE('ALRT'), ALRT_LowMemory) != nil) {
			::CautionAlert(ALRT_LowMemory, nil);
		}
		_showMemoryWarning = false;
	}
	*/
}

// ---------------------------------------------------------------------------
//	¥ TakeOffDuty
// ---------------------------------------------------------------------------
//	A Commander is going off duty
//
//	Subclasses should override this function if they wish to behave
//	differently when on duty than when off duty

void
CPakRatApp::TakeOffDuty()
{
	SetSleepTime(10);
}

// ---------------------------------------------------------------------------
//	¥ PutOnDuty
// ---------------------------------------------------------------------------
//	A Commander is going on duty.
//
//	inNewTarget is the Commander that is becoming the Target, which is
//	this Commander or one of its SubCommanders.
//
//	Subclasses should override this function if they wish to behave
//	differently when on duty than when off duty

void
CPakRatApp::PutOnDuty(
	LCommander*	/* inNewTarget */)
{
	SetSleepTime(0);
}




// ---------------------------------------------------------------------------
//	¥ DoReopenApp									[protected, virtual]
// ---------------------------------------------------------------------------
//	Support the Finder's "re-open application" ('rapp') Apple Event. From
//	Apple TechNote 1102 (on Mac OS 8):
//
//		The Finder now sends a 're-open application' Apple event ('rapp') to
//		applications when the application is already running and it is opened
//		from one of the Finder's windows (either by a double click or by
//		selecting the application and choosing the Open command). Applications
//		receiving a 'rapp' event (when they do not have any windows open) should
//		open a new untitled document just as they would when processing an 'oapp'
//		event.

void
CPakRatApp::DoReopenApp()
{
		// Given the suggested course of action by TN1102, the appropriate
		// action to take would be the equivalent of calling StartUp(). You
		// could call that here if you wish, but PowerPlant (nor this stationery)
		// will do that by default as StartUp() may contain code that one would
		// only wish to execute in response to an 'oapp' event, e.g. displaying
		// a splash screen. Additionally, 'rapp' should only open the new
		// untitled document if there are no other windows open.
		//
		// Given these circumstances, we'll create a new untitled document
		// (cmd_New) if no regular or modal windows are open (the TN didn't
		// address layers, but we'll assume this is what they mean).
	
	if ((UDesktop::FetchTopRegular() == nil)
		&& (UDesktop::FetchTopModal() == nil)) {
		ObeyCommand(cmd_New, nil);
	}
}


// ---------------------------------------------------------------------------
//	¥ ObeyCommand									[public, virtual]
// ---------------------------------------------------------------------------
//	Respond to Commands. Returns true if the Command was handled, false if not.

Boolean
CPakRatApp::ObeyCommand(
	CommandT	inCommand,
	void*		ioParam)
{
	Boolean		cmdHandled = true;	// Assume we'll handle the command
	CPreferences *prefs = preferences();
	Boolean value;
	string valueString;

	switch (inCommand) {
	
		case cmd_About:
		
			// find some suitable music
			CWav *wav = nil;
			wav = soundWithPathName(nil,  _preferences->valueForKey("aboutMusic").c_str());
			if (!wav) wav = soundWithPathName(nil,  "music/win.wav");
			if (!wav) wav = soundWithPathName(nil,  "music/win.mp3");
			
			if (wav) {
				wav->loop(true);
				wav->play();
			}
			// FIXME: simple about no longer works
			DoAboutBox(false);
			if (wav)
				delete wav;
			break;
		
	// global settings
		case cmd_interpolation:
			value = prefs->booleanForKey("useInterpolation");
			prefs->setBooleanForKey(!value, "useInterpolation");
			return true;
		case cmd_repeatanimations:	
			value = prefs->booleanForKey("repeatAnimations");
			prefs->setBooleanForKey(!value, "repeatAnimations");
			return true;
		case cmd_assemblemodels:
			value = prefs->booleanForKey("assembleModels");
			prefs->setBooleanForKey(!value, "assembleModels");
			return true;
		case cmd_showinfo:
			value = prefs->booleanForKey("showHUD");
			prefs->setBooleanForKey(!value, "showHUD");
			return true;
		case cmd_playsounds:
			value = prefs->booleanForKey("playSounds");
			prefs->setBooleanForKey(!value, "playSounds");
			return true;
		case cmd_animateShaders:
			value = prefs->booleanForKey("animateShaders");
			prefs->setBooleanForKey(!value, "animateShaders");
			CShader::_animateShaders = prefs->booleanForKey("animateShaders");
			return true;

		case cmd_backgroundgrey:
			valueString = prefs->valueForKey("backgroundColor");
			prefs->setValueForKey("grey", "backgroundColor");
			break;
			
		case cmd_backgroundblack:
			valueString = prefs->valueForKey("backgroundColor");
			prefs->setValueForKey("black", "backgroundColor");
			break;

		case cmd_backgroundwhite:	
			valueString = prefs->valueForKey("backgroundColor");
			prefs->setValueForKey("white", "backgroundColor");
			break;

		case cmd_backgroundchoose:	
			valueString = prefs->valueForKey("backgroundColor");
			// ¥ Display the color picker and allow the user to select a new color
			RGBColor	outColor;
			RGBColor	inColor = { 0,0,0 };
			Point		where = { 0, 0 };
			Str255	prompt = "\pSelect background color:";
			UDesktop::Deactivate ();
			if ( ::GetColor ( where, prompt, &inColor, &outColor )) 
			{
				prefs->setValueForKey(colorToString(outColor), "backgroundColor");
			}
			UDesktop::Activate ();
			break;

		case cmd_paletteQuake:
			valueString = prefs->valueForKey("palette");
			prefs->setValueForKey("quakeI", "palette");
			break;
		case cmd_paletteHexen:
			valueString = prefs->valueForKey("palette");
			prefs->setValueForKey("hexenII", "palette");
			break;
		case cmd_openDocumentPath:
			openDocumentWithPath((char*)ioParam);
			break;

		default: {
			cmdHandled = LDocApplication::ObeyCommand(inCommand, ioParam);
			break;
		}
	}
	
	return cmdHandled;
}

// ---------------------------------------------------------------------------
//	¥ HandleKeyPress												  [public]
// ---------------------------------------------------------------------------
//	Tab switches the Target to the next item in the TabGroup.
//	Shift-Tab to the previous item.
//	All other keystrokes (and Tabs with modifiers other than Shift)
//		get passed up.

Boolean CPakRatApp::HandleKeyPress(const EventRecord	&inKeyEvent)
{
	string command = preferences()->commandForKeyEvent(inKeyEvent);
	
	if (command == "dumpHeap") {
		CMemoryTracker::dumpHeap();
		CPakStream::dumpHeap();
		return true;
	}
	
	return false;
}


// ---------------------------------------------------------------------------
//	¥ FindCommandStatus								[public, virtual]
// ---------------------------------------------------------------------------
//	Determine the status of a Command for the purposes of menu updating.

void
CPakRatApp::FindCommandStatus(
	CommandT	inCommand,
	Boolean&	outEnabled,
	Boolean&	outUsesMark,
	UInt16&		outMark,
	Str255		outName)
{
	CPreferences *prefs = preferences();
	string stringValue;


	switch (inCommand) {
	
	
		case cmd_recentmenu:
			outEnabled = prefs->recentFiles()->size() > 0;
			outUsesMark = false;
			break;

	// global settings
		case cmd_interpolation:
			outEnabled = true;
			outUsesMark = true;
			outMark = prefs->booleanForKey("useInterpolation") ? checkMark : noMark;
			break;
		case cmd_repeatanimations:	
			outEnabled = true;
			outUsesMark = true;
			outMark = prefs->booleanForKey("repeatAnimations") ? checkMark : noMark;
			break;
		case cmd_assemblemodels:
			outEnabled = true;
			outUsesMark = true;
			outMark = prefs->booleanForKey("assembleModels") ? checkMark : noMark;
			break;
		case cmd_showinfo:
			outEnabled = true;
			outUsesMark = true;
			outMark = prefs->booleanForKey("showHUD") ? checkMark : noMark;
			break;
		case cmd_playsounds:
			outEnabled = true;
			outUsesMark = true;
			outMark = prefs->booleanForKey("playSounds") ? checkMark : noMark;
			break;
		case cmd_animateShaders:
			outEnabled = true;
			outUsesMark = true;
			outMark = prefs->booleanForKey("animateShaders") ? checkMark : noMark;
			break;
			
			
		case cmd_backgroundcolormenu:
		case cmd_palettemenu:	
			outEnabled = true;
			outUsesMark = false;
			break;
			
		case cmd_backgroundgrey:
			stringValue = prefs->valueForKey("backgroundColor");
			outEnabled = true;
			outUsesMark = true;
			outMark = stringValue == "grey" || stringValue == "" ? checkMark : noMark;
			break;
			
		case cmd_backgroundblack:
			outEnabled = true;
			outUsesMark = true;
			outMark = prefs->valueForKey("backgroundColor") == "black" ? checkMark : noMark;
			break;
			
		case cmd_backgroundwhite:	
			outEnabled = true;
			outUsesMark = true;
			outMark = prefs->valueForKey("backgroundColor") == "white" ? checkMark : noMark;
			break;
			
		case cmd_backgroundchoose:	
			outEnabled = true;
			outUsesMark = true;
			int index = prefs->valueForKey("backgroundColor").find("rgb");
			outMark = index >= 0 ? checkMark : noMark;
			break;

		case cmd_paletteQuake:
			stringValue = prefs->valueForKey("palette");
			outEnabled = true;
			outUsesMark = true;
			outMark = stringValue == "quakeI" || stringValue == "" ? checkMark : noMark;
			break;
			
		case cmd_paletteHexen:
			stringValue = prefs->valueForKey("palette");
			outEnabled = true;
			outUsesMark = true;
			outMark = stringValue == "hexenII" ? checkMark : noMark;
			break;
			
		default: {
			LDocApplication::FindCommandStatus(inCommand, outEnabled,
											outUsesMark, outMark, outName);
			break;
		}
	}
	
	if (LGrowZone::GetGrowZone()->MemoryIsLow()) {
		switch (inCommand) {
		
			case cmd_New:
			case cmd_Open:
			case cmd_PageSetup:
			case cmd_Print:
			case cmd_Copy:
			case cmd_SaveCopyAs:
			case cmd_Save:
			case cmd_Preferences:
			case cmd_About:
				outEnabled = false; // No!
				break;
		}
	}
}

#pragma mark -

void CPakRatApp::OpenControls()
{
}


// ---------------------------------------------------------------------------
//	¥ OpenDocument									[protected, virtual]
// ---------------------------------------------------------------------------
// This method is called when a file is chosen from the Open File dialog.

void
CPakRatApp::OpenDocument(
	FSSpec*		inMacFSSpec)
{

	// FIXME - put up memory warning again
	// this should catch files dragged and dropped from the Finder
	if (LGrowZone::GetGrowZone()->MemoryIsLow())
		return;
		
	LDocument*	theDoc = LDocument::FindByFileSpec(*inMacFSSpec);
	
		// If the document is already open make it the current document,
		// else create a new document.
	if (theDoc != nil) {
		theDoc->MakeCurrent();
	} else {
	
		// determine which kind of document to make based on file extension
		FSSpec name = *inMacFSSpec;
		p2cstrcpy(( char*)name.name,(const unsigned char *)name.name);
		string filename = (char*)name.name;
		
		string package, file, extension;
		decomposeEntryName(filename,  package, file, extension);
		extension = lowerString(extension);
		
		if (extension == "pak" 
			|| extension == "wad" 
			|| extension == "zip" 
			|| extension == "pk3"

			// these need to be opened in a CFSArchive 
			|| extension == "skin"
			|| extension == "mdl"
			|| extension == "md2"
			|| extension == "mdr"
			|| extension == "md3"
			|| extension == "bsp"
			
		)
			theDoc = new CResourceDocument(this, inMacFSSpec, extension);
		else {
		
			FSSpec spec = *inMacFSSpec;
			string docPath = fileSpecToPath(spec);
			gRecentFilesMenu->addRecentFile(docPath);
			
			// FIXME - this doesn't come from a CFileArchive though...
			CPakStream *inItem = new CPakStream(spec);
			OpenEditorForItem(nil, docPath.c_str());
		}
	}
}


// ---------------------------------------------------------------------------
//	¥ MakeNewDocument								[protected, virtual]
// ---------------------------------------------------------------------------
// This method creates a new document and installs it into the application's
// Apple Event Object Model hierarchy.

LModelObject*
CPakRatApp::MakeNewDocument()
{
	FSSpec aSpec;
	OSErr anErr = FSMakeFSSpec(0, 0, "\pSonata:Games:quake3:baseq3:pak0.pk3", &aSpec); 		// Quake III
	if (anErr == noErr)
		return new CResourceDocument(this, &aSpec, "pk3");
	return nil;
}


// ---------------------------------------------------------------------------
//	¥ ChooseDocument								[protected, virtual]
// ---------------------------------------------------------------------------
// This method uses the PowerPlant Standard Dialogs to let the user choose a
// document to open.

void
CPakRatApp::ChooseDocument()
{
	PP_StandardDialogs::LFileChooser	chooser;
	
		// Open any/all TEXT files
	
	NavDialogOptions*	options = chooser.GetDialogOptions();
	if (options != nil) {
		options->dialogOptionFlags =	kNavDefaultNavDlogOptions
										+ kNavSelectAllReadableItem;
	}
	
	LFileTypeList types = LFileTypeList(fileTypes_All);

	if (chooser.AskOpenFile(types)) { // LFileTypeList(ResType_Text)
		AEDescList		docList;
		chooser.GetFileDescList(docList);
		SendAEOpenDocList(docList);
	}
}


// ---------------------------------------------------------------------------
//	¥ PrintDocument									[protected, virtual]
// ---------------------------------------------------------------------------
// This method is called in response to a Print request

void
CPakRatApp::PrintDocument(
	FSSpec*		inMacFSSpec)
{
		// Create a new document using the file spec.
		// FIXME - get the extension
	CResourceDocument*	theDocument = new CResourceDocument(this, inMacFSSpec, "PAK");

		// Tell the document to print.
	theDocument->DoPrint();
}


#pragma mark -

// #include <Files.h>

void CPakRatApp::FindQuake()
{
/*
	DTPBRec param;
	Str255 volume = "\pSonata";
	Str255 quake = "\pMacQuake3";
	
	param.ioCompletion = nil;
	param.ioNamePtr = &volume;
	param.ioDTRefNum = 0;

	OSErr = PBDTGetPath(&param, false);

	param.ioCompletion = nil;
	param.ioNamePtr = &volume;
	//param.ioDTRefNum = ;
	param.ioIndex = 0;
	param.ioFileCreator = 'IDQ3';
	
	OSErr = PBDTGetAPPL(&param, false);
	
	dprintf("ioAPPLParID %ld\n", param.ioAPPLParID);
	
	FSSpec spec;
	FSMakeSpec(0, param.ioAPPLParID, quake, &spec);
	FInfo finfo;
	FSpGetFInfo(&spec, finfo);	
*/
}

// find a resource in all open documents
CPakStream *CPakRatApp::itemWithPathName(CFileArchive *pak, const char *path)
{
	CPakStream *pakItem = nil;

	if (pak) {
		pakItem = pak->itemWithPathName(path);
	} else {
		pakItem = CFileArchive::itemWithPathNameSearchAll(path);
	}
		
	return pakItem;
}

void CPakRatApp::MemoryIsLow() 
{ 
//	_showMemoryWarning = true; 
};




Boolean CPakRatApp::updateSkin(const char *path, char *skindata, UInt32 size)
{
	Boolean result = false;
	// find a suitable candidate
	CViewer *skinViewer = CViewer::viewerForTypeWithPath("md3", path);
	if (!skinViewer) 
		skinViewer = CViewer::viewerForTypeWithPath("mdr", path);
	
	if (skinViewer) {
		CModelInstance *skinModel = skinViewer->model();
		if (skinModel) {		
			skinViewer->focusGLPane();
			skinModel->parseSkin(skindata, size);
			skinModel->resources()->parseShaders();
			skinViewer->modelPane()->SetModel(skinModel);
			result = true;
		}
	}
	return result;
}

Boolean CPakRatApp::ApplySkin(CFileArchive *pak, const char *path)
{
	Boolean result = false;
	// find a suitable candidate
	CViewer *skinViewer = CViewer::viewerForTypeWithPath("md3", path);
	if (!skinViewer) 
		skinViewer = CViewer::viewerForTypeWithPath("mdr", path);
	
	if (skinViewer) {
		CModelInstance *skinModel = skinViewer->model();
		if (skinModel) {	
			
		
			// determine the skin name
			skinViewer->focusGLPane();
			skinModel->applySkinNamed(pak, packageNameFromPath(path).c_str(), skinNameFromPath(path).c_str());
			skinModel->resources()->parseShaders();
			skinViewer->modelPane()->SetModel(skinModel);
			result = true;
		}
	}
	return result;
}

CWav *CPakRatApp::soundWithPathName(CFileArchive *pak, const char *path) 
{
	CPakStream *sound = nil;
	CWav *wav = nil;
	try {
		sound = itemWithPathName(pak, path);
		if (sound) {
			wav = new CWav(sound);
			delete sound;
		}
	} catch (...) {
		// no sound, too bad.
		if (wav) 
			delete wav;
	}
	return wav;
}


Boolean CPakRatApp::openDocumentWithPath(const char *s)
{
	string path = s;
	FSSpec spec;
	pathToFSSpec( path, spec);		
	string newpath = fileSpecToPath(spec) ;
	dprintf("cmd_openDocumentPath %s -> %s\n", path.c_str(), newpath.c_str());
	OpenDocument(&spec);
	gRecentFilesMenu->addRecentFile(s);
	return true;
}

void CPakRatApp::OpenEditorForItem(CFileArchive *pak, const char *path, const char *itemname, int modifiers)
{
	Boolean commandDown = (modifiers & cmdKey) != 0;
	Boolean shiftDown = (modifiers & shiftKey) != 0;
	Boolean optionDown = (modifiers & optionKey) != 0;
	dprintf("--------------------- start OpenEditorForItem %s\n", path);
	string package, file, extension;
	decomposeEntryName(path,  package, file, extension);
	CViewer *viewer = nil;
	string type = lowerString(extension);
	
	if (type == "skin" && !commandDown) {
		if (ApplySkin(pak, path))
			return;
	}
	
	// if the viewer is already open bring it to front
	if (!commandDown || strlen(itemname)) {
		viewer = CViewer::viewerWithPath(path);
		if (viewer) {
			LWindow *window = viewer->window();
			if (window) {
			
				// Bring the window to the front.
				UDesktop::SelectDeskWindow( window );
				
				if (strlen(itemname)) 
					viewer->selectItemNamed(itemname);
			}
		}
	}
	
	if (!viewer && pak) {
		viewer = new CViewer(pak->document());
		if (!viewer->SetupViewer(pak, path, itemname, modifiers))
			delete viewer;
	} else if (!viewer) {
		viewer = new CViewer(this);
		if (!viewer->SetupViewer(nil, path, itemname, modifiers))
			delete viewer;
	}
	
	dprintf("--------------------- end OpenEditorForItem \n");
}



// ---------------------------------------------------------------------------
//	¥ LMenuBar								Constructor				  [public]
// ---------------------------------------------------------------------------

CPakRatMenuBar::CPakRatMenuBar(
	ResIDT	inMBARid) : LMenuBar(inMBARid)
{
}

// ---------------------------------------------------------------------------
//	¥ FindKeyCommand												  [public]
// ---------------------------------------------------------------------------
//	The key equivalents for the animation menu were masking out 
//  all other alphabetic key equivalents. Hide them before processing
//	key commands.

CommandT
CPakRatMenuBar::FindKeyCommand(
	const EventRecord&	inKeyEvent,
	SInt32&				outMenuChoice) const
{
	gAnimationMenu->HideCommands();
	CommandT	menuCmd = LMenuBar::FindKeyCommand(inKeyEvent,outMenuChoice);
	gAnimationMenu->ShowCommands();

	return menuCmd;
}

