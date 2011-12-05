/* 
	CWav.cpp

	Author:			Tom Naughton
	Description:	<describe the CWav class here>
*/

#include <map>
#include <string>
#include <iostream.h>
#include <QuickTimeComponents.h>

#include "CWav.h"
#include "CPakStream.h"
#include "CPakRatApp.h"
#include "utilities.h"



CWav::CWav(CPakStream *inItem)
{
    _handle = nil;
    _dataRef = nil;
	_movie = nil;

	char *p = (char*) inItem->getData("wav data");
	if (p) {
		_movie = MakeSoundMovie(p, inItem->getSize(), inItem->dataType());
		CMemoryTracker::safeFree(p);
	}
}


CWav::~CWav()
{
	dprintf("~CWav\n");
	stop();
	if (_movie) {
		::DisposeMovie(_movie);
	}
    OSErr err = ::GetMoviesError();
	if (err) {
		dprintf("DisposeMovie error %d\n",err);
	}
	if (_dataRef) 
		::DisposeHandle(_dataRef);
	if (_handle)
		::DisposeHandle(_handle);
}


void CWav::loop(Boolean loop)
{
#pragma unused (loop)
	TimeBase		myTimeBase = NULL;
	long 			myFlags = 0L;
	OSErr			myErr = paramErr;
	Boolean			isPalindrome = false;

	// make sure we've got a movie
	if(!_movie)
		return;
	
	myErr = noErr;
		
	// set the movie's play hints to enhance looping performance
	::SetMoviePlayHints(_movie, hintsLoop, hintsLoop);
	
	// set the looping flag of the movie's time base
	myTimeBase = ::GetMovieTimeBase(_movie);
	myFlags = ::GetTimeBaseFlags(myTimeBase);
	myFlags |= loopTimeBase;
	
	// set or clear the palindrome flag, depending on the specified setting
	if (isPalindrome)
		myFlags |= palindromeLoopTimeBase;
	else
		myFlags &= ~palindromeLoopTimeBase;
		
	::SetTimeBaseFlags(myTimeBase, myFlags);

}

void CWav::play()
{
	if(!_movie)
		return;
		
    ::SetMovieVolume(_movie, kFullVolume);
    ::GoToBeginningOfMovie(_movie);
    ::StartMovie(_movie);
    OSErr err = ::GetMoviesError();
	if (err) {
		dprintf("StartMovie error %d\n",err);
	}
}

void CWav::stop()
{
	if(!_movie)
		return;

    ::StopMovie(_movie);
    OSErr err = ::GetMoviesError();
	if (err) {
		dprintf("StopMovie error %d\n",err);
	}
}

Movie CWav::MakeSoundMovie(Ptr waveDataPtr, long waveDataSize, string inType)
{
    Movie                   movie = nil;
    Track                   targetTrack = nil;
    TimeValue               addedDuration = 0;
    long                    outFlags = 0;
    OSErr                   err;
    ComponentResult         result;
	long 					inFlags;
	
    _handle = ::NewHandleClear(waveDataSize);
    if (!_handle)
    	goto fail;
    ::BlockMoveData(waveDataPtr, *_handle, waveDataSize);
    err = ::PtrToHand(&_handle,
                    &_dataRef,
                    sizeof(Handle));
    if (err) goto fail;

	Component c = nil;
	ComponentDescription cd;
//	ComponentDescription description;

	cd.componentFlags = 0;
	cd.componentFlagsMask = 0;

	if (inType == "wav") {
		cd.componentType = MovieImportType;
		cd.componentSubType = kQTFileTypeWave;
		cd.componentManufacturer = 'soun';
	} else if (inType == "mid" ) {
		cd.componentType = MovieImportType;
		cd.componentSubType = 0;
		cd.componentManufacturer = 'musi';
	} else if (inType == "mp3" ) {
		cd.componentType = MovieImportType;
		cd.componentSubType = 0;
		cd.componentManufacturer = 'soun';
	} 
	
	for(c = nil; !movie && (c = ::FindNextComponent(c, &cd));) {

			MovieImportComponent importer;
			Boolean canceled = false;
			
			/*
			Handle componentName = ::NewHandle(0);
			err = ::GetComponentInfo(c, &description, componentName, 0, 0);
			dprintf("trying to import: %s, %s, %s, %#s\n", 
				OSTypeToString(description.componentType).c_str(), 
				OSTypeToString(description.componentSubType).c_str(),  
				OSTypeToString(description.componentManufacturer).c_str(), 
				(*componentName));
			::DisposeHandle(componentName);
			*/

			importer = ::OpenComponent(c);
			inFlags = movieImportCreateTrack;

			if (importer) {
				movie = ::NewMovie(newMovieActive);
				result = ::MovieImportDataRef(importer,
				                        _dataRef,
				                        HandleDataHandlerSubType,
				                        movie,
				                        nil,
				                        &targetTrack,
				                        nil,
				                        &addedDuration,
				                        inFlags,
				                    	&outFlags);
				dprintf("result %d\n", result);  
		        
		        if (result != noErr)  {
					::DisposeMovie(movie);
					movie = 0;
		        }     
	        }
			CloseComponent(importer);
		}
		
		return movie;
fail:
	if (movie)
		::DisposeMovie(movie);
	dprintf("could not create movie\n");
	return nil;
}
