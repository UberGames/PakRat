/* 
	CWad.h

	Author:			Tom Naughton
	Description:	<describe the CWad class here>
*/

#ifndef CWad_H
#define CWad_H

#include "CFileArchive.h"

// Wod structures

typedef struct
{
    // Should be "IWAD" or "PWAD".
    char		identification[4];		
    int			numlumps;
    int			infotableofs;
    
} wadinfo_t;


typedef struct
{
    int			filepos;
    int			size;
    char		name[8];
    
} filelump_t;

class CWad: public CFileArchive
{
public:

	CWad(CFileArchive *inParent);
	CWad(FSSpec inFileSpec);
	virtual ~CWad();

	virtual Boolean init();
	virtual long SizeOfItemAtIndex(string itemName, UInt32 inIndex);
	virtual CPakStream *LoadItemAtIndex(CFileArchive *inParent, string itemName, UInt32 inIndex);

private:

	wadinfo_t 		_wadheader;
	filelump_t  	*_wad_entries;
	long 			_diroffset;
	long 			_dirsize;
	long 			_numlumps;
};

#endif	// CWad_H
