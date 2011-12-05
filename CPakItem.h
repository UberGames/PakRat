/* 
	CFileArchiveItem.h

	Author:			Tom Naughton
	Description:	<describe the CFileArchiveItem class here>
*/

#ifndef CFileArchiveItem_H
#define CFileArchiveItem_H

class CFileArchive;

class CFileArchiveItem
{
public:
	CFileArchiveItem(FSSpec inFileSpec);
	CFileArchiveItem(CFileArchive *inArchive, string inName, long inIndex, char *inData, long inSize);
	virtual ~CFileArchiveItem();
	string dataType();
	long size() { return _size; }

	StringPtr itemTitle();
	string pathName();

	char *_data;
	long _size;
	CFileArchive *_archive;
	string _name;
	long _index;
	
	string name() { return _name; }

protected:

	void LoadFromPak();

};

#endif	// CFileArchiveItem_H
