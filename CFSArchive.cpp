/* 
	CFSArchive.cpp

	Author:			Tom Naughton
	Description:	<describe the CFSArchive class here>
*/

#include "CFSArchive.h"
#include "CPakStream.h"
#include "String.h"

extern "C" {
	#include "MoreFiles.h"
	#include "MoreFilesExtras.h"
}

CFSArchive::CFSArchive(CFileArchive *inParent) : CFileArchive(inParent)
{
}

CFSArchive::CFSArchive(FSSpec inFileSpec) : CFileArchive(inFileSpec)
{
	_loaded = false;
}


CFSArchive::~CFSArchive()
{
	dprintf("~CFSArchive()\n");
}


Boolean CFSArchive::init()
{	
	// find the parent directory
	FSSpec outSpec;
	long directory;
	
	GetParentID(_fileSpec.vRefNum, 
		_fileSpec.parID, 
		_fileSpec.name,
		&directory);

	OSErr anErr = FSMakeFSSpec(_fileSpec.vRefNum, directory, 0, &outSpec);

	traverseDirectory(outSpec);
	return true;
	
}

void CFSArchive::traverseDirectory(FSSpec inFileSpec)
{
	_loaded = true;
	
	CInfoPBRec folderInfo;
	Str31 dirFileName;
	LString::CopyPStr( inFileSpec.name, dirFileName, sizeof(Str31) );
	long pakIndex = 0;
	
	// find the info for this directory (mostly to get it's dirID)
	folderInfo.hFileInfo.ioCompletion = nil;
	folderInfo.hFileInfo.ioNamePtr = dirFileName;
	folderInfo.hFileInfo.ioVRefNum = inFileSpec.vRefNum;
	folderInfo.hFileInfo.ioFDirIndex = 0;
	folderInfo.hFileInfo.ioDirID = inFileSpec.parID;

	ThrowIfOSErr_(::PBGetCatInfoSync( &folderInfo ));
	SInt16 index = 1;

	while ( true ) {
	
		// and recurse through this folder to list its contents
		
		CInfoPBRec fileInfo;
		Str31 fileName;
		
		fileInfo.hFileInfo.ioCompletion = nil;
		fileInfo.hFileInfo.ioNamePtr = fileName;
		fileInfo.hFileInfo.ioVRefNum = folderInfo.hFileInfo.ioVRefNum;
		fileInfo.hFileInfo.ioFDirIndex = index++;
		fileInfo.hFileInfo.ioDirID = folderInfo.hFileInfo.ioDirID;
	
		OSErr err = ::PBGetCatInfoSync( &fileInfo );	
		if ( err != noErr )
			break;
				
		// we ignore invis stuff
		if ( fileInfo.hFileInfo.ioFlFndrInfo.fdFlags & kIsInvisible )
			continue;

		// make an FSSpec for the file
		
		FSSpec theSpec;
		
		ThrowIfOSErr_(::FSMakeFSSpec( folderInfo.hFileInfo.ioVRefNum,
						folderInfo.hFileInfo.ioDirID,
						fileName,
						&theSpec ));
				
		FSSpec tempSpec;		
		p2cstrcpy((char*)tempSpec.name, (const unsigned char *)theSpec.name);
		string entryName = (char*)tempSpec.name;
			
		string package, file, extension;
		decomposeEntryName(lowerString(entryName), package, file, extension);

		if ( fileInfo.hFileInfo.ioFlAttrib & ioDirMask ) {
			// it's a directory
			CFSArchive *aPackage = new CFSArchive(theSpec);
			aPackage->setName(file.c_str());
			aPackage->setParent(this);
			addPackageWithName(aPackage, file.c_str());
			
		} else {
			// it's a file
			addFileWithName(folderInfo.hFileInfo.ioDirID, (file + "." + extension).c_str());
		}
	}
}

CFileArchive::database_type *CFSArchive::getSubpackages()
{
	if (!_loaded) {
		traverseDirectory(_fileSpec);
	}
	return CFileArchive::getSubpackages();
}

CFileArchive::file_database_type *CFSArchive::getFiles()
{
	if (!_loaded) {
		traverseDirectory(_fileSpec);
	}
	return CFileArchive::getFiles();
}


CFileArchive *CFSArchive::subPackage(const char*name) 
{ 
	if (!_loaded) {
		traverseDirectory(_fileSpec);
	}
	return CFileArchive::subPackage(name);
}


long CFSArchive::SizeOfItemAtIndex(const char *itemName, UInt32 inIndex)
{
	FSSpec spec = _fileSpec;
	spec.parID = inIndex;
	long dataSize, rsrcSize;
	
	c2pstrcpy((unsigned char *)spec.name, (const  char *)itemName);

	OSErr err = GetFileSize(spec.vRefNum,
							spec.parID,
							spec.name,
							&dataSize,
							&rsrcSize);

	return dataSize;
}

CPakStream *CFSArchive::LoadItemAtIndex(CFileArchive *inParent, const char *itemName, UInt32 inIndex)
{
	FSSpec spec = _fileSpec;
	spec.parID = inIndex;
	c2pstrcpy((unsigned char*)spec.name, (const char *)itemName);
	CPakStream *stream = new CPakStream(spec);
	stream->setArchive(inParent);
	return stream;
}



