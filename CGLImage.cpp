/* 
	CGLImage.cpp

	Author:			Tom Naughton
	Description:	<describe the CGLImage class here>
*/


#include "CImage.h"
#include "CGLImage.h"
#include "CShader.h" // for SHADER_CLAMP
#include "AppConstants.h"

CGLImage::CGLImage() : CImage()
{
	_bind = -1;
	_hasAlpha = false;
	_data = nil;
}

CGLImage::CGLImage(LGWorld *gWorld) : CImage()
{
	_bind = -1;
	_hasAlpha = false;
	_data = nil;
	_flags = 0;
	
	initWithGWorld(gWorld);
}



CGLImage::~CGLImage()
{
	if (_bind != -1)
		glDeleteTextures(1, &_bind);
	if (_data)
		CMemoryTracker::safeFree(_data);
}

CGLImage *CGLImage::copy()
{
	CGLImage *theClone = new CGLImage();
	theClone->_bounds = _bounds;
	theClone->_hasAlpha = _hasAlpha;
	theClone->_bind = -1;
	
	int bytes = 3;
	if (_hasAlpha) bytes++;	
	int size  = (width()+1) * (height()+1) * bytes;
	
	theClone->_data = (unsigned char *)CMemoryTracker::safeAlloc(size, 1, "image_rgb_t _data");
	memcpy(theClone->_data, _data, size);
	
	return theClone;
}

OSErr CGLImage::initImage(CPakStream *inItem)
{
#pragma unused (inItem)
	OSErr myErr = noErr;
	return myErr;
}

OSErr CGLImage::initWithGWorld(LGWorld *inWorld)
{
	OSErr myErr = noErr;
	GWorldPtr world = inWorld->GetMacGWorld();
	PixMapHandle pixMap = ::GetGWorldPixMap(world);
	_bounds = (**pixMap).bounds;
	_depth = (**pixMap).pixelSize;
	
	if (pixMap) {
		if (_depth == 16)
			myErr = initWith16BitGWorld(inWorld);
		else if (_depth == 24)
			myErr = initWith32BitGWorld(inWorld);
		else 
			myErr = initWith32BitGWorld(inWorld);
	}
	return myErr;		
}


OSErr CGLImage::initWith16BitGWorld(LGWorld *inWorld)
{
	OSErr 		myErr = noErr;

	// prepare gworld for drawing
	CGrafPtr	currentPort;
	GDHandle	currentDevice;
	::GetGWorld( &currentPort, &currentDevice );
	GWorldPtr world = inWorld->GetMacGWorld();
	::SetGWorld( world, nil );
	PixMapHandle pixMap = ::GetGWorldPixMap( world );
	::LockPixels(pixMap);

	UInt32 rowBytes = (**pixMap).rowBytes & 0x7FFF;
	UInt32 index;
	UInt32 x,y;
	UInt16 pixel;
	UInt8 *c;
	UInt8 *cy = (UInt8 *)::GetPixBaseAddr(pixMap);

	_data = (unsigned char *)CMemoryTracker::safeAlloc((width()+1) * (height()+1) * 3,1, "image_rgb_t _data");
	if (!_data) {
		myErr = kError_outofmemory;
		goto fail;
	}

	for(index = 0, y = 0; y < height(); y++) {
		for(x = 0; x < width(); x++) {

			// setup up pixel cursor
			
			c = cy + (y * rowBytes + x*2);
			pixel = *((UInt16*)c);
			
			_data[(index)*3] = (pixel >> 10 & 0x1F) << 3;
			_data[(index)*3 + 1] = (pixel >> 5 & 0x1F) << 3;
			_data[(index)*3 + 2] = (pixel & 0x1F) << 3;

			index++;
		}
	}

	// finished drawing in gworld	
	::UnlockPixels( pixMap );
	::SetGWorld( currentPort, currentDevice );

	return myErr;
	
fail:

	if (_data)
		CMemoryTracker::safeFree(_data);
	dprintf("initWith16BitGWorld failed!\n");
	
	::UnlockPixels( pixMap );
	::SetGWorld( currentPort, currentDevice );
	
	return myErr;
}

OSErr CGLImage::initWith32BitGWorld(LGWorld *inWorld)
{
	OSErr 		myErr = noErr;

	// prepare gworld for drawing
	CGrafPtr	currentPort;
	GDHandle	currentDevice;
	::GetGWorld( &currentPort, &currentDevice );
	GWorldPtr world = inWorld->GetMacGWorld();
	::SetGWorld( world, nil );
	PixMapHandle pixMap = ::GetGWorldPixMap( world );
	::LockPixels(pixMap);

	UInt32 rowBytes = (**pixMap).rowBytes & 0x7FFF;
	UInt32 index;
	UInt32 x,y;
	UInt32 pixel;
	UInt8 *c;
	UInt8 *cy = (UInt8 *)::GetPixBaseAddr(pixMap);


	_data = (unsigned char *)CMemoryTracker::safeAlloc((width()+1) * (height()+1) * 3,1, "image_rgb_t _data");
	if (!_data) {
		myErr = kError_outofmemory;
		goto fail;
	}

	for(index = 0, y = 0; y < height(); y++) {
		for(x = 0; x < width(); x++) {

			// setup up pixel cursor
			
			c = cy + (y * rowBytes + x*4);
			pixel = *((UInt32*)c);
			
			_data[(index)*3] = (pixel >> 16 & 0xFF) ;
			_data[(index)*3 + 1] = (pixel >> 8 & 0xFF) ;
			_data[(index)*3 + 2] = (pixel & 0xFF) ;

			index++;
		}
	}

	// finished drawing in gworld	
	::UnlockPixels( pixMap );
	::SetGWorld( currentPort, currentDevice );

	return myErr;

fail:

	dprintf("initWith16BitGWorld failed!\n");
	
	if (_data)
		CMemoryTracker::safeFree(_data);
	return myErr;
}

UInt32 CGLImage::GenTexture()
{
	static long bindSeed = 1;
	return bindSeed++;
}

void CGLImage::uploadGLImage(int lodbias, UInt32 flags)
{
	int format = _hasAlpha ? GL_RGBA : GL_RGB;
	dprintf("CGLImage::uploadGLImage\n");
	
	_flags = flags;
	
// This is a workaround for textures getting deleted from the wrong context
//	glGenTextures(1, &_bind);
	_bind = GenTexture();
	
	
    glBindTexture( GL_TEXTURE_2D, _bind );
    
    // Scale image down for biased level of detail (lowered texture quality) 
    // Need a pref for this
	lodbias = 0;
    if (lodbias > 0 && _data)  {
	    int w = width(), h = height();
    	int newWidth =  w / (1 << lodbias);
    	int newHeight = h  / (1 << lodbias);
	    int size = newWidth * newHeight * (format == GL_RGB ? 3 : 4);
		UInt8 *tex = (unsigned char *) CMemoryTracker::safeAlloc(1, size, "tex");
		if (tex) {
			gluScaleImage(format, w, h, GL_UNSIGNED_BYTE, _data,  newWidth, newHeight, GL_UNSIGNED_BYTE, tex);
			CMemoryTracker::safeFree(_data);
			_data = tex;   
	    	::SetRect(&_bounds, 0, 0, newWidth, newHeight);
		}
    }
    
	setupTexParams(_flags);
	Boolean nomipmaps = false; // flags & TEXFILE_NOMIPMAPS;
    if (nomipmaps) {
		glTexImage2D( GL_TEXTURE_2D, 0, 4, width(), height(), 0, format, GL_UNSIGNED_BYTE, _data );
    } else {
		gluBuild2DMipmaps(GL_TEXTURE_2D, 4, width(), height(), format, GL_UNSIGNED_BYTE, _data);
    }
}

void CGLImage::setupTexParams(UInt32 flags)
{
	Boolean nomipmaps = false; // flags & TEXFILE_NOMIPMAPS;

//	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	 
    if (flags & SHADER_CLAMP)
    {
    	dprintf("flags & SHADER_CLAMP\n");
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    } else  {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    if (nomipmaps) {
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);        
    } else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
    }
}

CGLImage *CGLImage::whiteImage()
{
	CGLImage *white = new CGLImage();
	::SetRect(&white->_bounds, 0, 0, 64, 64);
	white->_hasAlpha = 0;
	int size = white->width() * white->height();
	white->_data = (UInt8*) CMemoryTracker::safeAlloc(size, sizeof(RGB), "whiteImage");
	
	if (white->_data) {
		UInt8 *p = white->_data;
		for (int i = 0; i < size ; i++) {
			*p++ = 0xFF;
			*p++ = 0xFF;
			*p++ = 0xFF;
		}
		return white;
	}
	
fail:

	delete white;
	return 0;
}

void CGLImage::bindTexture()
{
	//setupTexParams(_flags);
	glBindTexture(GL_TEXTURE_2D, _bind);
}


LGWorld *CGLImage::CreateGWorld()
{
	// create gWorld
	Rect bounds = _bounds;
	GWorldPtr worldPtr;
	
	dprintf("CGLImage::CreateGWorld\n");
	QDErr err = ::NewGWorld(&worldPtr, 32, &bounds, 0, 0, 0);
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
	
	unsigned long rowBytes = (**pixMap).rowBytes & 0x7FFF;
	GLubyte *p = _data;
	char *c;
	char *cy = ::GetPixBaseAddr(pixMap);
	int x,y;
	
	for(y = 0; y < height(); y++) {
	
		c = cy;
		
		for(x = 0; x < width(); x++) {

			// convert pixel
			
			*c++ = 0; 
			*c++ = *p++; // red
			*c++ = *p++; // green
			*c++ = *p++; // blue
			
			if (_hasAlpha) 
				*(c-4) = *p++;
			
		}
		cy += rowBytes; 

	}
	::UnlockPixels(pixMap);
	::SetGWorld( currentPort, currentDevice );
	return new LGWorld(worldPtr);
}




