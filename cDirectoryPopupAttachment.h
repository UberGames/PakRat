#ifndef _c_Directory_Popup_Attachment_h
#define _c_Directory_Popup_Attachment_h

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

	class cDirectoryPopupAttachment : public LAttachment{
		
		public:
			cDirectoryPopupAttachment(ResIDT inMenuID);
			
		protected:
			ResIDT	mMenuID;
			
			virtual void	ExecuteSelf(	MessageT		inMessage,
							void			*ioParam);
							
			Boolean 	TrackDirectoryPopup (FSSpec* ioFileSpec, WindowPtr inWindow, Point inHitPt);
			MenuHandle 	BuildDirectoryPopup (const FSSpec* inFileSpec);
					
			static LFile* 	GetWindowDocumentFile(LWindow* inWindow);
			static OSStatus GetWindowTitleTextRgn (WindowPtr inWindow, RgnHandle ioTitleTextRgn);
			static OSStatus FindParentSpec (FSSpec* ioFileSpec);
			static OSType 	GetVolumeKind (SInt16 inVolumeRefNum);
			
			static OSErr 	OpenFinderObject (const FSSpec* inThing);
			static OSErr 	CreateFinderEvent (AEEventClass inEventClass, AEEventID inEventID, AppleEvent* outAE);
			static OSErr 	FindProcess (OSType inProcessType, OSType inProcessSignature, ProcessSerialNumber* outPSN);
			
			static void 	BlockClear ( void * ioBlockPtr, Size inBlockSize );
			
			enum
			{
				kVolumeKindCDROM	=	'Appl' ,		//	CD-ROM
				kVolumeKindDiskImage	=	'Mung' ,		//	disk image
				kVolumeKindShrinkWrap	=	'Shri' ,		//	ShrinkWrap 2.1+ disk image
				kVolumeKindFloppy	=	'Sony' ,		//	floppy
				kVolumeKindFileServer	=	'AFPT' ,		//	AppleShare file server
				kVolumeKindRAMDisk	=	'EDis'			//	RAM disk
			};

			enum
			{
				kFinderIconIndicator	=	0x1A ,			//	menu item cmd for "Finder" icons
				kFinderIconBaseID	=	-4000 ,
				kStandardFileStrings	=	-6046 ,
				kIndTrashName		=	2
			};

			enum
			{
				kFinderType		=	'FNDR',
				kFinderSignature	=	'MACS'
			} ;

			
	};

#endif