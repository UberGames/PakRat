/* 
	CTga.h

	Author:			Tom Naughton
	Description:	<describe the CTga class here>
*/

#ifndef CTga_H
#define CTga_H

#include <FixMath.h>
#include <ImageCompression.h>
#include <Movies.h>
#include <QuickTimeComponents.h>

#include "CGLImage.h"

struct TargaHeader {
	unsigned char   id_length, colormap_type, image_type;
	unsigned short  colormap_index, colormap_length;
	unsigned char   colormap_size;
	unsigned short  x_origin, y_origin, width, height;
	unsigned char   pixel_size, attributes;
};


class CTga: public CGLImage
{
public:
	CTga();
	virtual ~CTga();
	
	virtual OSErr initImage(CPakStream *inItem); // class specific initialization

protected:
	
	void ScanHeader(char *p, Rect &rect, UInt16 &depth);
	
};

#endif	// CTga_H
