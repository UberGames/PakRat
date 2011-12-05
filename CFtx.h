/* 
	CFtx.h

	Author:			Tom Naughton
	Description:	support for ftx texture format

*/

#ifndef CFtx_H
#define CFtx_H

#include "CGLImage.h"
#include "CImage.h"

typedef struct
{
	UInt32		width;
	UInt32		height;
	UInt32 		data;
} ftx_t;


class CFtx: public CGLImage
{

public:

	CFtx();
	virtual ~CFtx();
	virtual OSErr initImage(CPakStream *inItem);
	virtual OSErr initImageFromFtx(char *dataStart, UInt32 dataSize);
};

#endif	// CFtx_H
