/* 
	CQTImage.cpp

	Author:			Tom Naughton
	Description:	uses QuickTime to open images
	
*/

#include <QuickTimeComponents.h>

#include "CQTImage.h"
#include "utilities.h"
#include "CPakRatApp.h"
#include "AppConstants.h"
#include "CTypeRegistry.h"
#include "gl.h"

#define	TOTALCOLORS	255




CQTImage::CQTImage() : CGLImage()
{
}


CQTImage::~CQTImage() 
{
}



OSErr CQTImage::initImage(CPakStream *inItem)
{
	OSErr myErr = noErr;
	string package, file, extension;
	decomposeEntryName(inItem->pathName(), package, file, extension);
	resource_type_t resType = gRegistry->extensionToType(lowerString(extension).c_str());

	char * p = inItem->getData("pcx raw data");
	long size = inItem->getSize();
	
	if (p) {
		myErr = initImageFromData(p, size, gRegistry->QTComponentType(resType));
		CMemoryTracker::safeFree(p);
	} else {
		myErr = kError_outofmemory;
	}
	return myErr;
}



OSErr CQTImage::initImageFromData(char *p, UInt32 size, OSType  componentSubType)
{
	OSErr result = noErr;
	GWorldPtr					saveWorld, theGWorldPtr = nil;
	GDHandle					saveDevice;
	PixMapHandle				thePixMapHdl;
	Handle						dataRef;
	OSType               		dataRefType = 'hndl';
	long						maxTextSize;
	Boolean						resize = true;
	GraphicsImportComponent		gi = nil;
	
	glGetIntegerv( GL_MAX_TEXTURE_SIZE, &maxTextSize);

	// set up data ref
    Handle handle = ::NewHandleClear(size);
   if (!handle)
    	goto exit;
    ::BlockMoveData(p, *handle, size);
    result = ::PtrToHand(&handle,
                    &dataRef,
                    sizeof(Handle));
	if (result) goto exit;
	
	
	// Save GWorld
	::GetGWorld (&saveWorld, &saveDevice);

	// PREP IMPORTER COMPONENT
	
	
	gi = ::OpenDefaultComponent('grip', componentSubType); // load importer for this image
	if (!gi)
	{
		dprintf("Graphics Importer Component not available\n");
		goto exit;
	}
	
	
	result = ::GraphicsImportSetDataReference(gi, dataRef, dataRefType);
	if ( result != noErr )
	{
		dprintf("Unable to GraphicsImportSetDataReference \n");
		goto exit;
	}

	Rect	bounds;
	result = ::GraphicsImportGetBoundsRect( gi, &bounds); // get dimensions of image
	if ( result != noErr )
	{
		dprintf("Unable to GraphicsImportGetBoundsRect \n");
		goto exit;
	}
	
	int width = bounds.right - bounds.left;
	int height = bounds.bottom - bounds.top;
	
	if( ( width != NextPowerOf2( width)) || ( height != NextPowerOf2( height)))
	{
		//LOG("Image not ^2");
		// The Image isn't ^2 so scale it to fit.
		if( resize)
		{
			//LOG("DO RESIZE");
			int newHeight = NextPowerOf2( height);
			if( newHeight > maxTextSize)
				newHeight = maxTextSize;
				
			int newWidth =  NextPowerOf2( width);
			if( newWidth > maxTextSize)
				newWidth = maxTextSize;
				
			Rect newBounds = { 0, 0, NextPowerOf2( newHeight), NextPowerOf2( newWidth)};
			result = GraphicsImportSetBoundsRect( gi, &newBounds);
			bounds = newBounds;
		}
		else
		{
			dprintf("Image dimensions not ^2\n");
			goto exit;
		}
	}
		
	result = ::NewGWorld( &theGWorldPtr, 32, &bounds, NULL, NULL,  0);
	if ( result != noErr )
	{
		dprintf("Unable to Create GWORLD \n");
		goto exit;
	}

	// DRAW INTO THE GWORLD
	thePixMapHdl = ::GetGWorldPixMap( theGWorldPtr);
	if( ::LockPixels( thePixMapHdl) == false)
	{
		dprintf("Unable to Create Pixmap \n");
		goto exit;
	}
	
	::GraphicsImportSetGWorld( gi, theGWorldPtr, NULL);	// set the gworld to draw image into
	result = ::GraphicsImportDraw( gi);// draw into gworld
	
	LGWorld *world = new LGWorld(theGWorldPtr);
	if (world) {
		result = initWithGWorld(world);
		delete world;
	}
	
	
	SetGWorld (saveWorld, saveDevice); //ensure gdevice is restored
	
	if (result != noErr)
	{
		dprintf("Failed to import image\n");
		UnlockPixels( thePixMapHdl);  // cleanup
		goto exit;
	}
	
exit:
	if (theGWorldPtr)
		::DisposeGWorld( theGWorldPtr);

	if (gi)
		::CloseComponent( gi);

	
	if (handle)
		::DisposeHandle(handle);
	
	return result;
}

UInt32 CQTImage::NextPowerOf2(UInt32 in)
{
     in -= 1;

     in |= in >> 16;
     in |= in >> 8;
     in |= in >> 4;
     in |= in >> 2;
     in |= in >> 1;

     return in + 1;
}

