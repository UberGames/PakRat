/* 
	CFtx.cpp

	Author:			Tom Naughton
	Description:	This is a very skanky class that needs to be rewritten
	
*/

#include "CFtx.h"
#include "utilities.h"
#include "CPakRatApp.h"
#include "AppConstants.h"

CFtx::CFtx() : CGLImage()
{
}


CFtx::~CFtx() 
{
}


OSErr CFtx::initImage(CPakStream *inItem)
{
	OSErr myErr = noErr;
	char * p = inItem->getData("ftx raw data");
	long size = inItem->getSize();
	
	if (p) {
		myErr = initImageFromFtx(p, size);
		CMemoryTracker::safeFree(p);
	} else {
		myErr = kError_outofmemory;
	}
	return myErr;
}

OSErr CFtx::initImageFromFtx(char *dataStart, UInt32 dataSize)
{
#pragma unused (dataSize)
	OSErr myErr = noErr;
	ftx_t *header = (ftx_t*)dataStart;
	unsigned char *indexdata;
	
	// swap header
	header->height = swapLong(header->height);
	header->width = swapLong(header->width);
	

	// Get image dimensions
	_hasAlpha = true;
	::SetRect(&_bounds, 0, 0, header->width, header->height);
	_data = (unsigned char *)CMemoryTracker::safeAlloc(header->width * header->height, 4, "ftx _data");
	if (!_data) {
		myErr = kError_outofmemory;
		goto fail;
	}
	
	//convert to rgb format
	indexdata = (unsigned char*) &header->data;
	int pixels = header->width*header->height;
	for (int x=0; x< pixels; x++) {
		_data[(x)*4] = *indexdata++;
		_data[(x)*4 + 1] = *indexdata++;
		_data[(x)*4 + 2] = *indexdata++;
		_data[(x)*4 + 3] = *indexdata++;
	}

fail:		
	return myErr;	
}

