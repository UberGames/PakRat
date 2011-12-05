/* 
	CWal.cpp

	Author:			Tom Naughton
	Description:	This is a very skanky class that needs to be rewritten
	
*/

#include "CWal.h"
#include "utilities.h"
#include "CPakRatApp.h"
#include "AppConstants.h"

CWal::CWal() : CGLImage()
{
}


CWal::~CWal() 
{
}


OSErr CWal::initImage(CPakStream *inItem)
{
	OSErr myErr = noErr;
	char * p = inItem->getData("wal raw data");
	long size = inItem->getSize();
	
	if (p) {
		myErr = initImageFromWal(p, size);
		CMemoryTracker::safeFree(p);
	} else {
		myErr = kError_outofmemory;
	}
	return myErr;
}

OSErr CWal::initImageFromWal(char *dataStart, UInt32 dataSize)
{
#pragma unused (dataSize)
	OSErr myErr = noErr;
	wal_t *header = (wal_t*)dataStart;
	unsigned char *indexdata;
	
	// swap header
	header->height = swapLong(header->height);
	header->width = swapLong(header->width);
	header->mip1_offs = swapLong(header->mip1_offs);
	header->mip2_offs = swapLong(header->mip2_offs);
	header->mip3_offs = swapLong(header->mip3_offs);
	header->mip4_offs = swapLong(header->mip4_offs);
	
	// choose a palette
	// FIXME: should there be a pref for this?
	UInt8 *palette = (UInt8 *)quake2Palette;

	// Get image dimensions
	::SetRect(&_bounds, 0, 0, header->width, header->height);
	_data = (unsigned char *)CMemoryTracker::safeAlloc(header->width * header->height, 3, "wal _data");
	if (!_data) {
		myErr = kError_outofmemory;
		goto fail;
	}
	
	//convert to rgb format
	indexdata = (unsigned char*) dataStart + header->mip1_offs;
	int pixels = header->width*header->height;
	for (int x=0; x< pixels; x++) {
		_data[(x)*3] = palette[ ((int)indexdata[x])*3 ];
		_data[(x)*3 + 1] = palette[ ((int)indexdata[x])*3 + 1];
		_data[(x)*3 + 2] = palette[ ((int)indexdata[x])*3 + 2];
	}

fail:		
	return myErr;	
}

