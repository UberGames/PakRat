/* 
	CImage.cpp

	Author:			Tom Naughton
	Description:	<describe the CImage class here>
*/

#include <UGWorld.h>


#include "CImage.h"
#include "CGLImage.h"
#include "CPakStream.h"
#include "utilities.h"
#include "CPakRatApp.h"



CImage::CImage()
{
	::SetRect(&_bounds, 0, 0, 0, 0);
	_depth = 32;
}


CImage::~CImage()
{	
}


PicHandle CImage::CreatePict()
{

	LGWorld *gWorld = CreateGWorld();
	RgnHandle theCurrentClipRgn = ::NewRgn();
	::GetClip(theCurrentClipRgn);				// Save the current clip region
	
	Rect			thePICTRect		= {0, 0, _bounds.bottom, _bounds.right};
	OpenCPicParams	myOpenParams	= { {0,0, _bounds.bottom, _bounds.right}, _72dpi, _72dpi, -2, 0, 0};
	PicHandle		thePicH			= ::OpenCPicture(&myOpenParams);
	
	if (gWorld) {
		::ClipRect(&thePICTRect);
		GrafPtr thePort;
		::GetPort(&thePort);
		gWorld->CopyImage(thePort, _bounds, srcCopy);
		delete gWorld;
	}
	
	::ClosePicture();
	::SetClip(theCurrentClipRgn);				// Restore the clip region

	return thePicH;
}

LGWorld *CImage::CreateGWorld()
{
	return nil;
}

