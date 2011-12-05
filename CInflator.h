/* 
	CInflator.h

	Author:			Tom Naughton
	Description:	<describe the CInflator class here>
*/

#ifndef CInflator_H
#define CInflator_H

extern "C" {
	#include "zlib.h"
}

class CInflator
{
public:
	CInflator();
	virtual ~CInflator();

	void Inflate(Byte *compr, uLong comprLen, Byte *uncompr, uLong uncomprLen);
	void Test();
	
private:

	z_stream d_stream; /* decompression stream */
	void ReportError(int error);

};

#endif	// CInflator_H
