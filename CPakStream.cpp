/* 
	CPakStream.cpp

	Author:			Tom Naughton
	Description:	<describe the CPakStream class here>
*/

#include "CFileArchive.h"
#include "CPakStream.h"
#include "utilities.h"
#include "CPakRatApp.h"

CPakStream::item_set CPakStream::_itemSet;

CPakStream::CPakStream(FSSpec inFileSpec)
{
	_index = -1;
	_archive = nil;
	_dataCursor = 0;

	_itemSet.insert(this);
	LFileStream *file = new LFileStream(inFileSpec);
	char name[255];
	p2cstrcpy(name, inFileSpec.name);
	_name = name;
	
	file->OpenDataFork(fsWrPerm);
	
	_dataHandle = ::NewHandle(file->GetLength());
	if (!_dataHandle)
		goto fail;
		
	::HLock(_dataHandle);
	SInt32	length = file->GetLength();
	file->GetBytes(*_dataHandle, length);
	file->CloseDataFork();
	::HUnlock(_dataHandle);
	_size = ::GetHandleSize(_dataHandle);

	delete file;
	
//	dprintf("CPakStream loaded %s\n",pathName().c_str());
	return;
	
fail:
	delete file;
	dprintf("could not load pak item\n");
	return;
}

CPakStream::CPakStream(CFileArchive *inArchive, string inName, long inIndex, Handle inHandle)
{
	_itemSet.insert(this);
	_archive = inArchive;
	_name = inName;
	_index = inIndex;
	_dataHandle = inHandle;
	_dataCursor = 0;
	if (_dataHandle)
		_size = ::GetHandleSize(_dataHandle);
//	dprintf("CPakStream loaded %s\n",pathName().c_str());
}


CPakStream::~CPakStream()
{
	_itemSet.erase(this);
	::HUnlock(_dataHandle);
	::DisposeHandle(_dataHandle);
	//dprintf("~CPakStream %s\n",pathName().c_str());
}

char *CPakStream::getData(char *tag)
{
	resetCursor();
	char *p = (char*)CMemoryTracker::safeAlloc(1, getSize(), tag, false);
	if(p && !getBytes(getSize(), (char*)p)) {
		CMemoryTracker::safeFree(p);
		p = nil;
	}
		
	return p;
}

OSErr CPakStream::copyDataToFile(FSSpec *spec)
{
	LFile outFile(*spec);
	
	if (_dataHandle) {
		::HLock(_dataHandle);
	} else {
		return fnfErr;
	}
	
	try {
		outFile.OpenDataFork(fsWrPerm);
		outFile.WriteDataFork(*_dataHandle, getSize());
		outFile.CloseDataFork();
	} catch (...) {
	
		// oh well
		dprintf("exception generated writing data!\n");
	}

	return noErr;
}


string CPakStream::pathName()
{
	string path;
	
	if (_archive)
		path += _archive->pathName();
	path += _name;
	return path;
}



string CPakStream::dataType()
{
	string package, file, extension;
	decomposeEntryName(_name, package, file, extension);
	return extension;
}

char *CPakStream::address()
{
	char *p = 0;
	
	// never gets unlocked
	if (_dataHandle) {
		::HLock(_dataHandle);
		p = (*_dataHandle);
	} 
	return p;
}

Boolean CPakStream::getBytes(unsigned long nBytes, void *bytes)
{
	char *p = address();
	
	ThrowIf_(!p);	// sanity check
	ThrowIf_(_dataCursor + nBytes > _size);	// sanity check
		
	memcpy(bytes,  p + _dataCursor, nBytes);
	_dataCursor += nBytes;
	
	return true;
}

void CPakStream::seek(UInt32 location)
{
	_dataCursor = location;
}


void CPakStream::dumpHeap()
{
	dprintf("Allocated loaders:\n");
	item_set_iterator e = _itemSet.begin();
	while (e != _itemSet.end()) {
		CPakStream *item = *e;
		string viewerPath = item->pathName();
		dprintf("	%s\n",viewerPath.c_str());
	 	e++;
	}
}