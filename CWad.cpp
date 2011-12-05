/* 
	CWad.cpp

	Author:			Tom Naughton
	Description:	<describe the CWad class here>
*/

#include "CWad.h"
#include "utilities.h"
#include "CPakStream.h"
#include "CPakRatApp.h"


CWad::CWad(CFileArchive *inParent ) : CFileArchive(inParent)
{
}

CWad::CWad(FSSpec inFileSpec) : CFileArchive(inFileSpec)
{
}

CWad::~CWad()
{
	if (_wad_entries)
		CMemoryTracker::safeFree(_wad_entries);
	dprintf("~CWad()\n");
}


Boolean CWad::init()
{	
	int         i;
	SInt32 		length;
	string package, file, extension;
	string entryName;
	
	_fileStream->OpenDataFork(fsRdPerm);
	_fileStream->SetMarker(0, streamFrom_Start);
	length = sizeof(wadinfo_t);
	_fileStream->GetBytes(&_wadheader, length);

	_diroffset = swapLong(_wadheader.infotableofs);
	_fileStream->SetMarker(_diroffset, streamFrom_Start);
	_numlumps = swapLong(_wadheader.numlumps);
	_dirsize = _numlumps * sizeof(filelump_t);

	_wad_entries = (filelump_t *)CMemoryTracker::safeAlloc(_numlumps, sizeof(filelump_t), "_wad_entries");
	if (!_wad_entries)
		goto fail;
	_fileStream->GetBytes(_wad_entries, _dirsize);

	for (i=0;i<_numlumps;i++) {
	
		// extract name
		char s[9];
		int j;
		for(j = 0; j < 7; ++j) {
			s[j] = _wad_entries[i].name[j];
			if (!s[j])
				break;
		}
		s[++j] = 0;

		entryName = (const char*)s;
		this->addFileWithName(i, entryName.c_str());

	}
	  
	 return true;
	 
fail:
	dprintf("could not load wad entries\n");
	return false;
}


// called only on the root pak
long CWad::SizeOfItemAtIndex(string itemName, UInt32 inIndex)
{
#pragma unused (itemName)
	// fixme assert this is the root package
	return swapLong(_wad_entries[inIndex].size);
}

// called only on the root pak
CPakStream *CWad::LoadItemAtIndex(CFileArchive *inParent, string itemName, UInt32 inIndex)
{

	// fixme assert this is the root package
	
	long offset = swapLong(_wad_entries[inIndex].filepos);
	long pakSize = swapLong(_wad_entries[inIndex].size);
	
	dprintf("itemName %s wad item %d offset %d size %d\n", itemName.c_str(), inIndex, offset, pakSize);

	_fileStream->SetMarker(offset, streamFrom_Start);
	Handle pakData = ::NewHandle(pakSize);
	::HLock(pakData);
	_fileStream->GetBytes(*pakData, pakSize);
	::HUnlock(pakData);
	
	return new CPakStream(inParent, itemName, inIndex, pakData);
}

