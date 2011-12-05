#include <stdio.h>
#include <stdlib.h>
#include <TextUtils.h>
#include "utilities.h"


extern "C" {
	#include "MoreFiles.h"
	#include "MoreFilesExtras.h"
}

#define LOWER_STRING_BUFFER_SIZE 512

char *hexDigits = "0123456789ABCDEF";
char outString[9];

char *byteHexString(unsigned char l)
{
	
	outString[0] = hexDigits[(l>>4)&15];
	outString[1] = hexDigits[l&15];
	outString[2] = 0;

	return outString;
}

char *shortHexString(unsigned short l)
{
	
	outString[0] = hexDigits[(l>>12)&15];
	outString[1] = hexDigits[(l>>8)&15];
	outString[2] = hexDigits[(l>>4)&15];
	outString[3] = hexDigits[l&15];
	outString[4] = 0;

	return outString;
}

char *longHexString(unsigned long l)
{
	outString[0] = hexDigits[(l>>28)&15];
	outString[1] = hexDigits[(l>>24)&15];
	outString[2] = hexDigits[(l>>20)&15];
	outString[3] = hexDigits[(l>>16)&15];
	outString[4] = hexDigits[(l>>12)&15];
	outString[5] = hexDigits[(l>>8)&15];
	outString[6] = hexDigits[(l>>4)&15];
	outString[7] = hexDigits[l&15];
	outString[8] = 0;

	return outString;
}


void outputTabs(int tabs)
{
	while(tabs--)
		dprintf("   ");
}

string stripExtension(string entryName) 
{
	// get rid of the file extension
	string package, file, extension;
	decomposeEntryName(entryName, package, file, extension);
	return package + file;
}

string fileExtension(string entryName) 
{
	string package, file, extension;
	decomposeEntryName( entryName, package, file, extension);
	return lowerString(extension);
}

string fileInDirectory(string inFile, string directory)
{
	string package, file, extension;
	decomposeEntryName(directory, package, file, extension);
	string path = package;
	decomposeEntryName(inFile, package, file, extension);
	string result = path + file + "." + extension;
	
	dprintf("directory: %s\nfile: %s\nresult: %s\n", directory.c_str(), file.c_str(), result.c_str());
	return result;
}

Boolean fileExists(string inPath) 
{
	FSSpec spec;
	long dataSize;
	long rsrcSize;
	OSErr err;

	pathToFSSpec(inPath, spec) ;
	
	err = GetFileSize(spec.vRefNum,
					spec.parID,
					spec.name,
					&dataSize,
					&rsrcSize);
					
	return err == noErr;
}


Boolean directoriesMatch(string s1, string s2) 
{
	string package, file, extension;
	decomposeEntryName( s1, package, file, extension);
	s1 = package;
	decomposeEntryName( s2, package, file, extension);
	s2 = package;
	
	return s1 == s2;
}

#if 0
extern "C" {
	int _stricmp(const char *s1, const char *s2)
	{
		string string1 = s1, string2 = s2;
		return strcmp(lowerString(s1).c_str(), lowerString(s2).c_str());
	}
}
#endif

string lastPathComponent(string entryName) 
{
	string package = "";
	string lastcomponent = "";
	
  	while (entryName.length() > 0) {
		string name;
	
		// peel off a name
		int slashIndex = entryName.find('/');
		if (slashIndex >= 0) {
		
			// found a slash, it's part of the path
			name = entryName.substr(0,slashIndex);
			entryName = entryName.substr(slashIndex+1, entryName.length());	
			package += name + '/';
			lastcomponent = name;
			
		}
	}
	return lastcomponent;
}

string skinNameFromPath(string filename) 
{
	int i=filename.find_last_of("_");
	int j=filename.find_last_of(".");
	
	if (i==-1 || j==-1 && j - (i+1) > 0)
		return "";
	else
		return filename.substr(i+1, j - (i+1));
}

string packageNameFromPath(string path)
{
	string package, junkfile, junkextension;
	decomposeEntryName(path,  package, junkfile, junkextension);
	return package;
}


string playerNameFromPath(string path)
{
	string package, junkfile, junkextension;
	decomposeEntryName(path,  package, junkfile, junkextension);
	return lastPathComponent(package);
}

string fixSlashes(string path)
{
	int index = 0;
	while (index >= 0) {
		index = path.find('\\');
		if (index >= 0)
			path[index] = '/';
	}
	return path;
}

string unixToMacPath(string path)
{
	// remove leading '/'
	
	if (path[0] == '/')
		path = path.substr(1, path.length());	
		
	int index = 0;
	while (index >= 0) {
		index = path.find('/');
		if (index >= 0)
			path[index] = ':';
	}
	return path;
}

string uniqueName()
{
	static SInt32 count = 0;
	return "uniquename" + integerString(count++);
}

UInt32 nextGLName()
{
	static UInt32 glNames = 0;
	glNames++;
	return glNames;
}



string integerString(SInt32 num)
{
	char s[256];
	sprintf(s, "%ld", num );
	return string(s);
}

string OSTypeToString(OSType type)
{
	string s = "xxxx";
	s[0] = (type >> 24) & 0xFF;
	s[1] = (type >> 16) & 0xFF;
	s[2] = (type >> 8) & 0xFF;
	s[3] = (type >> 0) & 0xFF;
	return s;
}

SInt32 intValue(string s)
{
	char *place;
	return strtol(s.c_str(), &place, 10);
}

void decomposeEntryName(string entryName, string &package, string &file, string &extension)
{
	package = "";
	file = "";
	extension = "";
	
  	while (entryName.length() > 0) {
		string name;
	
		// peel off a name
		int slashIndex = entryName.find('/');
		if (slashIndex >= 0) {
		
			// found a slash, it's part of the path
			name = entryName.substr(0,slashIndex);
			entryName = entryName.substr(slashIndex+1, entryName.length());	
			package += name + '/';
			
		} else {
			
			// find LAST dot
			int dotIndex = -1;
			int foundDot = -1;
			do {
				foundDot = entryName.find('.', foundDot+1);
				if (foundDot >= 0)
					dotIndex = foundDot;
			} while (foundDot >= 0);
			
			if (dotIndex >= 0) {
			
				// filename and extension
				file = entryName.substr(0,dotIndex);
				extension = entryName.substr(dotIndex+1, entryName.length());	
				entryName = "";
			
			} else {
			
				// no extension
				file = entryName;
				entryName = "";
			}
		}
	}
}

float deg2Rad(float theta)
{
	return  pi/2.0 * 360.0 / theta;
}

float rad2deg(float theta)
{
	return  pi/2.0 * 360.0 / theta;
}

void scaleRectPreservingAspectRation(Rect &theRect, UInt16 newWidth, UInt16 newHeight)
{
	float scale;
	UInt16 scaledHeight;
	
	// try fitting width
	scale = (float)newWidth / (float)(theRect.right - theRect.left);
	scaledHeight =  (theRect.bottom - theRect.top) * scale;
	
	if (scaledHeight <= newHeight) {
		theRect.bottom = (theRect.bottom - theRect.top) * scale + theRect.top;
		theRect.right = (theRect.right - theRect.left) * scale + theRect.left;
		return;
	}
	
	// fit height
	scale = (float)newHeight / (float)(theRect.bottom - theRect.top);
	theRect.bottom = (theRect.bottom - theRect.top) * scale + theRect.top;
	theRect.right = (theRect.right - theRect.left) * scale + theRect.left;
}


string lowerString(string s)
{
	char out[LOWER_STRING_BUFFER_SIZE+1];
	const char *p = s.c_str();
	char *n = out;
	int i;
	
	if (s.length() >= LOWER_STRING_BUFFER_SIZE)
		dprintf("lowerString buffer needs to be bigger!");

	for (i = 0; p[i] && i < LOWER_STRING_BUFFER_SIZE; i++)
		n[i] = std::tolower(p[i]);
	n[i] = 0;
		
	return out;
}

Boolean stringStartsWith(string s, string start)
{
	return (s.find(start) == 0);
}

string getLODPostfix(string filename) 
{
	int i=filename.find_last_of("_");
	if (i!=-1) {
		if (intValue(filename.substr(i+1, filename.length())) > 0)
			return filename.substr(i, filename.length());
	}
	return "";
}

string getLODPrefix(string filename) 
{
	// make sure there is an LOD
	string lod = getLODPostfix(filename);
	if (lod == "")
		return filename;
	
	int i=filename.find_last_of("_");
	if (i==-1)
		return filename;
	else
		return filename.substr(0, i);
}


string nextLine(char* &p, char *e)
{	
	char *lineStart = p;
	
	// find eol
	while (p < e && *p != '\n' && *p != '\r')
		p++;
		
	char *lineEnd = p;

	// find start of next line
	while (p < e && (*p == '\n' || *p == '\r'))
		p++;
		
	// make  a string
	char c = *lineEnd;
	*lineEnd = 0;
	string result = lineStart;
	*lineEnd = c;
	
	return result;
}

CMemoryTracker::trackmap CMemoryTracker::_trackMap;

void *CMemoryTracker::safeAlloc(UInt32 size)
{
	return safeAlloc(1, size, "unnamed", true);
}
void *CMemoryTracker::safeAlloc(UInt32 count, UInt32 size, char *name, Boolean canFail)
{
	static int failCount = 1;
	if (canFail) {
		if (LGrowZone::GetGrowZone()->MemoryIsLow()) {
			dprintf("CMemoryTracker::safeAlloc: Memory is too low for %s\n", name);
			return nil;
		}
	}

	void *mem = nil;
	Handle handle = nil;

// use this to simulate out of memory problems	
//	if (!failCount++ % 200)
		handle = ::NewHandleClear(count * size);
	if (handle) {
		::HLock(handle);
		CMemoryTracker *tracker = new CMemoryTracker();
		tracker->_name = name;
		tracker->_size = count * size;
		tracker->_handle = handle;
		//dprintf("alloc: %s\n", tracker->_name);

		mem = (void*) *(tracker->_handle);
		_trackMap[mem] = tracker;
	}
	return mem;
}

void CMemoryTracker::safeFree(void *mem)
{
	if (!mem)
		return;
		
	trackmap_iterator iter = _trackMap.find(mem);
	if (iter != _trackMap.end()) {
		trackmap_value thePair = *iter;
			
	 	::DisposeHandle(thePair.second->_handle);
		_trackMap.erase(iter);
	 	//dprintf("free: %s\n", thePair.second->_name);
	 	delete thePair.second;
			
	} else {
		#if debS
			dprintf("attempt to free unknown block!\n");
			DebugStr("\pattempt to free unknown block");
		#endif 
	}
	//mem = nil;
}

void CMemoryTracker::dumpHeap()
{
	dprintf("Allocated blocks:\n");
	trackmap_iterator e = _trackMap.begin();
	while (e != _trackMap.end()) {
	 	trackmap_value thePair = *e;
	 	CMemoryTracker *tracker = thePair.second;
	 	dprintf("	0x%X %s %ld\n", thePair.first, thePair.second->_name, thePair.second->_size);
	 	e++;
	}
}

string fileSpecToPath(FSSpec inFileSpec) 
{
	OSErr anErr = noErr;
	FSSpec tempSpec = inFileSpec;
	p2cstrcpy((char*)tempSpec.name, (const unsigned char*)tempSpec.name);
	string path =  (char*)&tempSpec.name;
	
	do {
		long directory;
		FSSpec outSpec = inFileSpec;
		
		GetParentID(inFileSpec.vRefNum, 
			inFileSpec.parID, 
			inFileSpec.name,
			&directory);

		anErr = FSMakeFSSpec(inFileSpec.vRefNum, directory, "\p", &outSpec);
		if (!anErr) {
			inFileSpec = outSpec;			
			tempSpec = outSpec;
			p2cstrcpy((char*)tempSpec.name,(const unsigned char*)tempSpec.name);
			string s = (char*)&tempSpec.name;
			path = s + "/" + path;
			inFileSpec = outSpec;
		}
	} while (!anErr);
	return "/" + path;
}

void pathToFSSpec(string inPath, FSSpec &inspec) 
{
	string macPath = unixToMacPath(inPath);
	FSSpec spec;
	Boolean isDirectory;
	Str255 path;
	
	c2pstrcpy((unsigned char*)&path, macPath.c_str());
	
	OSErr err = GetObjectLocation((short)0,
						  (long)0,
						  (unsigned char*)&path,
						  &spec.vRefNum,
						  &spec.parID,
						  (unsigned char*)&spec.name,
						  &isDirectory);
								  
	inspec = spec;
}




Boolean ancestorDirectoryWithName(string name, FSSpec &inFileSpec)
{
	OSErr anErr = noErr;
	FSSpec saveSpec = inFileSpec;
	
	do {
		FSSpec outSpec;
		long directory;
		
		GetParentID(inFileSpec.vRefNum, 
			inFileSpec.parID, 
			inFileSpec.name,
			&directory);

		anErr = FSMakeFSSpec(inFileSpec.vRefNum, directory, "\p", &outSpec);
		if (!anErr) {
			inFileSpec = outSpec;			
			FSSpec tempSpec = outSpec;
			p2cstrcpy((char*)tempSpec.name, (const unsigned char*)tempSpec.name);
			string s = (char*)&tempSpec.name;
			dprintf("s = %s name = %s\n", s.c_str(), name.c_str());
			
			if (lowerString(s) == lowerString(name)) {
				dprintf("found!\n");
				return true;
			}
		}
	} while (!anErr);
	
	inFileSpec = saveSpec;
	return false;
}


#if PP_Target_Carbon
#else
void c2pstrcpy(unsigned char *dst, const char *src)
{
	strcpy((char*)dst, src);
	c2pstr((char*)dst);
}

void p2cstrcpy(char *dst, const unsigned char *src)
{
	strcpy(dst, (const char*)src);
	p2cstr((unsigned char*)dst);
}
#endif
