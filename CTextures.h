/* 
	CTextures.h

	Author:			Tom Naughton
	Description:	<describe the CTextures class here>
*/

#ifndef CTextures_H
#define CTextures_H

#include <string>

class CGLImage;
class CFileArchive;
class CPakStream;

using std::string;

class CTextures
{
public:
	CTextures();
	virtual ~CTextures();

	static CGLImage *loadTexture(CFileArchive *pak, string texture);
	static CGLImage *loadTexture(CPakStream *pakItem);

};

#endif	// CTextures_H
