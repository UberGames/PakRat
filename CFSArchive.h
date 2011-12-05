/* 
	CFSArchive.h

	Author:			Tom Naughton
	Description:	<describe the CFSArchive class here>
*/

#ifndef CFSArchive_H
#define CFSArchive_H

#include "CFileArchive.h"


class CFSArchive: public CFileArchive
{
public:
	CFSArchive(CFileArchive *inParent);
	CFSArchive(FSSpec inFileSpec);
	virtual ~CFSArchive();
	
	virtual Boolean init();
	virtual long SizeOfItemAtIndex(const char *itemName, UInt32 inIndex);
	virtual CPakStream *LoadItemAtIndex(CFileArchive *inParent, const char *itemName, UInt32 inIndex);
	virtual CFileArchive *subPackage(const char *name);

	virtual database_type *getSubpackages();	
	virtual file_database_type *getFiles();

	static Boolean parentDirectoryOfAncestorWithName(const char *directory, FSSpec &inFileSpec);

private:

	void traverseDirectory(FSSpec inFileSpec);
	Boolean _loaded;

};

#endif	// CFSArchive_H
