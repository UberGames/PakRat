/* 
	CQTImage.h

	Author:			Tom Naughton
	Description:	uses QuickTime to open images

*/

#ifndef CQTImage_H
#define CQTImage_H

#include "CGLImage.h"
#include "CImage.h"


class CQTImage: public CGLImage
{

public:

	CQTImage();
	virtual ~CQTImage();
	virtual OSErr initImage(CPakStream *inItem);

	OSErr initImageFromData(char *p, UInt32 size, OSType  componentSubType);
	GraphicsImportComponent GetGraphicsImporterForFileType(int type);
	UInt32 NextPowerOf2(UInt32 in);

};

#endif	// CQTImage_H
