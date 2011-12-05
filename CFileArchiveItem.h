/* 
	CPakStream.h

	Author:			Tom Naughton
	Description:	<describe the CPakStream class here>
*/

#ifndef CPakStream_H
#define CPakStream_H

#include <iostream.h>
#include "CFileArchive.h"


class CPakStream
{
public:
	CPakStream(FSSpec inFileSpec);
	CPakStream(CFileArchive *inArchive, string inName, long inIndex, Handle inHandle);
	virtual ~CPakStream();
	string dataType();

	StringPtr itemTitle();
	string pathName();

	CFileArchive *_archive;
	string _name;
	long _index;
	
	string name() { return _name; }
	CFileArchive *fileArchive() { return _archive; }
	CFileArchive *rootArchive() { if (_archive) return _archive->rootArchive(); else return nil; }
	
	Boolean getBytes(unsigned long nBytes, void *bytes);
	long GetSize() { return _size; };
	char *getData(char *name);
	void resetCursor() { _dataCursor = 0; };

	static void dumpHeap();

private:

	typedef TArray<CPakStream*> item_array;
	static item_array		_allocatedItems;

	Handle	_dataHandle;
	UInt32 	_dataCursor;
	UInt32 	_size;
	void LoadFromPak();

};

#endif	// CPakStream_H
