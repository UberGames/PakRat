/* 
	CFileArchive.cpp

	Author:			Tom Naughton
	Description:	<describe the CFileArchive class here>
*/

#include "CFileArchive.h"
#include "utilities.h"

CFileArchive::archiveset CFileArchive::_archiveSet;

CFileArchive::CFileArchive(CFileArchive *inParent)
{
	_parent = inParent;
	_cachedSize = -1;
	_fileStream = nil;
}

CFileArchive::CFileArchive(FSSpec inFileSpec) 
{
	FSSpec theSpec;
	_fileSpec = inFileSpec;	
	theSpec = inFileSpec; // mangle a copy
	
	p2cstrcpy((char*)theSpec.name, (const unsigned char *)theSpec.name);
	_filename = (char*)theSpec.name;
	_fileStream = new LFileStream(inFileSpec);
	_parent = nil;
	_cachedSize = -1;
}

CFileArchive::~CFileArchive()
{

	// iterate over subpackages
	CFileArchive::iterator e = _subpackages.begin();
	while (e != _subpackages.end()) {
	 	pair_type thePair = *e;
	 	delete thePair.second;
	 	e++;
	}
	if (_fileStream)
		delete _fileStream;
	_fileStream = nil;
	
	_archiveSet.erase(this);
}

void CFileArchive::registerArchive()
{
	_archiveSet.insert(this);
}

Boolean CFileArchive::init()
{
	return false;	
}

CFileArchive *CFileArchive::rootArchive()
{
	if (_parent == nil)
		return this; 
	else
		return _parent->rootArchive();
}

// inIndex -1 for size of whole package
long CFileArchive::size(const char *itemName, long inIndex)
{
	long outSize = 0;
	CFileArchive *rootArchive = this->rootArchive();
	
	
		if (inIndex > 0) {
		
			// size of an item
			outSize = rootArchive->SizeOfItemAtIndex(itemName, inIndex);
			
		} else {
				
			// size of this package
			if (_cachedSize < 0) {
				// iterate over subpackages
				CFileArchive::iterator e = _subpackages.begin();
				while (e != _subpackages.end()) {
				 	pair_type thePair = *e;
				 	outSize += thePair.second->size(thePair.first.c_str(), inIndex);
				 	e++;
				}
				
				// iterate over items
				CFileArchive::file_iterator f = _filesmap.begin();
				while (f != _filesmap.end()) {
				 	file_pair_type thePair = *f;
				 	outSize += rootArchive->SizeOfItemAtIndex(thePair.first.c_str(), thePair.second);
				 	f++;
				}
				_cachedSize = outSize;
			} else {
				outSize = _cachedSize;
			}
		}
	
	return outSize;
}

CFileArchive *CFileArchive::packageWithName(const char *inEntryName, Boolean create)
{
	CFileArchive *thePackage = this;
	
	// FIXME - really need a case insensitive compare in the <map>
	// Strings in the UI should be displayed with the original
	// case information.
	
	string entryName = lowerString(inEntryName);
	
  	while (entryName.length() > 1) {
		string name;
	
		// peel off a name
		int slashIndex = entryName.find('/');
		if (slashIndex >= 0) {
		
			// found a slash, it's part of the path
			name = entryName.substr(0,slashIndex);
			entryName = entryName.substr(slashIndex+1, entryName.length());	
			
		} else {
		
			// called with no slashes in path
			name = entryName;			
			entryName = "";	
		}
		
		CFileArchive *aPackage = nil;
		
		// handle "/models/players/" (first path element is zero length)
		if (name.length() == 0)	
			aPackage = thePackage;
		else 
			aPackage = thePackage->subPackage(name.c_str());
			
		// find anything?	
		if (!aPackage && create) {
			aPackage = new CFileArchive(thePackage);
			aPackage->setName(name.c_str());
			thePackage->addPackageWithName(aPackage, name.c_str());
		} else if (!aPackage) {
			return nil;
		}
		
		thePackage = aPackage;
	}
	
	return thePackage;
}

CFileArchive::database_type *CFileArchive::getSubpackages()
{
	return &_subpackages;
}

CFileArchive::file_database_type *CFileArchive::getFiles()
{
	return &_filesmap; 
}

void CFileArchive::addPackageWithName(CFileArchive *package, const char *name) 
{
	_subpackages[lowerString(name)] = package; 
 
}

void CFileArchive::addFileWithName(long index, const char *name) 
{
	_filesmap[lowerString(name)] = index; 
}


CFileArchive *CFileArchive::subPackage(const char *inName) 
{ 
	database_type *database = getSubpackages();	

	string name = lowerString(inName);
	
	// check to see if it's there
	// (if you don't do this it will add one)
	if(database->find(name) == database->end())
		return nil;

	return (*database)[lowerString(name)]; 
}


// find a named item in this package i.e. "tris.md2"	
CPakStream *CFileArchive::itemWithName(const char *inItemName)
{
	file_database_type *filesmap = getFiles();
	string itemName = lowerString(inItemName);
	
	if(filesmap->find(itemName) == filesmap->end())
		return nil;
	
	long pakIndex = (*filesmap)[itemName];
			
	CFileArchive *rootArchive = this->rootArchive();
	return rootArchive->LoadItemAtIndex(this, itemName.c_str(), pakIndex);
}

// FIXME - there's no way to tell if this fails
long CFileArchive::indexOfItemWithName(const char *inItemName)
{
	file_database_type *filesmap = getFiles();
	string itemName = lowerString(inItemName);
	
	if(filesmap->find(itemName) == filesmap->end())
		return nil;
	
	long pakIndex = (*filesmap)[itemName];
	return pakIndex;	
}


CPakStream *CFileArchive::itemWithPathNameSearchAll(const char *itemName)
{
	CPakStream *result = nil;
	
	// check all archives
	archiveset_iterator e = _archiveSet.begin();
	while (!result && e != _archiveSet.end()) {
	 	result = (*e)->localItemWithPathName(itemName);
	 	e++;
	}
	
	return result;
}

void CFileArchive::appendPathNamesOfTypeSearchAll(StringList *result, const char *path, const char *extension)
{
	// check all archives
	archiveset_iterator e = _archiveSet.begin();
	while (e != _archiveSet.end()) {
	 	(*e)->appendPathNamesOfType(result, path, extension);
	 	e++;
	}
}

void CFileArchive::appendPathNamesOfType(StringList *result, const char *path, const char *extension)
{
	CFileArchive *archive = 0;
	
	// find package to search
	string package, package1, file, extension1;
	decomposeEntryName(path,  package, file, extension1);
	if (package.length()) {
		archive = this->packageWithName(package.c_str(), false);
		if (archive)
			archive->appendPathNamesOfType(result, "", extension);
		return;
	}
	
	// iterate through looking for files of type extension
	CFileArchive::file_iterator e = getFiles()->begin();
	
	while (e != getFiles()->end()) {
	 	file_pair_type thePair = *e;
	 	decomposeEntryName(thePair.first, package1, file, extension1);
	 	if (extension1 == extension) {
	 		string path = package + thePair.first;
	 		dprintf("%s\n", path.c_str());
	 		result->push_front(package + thePair.first);
	 	}
	 	e++;
	}
}


CPakStream *CFileArchive::itemWithPathName(const char *itemName, Boolean searchAll)
{
	CPakStream *result;
	
	// check this archive first
	result = localItemWithPathName(itemName);
	
	// check all archives
	if (searchAll) {
		archiveset_iterator e = _archiveSet.begin();
		while (!result && e != _archiveSet.end()) {
			if (*e != this)
		 		result = (*e)->localItemWithPathName(itemName);
		 	e++;
		}
	}
	
	return result;
}

// find a named item in this package or a subpackage i.e. "models/monsters/bitch/tris.md2"	
CPakStream *CFileArchive::localItemWithPathName(const char *inItemName)
{
	string itemName = lowerString(inItemName);
	CFileArchive *archive = this;
	CPakStream *result = nil;
	
	string package, file, extension;
	decomposeEntryName(itemName,  package, file, extension);

	if (package.length() > 0) {
		archive = archive->packageWithName(package.c_str(), false);
	}

	if (archive) {
		result = archive->itemWithName((file + "." + extension).c_str());
	}

	return result;
}

string CFileArchive::pathName()
{
	string result;
	
	if (_parent == nil) {
		result =  _name + "/";
	} else {
		result = string(_parent->pathName()) + _name + "/";
	}
	return result;
}

void CFileArchive::ConsoleDump(int indent)
{
	CFileArchive::iterator e = getSubpackages()->begin();
	
	while (e != getSubpackages()->end()) {
	 	outputTabs(indent);
	 	pair_type thePair = *e;
	 	dprintf("%s\n", thePair.first.c_str());
	 	thePair.second->ConsoleDump(indent+1);
	 	e++;
	}
}

void CFileArchive::ConsoleDumpFiles(int indent)
{
	CFileArchive::file_iterator e = getFiles()->begin();
	
	while (e != getFiles()->end()) {
	 	outputTabs(indent);
	 	file_pair_type thePair = *e;
	 	dprintf("%s\n", thePair.first.c_str());
	 	e++;
	}
}



long  CFileArchive::SizeOfItemAtIndex(const char *itemName, UInt32 inIndex) // pure virtual?
{
#pragma unused (itemName, inIndex)
	dprintf("CFileArchive::SizeOfItemAtIndex - someone is calling my pseudovirtual function!\n");
	return -1;
}

CPakStream *CFileArchive::LoadItemAtIndex(CFileArchive *inParent, const char *itemName, UInt32 inIndex) // pure virtual?
{
#pragma unused (inParent, itemName, inIndex)
	dprintf("CFileArchive::CPakStream *LoadItemAtIndex - someone is calling my pseudovirtual function!\n");
	return nil;
}


// ---------------------------------------------------------------------------
//	¥ UsesFileSpec													  [public]
// ---------------------------------------------------------------------------
//	Returns whether the Document's File has the given FSSpec

Boolean
CFileArchive::UsesFileSpec(
	const FSSpec&	inFileSpec) const
{

	return (inFileSpec.vRefNum == _fileSpec.vRefNum) &&
		   (inFileSpec.parID == _fileSpec.parID) &&
		   ::EqualString(inFileSpec.name, _fileSpec.name, false, true);

}
