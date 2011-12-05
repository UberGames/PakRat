/* 
	.h

	Author:			Tom Naughton
	Description:	<describe the CFileArchive class here>
*/

#ifndef CPak_H
#define CPak_H

#include "CFileArchive.h"



// Pak structures


typedef struct
{ 
	UInt8 magic[4];             // Name of the new WAD format = "PACK"
	long diroffset;              // Position of WAD directory from start of file
	long dirsize;                // Number of entries * 0x40 (64 UInt8)
} pakheader_t;


typedef struct
{ 
	UInt8 filename[0x38];       // Name of the file, Unix style, with extension,
	                           // 56 chars, padded with '\0'.
	long offset;                 // Position of the entry in PACK file
	long size;                   // Size of the entry in PACK file
} pakentry_t;

typedef struct char_link_s
{
	UInt8 data[256];
	struct char_link_s *next;
} char_link;

class CPakStream;
class LFileStream;

class CPak: public CFileArchive
{

public:

	CPak(CFileArchive *inParent);
	CPak(FSSpec inFileSpec);
	virtual ~CPak();

	virtual Boolean init();
	virtual long SizeOfItemAtIndex(const char *itemName, UInt32 inIndex);
	virtual CPakStream *LoadItemAtIndex(CFileArchive *inParent, const char *itemName, UInt32 inIndex);

private:

	// these values are only valid for the root package
	int				_num_pak_entries;
	pakentry_t  	*_pak_entries;
	long 			_diroffset;
	long 			_dirsize;
	pakheader_t 	_pakheader;
};


#endif	// CPak_H
