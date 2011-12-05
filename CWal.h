/* 
	CWal.h

	Author:			Tom Naughton
	Description:	converts Pcx files to RGB

*/

#ifndef CWal_H
#define CWal_H

#include "CGLImage.h"
#include "CImage.h"

typedef struct
{
	char	name[32];
	int		width;
	int		height;
	int		mip1_offs;
	int		mip2_offs;
	int		mip3_offs;
	int		mip4_offs;
	char	zeros[44];
	
	
} wal_t;

class CWal: public CGLImage
{

public:

	CWal();
	virtual ~CWal();
	virtual OSErr initImage(CPakStream *inItem);
	virtual OSErr initImageFromWal(char *dataStart, UInt32 dataSize);
};

#endif	// CWal_H
