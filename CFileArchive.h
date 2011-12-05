/* 
	CFileArchive.h

	Author:			Tom Naughton
	Description:	<describe the CFileArchive class here>
*/

#ifndef CFileArchive_H
#define CFileArchive_H

#include <set>
#include <map>
#include <list>
#include <string>
#include <iostream.h>
#include "utilities.h"

using std::map;
using std::pair;
using std::set;
using std::list;
using std::string;

class CPakStream;
class CResourceDocument;


class CFileArchive
{

public:

    // Get a list of shader script files 
	CFileArchive(CFileArchive *inParent);
	CFileArchive(FSSpec inFileSpec);
	virtual ~CFileArchive();

	virtual Boolean 		init();
	virtual void 			addPackageWithName(CFileArchive *package, const char *name);
	virtual void 			addFileWithName(long index, const char *name);
	CFileArchive 			*packageWithName(const char *entryName, Boolean create);
	CPakStream 				*itemWithName(const char *itemName);
	CPakStream 				*itemWithPathName(const char *itemName, Boolean searchAll = true);
	long 					indexOfItemWithName(const char *itemName);
	static CPakStream 		*itemWithPathNameSearchAll(const char *itemName);
	CFileArchive 			*rootArchive();
	void 					appendPathNamesOfTypeSearchAll(StringList *result, const char *path, const char *extension);
	void 					appendPathNamesOfType(StringList *result, const char *path, const char *extension);
	string					name() { return _name; }
	void 					setName(const char *n) { _name = n; }
	void 					setParent(CFileArchive *p) { _parent = p; }
	string					pathName();
	long 					size(const char *itemName, long inIndex);
	
	virtual CFileArchive 	*subPackage(const char *name);
	virtual long 			SizeOfItemAtIndex(const char *itemName, UInt32 inIndex); // pure virtual?
	virtual void 			ConsoleDump(int indent);
	virtual void 			ConsoleDumpFiles(int indent);

	// called only on the root package
	virtual CPakStream 		*LoadItemAtIndex(CFileArchive *inParent, const char *itemName, UInt32 inIndex); // pure virtual?

	void 					setDocument(CResourceDocument *inDoc)  { _document = inDoc; }
	CResourceDocument 		*document()  { return _document; }

	void 					registerArchive();
	
	FSSpec 					fsspec() { return _fileSpec; };
	virtual Boolean			UsesFileSpec(const FSSpec&	inFileSpec) const;

	// This is the C++ Standard Template Library's answer to NSDictionary
	// (sick but functional)
	
	typedef pair<string, CFileArchive*> pair_type;
	typedef map<string, CFileArchive*> database_type;
	typedef database_type::value_type entry_type;
	typedef database_type::iterator iterator;

	typedef pair<string, long> file_pair_type;
	typedef map<string, long> file_database_type;
	typedef file_database_type::value_type file_entry_type;
	typedef file_database_type::iterator file_iterator;

	virtual database_type *getSubpackages();	
	virtual file_database_type *getFiles();

	
protected:

	FSSpec			 _fileSpec;
	LFileStream 	*_fileStream;
	
private:

	database_type _subpackages;			// holds the subpackages
	file_database_type _filesmap;	// holds files in this package

	CPakStream 				*localItemWithPathName(const char *itemName);
	CResourceDocument 		*_document;
	CFileArchive 			*_parent; // nil for root package
	string					_name;
	
	// only valid for root
	string 					_filename;
	long					_cachedSize;
	
	typedef set<CFileArchive*> archiveset;
	typedef archiveset::iterator archiveset_iterator;

	static archiveset _archiveSet;

};

#endif	// CFileArchive_H
