/* 
	CPcx.h

	Author:			Tom Naughton
	Description:	converts Pcx files to RGB

*/

#ifndef CPcx_H
#define CPcx_H

#include "CGLImage.h"
#include "CImage.h"


class CPcx: public CGLImage
{

public:

	CPcx();
	virtual ~CPcx();
	virtual OSErr initImage(CPakStream *inItem);
	virtual OSErr initImageFromPCX(char *dataStart, UInt32 dataSize);
	virtual OSErr initImageFromPCXWithPalette(char *dataStart, UInt32 dataSize, UInt32 inWidth, UInt32 inHeight, UInt8 *palette, Boolean decoderle = true);

};

#endif	// CPcx_H
