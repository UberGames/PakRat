/* 
	CImage.h

	Author:			Tom Naughton
	Description:	<describe the CImage class here>
*/

#ifndef CImage_H
#define CImage_H

#include <agl.h>
#include <glu.h>

#include "CPakStream.h"

class LGWorld;
class CGLImage;



typedef struct RGBIMAGE
{
	int width;
	int height;
	unsigned char *data;
} image_rgb_t;


class CImage
{
public:

	
	CImage();
	virtual ~CImage();
	
	virtual PicHandle 	CreatePict();
	virtual LGWorld 	*CreateGWorld();

	Rect 				bounds() { return _bounds; };
	UInt16  			depth() { return _depth; }
	int					width() { return _bounds.right - _bounds.left; }
	int					height() { return _bounds.bottom - _bounds.top; }
	
	
protected:

	Rect	_bounds;
	UInt16 _depth;

};

#define _72dpi 0x00480000

#endif	// CImage_H
