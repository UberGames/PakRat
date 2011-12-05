/* 
	CPreferences.h

	Author:			Tom Naughton
	Description:	<describe the CPreferences class here>
*/

#ifndef CPreferences_H
#define CPreferences_H

#include <list>
#include <map>
#include <string>
#include <iostream.h>

class LPreferencesFile;
using std::string;
using std::map;
using std::list;

string quotedString(string inString);
string colorToString(RGBColor color);
RGBColor stringToColor(string color);

class CPreferences
{
public:
	CPreferences();
	virtual ~CPreferences();
	
	void 		writePreferences();
	
	string		commandForKeyEvent(const EventRecord&	inKeyEvent);
	Boolean 	booleanForKey(string key);
	string 		valueForKey(string key);

	void	 	setBooleanForKey(Boolean b, string key);
	void		setValueForKey(string value, string key);
	void 		addRecentFile(string path);
	string 		recentFileAtIndex(SInt16 index);

	typedef map<string, string> pref_map_type;
	typedef pref_map_type::iterator pref_iterator;
	typedef list<string> recent_type;
	typedef recent_type::iterator recent_type_iterator;

	recent_type *recentFiles() { return &_recentFiles; }
	
private:

	void 				parsePreferences(Handle preferencesHandle);

	pref_map_type		_keyBindings;
	pref_map_type		_settings;
	recent_type			_recentFiles;
	
	LPreferencesFile	*_preferencesFile;
	Boolean 			_preferencesFileisWritable;			

};

#endif	// CPreferences_H
