#include "cDirectoryPopupAttachment.h"

// ===========================================================================
//	cDirectoryPopupAttachment.cpp	Version 1.0
//	Written by Joakim Braun		Based on Marco Piovanelli's DirectoryPopup 1.0.4
// ===========================================================================
//
//	A PowerPlant LAttachment subclass that manages a "directory popup" menu. 
//	When command-clicking in a document window's title text region, a popup menu is shown,
//	tracing	the file path, allowing the user to have the Finder open any of the folders.	

//	HOW TO USE:	*	Simply add a cDirectoryPopupAttachment attachment to the application object:
//				
//					AddAttachment(new cDirectoryPopupAttachment(kDirectoryPopMenuID));

//			*	cDirectoryPopupAttachment works on all windows whose LCommander chain
//				include an LSingleDoc (or derived class). The LSingleDoc must have been
//				saved to disk (so that the attachment can get a valid FSSpec from it).
//				If it hasn't been saved to disk, no popup is shown.

//	THANKS:		As noted above, cDirectoryPopupAttachment is a PowerPlant rewrite 
//			of Marco Piovanelli's DirectoryPopup 1.0.4, the code of which he has kindly allowed me to swipe. 
//			DirectoryPopup and more can be found at Marco's Web site, http://www.merzwaren.com/.
//			Any blame for errors in cDirectoryPopupAttachment rests, of course, with me. 
				
//	cDirectoryPopupAttachment is free for any and all use. 
//	Do not distribute modified source code under my name.

//	No support promised, no liability accepted. Provided "as is". 
//	That said, I can be reached at braun@swipnet.se.

//	Change history:
//	1.0	August 29, 1998		First release

#pragma mark -

// -----------------------------------------------------------------
//	¥	Constructor 
// -----------------------------------------------------------------
//	Takes a menu ID, later used to create popup. This is NOT a resource ID.
//	See Apple's documentation of ::NewMenu() in Inside Mac: Macintosh TB Essentials.

cDirectoryPopupAttachment::cDirectoryPopupAttachment(ResIDT inMenuID){

	mMenuID 	= inMenuID;
	mMessage 	= msg_Event;
}

// -----------------------------------------------------------------
//	¥	ExecuteSelf()	(Overridden from LAttachment)
// -----------------------------------------------------------------
//	Trap command clicks in window's title text regions, show pop if it's a document window

void	cDirectoryPopupAttachment::ExecuteSelf(MessageT	 inMessage, void *ioParam){
	
	mExecuteHost = true;
	
	switch (inMessage){
				
		case msg_Event:
			
			EventRecord	*event = (EventRecord*) ioParam;
			WindowRef	window			= NULL;
			LWindow*	windowObject 		= NULL;
			LFile*		windowFile		= NULL;
			FSSpec		ioFileSpec;
			
			// If it's a command-click in a window's drag region...
			if(event->what == mouseDown && event->modifiers & cmdKey && ::FindWindow(event->where, &window) == inDrag){
				
				// Get PP LWindow object associated with Mac window
				windowObject 	= LWindow::FetchWindowObject(window);
				ThrowIfNil_(windowObject);
				
				// Try to get LFile associated with LWindow
				windowFile	= GetWindowDocumentFile(windowObject);
				
				if(windowFile){
					
					// Get LFile's FSSpec
					windowFile->GetSpecifier(ioFileSpec);
					
					if(TrackDirectoryPopup(&ioFileSpec, window, event->where))
						OpenFinderObject(&ioFileSpec);
					
					mExecuteHost = false;
				}
			}
			
			break;
	}
}

#pragma mark -

// -----------------------------------------------------------------
//	¥	TrackDirectoryPopup()
// -----------------------------------------------------------------
//	We call this when we detect a command-click in a window's drag bar.
//	On entry, ioFileSpec points to the FSSpec associated with the window.
//	On exit, ioFileSpec points to the selected folder (if a selection was made).
//	If this function determines that this click should show a directory
//	pop-up menu, it displays it and does all the mouse tracking.
//	It then returns true if a selection was made.

//	This throws exceptions at various points, but the exceptions are caught in the function
//	and won't show up in non-debug versions of a program, annoying though they may be
//	during debugging.


Boolean cDirectoryPopupAttachment::TrackDirectoryPopup (FSSpec* ioFileSpec, WindowPtr inWindow, Point inHitPt){

	StRegion	titleTextRgn;
	MenuHandle 	popup 		= NULL;
	Point 		corner;
	SInt16 		menuChoice 	= 0;
	Boolean 	result 		= false;

	try{
	
		// Make sure inWindow is selected
		ThrowIf_(!::IsWindowHilited(inWindow));

		// Get window's title text region
		ThrowIfOSErr_(GetWindowTitleTextRgn(inWindow, titleTextRgn));

		// Determine whether the click went into the title text area.
		ThrowIf_(!::PtInRgn(inHitPt, titleTextRgn));

		// Calculate the coordinates of the top/left corner of the pop-up menu
#if PP_Target_Carbon
		Rect titleTextRect;
		::GetWindowBounds(inWindow, kWindowTitleTextRgn, &titleTextRect);
		corner = *(Point*)(&titleTextRect);
#else
		corner = *(Point*)(&(*(RgnHandle)titleTextRgn)->rgnBBox);
		corner.h -= 21;
#endif
		corner.v += 1;

		// Build the directory pop-up menu
		popup = BuildDirectoryPopup(ioFileSpec);
		ThrowIfNil_(popup);

		// Insert the menu into the hierarchical portion of the menu list
		::InsertMenu (popup, hierMenu);

		// Pop up the menu
		menuChoice = (SInt16) ::PopUpMenuSelect (popup,  corner.v, corner.h, 1);

		// Remove the pop-up menu from the menu list
		::DeleteMenu(mMenuID);
		
		// Dispose memory allocated by ::NewMenu()
		::DisposeMenu (popup);

		//	see if an item up in the hierarchy was selected
		if (menuChoice > 1){
		
			for (; menuChoice > 1; menuChoice--)
				FindParentSpec(ioFileSpec);
			
			result = true;
		}
	}
	catch(...){
		
		// Get rid of popup, swallow exceptions
		if (popup != NULL)
			::DisposeMenu (popup);
	}

	return result;
}

// -----------------------------------------------------------------
//	¥	BuildDirectoryPopup()
// -----------------------------------------------------------------
//	Construct path popup menu from FSSpec

MenuHandle cDirectoryPopupAttachment::BuildDirectoryPopup (const FSSpec* inFileSpec){

	MenuHandle	popup 			= nil;
	FSSpec 		spec 			=* inFileSpec;
	Str255 		trashName;
	SInt16		item			= 0,
			icon			= 0,
			volume			= 0;
	SInt32 		desktopDirectory 	= 0,
			trashDirectory 		= 0,
			thisDirectory		= 0,
			trashNameLength 	= 0;
	OSType 		volumeKind		= 0;
	OSStatus 	err			= noErr;

	try{
	
		// Find the IDs of the desktop and trash directories for the specified volume
		::FindFolder (spec.vRefNum, kDesktopFolderType, kDontCreateFolder, &volume, &desktopDirectory);
		::FindFolder (spec.vRefNum, kTrashFolderType, kDontCreateFolder, &volume, &trashDirectory);

		// Create an uninitialized menu from scratch
		popup = ::NewMenu (mMenuID, "\px");
		ThrowIfNil_(popup);

		do {
			// Add a new item to the menu
			item++;
			::AppendMenu (popup, "\px");

			// Find out which icon to use for this item
			if (item == 1)
				
				// First item in the pop-up menu uses a document icon
				icon = kGenericDocumentIconResource;
			
			else if (spec.parID == fsRtParID)
			{
				// Use a disk icon for root level directory
				// Default icon is the generic hard disk icon
				icon = kGenericHardDiskIconResource;

				// Find out volume kind
				volumeKind = GetVolumeKind (spec.vRefNum);

				// Use special icons for floppy and file server volumes
				if ((volumeKind == kVolumeKindFloppy) ||
					 (volumeKind == kVolumeKindDiskImage) ||
					 (volumeKind == kVolumeKindShrinkWrap))
				{
					icon = kFloppyIconResource;
				}
				
				else if (volumeKind == kVolumeKindFileServer)
				{
					icon = kGenericFileServerIconResource;
				}
				
				// The Standard File package that comes with System 7.0 Update 3.0
				// (built in System 7.5 and newer) has additional small icons
				// for CD-ROM and RAM disk volumes
				else if (UEnvironment::HasGestaltAttribute(gestaltStandardFileAttr, gestaltStandardFileHasDynamicVolumeAllocation))
				{
					if (volumeKind == kVolumeKindCDROM)
					{
						icon = kGenericCDROMIconResource;
					}
					else if (volumeKind == kVolumeKindRAMDisk)
					{
						icon = kGenericRAMDiskIconResource;
					}
				}
			}
			else if (thisDirectory == trashDirectory)
			{
				// If this directory is the trash directory, use the trash icon
				// and the real trash name
				icon = kTrashIconResource;
				::GetIndString (trashName, kStandardFileStrings, kIndTrashName);
				trashNameLength = StrLength (trashName);
				if ((trashNameLength > 0) && (trashNameLength <= 63))
				{
					::BlockMoveData (trashName, &spec.name, trashNameLength + 1);
				}
			}
			else
			{
				// In all other cases, use the open folder icon
				icon = kOpenFolderIconResource;
			}

			// Set name and icon of current Finder object
			::SetMenuItemText (popup, item, spec.name);
			::SetItemIcon (popup, item, icon - kFinderIconBaseID);
			::SetItemCmd (popup, item, kFinderIconIndicator);

			// Break out if we have reached a desktop child or the trash directory
			if ((spec.parID == desktopDirectory) || (thisDirectory == trashDirectory))
			{
				break;
			}

			// Otherwise, find the file specification of the parent directory
			thisDirectory = spec.parID;
			err = FindParentSpec (&spec);
		}
		while (err == noErr);
	}
	catch(...){
		
		// swallow exceptions
	}

	return popup;
}						

// -----------------------------------------------------------------
//	¥	GetWindowDocumentFile()
// -----------------------------------------------------------------
//	Get LFile associated with any LSingleDoc in a LWindow's LCommander hierarchy.
//	May return NULL, in which case the window wasn't associated with a document.

LFile*	cDirectoryPopupAttachment::GetWindowDocumentFile(LWindow* inWindow){
	
	LFile*		result	= NULL;
	LSingleDoc*	theDoc 	= NULL;
	LCommander*	currCmdr = dynamic_cast<LCommander*>(inWindow);
	
	// Walk the LWindow's LCommander hierarchy until we find an LSingleDoc (or the NULL commander/application object)
	while(currCmdr && !theDoc){
		
		theDoc = dynamic_cast<LSingleDoc*>(currCmdr);
		
		if(!theDoc)
			currCmdr = currCmdr->GetSuperCommander();
	}
	
	if(theDoc)
		result = theDoc->GetFile();
		
	return result;
}

// -----------------------------------------------------------------
//	¥	GetWindowTitleTextRgn()
// -----------------------------------------------------------------
//	Get RgnHandle of area occupied by window title.

OSStatus cDirectoryPopupAttachment::GetWindowTitleTextRgn (WindowPtr inWindow, RgnHandle ioTitleTextRgn)
{
#if TARGET_RT_MAC_CFM
	if (&GetWindowRegion != nil)
#else
	if (UEnvironment::IsAppearanceRunning())
#endif
	{
		// If we have Appearance running, let system calculate title region
		return ::GetWindowRegion (inWindow, kWindowTitleTextRgn, ioTitleTextRgn);
	}
	else
	{
#if PP_Target_Carbon
		// If we don't have Appearance, build our region by hand
		Rect structRect;
		Rect contentRect;
		SInt16 titleWidth;

		// Get the bounding box of the structure region
		//structRect = (*((WindowPeek) inWindow)->strucRgn)->rgnBBox;
		::GetWindowBounds(inWindow, kWindowStructureRgn, &contentRect);

		// And the bounding box of the content region
		//contentRect = (* ((WindowPeek) inWindow)->contRgn)->rgnBBox;
		::GetWindowBounds(inWindow, kWindowContentRgn, &contentRect);

		// Get title width
		Rect titleTextRect;
		::GetWindowBounds(inWindow, kWindowTitleTextRgn, &titleTextRect);
		
		// Convert this rectangle to a region
		::RectRgn (ioTitleTextRgn, &titleTextRect);

		return noErr;
#else
		// If we don't have Appearance, build our region by hand
		Rect structRect;
		Rect contentRect;
		Rect titleTextRect;
		SInt16 titleWidth;

		// Get the bounding box of the structure region
		structRect = (*((WindowPeek) inWindow)->strucRgn)->rgnBBox;

		// And the bounding box of the content region
		contentRect = (* ((WindowPeek) inWindow)->contRgn)->rgnBBox;

		// Get title width
		titleWidth = GetWindowTitleWidth (inWindow);

		// Calculate the title text rect
		titleTextRect.left = structRect.left +
			(((structRect.right - structRect.left) - titleWidth - 1) / 2);
		titleTextRect.right = titleTextRect.left + titleWidth;
		titleTextRect.top = structRect.top + 2;
		titleTextRect.bottom = contentRect.top - 2;

		// Convert this rectangle to a region
		::RectRgn (ioTitleTextRgn, &titleTextRect);

		return noErr;
#endif
	}
}

// -----------------------------------------------------------------
//	¥	FindParentSpec()
// -----------------------------------------------------------------
//	Find parent folder containing an item.

OSStatus cDirectoryPopupAttachment::FindParentSpec (FSSpec* ioFileSpec)
{
	CInfoPBRec 	pb;
	OSStatus 	err = noErr;

	BlockClear (&pb, sizeof (pb));
	
	pb.dirInfo.ioVRefNum 	= ioFileSpec->vRefNum;
	pb.dirInfo.ioDrDirID 	= ioFileSpec->parID;
	pb.dirInfo.ioFDirIndex 	= -1;
	pb.dirInfo.ioNamePtr 	= ioFileSpec->name;

	err = ::PBGetCatInfoSync (&pb);

	ioFileSpec->parID = pb.dirInfo.ioDrParID;

	return err;
}

// -----------------------------------------------------------------
//	¥	GetVolumeKind()
// -----------------------------------------------------------------
//	Return what sort of volume we have. Used for deciding what icon to show in popup.

OSType cDirectoryPopupAttachment::GetVolumeKind (SInt16 inVolumeRefNum)
{
	OSType 		volumeKind = 0;
	HParamBlockRec 	pb;
	DCtlPtr 	pDCtlEntry;
	DRVRHeaderPtr 	pDriver;

	BlockClear (&pb, sizeof (pb));
	pb.volumeParam.ioVRefNum = inVolumeRefNum;
	
#warning GetDCtlEntry removed for Carbon
#if PP_Target_Carbon
#else
	if (::PBHGetVInfoSync (&pb) == noErr)
	{
		// Get a pointer to the driver control entry of the volume's driver
		pDCtlEntry =* ::GetDCtlEntry (pb.volumeParam.ioVDRefNum);

		// Get a pointer to the driver itself
		pDriver = (DRVRHeaderPtr) pDCtlEntry->dCtlDriver;

		// If the driver is RAM-based, dCtlDriver is really a handle
		if (pDCtlEntry->dCtlFlags &dRAMBasedMask)
		{
			pDriver =* (DRVRHeaderHandle) pDriver;
		}

		// Extract driver "tag" from driver name (letters 2 to 5)
		volumeKind =* (OSType *) (&pDriver->drvrName [2]);
	}

#endif
	return volumeKind;
}

#pragma mark -

// -----------------------------------------------------------------
//	¥	OpenFinderObject()
// -----------------------------------------------------------------
//	Use AppleEvents to tell Finder to open a folder's window

OSErr cDirectoryPopupAttachment::OpenFinderObject (const FSSpec* inThing)
{
	FSSpec 		thing =* inThing;
	StAEDescriptor 	ae, 
			reply,
		 	thingAlias, 
			parentAlias, 
			thingList;
	ProcessSerialNumber finderPSN;
	OSErr 		err = noErr;

	try{
		//	create a minimal alias for the specified object
		err = ::NewAliasMinimal (&thing, (AliasHandle *) &thingAlias.mDesc.dataHandle);
		ThrowIfOSErr_(err);
		thingAlias.mDesc.descriptorType = typeAlias;

		if (UEnvironment::HasGestaltAttribute(gestaltFinderAttr, gestaltOSLCompliantFinder))
		{
			//	if the Finder is OSL-compliant, we can send it a
			//	straightforward "open" (aevt/odoc) event
			err = CreateFinderEvent (kCoreEventClass, kAEOpen, ae);
			ThrowIfOSErr_(err);

			//	the direct parameter of the event is simply the file alias
			err = ::AEPutParamDesc (ae, keyDirectObject, thingAlias);
			ThrowIfOSErr_(err);
		}
		else
		{
			// Older, non-OSL-compliant, Finders (from version 7.0 onward)
			// require a custom "open selection" event (FNDR/sope) with much
			// more complicated parameters

			err = CreateFinderEvent (kCoreEventClass, kAEOpenSelection, ae);
			ThrowIfOSErr_(err);

			// Create a full alias for the parent directory (enclosing folder)
			// of specified object; if the object is at root level, create a full
			// alias for the object itself
			if (thing.parID != fsRtParID){
				
				err = FindParentSpec (&thing);
				ThrowIfOSErr_(err);
			}

			err = ::NewAlias (nil, &thing, (AliasHandle *) &parentAlias.mDesc.dataHandle);
			ThrowIfOSErr_(err);
			parentAlias.mDesc.descriptorType = typeAlias;

			// Add the alias record for the parent directory to the Apple event, as direct object
			err = ::AEPutParamDesc (ae, keyDirectObject, parentAlias);
			ThrowIfOSErr_(err);

			// Create a list of descriptors for the objects to show
			err = ::AECreateList (nil, 0, false, thingList);
			ThrowIfOSErr_(err);
			
			err = ::AEPutDesc (thingList, 0, thingAlias);
			ThrowIfOSErr_(err);

			// Add the object list to the Apple event, as a keySelection parameter
			err = ::AEPutParamDesc (ae, keySelection, thingList);
			ThrowIfOSErr_(err);
		}

		// Bring the Finder to the foreground
		err = FindProcess(kFinderType, kFinderSignature, &finderPSN);
		ThrowIfOSErr_(err);
		
		err = ::SetFrontProcess(&finderPSN);
		ThrowIfOSErr_(err);

		// Send the apple event
		err = ::AESend (ae, reply, kAENoReply + kAECanSwitchLayer, kAENormalPriority, kAEDefaultTimeout, nil, nil);
		ThrowIfOSErr_(err);
	
	}
	catch(...){
		
		// Swallow exceptions, error code is in err and will be returned
	}

	return err;
}

// -----------------------------------------------------------------
//	¥	CreateFinderEvent()
// -----------------------------------------------------------------
//	Stuff Finder PSN into apple event

OSErr cDirectoryPopupAttachment::CreateFinderEvent (AEEventClass inEventClass, AEEventID inEventID, AppleEvent* outAE)
{
	ProcessSerialNumber 	finderPSN;
	StAEDescriptor		finderAddress;
	OSErr err		= noErr;

	try{
	
		// Find the process serial number of the Finder
		err = FindProcess(kFinderType, kFinderSignature, &finderPSN);
		ThrowIfOSErr_(err);

		// Create an address descriptor for the target application based on the PSN
		err = ::AECreateDesc (typeProcessSerialNumber, &finderPSN, sizeof (finderPSN), finderAddress);
		ThrowIfOSErr_(err);

		// Create the Apple event
		err = ::AECreateAppleEvent (inEventClass, inEventID, finderAddress, kAutoGenerateReturnID, kAnyTransactionID, outAE);
		ThrowIfOSErr_(err);
	
	}
	catch(...){
		
		// Swallow exceptions, error code is in err and will be returned
	}

	return err;
}

// -----------------------------------------------------------------
//	¥	FindProcess()
// -----------------------------------------------------------------
//	Find process serial number of a given type and signature

OSErr cDirectoryPopupAttachment::FindProcess (OSType inProcessType, OSType inProcessSignature, ProcessSerialNumber* outPSN)
{
	ProcessInfoRec info;

	// Init process info record
	BlockClear (& info, sizeof (info));
	info.processInfoLength = sizeof (info);
	
	// Start at beginning of process list
	outPSN->lowLongOfPSN 	= kNoProcess;
	outPSN->highLongOfPSN 	= kNoProcess;

	// Walk the process list, looking for the given creator
	while ((::GetNextProcess (outPSN) == noErr) && (::GetProcessInformation (outPSN, & info) == noErr))
	{
		if ((info.processType == inProcessType) && (info.processSignature == inProcessSignature))
		{
			return noErr;
		}
	}

	return procNotFound;
}

// -----------------------------------------------------------------
//	¥	BlockClear()
// -----------------------------------------------------------------
//	Clear a block of memory

void cDirectoryPopupAttachment::BlockClear ( void * ioBlockPtr, Size inBlockSize )
{
	register char * p = ( char * ) ioBlockPtr ;

	while ( -- inBlockSize >= 0 )
	{
		* p ++ = 0 ;
	}
}
