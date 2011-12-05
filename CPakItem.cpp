/* 
	CFileArchiveItem.cpp

	Author:			Tom Naughton
	Description:	<describe the CFileArchiveItem class here>
*/

#include "CFileArchive.h"
#include "CFileArchiveItem.h"
#include "utilities.h"

CFileArchiveItem::CFileArchiveItem(FSSpec inFileSpec)
{
	LFileStream *file = new LFileStream(inFileSpec);
	file->OpenDataFork(fsRdWrPerm);
	long pakSize = file->GetLength();
	char *pakData = (char*)malloc(pakSize);
	file->GetBytes(pakData, pakSize);
	file->CloseDataFork();
	delete file;

	_pak = nil;
	_name = p2cstr(inFileSpec.name);
	_index = -1;
	_data = pakData;
	_size = pakSize;
}

CFileArchiveItem::CFileArchiveItem(CFileArchive *inPak, string inName, long inIndex, char *inData, long inSize)
{
	_pak = inPak;
	_name = inName;
	_index = inIndex;
	_data = inData;
	_size = inSize;
}


CFileArchiveItem::~CFileArchiveItem()
{
}

string CFileArchiveItem::pathName()
{
	string path;
	
	if (_pak)
		path += _pak->pathName();
	path += _name;
	dprintf("CFileArchiveItem::pathName %s\n", path.c_str());
	return path;
}

static char _pTitle[1024];
StringPtr CFileArchiveItem::itemTitle()
{
	string title = pathName();
	strcpy(_pTitle, title.c_str());
	return (StringPtr) c2pstr(_pTitle);
	
}


string CFileArchiveItem::dataType()
{
	string package, file, extension;
	decomposeEntryName(_name, package, file, extension);
	return extension;
}
