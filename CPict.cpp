/* 
	CPict.cpp

	Author:			Tom Naughton
	Description:	<describe the CPict class here>
*/

#include "CPict.h"



CPict::CPict() : CImage()
{
}


CPict::~CPict()
{
}

CPict::CPict(PicHandle picH) : CImage()
{
	_bounds = (*picH)->picFrame;
	_pic = picH;
}

CPict::CPict(LGWorld *gWorld) : CImage()
{
	RgnHandle theCurrentClipRgn = ::NewRgn();
	::GetClip(theCurrentClipRgn);				// Save the current clip region

	gWorld->GetBounds(_bounds);
	
	Rect			thePICTRect		= {0, 0, _bounds.bottom, _bounds.right, };
	OpenCPicParams	myOpenParams	= { {0,0, _bounds.bottom, _bounds.right }, _72dpi, _72dpi, -2, 0, 0};
	PicHandle		thePicH			= ::OpenCPicture(&myOpenParams);
	
	if (gWorld) {
		::ClipRect(&thePICTRect);
		GrafPtr thePort;
		::GetPort(&thePort);
		
	//	gWorld->BeginDrawing();
		gWorld->CopyImage(thePort, _bounds, srcCopy);
	//	gWorld->EndDrawing();
	}
	
	::ClosePicture();
	::SetClip(theCurrentClipRgn);				// Restore the clip region
	::DisposeRgn(theCurrentClipRgn);	

	_pic =  thePicH;
}

OSErr CPict::initImage(CPakStream *inItem)
{
#pragma unused (inItem)
	return noErr;
}

CPict *CPict::CPict::copy()
{
	CPict *theClone = new CPict();
	Handle h = (Handle)_pic;
	::HandToHand(&h);
	theClone->_bounds = (*_pic)->picFrame;
	theClone->_pic = (PicHandle)h;
	return theClone;
}

LGWorld *CPict::CreateGWorld(void)
{
	_bounds = (*_pic)->picFrame;
	GWorldPtr worldPtr;
	
	QDErr err = ::NewGWorld(&worldPtr, 32, &_bounds, 0, 0, 0);
	if (err != noErr)
		return nil;
	
	// prepare gworld for drawing
	CGrafPtr	currentPort;
	GDHandle	currentDevice;
	::GetGWorld( &currentPort, &currentDevice );
	::SetGWorld( worldPtr, nil );
	PixMapHandle pixMap = ::GetGWorldPixMap( worldPtr );
	::LockPixels(pixMap);
	::EraseRect( &(**pixMap).bounds );
	
	::DrawPicture(_pic, &(*_pic)->picFrame);
	
	// finished drawing in gworld	
	::UnlockPixels( pixMap );
	::SetGWorld( currentPort, currentDevice );
	return new LGWorld(worldPtr);
}


PicHandle CPict::CreatePict()
{
	if (_pic) {
		Handle hand = (Handle)_pic;
		::HandToHand(&hand);
		return (PicHandle) hand;
	}else
		return CImage::CreatePict();
}


void CPict::SaveToFile(const FSSpec &outFSSpec)
{
	unsigned char header[512];
	char *picdata = *((Handle)_pic);
	UInt32 size = ::GetHandleSize((Handle)_pic);
	SInt32	ioByteCount;
	short	count;
	
	for (count = 0; count < 512; count++)
		header[count] = 0x00;
	
	::HLock((Handle)_pic);
	// FIXME: delete file if it exists
	//FSDelete(outFSSpec.name,outFSSpec.vRefNum);
	LFileStream *file = new LFileStream(outFSSpec);
	file->CreateNewDataFile('ogle','PICT',smSystemScript);
	file->OpenDataFork(fsRdWrPerm);
	ioByteCount = 512;
	file->PutBytes(header, ioByteCount);
	ioByteCount = size;
	file->PutBytes(picdata, ioByteCount);
	file->CloseDataFork();
	::HUnlock((Handle)_pic);
	delete file;
}

