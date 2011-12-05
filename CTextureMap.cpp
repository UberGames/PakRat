/* 
	CTextureMap.cpp

	Author:			Tom Naughton
	Description:	<describe the CTextureMap class here>
*/

#include "CImage.h"
#include "CTextureMap.h"



CTextureMap::CTextureMap()
{
	_pic = nil;
	_size = 0;
}


CTextureMap::~CTextureMap()
{
	if (_pic)
		::KillPicture(_pic);
}

Boolean CTextureMap::NextSize(UInt32 width, UInt32 height)
{
	Boolean result = false;
	if (width * height > _size) {
		_size = width * height;
		::SetRect(&_bounds, 0, 0, width, height);
		result = true;
		dprintf("NextSize %d,%d\n", width, height);
	}
	return result;
}


void CTextureMap::BeginMap()
{
	if (!_pic) {
		Rect			thePICTRect		= {0, 0, _bounds.bottom, _bounds.right};
		OpenCPicParams	myOpenParams	= { {0,0, _bounds.bottom, _bounds.right}, _72dpi, _72dpi, -2, 0, 0};

		_pic = ::OpenCPicture(&myOpenParams);
		StartClip();
		::PaintRect(&_bounds);
		EndClip();
	}
}

void CTextureMap::BeginTriangle()
{
	dprintf("BeginTriangle\n");
	_vertind = 0;
}

void CTextureMap::AddVertex(float x, float y)
{
	//dprintf("	AddVertex x=%f y=%f\n", x, y);
	StartClip();
	if (_vertind == 0) {
		::MoveTo(x * _bounds.right, y * _bounds.bottom);
		_firstX = x;
		_firstY = y;
	} else {
		::ForeColor(whiteColor);
		::LineTo(x * _bounds.right, y * _bounds.bottom);
	}
	_vertind++;
	EndClip();
}

void CTextureMap::EndTriangle()
{
	StartClip();
	::LineTo(_firstX * _bounds.right, _firstY * _bounds.bottom);
	EndClip();
}

void CTextureMap::EndMap()
{
	::ClosePicture();
}

void CTextureMap::StartClip()
{
	_theCurrentClipRgn = ::NewRgn();
	::GetClip(_theCurrentClipRgn);				// Save the current clip region
	::ClipRect(&_bounds);
}

void CTextureMap::EndClip()
{
	::SetClip(_theCurrentClipRgn);				// Restore the clip region
	::DisposeRgn(_theCurrentClipRgn);
}
