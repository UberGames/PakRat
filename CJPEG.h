/* 
	CJPEG.h

	Author:			Tom Naughton
	Description:	<describe the CJPEG class here>
*/

#ifndef CJPEG_H
#define CJPEG_H

#include <FixMath.h>
#include <ImageCompression.h>
#include <Movies.h>
#include <QuickTimeComponents.h>
//#include <StandardFile.h>

#include "CGLImage.h"


#define kImageFileCreator		FOUR_CHAR_CODE('ogle')
#define kBufferSize				codecMinimumDataSize		// data unload buffer size


// JFIF (JPEG) markers
enum {
	kSOFMarker					= 0xC0,						// start Of Frame N
	kSOFMarker2					= 0xC2,
	kDHTMarker					= 0xC4,						// DHT Marker
	kDACMarker					= 0xCC,						// DAC

	kRSTOMarker					= 0xD0,						// RSTO marker
	kSOIMarker					= 0xD8, 					// image start marker
	kEOIMarker					= 0xD9,						// image end marker
	kSOSMarker					= 0xDA,						// SOS marker
	kDQTMarker					= 0xDB,						// DQT marker

	kAPPOMarker					= 0xE0,						// APPO marker
	kCommentMarker				= 0xFE,						// comment marker

	kStartMarker				= 0xFF						// marker loacted after this byte
};

// a record to hold info about the JPEG data while we're loading it from disk
struct DLDataRec
{	
	char 						*p;					// ref number of current file
	long						fFileLength;			
	Ptr							fOrigPtr;					// address of start of buffer
	Ptr							fEndPtr;					// address of end of buffer
};
typedef struct DLDataRec DLDataRec;
typedef DLDataRec *DLDataPtr, **DLDataHnd;


class CJPEG: public CGLImage
{
public:

	CJPEG();
	virtual ~CJPEG();
	
	virtual OSErr initImage(CPakStream *inItem); // class specific initialization

protected:

	OSErr QTJPEG_ReadJPEG (char *p,  long mySize);
	OSErr QTJPEG_ReadJPEGHeader (char *&p, long mySize, ImageDescriptionHandle theDesc, Rect *theRect);

};

UInt8 QTJPEG_FindNextMarker (char *&p, long mySize);
OSErr QTJPEG_HandleAPPOMarker (UInt8 theMarker, char *&p, ImageDescriptionHandle theDesc, Boolean *readingExtension);
OSErr QTJPEG_HandleSOFMarker (char *&p, ImageDescriptionHandle theDesc, Boolean readingExtension);
void QTJPEG_HandleSOSMarker (char *&p);
void QTJPEG_SkipLength (char *&p);
OSErr QTJPEG_NewJPEGWorld (GWorldPtr *theWorld, short theDepth, Rect theRect);
pascal OSErr QTJPEG_DataLoadingProc (Ptr *theData, long theBytesNeeded, long theRefCon);

#endif	// CJPEG_H
