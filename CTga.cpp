/* 
 Code based on:

 ZImageLoaderTGA 1.0   97/3/5
 Loader for Truevision Targa (TGA) images.
 Copyright (c) 1997, Marcel Schoen and Andre Pinheiro

 Code by Marcel Schoen (Marcel.Schoen@village.ch)
 Documentation/Arrangements by Andre Pinheiro (l41325@alfa.ist.utl.pt)

 All rights reserved.

 Published by JavaZine - Your Java webzine

 Permission to use, copy, modify, and distribute this software
 and its documentation for NON-COMMERCIAL or COMMERCIAL purposes
 and without fee is hereby granted provided that the copyright
 and "published by" messages above appear in all copies.
 We will not be held responsible for any unwanted effects due to
 the usage of this software or any derivative.
 No warrantees for usability for any specific application are
 given or implied.
*/


#include "CTga.h"
#include "CGLImage.h"
#include "utilities.h"
#include "CPakRatApp.h"
#include "AppConstants.h"


CTga::CTga() : CGLImage()
{
}


CTga::~CTga()
{
}


OSErr CTga::initImage(CPakStream *inItem)
{
	int                     columns, rows, numPixels;
	Byte                    *pixbuf;
	int                     row, column;
	Byte                    *targa_rgba = 0;
	TargaHeader             targa_header;

	OSErr myErr = noErr;
	char *imageData = (char*) inItem->getData("raw tga data");
	if (!imageData) { 
		myErr = kError_outofmemory;
		goto fail;
	}
	_hasAlpha = false;
	char *p = imageData;


	targa_header.id_length = nextByte(p);
	targa_header.colormap_type = nextByte(p);
	targa_header.image_type = nextByte(p);

	targa_header.colormap_index = swapShort(nextShort(p));
	targa_header.colormap_length = swapShort(nextShort(p));
	targa_header.colormap_size = nextByte(p);
	targa_header.x_origin = swapShort(nextShort(p));
	targa_header.y_origin = swapShort(nextShort(p));
	targa_header.width = swapShort(nextShort(p));
	targa_header.height = swapShort(nextShort(p));
	targa_header.pixel_size = nextByte(p);
	targa_header.attributes = nextByte(p);

	if (targa_header.image_type!=2 
		&& targa_header.image_type!=10) {
		dprintf ("LoadTGA: Only type 2 and 10 targa RGB images supported\n");
		myErr = kError_badversion;
		goto fail;
	}

	if (targa_header.colormap_type !=0 
		|| (targa_header.pixel_size!=32 && targa_header.pixel_size!=24)) {
		dprintf ("Texture_LoadTGA: Only 32 or 24 bit images supported (no colormaps)\n");
		myErr = kError_badversion;
		goto fail;       
	}

	columns = targa_header.width;
	rows = targa_header.height;
	numPixels = columns * rows;

	::SetRect(&_bounds, 0, 0, columns, rows);
	_hasAlpha = (targa_header.pixel_size == 32);
	int  texelSize = 4;
	if (!_hasAlpha)
		texelSize--;
	targa_rgba = (unsigned char*) CMemoryTracker::safeAlloc (numPixels*texelSize, sizeof(char),"tga data");
	if (!targa_rgba) {
		myErr = kError_outofmemory;
		goto fail;
	}

	if (targa_header.id_length != 0)
		p += targa_header.id_length;


	if (targa_header.image_type==2) {  // Uncompressed, RGB images
		for(row=rows-1; row>=0; row--) {
			pixbuf = targa_rgba + row*columns*texelSize;
			for(column=0; column<columns; column++) {
				unsigned char red,green,blue,alphabyte;
				switch (targa_header.pixel_size) {
				
					case 24:
						blue = nextByte(p);
						green = nextByte(p);
						red = nextByte(p);
						*pixbuf++ = red;
						*pixbuf++ = green;
						*pixbuf++ = blue;
						break;
						
					case 32:
						blue = nextByte(p);
						green = nextByte(p);
						red = nextByte(p);
						alphabyte = nextByte(p);
						*pixbuf++ = red;
						*pixbuf++ = green;
						*pixbuf++ = blue;
						*pixbuf++ = alphabyte;
						break;
				}
			}
		}
	} else if (targa_header.image_type==10) {   // Runlength encoded RGB images
		unsigned char red,green,blue,alphabyte,packetHeader,packetSize,j;
		for(row=rows-1; row>=0; row--) {
			pixbuf = targa_rgba + row*columns*texelSize;
			for(column=0; column<columns; ) {
				packetHeader=nextByte(p);
				packetSize = 1 + (packetHeader & 0x7f);
				if (packetHeader & 0x80) {        // run-length packet
					switch (targa_header.pixel_size) {
					
						case 24:
							blue = nextByte(p);
							green = nextByte(p);
							red = nextByte(p);
							alphabyte = 255;
							break;
							
						case 32:
							blue = nextByte(p);
							green = nextByte(p);
							red = nextByte(p);
							alphabyte = nextByte(p);
							break;
					}

					for(j=0;j<packetSize;j++) {
						*pixbuf++=red;
						*pixbuf++=green;
						*pixbuf++=blue;
						if (_hasAlpha)
							*pixbuf++=alphabyte;
						column++;
						if (column==columns) { // run spans across rows
							column=0;
							if (row>0)
							row--;
							else
							goto breakOut;
							pixbuf = targa_rgba + row*columns*texelSize;
						}
					}
				} else {                            // non run-length packet
					for(j=0;j<packetSize;j++) {
						switch (targa_header.pixel_size) {
						
							case 24:                                                        
								blue = nextByte(p);
								green = nextByte(p);
								red = nextByte(p);
								*pixbuf++ = red;
								*pixbuf++ = green;
								*pixbuf++ = blue;
								break;
								
							case 32:
								blue = nextByte(p);
								green = nextByte(p);
								red = nextByte(p);
								alphabyte = nextByte(p);
								*pixbuf++ = red;
								*pixbuf++ = green;
								*pixbuf++ = blue;
								*pixbuf++ = alphabyte;
								break;
						}
						column++;
						if (column==columns) { // pixel packet run spans across rows
							column=0;
							if (row>0)
							row--;
							else
							goto breakOut;
							pixbuf = targa_rgba + row*columns*texelSize;
						}                                               
					}
				}
			}
			breakOut:;
		}
	}

	_data = targa_rgba;
	CMemoryTracker::safeFree(imageData);
	return myErr;

fail:

	if (targa_rgba)
		CMemoryTracker::safeFree(targa_rgba);
	if (imageData)
		CMemoryTracker::safeFree(imageData);
	return myErr;
}


