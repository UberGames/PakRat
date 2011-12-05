/************************************************************************/
/*	Project...: C++ and ANSI-C Compiler Environment 					*/
/*	Name......: MWAbout.c												*/
/*	Purpose...: about box												*/
/*	Copyright.: ©Copyright 1993 by metrowerks inc. All rights reserved. */
/************************************************************************/

enum  {
	ditl_about = 607,		// about DITL
	ditl_about_items = 8,	// number of items in DITL
	ditl_about_background = 1,	// background about item
	ditl_about_creditsbox	// credits
};

#include <Gestalt.h>
#include <QDOffscreen.h>
#include <Sound.h>
#include <Resources.h>
#include <Fonts.h>
#include "AppConstants.h"

#define 	Fade_Levels 		12

typedef struct UserItemRec	{
	short p1;
	short p2;
	Rect box;
	short padding2;
} UserItemRec;

typedef struct DITLRec	{
	short numItems;
	UserItemRec theUserItems[1];
} DITLRec, **DITLHand;

void DoAboutBox(Boolean simpleAbout);
void DoSimpleAbout(void);
void SetUpGraphics(void);
void DoAnimation(void);
void CleanUpGraphics(void);
void GetItemRect(short inItem, Rect *outRect);
void DrawPictIntoNewGWorld(short inPICTid, short inDepth,
						   GWorldPtr *outGWorldP);
void DrawTextIntoNewGWorld(short inTEXTid, Rect *inBounds, short inDepth,
						   GWorldPtr *outGWorldP);

typedef enum AnimateState  {
	animate_Waiting,
	animate_Active,
	animate_Done
} AnimateState;

void MoveOffWorld(GWorldPtr theGWorldP, Rect *currPos, Rect *newPos,
				  Rect *imagePos);


typedef struct CreditsRec  {
	GWorldPtr theGWorldP;
	Rect viewRect;
	Rect pictRect;
	short pictHeight;
} CreditsRec;

void SetUpCredits(CreditsRec *inCreditsRec);

static short Pixel_Depth;
static CWindowPtr aboutBoxWindow;
static PixMapPtr windowPixMapP;
static GWorldPtr drawWorldP;
static PixMapPtr drawPixMapP;
static GWorldPtr backWorldP;
static PixMapPtr backPixMapP;

void DoAboutBox(Boolean simpleAbout)
{
	long qdVersion;
	GWorldPtr saveWorld;
	GDHandle saveDevice;

	Gestalt(gestaltQuickdrawVersion, &qdVersion);

	if (simpleAbout || (qdVersion < gestalt32BitQD) || (FreeMem() < 350000)) {
		GrafPtr savePort;
		//	Machine lacks 32bit QuickDraw or there's not enough memory.
		GetPort(&savePort);

		DoSimpleAbout();

		SetPort(savePort);

	} else {				//	OK to do animated about box.

		GetGWorld(&saveWorld, &saveDevice);

		SetUpGraphics();

		DoAnimation();

		CleanUpGraphics();

		PurgeMem(maxSize);
		
		SetGWorld(saveWorld, saveDevice);
	}
}

void DoSimpleAbout(void)
{
	WindowPtr aboutBoxWindow = GetNewWindow(WIND_MetroAbout, nil, (WindowPtr) -1L);
	PicHandle thePicture;
	Rect r;
	unsigned long ticks;

	ShowWindow(aboutBoxWindow);


#if PP_Target_Carbon
	SetGWorld(GetWindowPort(aboutBoxWindow), GetMainDevice());
#else
	SetGWorld((CGrafPort*)aboutBoxWindow, GetMainDevice());
#endif

	thePicture = GetPicture(PICT_Background);
	if (thePicture != nil) {
		r = (**thePicture).picFrame;

		DrawPicture(thePicture, &r);
		ReleaseResource((Handle)thePicture);
		
		/*
		GetItemRect(ditl_about_versionbox, &r);
		theText = GetResource('TEXT', TEXT_AboutVersion);
		HLock(theText);
		TextSize(9);
		TextFace(bold);
		TETextBox(*theText, GetHandleSize(theText), &r, teJustRight);
		ReleaseResource(theText);
		*/
		
		Delay(45, &ticks);

#if PP_Target_Carbon
		QDFlushPortBuffer(GetWindowPort(aboutBoxWindow), nil);
#endif
		while (!Button())
			;
	}
	DisposeWindow(aboutBoxWindow);
}

void SetUpGraphics(void)
{
	Rect backRect;
	PixMapHandle thePixMapH;

	Pixel_Depth = 16;

	//	Make GWorld for offscreen drawing.

	GetItemRect(ditl_about_background, &backRect);
	NewGWorld(&drawWorldP, Pixel_Depth, &backRect, nil, nil, 0);
	thePixMapH = GetGWorldPixMap(drawWorldP);
	HLockHi((Handle)thePixMapH);
	LockPixels(thePixMapH);
	drawPixMapP = *thePixMapH;

	//	Make GWorld for background.

	DrawPictIntoNewGWorld(PICT_Background, Pixel_Depth, &backWorldP);
	thePixMapH = GetGWorldPixMap(backWorldP);
	HLockHi((Handle)thePixMapH);
	LockPixels(thePixMapH);
	backPixMapP = *thePixMapH;

	//	Make Window.

	aboutBoxWindow = (CWindowPtr) GetNewCWindow(WIND_MetroAbout, nil,
												(WindowPtr) - 1L);
	ShowWindow((WindowPtr)aboutBoxWindow);
	
#if PP_Target_Carbon
	HLockHi((Handle)GetPortPixMap(GetWindowPort(aboutBoxWindow)));
	windowPixMapP = *GetPortPixMap(GetWindowPort(aboutBoxWindow));
#else
	HLockHi((Handle)aboutBoxWindow->portPixMap);
	windowPixMapP = *aboutBoxWindow->portPixMap;
#endif

	//	Draw background in Window.

#if PP_Target_Carbon
	SetGWorld(GetWindowPort(aboutBoxWindow), GetMainDevice());
#else
	SetGWorld(aboutBoxWindow, GetMainDevice());
#endif
	CopyBits((BitMapPtr)backPixMapP, (BitMapPtr)windowPixMapP, &backRect,
			 &backRect, srcCopy, nil);
			 
}

void SetUpCredits(CreditsRec *inCreditsRec)
{
	PicHandle pictureH;
	Rect picFrame;
	PixMapHandle thePixMapH;
	Rect	creditsBox;

	//	The only animation is the credits
	//	Therefore, we can reduce the size of the offscreen worlds for the background
	//	and scratch drawing to just cover the area where the credits are drawn.

	DisposeGWorld(backWorldP);
	DisposeGWorld(drawWorldP);

	GetItemRect(ditl_about_creditsbox, &inCreditsRec->viewRect);

	//	Make GWorld for offscreen drawing.

	NewGWorld(&drawWorldP, Pixel_Depth, &inCreditsRec->viewRect, nil, nil, 0);
	thePixMapH = GetGWorldPixMap(drawWorldP);
	HLockHi((Handle)thePixMapH);
	LockPixels(thePixMapH);
	drawPixMapP = *thePixMapH;

	//	Make GWorld for background.

	NewGWorld(&backWorldP, Pixel_Depth, &inCreditsRec->viewRect, nil,
			  GetGWorldDevice(drawWorldP), noNewDevice);
	thePixMapH = GetGWorldPixMap(backWorldP);
	HLockHi((Handle)thePixMapH);
	LockPixels(thePixMapH);
	backPixMapP = *thePixMapH;
	SetGWorld(backWorldP, nil);
	pictureH = GetPicture(PICT_Background);
	picFrame = (**pictureH).picFrame;
	DrawPicture(pictureH, &picFrame);
	ReleaseResource((Handle)pictureH);

	//	Make GWorld for credits.
	
	creditsBox = inCreditsRec->viewRect;
	DrawTextIntoNewGWorld(TEXT_Credits, &creditsBox,
			Pixel_Depth, &inCreditsRec->theGWorldP);
			
	inCreditsRec->pictRect.left = 0;
	inCreditsRec->pictRect.top = 0;
	inCreditsRec->pictRect.right = inCreditsRec->viewRect.right -
		inCreditsRec->viewRect.left;
	inCreditsRec->pictRect.bottom = inCreditsRec->viewRect.bottom -
		inCreditsRec->viewRect.top;
		
	inCreditsRec->pictHeight = creditsBox.bottom;
}


void MoveOffWorld(GWorldPtr theGWorldP, Rect *inCurrPos, Rect *inNewPos,
				  Rect *inOffPos)
{
	Rect drawRect;
	PixMapHandle offPixMapH;

	//	Create image in draw world.

	SetGWorld(drawWorldP, nil);

	//	Restore background at current position.

	CopyBits((BitMapPtr)backPixMapP, (BitMapPtr)drawPixMapP, inCurrPos,
			 inCurrPos, srcCopy, nil);

	//	Draw image at new position.

	offPixMapH = GetGWorldPixMap(theGWorldP);
	LockPixels(offPixMapH);
	CopyBits((BitMapPtr)(*offPixMapH), (BitMapPtr)drawPixMapP, inOffPos,
			 inNewPos, srcCopy, nil);
	UnlockPixels(offPixMapH);

	//	Draw union of old and new position to Window.

#if PP_Target_Carbon
	SetGWorld(GetWindowPort(aboutBoxWindow), GetMainDevice());
#else
	SetGWorld(aboutBoxWindow, GetMainDevice());
#endif
	UnionRect(inCurrPos, inNewPos, &drawRect);
	CopyBits((BitMapPtr)drawPixMapP, (BitMapPtr)windowPixMapP, &drawRect,
			 &drawRect, srcCopy, nil);
}

void DoAnimation(void)
{
	CreditsRec theCreditsRec;
	AnimateState animCreditsFade = animate_Waiting;
	AnimateState animCreditsScroll = animate_Waiting;

	RGBColor fadeColor;
	short grayIndex = 0;
	unsigned short grayLevels[Fade_Levels] =  {
		4369, 8738, 17476, 21845, 30583, 34952, 43690, 48059, 52428, 56979,
		61166, 65535
	};
	theCreditsRec.theGWorldP = nil;
	animCreditsFade = animate_Active;
	SetUpCredits(&theCreditsRec);

#if PP_Target_Carbon
			SetGWorld(GetWindowPort(aboutBoxWindow), GetMainDevice());
#else
			SetGWorld(aboutBoxWindow, GetMainDevice());
#endif

	while (!Button()) {
		unsigned long ticks;
		unsigned long endTicks;
		unsigned long startTicks = TickCount();
		
		MoviesTask(nil, 500);

		
		if ((animCreditsFade == animate_Active) &&
			(grayIndex >= Fade_Levels)) {
			animCreditsFade = animate_Done;
			Delay(90, &ticks);
			animCreditsScroll = animate_Active;
		}
		if ((animCreditsScroll == animate_Active) &&
			(theCreditsRec.pictRect.top > theCreditsRec.pictHeight)) {
			theCreditsRec.pictRect.top = 1;
			theCreditsRec.pictRect.bottom = theCreditsRec.viewRect.bottom -
				theCreditsRec.viewRect.top + 1;
		}
		
		if (animCreditsFade == animate_Active) {
			PixMapHandle thePixMapH;

			Delay(3, &ticks);
			SetGWorld(drawWorldP, nil);

			thePixMapH = GetGWorldPixMap(theCreditsRec.theGWorldP);
			LockPixels(thePixMapH);
			CopyBits((BitMapPtr)(*thePixMapH), (BitMapPtr)drawPixMapP,
					 &theCreditsRec.pictRect, &theCreditsRec.viewRect,
					 srcCopy, nil);
			UnlockPixels(thePixMapH);

			fadeColor.red = fadeColor.blue = fadeColor.green =
				grayLevels[grayIndex++];
			RGBForeColor(&fadeColor);
			PenMode(adMin);
			PaintRect(&theCreditsRec.viewRect);
			PenMode(patCopy);
			ForeColor(blackColor);

			CopyBits((BitMapPtr)backPixMapP, (BitMapPtr)drawPixMapP,
					 &theCreditsRec.viewRect, &theCreditsRec.viewRect, adMax,
					 nil);

#if PP_Target_Carbon
			SetGWorld(GetWindowPort(aboutBoxWindow), GetMainDevice());
#else
			SetGWorld(aboutBoxWindow, GetMainDevice());
#endif
			CopyBits((BitMapPtr)drawPixMapP, (BitMapPtr)windowPixMapP,
					 &theCreditsRec.viewRect, &theCreditsRec.viewRect,
					 srcCopy, nil);
		}
		if (animCreditsScroll == animate_Active) {
			PixMapHandle thePixMapH;
			short i, j;
			short creditsWidth =
				theCreditsRec.viewRect.right -theCreditsRec.viewRect.left -1;
			Rect wrapViewRect, wrapPictRect;

			SetGWorld(drawWorldP, nil);

			theCreditsRec.pictRect.top += 1;
			theCreditsRec.pictRect.bottom += 1;

			thePixMapH = GetGWorldPixMap(theCreditsRec.theGWorldP);
			LockPixels(thePixMapH);
			if (theCreditsRec.pictRect.bottom <= theCreditsRec.pictHeight) {
				CopyBits((BitMapPtr)(*thePixMapH), (BitMapPtr)drawPixMapP,
						 &theCreditsRec.pictRect, &theCreditsRec.viewRect,
						 srcCopy, nil);
			} else {
				wrapViewRect = theCreditsRec.viewRect;
				wrapViewRect.top = wrapViewRect.bottom -
					(theCreditsRec.pictRect.bottom -
					 theCreditsRec.pictHeight);
				wrapPictRect = theCreditsRec.pictRect;
				wrapPictRect.top = 0;
				wrapPictRect.bottom = wrapPictRect.top +
					(wrapViewRect.bottom - wrapViewRect.top);
				CopyBits((BitMapPtr)(*thePixMapH), (BitMapPtr)drawPixMapP,
						 &theCreditsRec.pictRect, &theCreditsRec.viewRect,
						 srcCopy, nil);
				CopyBits((BitMapPtr)(*thePixMapH), (BitMapPtr)drawPixMapP,
						 &wrapPictRect, &wrapViewRect, srcCopy, nil);
			}
			UnlockPixels(thePixMapH);

			PenMode(adMin);					// Fade out at top and fade in at bottom
			j = Fade_Levels - 1;
			for (i = 0; i < j; i++) {
				fadeColor.red = fadeColor.blue = fadeColor.green =
					grayLevels[i];
				RGBForeColor(&fadeColor);
				MoveTo(theCreditsRec.viewRect.left,
					   theCreditsRec.viewRect.top + i);
				Line(creditsWidth, 0);
				MoveTo(theCreditsRec.viewRect.left,
					   theCreditsRec.viewRect.bottom - i - 1);
				Line(creditsWidth, 0);
			}
			ForeColor(blackColor);
			PenMode(patCopy);

			CopyBits((BitMapPtr)backPixMapP, (BitMapPtr)drawPixMapP,
					 &theCreditsRec.viewRect, &theCreditsRec.viewRect, adMax,
					 nil);
#if PP_Target_Carbon
			SetGWorld(GetWindowPort(aboutBoxWindow), GetMainDevice());
#else
			SetGWorld(aboutBoxWindow, GetMainDevice());
#endif
			CopyBits((BitMapPtr)drawPixMapP, (BitMapPtr)windowPixMapP,
					 &theCreditsRec.viewRect, &theCreditsRec.viewRect,
					 srcCopy, nil);
		}
		endTicks = TickCount(); //	Timing loop.
		if (endTicks == startTicks) {	//	TickCount must change during each pass thru animation loop.
			while (endTicks == TickCount())
				;			//	this puts an upper speed limit on the animation rate.
		}
		
		
#if PP_Target_Carbon
		QDFlushPortBuffer(GetWindowPort(aboutBoxWindow), nil);
#endif
	}


	// Finished animation (user clicked). Flush the mousedown events

	FlushEvents(mDownMask | mUpMask | keyDownMask | keyUpMask | autoKeyMask,
				0);

	if (theCreditsRec.theGWorldP != nil)
		DisposeGWorld(theCreditsRec.theGWorldP);

}

void CleanUpGraphics(void)
{
	DisposeGWorld(drawWorldP);
	DisposeGWorld(backWorldP);
	DisposeWindow((WindowPtr)aboutBoxWindow);
}

void GetItemRect(short inItem, Rect *outRect)
{
	DITLHand theDITL;

	theDITL = (DITLHand) GetResource('DITL', ditl_about);
	*outRect = (**theDITL).theUserItems[inItem - 1].box;
}

void DrawPictIntoNewGWorld(short inPICTid, short inDepth,
						   GWorldPtr *outGWorldP)
{
	PicHandle thePicture;
	Rect picFrame;
	GWorldPtr saveWorld;
	GDHandle saveDevice;
	OSErr err;

	thePicture = GetPicture(inPICTid);
	picFrame = (**thePicture).picFrame;

	err = NewGWorld(outGWorldP, inDepth, &picFrame, nil,
					GetGWorldDevice(drawWorldP), noNewDevice);

	GetGWorld(&saveWorld, &saveDevice);
	SetGWorld (*outGWorldP, nil);
	LockPixels(GetGWorldPixMap(*outGWorldP));
	DrawPicture(thePicture, &picFrame);
	UnlockPixels(GetGWorldPixMap(*outGWorldP));
	SetGWorld(saveWorld, saveDevice);

	ReleaseResource((Handle)thePicture);
}

void DrawTextIntoNewGWorld(short inTEXTid, Rect *ioBounds, short inDepth,
						   GWorldPtr *outGWorldP)
{
	Handle theText;
	TEHandle	theTE;
	StScrpHandle	theStyle;
	GWorldPtr saveWorld;
	GDHandle saveDevice;
	OSErr err;
	
		// Put Text in a TERecord
	
	TextFont(applFont);	
	TextSize(9);
	theTE = TEStyleNew(ioBounds, ioBounds);
	theText = GetResource('TEXT', inTEXTid);
	theStyle = (StScrpHandle) GetResource('styl', inTEXTid);
	HLock(theText);
	HidePen();
	TEStyleInsert(*theText, GetHandleSize(theText), theStyle, theTE);
	ShowPen();
	ReleaseResource(theText);
	if (theStyle != nil) {
		ReleaseResource((Handle)theStyle);
	}
	TESetAlignment(teJustCenter, theTE);
	TECalText(theTE);
	
		// Determine height of the Text

	ioBounds->right = ioBounds->right - ioBounds->left;
	ioBounds->left = ioBounds->top = 0;
	ioBounds->bottom = TEGetHeight((**theTE).nLines, 0, theTE);
	
		// Create new GWorld that is the height of the Text
	err = NewGWorld(outGWorldP, inDepth, ioBounds, nil,
					GetGWorldDevice(drawWorldP), noNewDevice);

	GetGWorld(&saveWorld, &saveDevice);
	SetGWorld (*outGWorldP, nil);
	LockPixels(GetGWorldPixMap(*outGWorldP));
	
	EraseRect(ioBounds);

		// Draw Text inside GWorld
	(**theTE).viewRect = *ioBounds;
	(**theTE).destRect = *ioBounds;
	(**theTE).inPort = (GrafPtr) *outGWorldP;
	TEUpdate(ioBounds, theTE);
	TEDispose(theTE);
	
	InvertRect(ioBounds);			// White letters on black background

	UnlockPixels(GetGWorldPixMap(*outGWorldP));
	SetGWorld(saveWorld, saveDevice);
}

