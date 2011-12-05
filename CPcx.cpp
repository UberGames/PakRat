/* 
	CPcx.cpp

	Author:			Tom Naughton
	Description:	This is a very skanky class that needs to be rewritten
	
*/

#include "CPcx.h"
#include "utilities.h"
#include "CPakRatApp.h"
#include "CPakRatApp.h"
#include "AppConstants.h"

#define	TOTALCOLORS	255



CPcx::CPcx() : CGLImage()
{
}


CPcx::~CPcx() 
{
}



OSErr CPcx::initImage(CPakStream *inItem)
{
	OSErr myErr = noErr;
	char * p = inItem->getData("pcx raw data");
	long size = inItem->getSize();
	
	if (p) {
		myErr = initImageFromPCX(p, size);
		CMemoryTracker::safeFree(p);
	} else {
		myErr = kError_outofmemory;
	}
	return myErr;
}




OSErr CPcx::initImageFromPCX(char *dataStart, UInt32 dataSize)
{
	OSErr myErr = noErr;
	unsigned char *palette;
	long filesize=0;
	char chartemp=0;
	unsigned char repeat=0, color=0;
	int x=0,y=0;
	short *ip;
	char *p;
	
	//
	// Get file length and make sure file is large enough to be a PCX
	//
	if (dataSize < 128) {
		myErr = kError_corruptdata;
		goto fail;
	}

	//
	// Check PCX manufacturer
	//
	p = dataStart;
	if (*p != 0xA) {
		myErr = kError_badversion;
		goto fail;
	}

		
	//
	// Get image dimensions
	//
	p = dataStart + 8;
	ip = (short*) p;
	
	UInt32 width = swapShort(*ip++);
	UInt32 height = swapShort(*ip++);

	//
	// Load 256 color palette.  Quit if not 256 colors (didn't bother adding support for 16 color pcx)
	//
	p = dataStart + dataSize - 769;
	if (!*p++ == 0xC) {
		myErr = kError_missingpalette;
		goto fail;
	}
	palette = (unsigned char*) p;
	p = dataStart + 128;
	return initImageFromPCXWithPalette(p, dataSize - (p-dataStart), width + 1, height + 1, palette);

fail:

	dprintf("could not load pcx\n");
	return myErr;	
}

OSErr CPcx::initImageFromPCXWithPalette(char *dataStart, UInt32 dataSize, UInt32 inWidth, UInt32 inHeight, UInt8 *palette, Boolean decoderle)
{
#pragma unused (dataSize)
	OSErr myErr = noErr;
	unsigned char *indexdata;
	long filesize=0;
	char chartemp=0;
	int x=0,y=0;
	char *p = dataStart;

	::SetRect(&_bounds, 0, 0, inWidth, inHeight);
	
	//
	// Allocate mem for data
	//
	indexdata = (unsigned char*) CMemoryTracker::safeAlloc(inWidth * inHeight, 1, "pcx indexdata");
	if (!indexdata) {
		myErr = kError_outofmemory;
		goto fail;
	}

	//
	// Decode the image
	//
	unsigned char repeat=0, color=0;
	for (y=0; y < inHeight; y++)
		for (x=0; x < inWidth; x++) {
		
			if (!repeat) {
				color = *p++;
				if ( decoderle && (color & 0xC0)==0xC0) {
					repeat = color & 0x3F;
					color = *p++;
				} else {
					repeat=1;
				}
			}
			repeat--;
			
			indexdata[x+y*inWidth] = color;
		}

	_data = (unsigned char *)CMemoryTracker::safeAlloc(inWidth * inHeight * 3,1, "pcx _data");
	if (!_data) {
		myErr = kError_outofmemory;
		goto fail;
	}

	//convert to rgb format
	int pixels = inWidth * inHeight;
	for (x=0; x< pixels; x++) {
		_data[(x)*3] = palette[ ((int)indexdata[x])*3 ];
		_data[(x)*3 + 1] = palette[ ((int)indexdata[x])*3 + 1];
		_data[(x)*3 + 2] = palette[ ((int)indexdata[x])*3 + 2];
	}
	
	CMemoryTracker::safeFree(indexdata);
	
	return myErr;

fail:

	if (_data)
		CMemoryTracker::safeFree(_data);

	dprintf("could not load pcx\n");
	return myErr;
}


