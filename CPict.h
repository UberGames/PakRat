/* 
	CPict.h

	Author:			Tom Naughton
	Description:	<describe the CPict class here>
*/

#ifndef CPict_H
#define CPict_H

#include "CImage.h"


class CPict: public CImage
{

public:
	CPict();
	CPict(LGWorld *gWorld);
	CPict(PicHandle picH);
	virtual ~CPict();

	virtual OSErr 		initImage(CPakStream *inItem);
	virtual LGWorld 	*CreateGWorld();
	virtual PicHandle	getPicHandle() { return _pic; };
	virtual void		SaveToFile(const FSSpec &outFSSpec);
	virtual PicHandle 	CreatePict();
	virtual CPict 		*copy();

protected:

	PicHandle _pic;

};

#endif	// CPict_H
