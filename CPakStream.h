/* 
	CPakStream.h

	Author:			Tom Naughton
	Description:	<describe the CPakStream class here>
*/

#ifndef CFileArchiveItem_H
#define CFileArchiveItem_H

#include <iostream.h>
#include "CFileArchive.h"


class CPakStream
{
public:
	CPakStream(FSSpec inFileSpec);
	CPakStream(CFileArchive *inArchive, string inName, long inIndex, Handle inHandle);
	virtual ~CPakStream();
	
	string 			dataType();
	StringPtr 		itemTitle();
	string 			pathName();
	string 			name() { return _name; }
	CFileArchive 	*rootArchive() { if (_archive) return _archive->rootArchive(); else return nil; }
	void			setArchive(CFileArchive *archive) { _archive = archive; };
	Boolean 		getBytes(unsigned long nBytes, void *bytes);
	void 			seek(UInt32 location);
	long 			getSize() { return _size; };
	char 			*getData(char *name);
	char 			*address();
	OSErr			copyDataToFile(FSSpec *spec);
	UInt32			cursorPos() { return _dataCursor; };
	void 			resetCursor() { _dataCursor = 0; };
	
	
	Byte 		getByte();
	UInt16		getShort();
	UInt32		getLong();
	float 		getFloat();

	static void dumpHeap();

private:

	typedef set<CPakStream*> item_set;
	typedef item_set::iterator item_set_iterator;

	static item_set _itemSet;

	typedef TArray<CPakStream*> item_array;
	static item_array		_allocatedItems;

	CFileArchive *_archive;
	string _name;
	long _index;
	
	Handle	_dataHandle;
	UInt32 	_dataCursor;
	UInt32 	_size;
	void LoadFromPak();

};

inline Byte CPakStream::getByte()
{
	Byte result;
	getBytes(sizeof(Byte), &result);
	return result;
}

inline UInt16 CPakStream::getShort()
{
	UInt16 result;
	getBytes(sizeof(UInt16), &result);
	return result;
}

inline UInt32 CPakStream::getLong()
{
	UInt32 result;
	getBytes(sizeof(UInt32), &result);
	return result;
}

inline float CPakStream::getFloat()
{
	float result;
	getBytes(sizeof(float), &result);
	return result;
}



#endif	// CFileArchiveItem_H
