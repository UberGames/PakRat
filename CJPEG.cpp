/* 
	CJPEG.cpp

	Author:			Tom Naughton
	Description:	<describe the CJPEG class here>
*/

#include "CJPEG.h"
#include "QTUtilities.h"
#include "utilities.h"




CJPEG::CJPEG() : CGLImage()
{
}


CJPEG::~CJPEG()
{
}

OSErr CJPEG::initImage(CPakStream *inItem)
{
	// read the file data into an offscreen graphics world
	OSErr myErr;
	char * p = inItem->getData("jpeg");
	long size = inItem->getSize();
	
	if (p) {
		myErr = QTJPEG_ReadJPEG(p, size);
		CMemoryTracker::safeFree(p);
	}
	return myErr;
}



//////////
//
// QTJPEG_ReadJPEG
// Open a JPEG file with supplied FSSpec; the graphics world containing the image is returned through myGWorld.
//
//////////

OSErr CJPEG::QTJPEG_ReadJPEG (char *p, long mySize)
{
	ImageDescriptionHandle	 	myDesc = nil;
	GWorldPtr					oldWorld;
	GDHandle					oldGD;
	PixMapHandle				myPixMap;
	Ptr							myDataPtr;
	OSErr						myErr = paramErr;
	GWorldPtr 					myGWorld = nil;
	char						*start = p;
	
	if (!p)
		goto fail;
		
	// save the current graphics port
	::GetGWorld(&oldWorld, &oldGD);
	
	myDesc = (ImageDescriptionHandle)::NewHandle(sizeof(ImageDescription));
	if (!myDesc)
		goto fail;
		
	::HLock((Handle)myDesc);
	Rect bounds;
	myErr = QTJPEG_ReadJPEGHeader(p, mySize, myDesc, &bounds);
	if (myErr != noErr)
		goto fail;
			
	_depth =  (*myDesc)->depth;
	myErr = QTJPEG_NewJPEGWorld(&myGWorld, (*myDesc)->depth, bounds);
	if (myErr != noErr || myGWorld == NULL)
		goto fail;
	
	p = start;		
	myPixMap = ::GetGWorldPixMap(myGWorld);
	::LockPixels(myPixMap);
	::SetGWorld(myGWorld, NULL);

	myDataPtr = p;//StripAddress(p);
	myErr = ::DecompressImage(myDataPtr, myDesc, myPixMap, &bounds, &bounds, srcCopy, NULL);
	::UnlockPixels(myPixMap);

	LGWorld *world = new LGWorld(myGWorld);
	myErr = initWithGWorld(world);
	delete world;
	myGWorld = nil;

	// restore the previous graphics world 
	::SetGWorld(oldWorld, oldGD);
	
	//DisposeGWorld(myGWorld);
	::HUnlock((Handle)myDesc);				
	::DisposeHandle((Handle)myDesc);

	return(myErr);
	
fail:

	if (myGWorld) {
		DisposeGWorld(myGWorld);
	}
	if (myDesc) {
		HUnlock((Handle)myDesc);				
		DisposeHandle((Handle)myDesc);
	}

	return myErr;
}


//////////
//
// QTJPEG_ReadJPEGHeader
// Read the JPEG header and fill out the specified ImageDescription with needed info.
//
//////////

OSErr CJPEG::QTJPEG_ReadJPEGHeader (char *&p, long 	mySize, ImageDescriptionHandle theDesc, Rect *theRect)
{
	short					mySkip;
	UInt8 					myMarker;
	Boolean					isJFIF = false;
	Boolean					readingExtension = false;
	OSErr					myErr = noErr;
	
		
	(*theDesc)->dataSize = mySize;

	// loop forever
	while (true) {
		myMarker = QTJPEG_FindNextMarker(p, mySize);
		
		switch (myMarker) {
			case kSOIMarker:
				isJFIF = true;
				break;
				
			case kAPPOMarker + 0:
			case kAPPOMarker + 1:
			case kAPPOMarker + 2:
			case kAPPOMarker + 3:
			case kAPPOMarker + 4:
			case kAPPOMarker + 5:
			case kAPPOMarker + 6:
			case kAPPOMarker + 7:
			case kAPPOMarker + 8:
			case kAPPOMarker + 9:
			case kAPPOMarker + 10:
			case kAPPOMarker + 11:
			case kAPPOMarker + 12:
			case kAPPOMarker + 13:
			case kAPPOMarker + 14:
			case kAPPOMarker + 15:
				myErr = QTJPEG_HandleAPPOMarker(myMarker, p, theDesc, &readingExtension);
				if (myErr != noErr)
					return(myErr);
				break;
				
			case kCommentMarker:
				QTJPEG_SkipLength(p);
				break;
		
			case kSOFMarker + 0:		// start of frame header marker
			case kSOFMarker + 1:
			case kSOFMarker + 2:
			case kSOFMarker + 3:
			case kSOFMarker + 5:
			case kSOFMarker + 6:
			case kSOFMarker + 7:
			case kSOFMarker + 9:
			case kSOFMarker + 10:
			case kSOFMarker + 11:
			case kSOFMarker + 13:
			case kSOFMarker + 14:
			case kSOFMarker + 15:
				myErr = QTJPEG_HandleSOFMarker(p, theDesc, readingExtension);
				if (myErr != noErr)
					return(myErr);
					
				if (!readingExtension) {
					MacSetRect(theRect, 0, 0, (*theDesc)->width, (*theDesc)->height);
					return(noErr);
				}
				break;
				
			case kDACMarker:
				mySkip = nextShort(p) - 2;
				mySkip *= nextShort(p);
				p += mySkip;
				break;
				
			case kSOSMarker:
				QTJPEG_HandleSOSMarker(p);
				break;
				
			case kDHTMarker:
			case kDQTMarker:
			case kRSTOMarker:
				QTJPEG_SkipLength(p);
				break;
				
			case kEOIMarker:		// we reached the end of image
				// we are reading an extension so keep going
				if (readingExtension)
					readingExtension = false;
				break;
		}
	}
	
	return(myErr);
}


//////////
//
// QTJPEG_FindNextMarker
// Find the next marker in the specified file.
//
//////////

UInt8 QTJPEG_FindNextMarker (char *&p, long mySize)
{
#pragma unused (mySize)
	UInt8 			myMarker;
	
	// FIXME - should make sure nextByte is available
	myMarker = nextByte(p);
	
	while (myMarker == kStartMarker)
		myMarker = nextByte(p);

	while (myMarker == 0x00) {
		myMarker = nextByte(p);

		while (myMarker != kStartMarker)
			myMarker = nextByte(p);
			
		myMarker = nextByte(p);
	}
		
	return(myMarker);
}


//////////
//
// QTJPEG_HandleAPPOMarker
// Handle an APPO marker in the specified file.
//
//////////

OSErr QTJPEG_HandleAPPOMarker (UInt8 theMarker, char *&p, ImageDescriptionHandle theDesc, Boolean *readingExtension)
{
	Fixed			xRes, yRes;
	long 			myLength;
	short			myUnits;
	short			myVersion;
	UInt8			myExtension;
	UInt8 			myType[5];
	OSErr			myErr = noErr;

	// read skip bytes - header length - skip count
	myLength = nextShort(p) - 2;
	
	if ((theMarker == kAPPOMarker) && (myLength >= 14)) {
		myType[0] = nextByte(p);
		myType[1] = nextByte(p);
		myType[2] = nextByte(p);
		myType[3] = nextByte(p);
		myType[4] = nextByte(p);
		
		// check to see if we really have the JFIF header
		if ((myType[0] == 'J') &&
			(myType[1] == 'F') &&
			(myType[2] == 'I') &&
			(myType[3] == 'F')) {
			
	  		myVersion = nextShort(p);

			if (myVersion < 0x100)	
				return(paramErr); 	// don't know this
			else
				(*theDesc)->version = myVersion;
				
			myUnits = nextByte(p);
			xRes = nextShort(p);
	  		yRes = nextShort(p);

			switch (myUnits) {
				case 0:			// no res, just aspect ratio
					xRes = FixMul(72L << 16, xRes << 16);
					yRes = FixMul(72L << 16, yRes << 16);
					break;
					
				case 1:			// dots per inch
					xRes = xRes << 16;
					yRes = yRes << 16;
					break;
					
				case 2:			// dots per centimeter (we convert to dpi )
					xRes = FixMul(0x28a3d, xRes << 16);
					yRes = FixMul(0x28a3d, xRes << 16);		// yRes?? RTM
					break;	
					
				default:
					break;
			}
			
			(*theDesc)->hRes = xRes;
			(*theDesc)->vRes = yRes;

			myLength -= 12;
			p += myLength;
			
		} else {
			if ((myType[0] == 'J') &&
				(myType[1] == 'F') &&
				(myType[2] == 'X') &&
				(myType[3] == 'X')) { 
				
				*readingExtension = true;		// next markers are extensions; ignore them

				myExtension = nextByte(p);
				switch (myExtension) {
					case 0x10:
					case 0x11:
					case 0x13:
						break;
					default:
						return(paramErr);
				}
			}
		}
	} else
		p += myLength;

	return(myErr);
}


//////////
//
// QTJPEG_HandleSOFMarker
// Handle an SOF marker in the specified file.
//
//////////

OSErr QTJPEG_HandleSOFMarker (char *&p, ImageDescriptionHandle theDesc, Boolean readingExtension)
{
	short 			myWidth = 0;
	short 			myHeight = 0;
	short 			myComponents;
	short			myLength;
	StringPtr		myTitle = "\pPhoto - JPEG";
	OSErr			myErr = noErr;

	if (!readingExtension) {
		myLength = nextShort(p);
		nextByte(p);
		myHeight = nextShort(p);
		myWidth = nextShort(p);

		// make sure we do have something to display
		if ((myWidth != 0) && (myHeight != 0)) {
		
			// now set up the image description
			(*theDesc)->idSize 			= sizeof(ImageDescription);
			(*theDesc)->cType 			= FOUR_CHAR_CODE('jpeg');
			(*theDesc)->dataRefIndex 	= 0;
			(*theDesc)->revisionLevel 	= 0;
			(*theDesc)->vendor 			= 0;
			(*theDesc)->temporalQuality = 0;
			(*theDesc)->spatialQuality	= codecNormalQuality;
			(*theDesc)->width 			= myWidth;
			(*theDesc)->height 			= myHeight;
			(*theDesc)->frameCount 		= 1;
			BlockMove(myTitle, (*theDesc)->name, 13);
			(*theDesc)->clutID 			= -1;
			
			myComponents = nextByte(p);
			
			switch (myComponents) {
				case 1:		
					(*theDesc)->depth = 40;
					break;

				case 3:
					(*theDesc)->depth = 32;
					break;

				case 4:
					(*theDesc)->depth = 32;
					break;
										
				default:
					myErr = paramErr;
					return(myErr);
					break;
			}
			
			p += myLength - 8;
			return(noErr);
		}
		
	} else {
		myLength = nextShort(p) - 2;
		p += myLength;
		if (myErr != noErr)
			return(myErr);
	}
	
	// FIXME - why was this here?
	//CMemoryTracker::safeFree(myTitle);

	return(myErr);
}


//////////
//
// QTJPEG_HandleSOSMarker
// Handle an SOS marker in the specified file.
//
//////////

void QTJPEG_HandleSOSMarker (char *&p)
{
	short		myComponents;
	short		myWord;
	
	nextShort(p);
	myComponents = nextByte(p);
	
	for (myWord = 0; myWord < myComponents; myWord++)
		nextShort(p);
}



//////////
//
// QTJPEG_SkipLength
// Skip over the length word.
//
//////////

void QTJPEG_SkipLength (char *&p)
{
	UInt16		mySkip;
	
	mySkip = nextShort(p) - 2;
	p += mySkip;
}


//////////
//
// QTJPEG_NewJPEGWorld
// Return, through the theWorld parameter, a new offscreen graphics world suitable
// for drawing a JPEG image of the specified bitdepth into.
//
//////////

OSErr QTJPEG_NewJPEGWorld (GWorldPtr *theWorld, short theDepth, Rect theRect)
{
	GWorldPtr		oldPort;
	GDHandle		oldGD;
	PixMapHandle	myPixMap;
	CTabHandle		myCTab = NULL;
	OSErr			myErr = paramErr;
	
	if (theWorld != NULL) {
		// save the current graphics port
		GetGWorld(&oldPort, &oldGD);
	
		// if depth is greater than 32, then the image is grayscale
		if (theDepth > 32) {
			myCTab = GetCTable(theDepth);
			theDepth = theDepth - 32;
		}
		
		// first try to allocate a GWorld in the application's heap
		myErr = NewGWorld(theWorld, theDepth, &theRect, myCTab, NULL, 0L);
		
		// otherwise, try to allocate a GWorld in temporary memory
		if (myErr != noErr)
			myErr = NewGWorld(theWorld, theDepth, &theRect, myCTab, NULL, useTempMem);
	
		if ((myErr == noErr) && (theWorld != NULL))  {
			myPixMap = GetGWorldPixMap(*theWorld);
			if (LockPixels(myPixMap)) {
				SetGWorld(*theWorld, NULL);
				EraseRect(&theRect);
				UnlockPixels(myPixMap);
			}
		}
		
		SetGWorld(oldPort, oldGD);	
	}
	
	return(myErr);
}

