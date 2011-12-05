/* 
	CPakStream.cpp

	Author:			Tom Naughton
	Description:	<describe the CPakStream class here>
*/

#include "CFileArchive.h"
#include "CPakStream.h"
#include "utilities.h"
#include "CPakRatApp.h"

CPakStream::item_array CPakStream::_allocatedItems;

CPakStream::CPakStream(FSSpec inFileSpec)
{
	_allocatedItems.AddItem(this);
	LFileStream *file = new LFileStream(inFileSpec);
	
	// FIXME - this breaks inFileSpec
	_name = p2cstr(inFileSpec.name);
	
	_index = -1;
	_archive = nil;
	_dataCursor = 0;
	
	file->OpenDataFork(fsRdWrPerm);
	
	_dataHandle = ::NewHandle(file->GetLength());
	if (!_dataHandle)
		goto fail;
		
	::HLock(_dataHandle);
	file->GetBytes(*_dataHandle, file->GetLength());
	file->CloseDataFork();
	::HUnlock(_dataHandle);
	_size = ::GetHandleSize(_dataHandle);

	delete file;
	
	dprintf("CPakStream loaded %s\n",pathName().c_str());
	return;
	
fail:
	delete file;
	dprintf("could not load pak item\n");
	return;
}

CPakStream::CPakStream(CFileArchive *inArchive, string inName, long inIndex, Handle inHandle)
{
	_allocatedItems.AddItem(this);
	_archive = inArchive;
	_name = inName;
	_index = inIndex;
	_dataHandle = inHandle;
	_dataCursor = 0;
	if (_dataHandle)
		_size = ::GetHandleSize(_dataHandle);
	dprintf("CPakStream loaded %s\n",pathName().c_str());
}


CPakStream::~CPakStream()
{
	if (_allocatedItems.ContainsItem(this)) {
		_allocatedItems.Remove(this);
	}

	::HUnlock(_dataHandle);
	::DisposeHandle(_dataHandle);
	dprintf("~CPakStream %s\n",pathName().c_str());
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

string CPakStream::pathName()
{
	string path;
	
	if (_archive)
		path += _archive->pathName();
	path += _name;
	return path;
}

StringPtr CPakStream::itemTitle()
{
	static char _pTitle[1024];	
	string title = pathName();
	strcpy(_pTitle, title.c_str());
	return (StringPtr) c2pstr(_pTitle);
	
}


string CPakStream::dataType()
{
	string package, file, extension;
	decomposeEntryName(_name, package, file, extension);
	return extension;
}

Boolean CPakStream::getBytes(unsigned long nBytes, void *bytes)
{
	char *p = 0;
	// never gets unlocked
	if (_dataHandle) {
		::HLock(_dataHandle);
		p = (*_dataHandle);
	} 

	if (!p)
		return false;
		
	if (_dataCursor + nBytes > _size)
		return false;
		
	memcpy(bytes,  p + _dataCursor, nBytes);
	_dataCursor += nBytes;
	
	return true;
}

void CPakStream::dumpHeap()
{
	dprintf("Allocated loaders:\n");
	TArrayIterator<CPakStream*> iterator(
								(const_cast<TArray<CPakStream*>&>
								(_allocatedItems)));
								
	CPakStream* item = nil;
	while (iterator.Next(item) && (item != nil)) {
		string viewerPath = item->pathName();
		dprintf("	%s\n",viewerPath.c_str());
	}
}