/* 
	CWav.h

	Author:			Tom Naughton
	Description:	<describe the CWav class here>
*/

#ifndef CWav_H
#define CWav_H

#include <Movies.h>
#include <string>

using std::string;

class CPakStream;

class CWav
{
public:
	CWav(CPakStream *inItem);
	virtual ~CWav();
	
	void 	loop(Boolean loop);
	void 	play();
	void 	stop();
	Movie 	getMovie() { return _movie; }
	
private:

    Handle	_handle;
    Handle	_dataRef;
	Movie 	MakeSoundMovie(Ptr waveDataPtr, long waveDataSize, string inType);
	Movie 	_movie;
};

#endif	// CWav_H
