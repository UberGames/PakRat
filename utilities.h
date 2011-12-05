#pragma once
#include <map>
#include <string>
#include <string.h>
#include <list>
#include <iostream.h>
#include <stdio.h>

using std::map;
using std::string;
using std::list;

char *byteHexString(unsigned char l);
char *shortHexString(unsigned short l);
char *longHexString(unsigned long l);

typedef list<string> StringList;
typedef StringList::iterator StringList_iterator;


inline short swapShort(short l) {
	UInt8    b1,b2;

	b1 = l&255;
	b2 = (l>>8)&255;

	return (b1<<8) + b2;
}

inline long swapLong(long l)
{
	UInt8    b1,b2,b3,b4;

	b1 = l&255;
	b2 = (l>>8)&255;
	b3 = (l>>16)&255;
	b4 = (l>>24)&255;

	return ((int)b1<<24) + ((int)b2<<16) + ((int)b3<<8) + b4;
}

inline void swapLongInPlace(long &l) 
{
	l = swapLong(l);
}

inline void swapShortInPlace(short &l) 
{
	l = swapShort(l);
}

inline float swapFloat (float f)
{
	union
	{
		float	f;
		UInt8	b[4];
	} dat1, dat2;
	
	
	dat1.f = f;
	dat2.b[0] = dat1.b[3];
	dat2.b[1] = dat1.b[2];
	dat2.b[2] = dat1.b[1];
	dat2.b[3] = dat1.b[0];
	return dat2.f;
}

inline char nextByte(char* &p) {
	return *p++;
}

inline UInt16 nextShort(char* &p) {
	UInt16 *t = (UInt16*) p;
	UInt16 s;
	s = *t++;
	p = (char*)t;
	return s;
}

inline UInt32 nextLong(char* &p) {
	UInt32 *t = (UInt32*) p;
	UInt32 s;
	s = *t++;
	p = (char*)t;
	return s;
}


// path manipulation
void decomposeEntryName(string entry, string &package, string &file, string &extension);
string packageNameFromPath(string path);
string lastPathComponent(string pathName); 
string fileExtension(string entryName);
string stripExtension(string entryName);
Boolean directoriesMatch(string s1, string s2);
Boolean fileExists(string path);
string fileInDirectory(string inFile, string directory);
string fixSlashes(string path);
string unixToMacPath(string path);
Boolean ancestorDirectoryWithName(string name, FSSpec &inFileSpec);
string fileSpecToPath(FSSpec inspec);
void pathToFSSpec(string path, FSSpec &inspec);

string uniqueName();
UInt32 nextGLName();
string integerString(SInt32 num);
string OSTypeToString(OSType type);
SInt32 intValue(string s);
string nextLine(char* &p, char *e);
void outputTabs(int tabs);
float deg2Rad(float theta);
float rad2deg(float theta);
extern const double pi;
void scaleRectPreservingAspectRation(Rect &theRect, UInt16 newWidth, UInt16 newHeight);

string lowerString(string s);
extern "C" {
	int _stricmp(const char *s1, const char *s2);
}
Boolean stringStartsWith(string s, string start);


// model specific
string skinNameFromPath(string path);
string playerNameFromPath(string path);
string getLODPostfix(string filename);
string getLODPrefix(string filename) ;

#if PP_Target_Carbon
#else
void c2pstrcpy(char *dst, const char *src);
void p2cstrcpy(char *dst, const char *src);
#endif


// Memory allocation tracking
class CMemoryTracker
{

public:

	static void *safeAlloc(UInt32 size);
	static void *safeAlloc(UInt32 count, UInt32 size, char *name = "none" , Boolean canFail = true);
	static void safeFree(void *mem);
	static void dumpHeap();

private:

	char 	*_name;
	UInt32 	_size;
	Handle 	_handle;

	typedef map<void*,CMemoryTracker*> trackmap;
	typedef trackmap::value_type trackmap_value;
	typedef trackmap::iterator trackmap_iterator;

	static trackmap _trackMap;
	
};

