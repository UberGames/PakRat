/* 
	.cpp

	Author:			Tom Naughton
	Description:	<describe the CPak class here>
*/

#include <LFileStream.h>

#include "CPak.h"
#include "utilities.h"
#include "CPakStream.h"
#include "CPakRatApp.h"


CPak::CPak(CFileArchive *inParent) : CFileArchive(inParent)
{
}

CPak::CPak(FSSpec inFileSpec) : CFileArchive(inFileSpec)
{
}

CPak::~CPak()
{
	dprintf("~CPak()\n");
	if (_fileStream)
		_fileStream->CloseDataFork();
	
	if (_pak_entries)
		CMemoryTracker::safeFree(_pak_entries);
}




Boolean CPak::init()
{	
	int         i;
	SInt32 		length;
	string package, file, extension;

	_fileStream->OpenDataFork(fsRdPerm);
	_fileStream->SetMarker(0, streamFrom_Start);
	length = sizeof(pakheader_t);
	_fileStream->GetBytes(&_pakheader, length);

	_diroffset = swapLong(_pakheader.diroffset);
	_fileStream->SetMarker(_diroffset, streamFrom_Start);
	_dirsize = swapLong(_pakheader.dirsize);
	_num_pak_entries = _dirsize / (sizeof(pakentry_t));

	_pak_entries = (pakentry_t *)CMemoryTracker::safeAlloc(_num_pak_entries, sizeof(pakentry_t), "pakentry_t");
	if (!_pak_entries)
		goto fail;
		
	_fileStream->GetBytes(_pak_entries, _dirsize);

	for (i=0;i<_num_pak_entries;i++) {

		string entryName = (const char*)_pak_entries[i].filename;
		decomposeEntryName(entryName,  package, file, extension);
		CFileArchive *thePackage = packageWithName(package.c_str(), true);
		thePackage->addFileWithName(i, (file + "." + extension).c_str());

	}  
	
	return true;
	
fail:
	return false;
}

// called only on the root pak
long CPak::SizeOfItemAtIndex(const char *itemName, UInt32 inIndex)
{
#pragma unused (itemName)
	// fixme assert this is the root package
	return swapLong(_pak_entries[inIndex].size);
}

// called only on the root pak
CPakStream *CPak::LoadItemAtIndex(CFileArchive *inParent, const char *itemName, UInt32 inIndex)
{
	// fixme assert this is the root package
	
	long offset = swapLong(_pak_entries[inIndex].offset);
	long pakSize = swapLong(_pak_entries[inIndex].size);
	
	dprintf("itemName %s pakIndex %d offset %d size %d\n", itemName, inIndex, offset, pakSize);

	_fileStream->SetMarker(offset, streamFrom_Start);
	Handle pakData = ::NewHandle(pakSize);
	if (!pakData) 
		goto fail;
		
	::HLock(pakData);
	_fileStream->GetBytes(*pakData, pakSize);
	::HUnlock(pakData);

	return new CPakStream(inParent, itemName, inIndex, pakData);
	
fail:
	
	return 0;
}
