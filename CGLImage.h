/* 
	CGLImage.h

	Author:			Tom Naughton
	Description:	<describe the CGLImage class here>
*/

#ifndef CGLImage_H
#define CGLImage_H

#include <agl.h>
#include <glu.h>
#include "CImage.h"

typedef struct
{
	unsigned char Red;
	unsigned char Green;
	unsigned char Blue;
} RGB;

typedef struct
{
	unsigned char Red;
	unsigned char Green;
	unsigned char Blue;
	unsigned char Alpha;
} RGBA;

class CGLImage : public CImage
{

public:

	CGLImage();
	CGLImage(LGWorld *gWorld);
	virtual ~CGLImage();

	virtual OSErr 		initImage(CPakStream *inItem); // class specific initialization
	virtual OSErr 		initWithGWorld(LGWorld *world);
	virtual OSErr 		initWith32BitGWorld(LGWorld *world);
	virtual OSErr 		initWith16BitGWorld(LGWorld *world);
	CGLImage 			*copy();

	virtual LGWorld 	*CreateGWorld();
	virtual void 		setupTexParams(UInt32 flags = 0);
	virtual void 		uploadGLImage(int lodbias = 0, UInt32 flags = 0);
	virtual void		bindTexture();
	static UInt32 		GenTexture();
	static CGLImage		*whiteImage();
	
	Boolean 			hasAlpha() { return _hasAlpha; }
	GLubyte 			*data() { return _data; }	
	
	UInt32 				_bind;
	UInt32				_flags;


protected: 

	Boolean _hasAlpha;
	GLubyte *_data;

};

#endif	// CGLImage_H
