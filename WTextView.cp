//=================================================================
// ¥ WTextView
// Copyright© Timothy Paustian All Rights Reserved 1995-1998
//
// <mailto:paustian@bact.wisc.edu>
// <http://www.bact.wisc.edu/WTText/overview.html>
//=================================================================
//
// Purpose:
//
// File Creation Date:		3/5/99 8:25 PM
// Class Creation Date:		2/14/99 12:38 PM
// Class Last Modified:		2/14/99 2:20 PM
// Revision:
//
//=================================================================


#include "WTextView.h"
#include "WTextModel.h"
#include "UDebugNew.h"

#include <UCursor.h>
#include <UDrawingState.h>
#include <LStream.h>
#include <UMemoryMgr.h>
#include <UTextTraits.h>
#include <LScrollerView.h>
#include <PP_Messages.h>
#include <PP_DebugMacros.h>
#include <UKeyFilters.h>
#include <PP_KeyCodes.h>
#include <TArrayIterator.h>
#include <TArray.h>
#include <LHandleStream.h>
#include <UAppleEventsMgr.h>
#include <LString.h>
#include <UResourceMgr.h>
#include "WETabs.h"

#ifndef __SOUND__
#include <Sound.h>
#endif

#ifndef __AEREGISTRY__
#include <AERegistry.h>
#endif


#if	__PowerPlant__ 
									//these macros turn on debugging test for the text engine
	#include <PP_DebugMacros.h>	//The ones I use are in PPlant. WT_Debug contains just defines
									//as empty. You will have to roll your own if you want to use 
#else								//something else. Sorry :-)
#include "W_Debug.h"
#endif

SInt32	WTextView::sMaxScrollDelta 	= 32;
const	ResID	rUndoStringsID		= 128;
const	ResID	rRedoStringsID		= 129;

// ---------------------------------------------------------------------------
//	¥WTextView
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//


WTextView::WTextView()
:
	LDragAndDrop(
#if PP_Target_Carbon
		UQDGlobals::GetCurrentWindowPort(),
#else
		UQDGlobals::GetCurrentPort(),
#endif
		this)
{
	mForeColor = Color_Black;
	
	mBackColor = Color_White;
	
	mInitialTextID = 0;
	
	mTextAttributes = WDefaultAttrs;
	UInt32 wasteFlags = WDefaultFlags;
	
	InitTextView(wasteFlags);	// Initialize member variables
	AlignWasteViewRects();
}

// ---------------------------------------------------------------------------
//	¥WTextView
// ---------------------------------------------------------------------------
//
// Purpose: Construct from input parameters. 
//
// Revision:
//


WTextView::WTextView(
		const SPaneInfo& inPaneInfo,
		const SViewInfo& inViewInfo,
		UInt16 inTextAttributes,
		ResIDT inTextTraitsID,
		ResIDT inInitialTextID,
		UInt32 inWasteFlags )
:
	LView(
		inPaneInfo,
		inViewInfo),
	LDragAndDrop(
#if PP_Target_Carbon
		UQDGlobals::GetCurrentWindowPort(),
#else
		UQDGlobals::GetCurrentPort(),
#endif
		this)
{
	mForeColor = Color_Black;
	mBackColor = Color_White;

	mTextTraitsID = inTextTraitsID;
	mTextAttributes = inTextAttributes;
	mInitialTextID = inInitialTextID;
	
	InitTextView(inWasteFlags);
	AlignWasteViewRects();}

// ---------------------------------------------------------------------------
//	¥WTextView
// ---------------------------------------------------------------------------
//
// Purpose: construct from a stream. This will be the most common usage
//
// Revision:
//


WTextView::WTextView(
		LStream* inStream )
:
	LView(
		inStream),
	LDragAndDrop(
#if PP_Target_Carbon
		UQDGlobals::GetCurrentWindowPort(),
#else
		UQDGlobals::GetCurrentPort(),
#endif
		this)
{	
	//clear the attributes field, necessary for backward compatibility
	mTextAttributes = 0;
	
	// get the Text attributes (multistyle, editable, selectable wordwrap.
	*inStream >> mTextAttributes;
	
	// read in text traits ID
	*inStream >>mTextTraitsID;
	
	// get text resource ID
	*inStream >> mInitialTextID;
	
	UInt32 wasteFlags;
	
	mForeColor = Color_Black;
	
	// get the WASTE attributes
	*inStream >> wasteFlags;
	// read in the background color.
	inStream->ReadData(&mBackColor, sizeof(mBackColor));

	InitTextView(wasteFlags);
	AlignWasteViewRects();}

// ---------------------------------------------------------------------------
//	¥WTextView
// ---------------------------------------------------------------------------
//
// Purpose: Copy constructor
//
// Revision:
//


WTextView::WTextView(
		const WTextView & inOriginal )
: 	LView(inOriginal),
	LCommander(inOriginal),
#if PP_Target_Carbon
	LDragAndDrop(UQDGlobals::GetCurrentWindowPort(),this),
#else
	LDragAndDrop(UQDGlobals::GetCurrentPort(),this),
#endif
//	WText(inOriginal),
	LBroadcaster(inOriginal)
{
	//LView's copy constructor puts this view inside nothing,
	//but we'll need to be inside a view by the time we get to 
	//InitWasteEdit. The user will have to PutInside() after construction
	//if they want a different superview.
	mSuperView = inOriginal.mSuperView;	
	
	//get member data from the old instance
	mInScroller = inOriginal.mInScroller;

	mSavePos = inOriginal.mSavePos;
	mPrintPanels = new TArray<SInt32>((inOriginal.mPrintPanels)->GetItemsHandle());
	mTextAttributes = inOriginal.mTextAttributes;
	mForeColor = inOriginal.mForeColor;
	mBackColor = inOriginal.mBackColor;

	mTextTraitsID = inOriginal.mTextTraitsID;
	mScroller = inOriginal.mScroller;
	
	mInitialTextID = inOriginal.mInitialTextID;
	
	UInt32			wasteFlags;
	
	//set the waste flags
	wasteFlags = 0;
	if(inOriginal.FeatureFlag(weFInhibitColor, weBitTest))
		wasteFlags |= (1UL << weFInhibitColor);
	if(inOriginal.FeatureFlag(weFMonoStyled, weBitTest))
		wasteFlags |= (1UL << weFMonoStyled);
	if(inOriginal.FeatureFlag(weFInhibitRedraw, weBitTest))
		wasteFlags |= (1UL << weFInhibitRedraw);
	if(inOriginal.FeatureFlag(weFInhibitColor, weBitTest))
		wasteFlags |= (1UL << weFInhibitColor);
	if(inOriginal.FeatureFlag(weFDrawOffscreen, weBitTest))
		wasteFlags |= (1UL << weFDrawOffscreen);
	if(inOriginal.FeatureFlag(weFUseTempMem, weBitTest))
		wasteFlags |= (1UL << weFUseTempMem);
	if(inOriginal.FeatureFlag(weFInhibitRecal, weBitTest))
		wasteFlags |= (1UL << weFInhibitRecal);
	if(inOriginal.FeatureFlag(weFDragAndDrop, weBitTest))
		wasteFlags |= (1UL << weFDragAndDrop);
	if(inOriginal.FeatureFlag(weFIntCutAndPaste, weBitTest))
		wasteFlags |= (1UL << weFIntCutAndPaste);
	if(inOriginal.FeatureFlag(weFUndo, weBitTest))
		wasteFlags |= (1UL << weFUndo);
	if(inOriginal.FeatureFlag(weFReadOnly, weBitTest))
		wasteFlags |= (1UL << weFReadOnly);
	if(inOriginal.FeatureFlag(weFOutlineHilite, weBitTest))
			wasteFlags |= (1UL << weFOutlineHilite);
	if(inOriginal.FeatureFlag(weFAutoScroll, weBitTest))
		wasteFlags |= (1UL << weFAutoScroll);
	
	LongRect    	viewRect, destRect;
	
	//get data from the WASTE handle
	inOriginal.GetDestRect(destRect);
	inOriginal.GetViewRect(viewRect);
	
	//now set the values for this instance
	InitTextView(wasteFlags);
	
	mMargins = inOriginal.mMargins;
	
	SetViewRect(viewRect);
	SetDestRect(destRect);
	AlignWasteViewRects();
	
	SInt32			textLength;
	StHandleBlock  	text(0L);
	StHandleBlock	style(0L);
	StHandleBlock  	soup(0L);
	StHandleBlock	ruler(0L);
	StHandleBlock	para(0L);
	
	inOriginal.CopyRange(0, LONG_MAX, text, (StScrpHandle)style.Get(), soup, ruler, para);
	textLength = inOriginal.GetTextLength();
	InsertText(text, (StScrpHandle)style.Get(), soup, ruler, para, wDeleteBeforeInsert + wRefreshAfterInsert);
}

// ---------------------------------------------------------------------------
//	¥~WTextView
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//


WTextView::~WTextView()
{
	if (mWasteH != nil) {
		WEDispose(mWasteH);
	}
	// get rid of the info since we are dying. This was causing a memory leak.
	// 12/10/96
	DisposeOf_(mPrintPanels);
	if(mTextModel != nil)
		DisposeOf_(mTextModel);
}

// ---------------------------------------------------------------------------
//	¥InitTextView
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

void
WTextView::InitTextView(
		UInt32 inWasteFlags )
{
	LongRect	viewLRect;
	LongRect	destLRect;
	Rect		viewRect;
	Rect		destRect;

	SignalIf_(inWasteFlags < 0);
	
	// Get the size of the frame
	CalcLocalFrameRect(viewRect);

	mScroller = nil;
	mTextModel = nil;
	
	AdaptToEnclosingScroller();
	
	destRect = viewRect;
	
	// if wrapping is on, use the default small size
	if (mTextAttributes & wasteAttr_WordWrap) {
		destRect.right = destRect.left + mFrameSize.width;
	}
	else 
	{
		if (mImageSize.width > 0)
		{
			//Check to make sure this is not too small.
			//if you want it less than 200 remove this code.
			SignalIf_(mImageSize.width < 200); 
			destRect.right = destRect.left + mImageSize.width;
		}
		else	// otherwise use the large size
			destRect.right = destRect.left + WDefaultNoWrapWidth;
	}
	// we need to protect ourselves since the WASTE instance
	// is not built yet.

	Boolean oldRecon = mReconcileOverhang;
	mReconcileOverhang = false;
	// resize the image to whatever we decided above
	ResizeImageTo(	(destRect.right - destRect.left),
					(destRect.bottom - destRect.top), false);
	mReconcileOverhang = oldRecon;
	
	// make LongRects for WENew call
	WERectToLongRect(&viewRect, &viewLRect);
	WERectToLongRect(&destRect, &destLRect);
					
	WENew(&destLRect, &viewLRect, inWasteFlags, &mWasteH);
	
	// Store the wasteEdit handle in the refcon
	// we use this in the clickloop
	WTextView *	wastePtr = this;
	SetInfo(weRefCon, &wastePtr);
	
	// set up the clickLoop. Note that this uses UPPs
	// therefore it is stable on PPC as well as 68k
	// if this is CFM code
	static WEClickLoopUPP		clickLoop = nil;
	if (clickLoop == nil)
		clickLoop = NewWEClickLoopProc(MyClickLoop);
	SetInfo(weClickLoop, (Ptr)&clickLoop);
	
	static WEHiliteDropAreaUPP	dragProc = nil;
	if (dragProc == nil)
		dragProc = NewWEHiliteDropAreaProc(MyDragHiliteProc);
	SetInfo(weHiliteDropAreaHook, (Ptr)&dragProc);

	TextTraitsH	theTextTraits = nil;
	TextStyle	ts;
	RGBColor	defaultColor = WDefaultColor;
	
	if (mTextTraitsID != 0)
		theTextTraits = UTextTraits::LoadTextTraits(mTextTraitsID);

	if (theTextTraits == nil) 
	{
			// Set the default font, size and face.
		ts.tsFont = WDefaultFont;
		ts.tsSize = WDefaultSize;
		ts.tsFace = WDefaultFace;
		ts.tsColor = defaultColor;

		SetStyle( weDoAll, ts);
				
			// and alignment
				
		SetAlignment( weFlushDefault);
	}
	else 
	{
		ts.tsFont = (*theTextTraits)->fontNumber;
		ts.tsSize = (*theTextTraits)->size;
		ts.tsFace = (*theTextTraits)->style;
		ts.tsColor = (*theTextTraits)->color;
				
		SetStyle( weDoAll, ts);
				
#ifdef WE_JUSTIFY_BY_DEFAULT
		if ((*theTextTraits)->justification == weFlushDefault) 
		{
			(*theTextTraits)->justification = weJustify;
		}
#endif

		SetAlignment( (*theTextTraits)->justification);
	}

#ifdef WE_USE_TABS
	if (wasteAttr_TabSupport & mTextAttributes) 
	{
		WEInstallTabHooks(mWasteH);
	}
#endif	
	
	// a list of the panels offsets of a print record.
	// here we create an empty list with each item being the size of
	// and SInt32
	#pragma warn_hidevirtual off
	mPrintPanels = new TArray<SInt32>;
	#pragma warn_hidevirtual reset
	
	//added support for style and soup resources
	if(mInitialTextID != 0)
	{
		StResourceContext	theContext;
		StResource	initialText('TEXT', mInitialTextID);
		//for the next two, we do not want to fail if we don't find
		//this information. It is OK not to do anything, because the 
		//data handles will be nil going into InsertText. WASTE knows 
		//to ignore nil handles.
		StResource	initialStyle('styl', mInitialTextID, false, true);
		StResource	initialSoup('SOUP', mInitialTextID, false, true);
		
		StFeatureFlag	noUndo(weFUndo, weBitClear, mWasteH);
		
		InsertText(initialText, (StScrpRec**)initialStyle.Get(), 
						initialSoup);
		SetTextSelection(0, 0);
	}
}

// ---------------------------------------------------------------------------
//	¥MyDragHiliteProc
// ---------------------------------------------------------------------------
//
// Purpose: I let WASTE take this over. We call the PPlant functions but waste
//  decides when. This removed a whole bunch of hiliting anomilies during 
// 	scrolling. Yippee!
//
// Revision:
//

pascal OSErr
WTextView::MyDragHiliteProc(
		DragReference drag,
		Boolean hiliteFlag,
		WEReference hWE)
{
	OSErr	theErr = noErr;
	try
	{
		WTextView *	wastePtr;
#warning broken for Carbon?
#if PP_Target_Carbon
		WEGetInfo(weRefCon, &wastePtr, hWE);
#else
		WEGetInfo(weRefCon, &wastePtr, hWE);
#endif
		
		ValidateObj_(wastePtr);
		
		Point		where;
		
		::GetMouse(&where);
		if(!hiliteFlag )
		{
			if(wastePtr->mIsHilited)
			{
				wastePtr->UnhiliteDropArea(drag);
				wastePtr->mIsHilited = false;
			}
		}
		else
		{
			wastePtr->HiliteDropArea(drag);
		}
	}
	catch(LException & inErr)
	{
		theErr = inErr.GetErrorCode();
	}
	
	return theErr;
}

// ---------------------------------------------------------------------------
//	¥MyClickLoop
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

pascal Boolean
WTextView::MyClickLoop(
		WEReference hWE )
{
	Boolean	rval = true;
	
	try
	{
		Point			where;
		Rect			view;
		WTextView *	wasteView;
		SInt16			scrollUnit;
		
		WEGetInfo(weRefCon, &wasteView, hWE);
		
		ValidateObj_(wasteView);
		
		wasteView->CalcLocalFrameRect(view);
		::GetMouse(&where);
		
		// check to see if theSuperView is a LScroller. If it is, we want to 
		// scroll the view otherwise don't I love RTTI!
		if (wasteView->mInScroller) { // see PointInDropArea for explanation
			// is dragged out of the view, but is still inside the superView
			if (where.v >= view.bottom) {
				scrollUnit = where.v - view.bottom;
				scrollUnit = (scrollUnit < sMaxScrollDelta) ? scrollUnit : sMaxScrollDelta;
				wasteView->ScrollPinnedImageBy(0, 1 * scrollUnit, true);
			}							
			else if (where.h >= view.right) {
				scrollUnit = where.h - view.right;
				scrollUnit = (scrollUnit < sMaxScrollDelta) ? scrollUnit : sMaxScrollDelta;
				wasteView->ScrollPinnedImageBy(1 * scrollUnit, 0, true);
			}	
			else if (where.v <= view.top) {
				scrollUnit = view.top - where.v;
				scrollUnit = (scrollUnit < sMaxScrollDelta) ? scrollUnit : sMaxScrollDelta;
				wasteView->ScrollPinnedImageBy(0, -1 * scrollUnit, true);
			}
			else if (where.h <= view.left) {
				scrollUnit = view.left - where.h;
				scrollUnit = (scrollUnit < sMaxScrollDelta) ? scrollUnit : sMaxScrollDelta;
				wasteView->ScrollPinnedImageBy(-1 * scrollUnit, 0, true);
			}
			
		}
	}
	catch (...)
	{
		//don't know what else to do here, but stop the scrollling.
		//Maybe it would be better to let this propigate?
		rval = false;
	}
	// if we don't have a Scroller
	return rval;
}

// ---------------------------------------------------------------------------
//	¥SetTextModel
// ---------------------------------------------------------------------------
//
// Purpose: Set the text model for apple script. This class takes ownership
// of the model and will delete it.
//
// Revision:
//

void
WTextView::SetTextModel(
					WTextModel *	inTextModel)
{
	
	mTextModel = inTextModel;
	
	if(mTextModel != nil)
	{
		ValidateObj_(inTextModel);
		mTextModel->SetSearchPointers(0, GetTextLength());
	}
}

WTextModel*
WTextView::GetTextModel()
{
	return mTextModel;
}

// ---------------------------------------------------------------------------
//	¥SetStyle
// ---------------------------------------------------------------------------
//
// Purpose: override to set the style using apple events if possible.
//
// Revision:
//


OSErr
WTextView::AESetStyle(
		WEStyleMode			inMode,
		TextStyle			inTextStyle,
		bool				inDoAppleEvent)// = true
{
	OSErr	anErr = noErr;
	
	if(mTextModel != nil && inDoAppleEvent)
	{
		ValidateObj_(mTextModel);
		//always reset the serach pointers before doing a set
		//style
		mTextModel->SetSearchPointers(0, GetTextLength());
		StAEDescriptor	event;
		mTextModel->CreateStyle(inMode, inTextStyle, event);
		UAppleEventsMgr::SendAppleEvent(event);
	}
	else
	{
		anErr = WText::SetStyle(inMode, inTextStyle);
	}
	
	//the size of the view and dest rect may
	//change after a style call
	AdjustImageToText();
	
	CheckScroll();
	
	UserChangedText();
	
	SetUpdateCommandStatus(true);
	
	return anErr;
}

// ---------------------------------------------------------------------------
//	¥StyleCheck
// ---------------------------------------------------------------------------
//
// Purpose: A handy routine for setting the checkmarks in a menu.
//
// Revision:
//
 void			
 WTextView::StyleCheck(
				TextStyle			inStyle,
				Boolean &			outEnabled,
				Boolean &			outUsesMark,
				UInt16 &			outMark)
{
	outEnabled = true;
	outUsesMark = true;
	outMark = noMark;
	
	WEStyleMode		mode;
	bool		isContinuous;
	TextStyle	ts;
	//initialize to 0 to avoid extraneous stuff.
	ts.tsFace = 0;
	
	if(inStyle.tsFace != -1)	
		{//determine which style attributes are continuous over the current selection range
		//we'll need this information in order to check the Font/Size/Style menus properly
		mode = weDoFace;		//query about these attributes
		isContinuous = ContinuousStyle(mode, ts);
		if (isContinuous)
		{
			if ((ts.tsFace == normal) && (inStyle.tsFace == normal))
			{
				outUsesMark = true;
				outMark = checkMark;
			}
			else if(inStyle.tsFace & ts.tsFace)
			{
				outUsesMark = true;
				outMark = checkMark;
			}
		}
	} 
	else 
	{
		mode = weDoColor;
		isContinuous = ContinuousStyle(mode, ts);
		if(isContinuous)
		{
			RGBColor	* styleColor = &(inStyle.tsColor);
			RGBColor	* textColor = &(ts.tsColor);
			SInt32	comparison = BlocksAreEqual((Ptr)styleColor, (Ptr)textColor, sizeof(RGBColor));
			
			if(comparison == 0)
			{
				outUsesMark = true;
				outMark = checkMark;
			}
		}
	}
}

// ---------------------------------------------------------------------------
//	¥SetTextSelection
// ---------------------------------------------------------------------------
//
// Purpose: If during selection we scroll, then the scroll bars will need'
// to be adjusted. Also, changing the seleciton may cause a change
// in the menu items.
//
// Revision:
//

void
WTextView::SetTextSelection(
					SInt32		inStart,
					SInt32		inEnd)
{
	if(mTextAttributes & wasteAttr_Selectable)
	{
		WText::SetTextSelection(inStart, inEnd);
		CheckScroll();
		SetUpdateCommandStatus(true);
	}
}
	
// ---------------------------------------------------------------------------
//	¥InsertText
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

void
WTextView::InsertText(
		Ptr inTextPtr,
		SInt32 inTextLength,
		StScrpHandle inStylesH,
		Handle inSoupH,
		Handle inParaFormat,
		Handle inRulerScrap,
		UInt32 inFlags)
{
	//Check first two variables, the rest are allowed to be nil
	ValidatePtr_(inTextPtr);
	Assert_(inTextLength > 0);
	ValidateHandle_((Handle)mWasteH);
	
	{
		StFeatureFlag	noCalc(weFInhibitRecal, weBitSet, mWasteH);
	
		if (inFlags & wDeleteBeforeInsert)
			SelectAll();
	}
	
	SInt8	prevDraw;
	if(!(inFlags & wRefreshAfterInsert)) 
		prevDraw = FeatureFlag(weFInhibitRedraw, weBitSet);

	//
	// insert the text
	//
	{
		StFeatureFlag	clearReadOnly(weFReadOnly, weBitClear, mWasteH);
		FocusDraw();
		ThrowIfOSErr_(InsertFormattedText(inTextPtr, inTextLength,
						inStylesH, inSoupH, inParaFormat, inRulerScrap));
	}
	
	if(!(inFlags & wRefreshAfterInsert))
		FeatureFlag(weFInhibitRedraw, prevDraw);

	AdjustImageToText();
	
	CheckScroll();
	
	if (inFlags & wRefreshAfterInsert) 
	{
		FocusDraw();
		Refresh();
	}
	
	UserChangedText();
}

// ---------------------------------------------------------------------------
//	¥InsertText
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

void
WTextView::InsertText(
		Handle inTextH,
		StScrpHandle inStylesH,
		Handle inSoupH,
		Handle inParaFormat,
		Handle inRulerScrap,
		UInt32 inFlags)
{
	ValidateHandle_((Handle)mWasteH);
	ValidateHandle_(inTextH);
	
	{
		StFeatureFlag	noCalc(weFInhibitRecal, weBitSet, mWasteH);
	
		if (inFlags & wDeleteBeforeInsert)
			SelectAll();
	}
	
	bool	doRefresh = (inFlags & wRefreshAfterInsert) == true;
	SInt8	prevDraw;
	if(!doRefresh) 
		prevDraw = FeatureFlag(weFInhibitRedraw, weBitSet);

	//
	// insert the text
	//
	{
		StFeatureFlag	clearReadOnly(weFReadOnly, weBitClear, mWasteH);
		StHandleLocker	theLock(inTextH);
		SInt32	hSize = ::GetHandleSize(inTextH);
		FocusDraw();
		ThrowIfOSErr_(InsertFormattedText(*(inTextH), hSize,
						inStylesH, inSoupH, inParaFormat, inRulerScrap));
	}
	
	if(!(doRefresh))
		FeatureFlag(weFInhibitRedraw, prevDraw);

	AdjustImageToText();
	
	CheckScroll();
	
	if (doRefresh) {
		FocusDraw();
		Refresh();
	}
	
	UserChangedText();
}

// ---------------------------------------------------------------------------
//	¥InsertFromNativeH
// ---------------------------------------------------------------------------
//
// Purpose: A utility routine that takes a single handle and extracts all the
// needed parts (text, style, soup, paraformat and ruler from it.
//
// Revision:
//
void
WTextView::InsertFromNativeH(
					Handle	inTextData,
					UInt32	inFlags)
{
	ValidateHandle_(inTextData);
	
	LHandleStream *	streamHandle = new LHandleStream(inTextData);
	//create the handles
	Handle	theText=nil;
	Handle	theStyles=nil;
	Handle	theSoup=nil;
	Handle	theParaFormat=nil;
	Handle	theRuler=nil;
	try 
	{
		//read all the data out of the incoming composite handle
		*streamHandle>>theText;
		ValidateHandle_(theText);
		*streamHandle>>theStyles;
		ValidateHandle_(theStyles);
		*streamHandle>>theSoup;
		ValidateHandle_(theSoup);
		//there isn't always a soup. If the size of the handle is 0, then delete it
		if(::GetHandleSize(theSoup) == 0)
		{
			::DisposeHandle(theSoup);
			theSoup = nil;
		}
		#if WASTE_VERSION > 0x02000000
		*streamHandle>>theParaFormat;
		ValidateHandle_(theParaFormat);
		*streamHandle>>theRuler;
		ValidateHandle_(theRuler);
		#endif
		//finally insert the text
		InsertText(theText, (StScrpHandle)theStyles, theSoup, theParaFormat, theRuler, inFlags);
		//now clean up
		if(theText != nil)
			::DisposeHandle(theText);
		theText = nil;
		if(theStyles != nil)
			::DisposeHandle(theStyles);
		theStyles = nil;
		if(theSoup != nil)
			::DisposeHandle(theSoup);
		theSoup = nil;
		if(theParaFormat != nil)
			::DisposeHandle(theParaFormat);
		theParaFormat = nil;
		if(theRuler != nil)
			::DisposeHandle(theRuler);
		theRuler = nil;
		
		delete streamHandle;
	}
	catch(...)
	{
		//clean up the handles before we leave and then propigate the error
		if(theText != nil)
			::DisposeHandle(theText);
		theText = nil;
		if(theStyles != nil)
			::DisposeHandle(theStyles);
		theStyles = nil;
		if(theSoup != nil)
			::DisposeHandle(theSoup);
		theSoup = nil;
		if(theParaFormat != nil)
			::DisposeHandle(theParaFormat);
		theParaFormat = nil;
		if(theRuler != nil)
			::DisposeHandle(theRuler);
		theRuler = nil;
		
		delete streamHandle;
		//now that we have cleaned up, rethrow the error
		throw;
	}
}

// ---------------------------------------------------------------------------
//	¥CopyIntoNativeH
// ---------------------------------------------------------------------------
//
// Purpose: A utitliy routine that creates a single handle containing all the
// parts of a WASTE 2.0 text stream (text, style, soup, ruler and paraformat.
//
// Revision:
//

Handle
WTextView::CopyIntoNativeH(
				SInt32	inRangeStart,
				SInt32	inRangeEnd)
{
	//make sure we are cool
	Assert_(inRangeStart >= 0);
	Assert_(inRangeEnd >= 0);
	Assert_(inRangeStart < inRangeEnd);
	
	LHandleStream	* streamHandle = new LHandleStream();
	ValidatePtr_(streamHandle);
	Handle retHandle = nil;
	
	try
	{
		StHandleBlock	currData(0L);

		//grab the text
		ThrowIfOSErr_(StreamRange(inRangeStart, inRangeEnd, kTypeText, 0, currData));
		
		ValidateHandle_(currData.Get());
		
		//This should be safe since LHandleStream::SetLength creates
		//the data handle, if it is nil. Doing it this way
		//makes sure we get both the size of the data handle
		//and the data written to the stream.
		*streamHandle<<currData;
		
		//copy the styles
		ThrowIfOSErr_(StreamRange(inRangeStart, inRangeEnd, kTypeStyles, 0, currData));
		ValidateHandle_(currData.Get());
		*streamHandle<<currData;
		//copy the soup
		ThrowIfOSErr_(StreamRange(inRangeStart, inRangeEnd, kTypeSoup, 0, currData));
		ValidateHandle_(currData.Get());
		*streamHandle<<currData;
		
		#if WASTE_VERSION > 0x02000000
		//copy the para
		ThrowIfOSErr_(StreamRange(inRangeStart, inRangeEnd, kTypeParaFormat, 0, currData));
		ValidateHandle_(currData.Get());
		*streamHandle<<currData;
		//copy the ruler
		ThrowIfOSErr_(StreamRange(inRangeStart, inRangeEnd, kTypeRulerScrap, 0, currData));
		ValidateHandle_(currData.Get());
		*streamHandle<<currData;
		
		#endif
		//detach the data handle from the stream so that we don't loose
		//it when we delete LHandleStream
		retHandle = streamHandle->DetachDataHandle();
		
		DisposeOf_(streamHandle);
	}
	catch(...)
	{
		DisposeOf_(streamHandle);
		throw;
	}
	//make sure our handle is good before we send it off
	ValidateHandle_(retHandle);
	return retHandle;	
}

// ---------------------------------------------------------------------------
//	¥IsInSelection
// ---------------------------------------------------------------------------
//
// Purpose: Function takes in a mouse locaton in global coordinates and
//	determines if it is over the selction.
//
// Revision:
//

Boolean
WTextView::IsInSelection(
		Point inMouseLoc )
{
	SInt32	selStart, selEnd;
	char	dummyEdge = '\0';
	Boolean	onTopOfSelection = false;
	
	// get the selection Range
	GetTextSelection(selStart, selEnd);
	// if we have a selection range
	if (selEnd != selStart) {
		StRegion hiliteRgn(GetHiliteRgn(selStart, selEnd), false);
		OffsetRgn(hiliteRgn, -mPortOrigin.h, -mPortOrigin.v);
		// if this is on top of the selection then we want to adjust
		// the cursor we may need to adjust the ptLocation
		onTopOfSelection = ::PtInRgn(inMouseLoc, hiliteRgn);
	}
	return onTopOfSelection;
}

// ---------------------------------------------------------------------------
//	¥AdjustImageToText
// ---------------------------------------------------------------------------
//
// Purpose: This adjusts the image size to match the destRect.
//
// Revision:
//

void
WTextView::AdjustImageToText()
{
	LongRect theLongDestRect;
	GetDestRect(theLongDestRect);
		
	ResizeImageTo((theLongDestRect.right - theLongDestRect.left),
			(theLongDestRect.bottom - theLongDestRect.top), true);
}

// ---------------------------------------------------------------------------
//	¥AlignwasteViewRects
// ---------------------------------------------------------------------------
//
// Purpose: Align the view and destination rectangles of the Waste pane
//	This routine insures that the Frame.topLeft == viewRect.topLeft and
//	Image.topLeft == destRect.topLeft
//
// Revision:
//

void
WTextView::AlignWasteViewRects()
{
	Rect textFrame;
	
	if (FocusDraw() && CalcLocalFrameRect(textFrame)) {
		// make sure the frame and the viewRect are the same.
		LongRect theLongViewRect, theLongDestRect;
		WERectToLongRect(&textFrame, &theLongViewRect);
		SetViewRect(theLongViewRect);
		
		// make sure the image and the destRect are the same
		
		theLongDestRect.top = mImageLocation.v + mPortOrigin.v;
		theLongDestRect.left = mImageLocation.h + mPortOrigin.h;
		theLongDestRect.bottom = theLongDestRect.top + mImageSize.height;
		theLongDestRect.right = theLongDestRect.left + mImageSize.width;

		SetDestRect(theLongDestRect);
		
		if (mTextAttributes & wasteAttr_WordWrap) {
			CalText();
		}
	}
}

// ---------------------------------------------------------------------------
//	¥ReconcileFrameAndImage
// ---------------------------------------------------------------------------
//
// Purpose: Adjusts the Image so that it fits within the Frame
//
//	This is useful for short text blocks in a frame that you want to always
//	be at the top when the dest rect shrinks.
//
// Revision:
//

void
WTextView::ReconcileFrameAndImage(
		Boolean inRefresh )
{
	LView::ReconcileFrameAndImage(inRefresh);
	if (mReconcileOverhang) {
		LongRect destRect, viewRect;
		GetDestRect(destRect);
		GetViewRect(viewRect);
		SInt32 destHeight = destRect.bottom - destRect.top;
		if (destHeight < viewRect.bottom - viewRect.top) {
			if (destRect.top != viewRect.top) {
				destRect.top = viewRect.top;
				destRect.bottom = destHeight;
				SetDestRect(destRect);
				Update(nil);
				CheckScroll();
			}		
		}
	}
}

// ---------------------------------------------------------------------------
//	¥AdaptToNewSurroundings
// ---------------------------------------------------------------------------
//
// Purpose: The surroundings changed, take appropriate action. In this case it
// means to see if we are inside a scrolling view.
//
// Revision:
//

void
WTextView::AdaptToNewSurroundings()
{
	LView::AdaptToNewSurroundings();

	AdaptToEnclosingScroller();
}

// ---------------------------------------------------------------------------
//	¥AdaptToEnclosingScroller
// ---------------------------------------------------------------------------
//
// Purpose: Find if we are inside a scroller. Also set the margins. Note that
// we now only look for LScrollerViews since LScroller is depreciated
//
// Revision: 3/24/99 - Changed to that scrollers are still looked for even
// if the user doesn't want margins kept.
//		

void
WTextView::AdaptToEnclosingScroller()
{
	//You can now set this flag in the PPob.
	// figure out the margin between the scroller (if there is one)
		// and this viewRect. This gets used later
	LView *theSuperView = GetSuperView();
	while (theSuperView
				&&  (dynamic_cast<LScrollerView *>(theSuperView) == nil))
	{
		theSuperView = theSuperView->GetSuperView();
	}
	
	if (theSuperView == nil) 
	{
		mMargins.v = mMargins.h = 0;
		mInScroller = false;
		mScroller = nil;
	}
	else 
	{
		Rect portViewRect;
		CalcPortFrameRect(portViewRect);
		Rect scrollRect;
		theSuperView->CalcPortFrameRect(scrollRect);

		mInScroller = true;
		// This is safe since above we determined this is a LScroller
		mScroller = theSuperView;
		
		if (!(mTextAttributes & wasteAttr_DoMargins) || !mFrameBinding.left || !mFrameBinding.right) 
			mMargins.h = 0;
		else
			mMargins.h = portViewRect.left - scrollRect.left;
		
		if (!(mTextAttributes & wasteAttr_DoMargins) || !mFrameBinding.top || !mFrameBinding.bottom)
			mMargins.v = 0;
		else
			mMargins.v = portViewRect.top - scrollRect.top;
			
	}
}

// ---------------------------------------------------------------------------
//	¥AdjustCursorSelf
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

void
WTextView::AdjustCursorSelf(
		Point inPortPt,
		const EventRecord& /*inMacEvent*/ )
{
	if (FocusDraw()) {
		PortToGlobalPoint(inPortPt);
		if (!WText::AdjustCursor(inPortPt, nil)) {
			// Waste failed
			UCursor::SetIBeam();
		}
	}
}

// ---------------------------------------------------------------------------
//	¥ObeyCommand
// ---------------------------------------------------------------------------
//
// Purpose: Handle common commands for a Waste view. There are not many commands
// here since the view should not be handling commands, but it's controller should
// in most cases that would be the document class.
//
// Revision:
//

Boolean
WTextView::ObeyCommand(
		CommandT 	inCommand,
		void* 		ioParam )
{
	Boolean	cmdHandled = true;
	Boolean	doScrollCheck = true;
	AppleEvent	theEvent;
				
	FocusDraw();

	switch (inCommand) {
		case cmd_Undo:
			if (FeatureFlag(weFReadOnly, weBitTest))
				break;
			ThrowIfOSErr_(Undo());
			UserChangedText();
			SetUpdateCommandStatus(true);
			break;
		
		#if WASTE_VERSION > 0x02000000
	
		case cmd_Redo:
			if (FeatureFlag(weFReadOnly, weBitTest))
				break;
			ThrowIfOSErr_(Redo());
			UserChangedText();
			SetUpdateCommandStatus(true);
			break;
		#endif
		
		case cmd_Cut:
			if (FeatureFlag(weFReadOnly, weBitTest))
				break;
			if(mTextModel != nil)
			{
				//always reset the serach pointers before doing anything
				mTextModel->SetSearchPointers(0, GetTextLength());		
				mTextModel->SendEventToSelf(kAEMiscStandards, kAECut, theEvent);
				UAppleEventsMgr::SendAppleEvent(theEvent);
			}
			else
				ThrowIfOSErr_(Cut());
			UserChangedText();
			SetUpdateCommandStatus(true);
			break;
			
		case cmd_Copy:
			if (!(mTextAttributes & wasteAttr_Selectable))
				break;
			if(mTextModel != nil)
			{
				//always reset the search pointers before doing anything
				mTextModel->SetSearchPointers(0, GetTextLength());		
				mTextModel->SendEventToSelf(kAEMiscStandards, kAECopy, theEvent);
				UAppleEventsMgr::SendAppleEvent(theEvent);
			}
			else
			{
				ThrowIfOSErr_(Copy());
			}
			doScrollCheck = false;
			break;
			
		case cmd_Paste:
			if (FeatureFlag(weFReadOnly, weBitTest))
				break;
			if(mTextModel != nil)
			{
				//always reset the search pointers before doing anything
				mTextModel->SetSearchPointers(0, GetTextLength());		
				mTextModel->SendEventToSelf(kAEMiscStandards, kAEPaste, theEvent);
				UAppleEventsMgr::SendAppleEvent(theEvent);
			}	
			else
				ThrowIfOSErr_(Paste());
			
			UserChangedText();
			SetUpdateCommandStatus(true);
			break;
			
		case cmd_Clear: 
			if (FeatureFlag(weFReadOnly, weBitTest))
				break;
			if(mTextModel != nil)
			{
				//always reset the search pointers before doing anything
				mTextModel->SetSearchPointers(0, GetTextLength());		
				mTextModel->SendEventToSelf(kAEMiscStandards, kAEDelete, theEvent);
				UAppleEventsMgr::SendAppleEvent(theEvent);
			}
			else
			{
				Delete();
			}
			UserChangedText();
			SetUpdateCommandStatus(true);
			break;
			
		case msg_TabSelect:
			if (!IsEnabled()) {
				cmdHandled = false;
				break;
			} 
			// WEKey// else FALL THRU to SelectAll()
			
		case cmd_SelectAll:
			if (!(mTextAttributes & wasteAttr_Selectable))
				break;
			if(mTextModel != nil)
			{
				//always reset the search pointers before doing anything
				mTextModel->SetSearchPointers(0, GetTextLength());		
				mTextModel->SelectAll();
			}
			else
				SelectAll();
			break;
		
		default:
			doScrollCheck = false;
			cmdHandled = LCommander::ObeyCommand(inCommand, ioParam);
			break;
	}

	if (doScrollCheck) {
		// timp 9/30/96
		AdjustImageToText();
		CheckScroll();
	}		
	return cmdHandled;
}

// ---------------------------------------------------------------------------
//	¥FindCommandStatus
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

void
WTextView::FindCommandStatus(
		CommandT inCommand,
		Boolean& outEnabled,
		Boolean& outUsesMark,
		UInt16 & outMark,
		Str255 outName )
{
	outUsesMark = false;
	
	FocusDraw();
	
	switch (inCommand) {
	
		case cmd_Undo:
			if (FeatureFlag(weFReadOnly, weBitTest)) 
			{
				outEnabled = false;
			}
			else 
			{
				//Get the top most undo information, this is what we
				//would potentially undo
				WEActionKind actionKind; 
				#if WASTE_VERSION > 0x02000000
				actionKind = GetIndUndoInfo(1);
				#else
				Boolean redoFlag;
				actionKind = GetUndoInfo(redoFlag);
				#endif
				//Note I have set up the undo STR# resource to exactly
				//match the WEActionKind labels so we can use it
				//directly
				// Get the string to put in the undo menu item
				Str255	itemString;
				::GetIndString(itemString, rUndoStringsID, actionKind + 1);
				
				// Now copy it into outName to change the menu
				LString::CopyPStr(itemString, outName);
				// is nothing to undo
				outEnabled =	(actionKind > 0);
			}
			break;
		case cmd_Redo:
			if (FeatureFlag(weFReadOnly, weBitTest)) 
			{
				outEnabled = false;
			}
			else 
			{	
				#if WASTE_VERSION > 0x02000000
				//Get the top most redo information, this is what we
				//would potentially redo
				WEActionKind actionKind ;
				actionKind = GetIndUndoInfo(-1);
				
				//Note I have set up the redo STR# resource to exactly
				//match the WEActionKind labels so we can use it
				//directly
				// Get the string to put in the redo menu item
				Str255	itemString;
				::GetIndString(itemString, rRedoStringsID, actionKind + 1);
				
				// Now copy it into outName to change the menu
				LString::CopyPStr(itemString, outName);
				// is nothing to redo
				outEnabled =	(actionKind > 1);
				#endif
			}
			break;
		case cmd_Copy:
			if (!(mTextAttributes & wasteAttr_Selectable)) 
			{
				break;
			}
			else 
			{
				SInt32	start, end;
				GetTextSelection(start, end);
				outEnabled = (start != end);
			}
			break;

		case cmd_Cut:				// Cut, Copy, and Clear enabled
		case cmd_Clear:				//	 if something is selected
			if (FeatureFlag(weFReadOnly, weBitTest)) 
			{
				outEnabled = false;
			}
			else 
			{
				SInt32	start, end;
				GetTextSelection(start, end);
				outEnabled = (start != end);
			}
			break;
					
		case cmd_Paste:
			if (FeatureFlag(weFReadOnly, weBitTest)) 
			{
				outEnabled = false;
			}
			else 
			{			
				outEnabled = CanPaste();
			}
			break;
		
		case cmd_SelectAll:			// Check if any characters are present
			outEnabled = (GetTextLength() > 0);
			break;
			
		default:
			LCommander::FindCommandStatus(inCommand, outEnabled,
									outUsesMark, outMark, outName);
			break;
	}
}

// ---------------------------------------------------------------------------
//	¥SelectAll
// ---------------------------------------------------------------------------
//
// Purpose: Select all the text.
//
// Revision:
//

void
WTextView::SelectAll()
{
	StFeatureFlag	noScroll(weFAutoScroll, weBitClear, mWasteH);
	FocusDraw();
	SetTextSelection(0, LONG_MAX);
}

// ---------------------------------------------------------------------------
//	¥HandleKeyPress
// ---------------------------------------------------------------------------
//
// Purpose: Handle incoming keys for the waste view. Most get passed on to
// Waste
//
// Revision:
//

Boolean
WTextView::HandleKeyPress(
		const EventRecord& inKeyEvent )
{
	Boolean		keyHandled = true;
	Boolean		doScrollCheck = true;
	EKeyStatus	theKeyStatus = keyStatus_Input;
	SInt16		theKey = inKeyEvent.message & charCodeMask;
		
	if (!FeatureFlag(weFReadOnly, weBitTest)) 
	{
		//remember the target. We need to check this before doing scroll synch
		LCommander	*theTarget = GetTarget();
		
		SInt32	selStart, selEnd;
		GetTextSelection(selStart, selEnd );
		Boolean beforeSel = (selStart == selEnd);
		
		FocusDraw();
		
		if (inKeyEvent.modifiers & cmdKey) 
		{										// Always pass up when the
			theKeyStatus = keyStatus_PassUp;	// command key is down
		}
		else 
		{
			// UKeyFilters tells us what kind of key was hit and
			// from there we can take the appropriate action
			theKeyStatus = UKeyFilters::PrintingCharField(inKeyEvent);
		}
		
		// now check out what key was typed and take the correct action
		switch (theKeyStatus) {
		
			case keyStatus_Input:
				//when we first start typing, then update the menus
				//after that (when in the middle of a typing action
				//don't mess with menus. This greatly speeds things up.
				if(!IsTyping())
					SetUpdateCommandStatus(true);
				FocusDraw();
				Key(theKey, inKeyEvent.modifiers);
				UserChangedText();
				break;

			case keyStatus_TEDelete:
				FocusDraw();
				Key(theKey, inKeyEvent.modifiers);
				UserChangedText();
				if(!mDeleting)
				{
					mDeleting = true;
					SetUpdateCommandStatus(true);
				}
				break;
				
			case keyStatus_TECursor:
				//get some preliminary info about run
				//and paragraph data
				SInt32	runIndex = OffsetToRun(selStart);
				
				#if WASTE_VERSION > 0x02000000
				WEParaInfo rulerInfo;
				GetParaInfo(selStart, rulerInfo);
				#endif
				
				//OK now do the key stuff
				FocusDraw();	
				Key(theKey, inKeyEvent.modifiers);
				//Finally see if things have changed.
				SInt32 newStart, newEnd;
				GetTextSelection(newStart, newEnd);
				
				#if WASTE_VERSION > 0x02000000
				WEParaInfo newRulerInfo;
				GetParaInfo(newStart, newRulerInfo);
				#endif
				
				if(runIndex != OffsetToRun(newStart) 
					
					#if WASTE_VERSION > 0x02000000
					||
					(rulerInfo.paraRuler.alignment != newRulerInfo.paraRuler.alignment)
					#endif
					
					 )
				{
					SetUpdateCommandStatus(true);
				}
				
				break;
				
			case keyStatus_ExtraEdit:
				if (theKey == char_FwdDelete) 
				{
					FocusDraw();
					Key(theKey, inKeyEvent.modifiers);
					UserChangedText();
					if(!mDeleting)
					{
						mDeleting = true;
						SetUpdateCommandStatus(true);
					}
				}
				else {
					// these really aren't handled.
					keyHandled = LCommander::HandleKeyPress(inKeyEvent);
				}
				break;
				
			case keyStatus_Reject:
				// +++ Do something
				doScrollCheck = false;
				::SysBeep(1);
				break;
				
			case keyStatus_PassUp:
				if ((theKey == char_Return) || (theKey==char_Tab)) 
				{
					if(!IsTyping())
						SetUpdateCommandStatus(true);
					FocusDraw();
					Key(theKey, inKeyEvent.modifiers);
					UserChangedText();
					
				}
				else 
				{
					keyHandled = LCommander::HandleKeyPress(inKeyEvent);
				}
				break;
		}
		//if the selection changed from no selection
		//to a selection or the otherway around, we
		//want to update things.
		GetTextSelection( selStart, selEnd );
		if ( beforeSel != ( selStart == selEnd ) ) 
		{
			SetUpdateCommandStatus(true);
		}	
		
		if(mTextModel != nil 
			&&	(theKeyStatus == keyStatus_Input
			||	theKeyStatus == keyStatus_TEDelete
			||	theKey == char_FwdDelete) )
		{
			//if we are doing applescript, we need to update the search pointers
			//to all the text.
			ValidateObj_(mTextModel);
			mTextModel->SetSearchPointers(0, LONG_MAX);
		}
		//an optimization. We only check the menus at the begining of a delete sequence
		//here we set mDeleting back to false, if it is true and the user did not
		//type delete or fwd_delete
		if(mDeleting && !(keyStatus_TEDelete == theKeyStatus || theKey == char_FwdDelete))
		{
			mDeleting = false;
		}
		
		if (doScrollCheck && (GetTarget() == theTarget) ) 
		{
			AdjustImageToText();
			CheckScroll();
		}
	}
	else 
	{
		if (keyStatus_TECursor == theKeyStatus) 
		{ 
			FocusDraw();	
			Key(theKey, inKeyEvent.modifiers);
			keyHandled = true;
		}
		else 
		{		
			keyHandled = LCommander::HandleKeyPress(inKeyEvent);
		}
	}
	return keyHandled;
}

// ---------------------------------------------------------------------------
//	¥ApplyForeAndBackColors
// ---------------------------------------------------------------------------
//
// Purpose: Apply the Fore and Back Colors to the current pane. The fore and back
// colors are set in SetForeAndBackColors
//
// Revision:
//

void
WTextView::ApplyForeAndBackColors() const
{
	::RGBForeColor(&mForeColor);
	::RGBBackColor(&mBackColor);
}

// ---------------------------------------------------------------------------
//	¥SetForeAndBackColor
// ---------------------------------------------------------------------------
//
// Purpose: Specify the foreground and/or background colors for this WTextView
//
//	Specify nil for inForeColor and/or inBackColor to leave that
//	color trait unchanged
//
// Revision:
//

void
WTextView::SetForeAndBackColor(
		const RGBColor * inForeColor,
		const RGBColor * inBackColor )
{
	if (inForeColor != nil) {
		mForeColor = *inForeColor;
	}
	
	if (inBackColor != nil) {
		mBackColor = *inBackColor;
	}
}

// ---------------------------------------------------------------------------
//	¥BeTarget
// ---------------------------------------------------------------------------
//
// Purpose: We are becoming the target for user action, get prepared.
//
// Revision:
//

void
WTextView::BeTarget()
{
	if (FocusDraw()) 
	{				// Show active selection
		WText::Activate();
	}
	StartIdling();
	
	// check to see if theSuperView is a LScroller. If it is, we want to 
	// activate it
	if (mScroller != nil && (mTextAttributes & wasteAttr_ActDeactScroll)) {
		mScroller->Activate();
	}
}

// ---------------------------------------------------------------------------
//	¥DontBeTarget
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

void
WTextView::DontBeTarget()
{
	if (FocusDraw()) 
	{				// Show inactive selection
		WText::Deactivate();
	}
	StopIdling();					// Stop flashing the cursor
	
	// check to see if theSuperView is a LScroller. If it is, we want to 
	// deactivate it
	if (mScroller != nil && (mTextAttributes & wasteAttr_ActDeactScroll)) 
	{
		mScroller->Deactivate();
	}
}
// ---------------------------------------------------------------------------
//	¥ClickSelf
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

void
WTextView::ClickSelf(
		const SMouseDownEvent& inMouseDown )
{
	if (!(mTextAttributes & wasteAttr_Selectable))
		return;
		
	Point	mouseLoc;

	mouseLoc = inMouseDown.wherePort;
	mouseLoc.v += mPortOrigin.v;
	mouseLoc.h += mPortOrigin.h;

	// if its not selectable, then don't let them edit
	if (!IsTarget()) 
	{				
		if (FeatureFlag(weFDragAndDrop, weBitTest) &&
				WaitMouseMoved ( inMouseDown.macEvent.where )) 
		{
			// this is the begining of a drag so let WEClick handle it
			// if the end of the text is not in view, don't allow a drop.
			BeginDrag(mouseLoc, inMouseDown.macEvent);
		}
		// You do not want to view to become active if the
		// selection or cursor is not in a visible part of the view.

		SInt32 selStart, selEnd;
		GetTextSelection(selStart, selEnd); 
		if (!CursorInView(selStart)) 
		{
			if ((selStart == selEnd) || (!CursorInView(selEnd))) 
			{
				return;
			}
		}
		
		// if we have outline hilite and we are on top of the selection
		// then assume the user wants to do something with the selection
		SInt8 outlineHilite = FeatureFlag(weFOutlineHilite, weBitTest);
		if (outlineHilite && IsInSelection(mouseLoc)) 
		{
			SwitchTarget(this);
			return;
		}
		else 
		{
		// In this case we are not on the selection.
		// assume the user wants to start a cursor at the mousepoint.
			SInt8 prevScroll = FeatureFlag(weFAutoScroll, weBitClear);
			FocusDraw();
			SetTextSelection(0, 0);
			FeatureFlag(weFAutoScroll, prevScroll);
			SwitchTarget(this);
		}
	}
	else
	{
		FocusDraw();
		// we need to fool waste so that it calcs things right
		// shift over by the port origin. 9/28/96
		mouseLoc = inMouseDown.wherePort;
		mouseLoc.v += mPortOrigin.v;
		mouseLoc.h += mPortOrigin.h;
		EventModifiers modifiers = inMouseDown.macEvent.modifiers;
		UInt32 when = inMouseDown.macEvent.when;
		if (FeatureFlag(weFDragAndDrop, weBitTest) &&
				WaitMouseMoved ( inMouseDown.macEvent.where )) 
		{
			// this is the begining of a drag so let WEClick handle it
			// 1/16/97 timp
			// if the end of the text is not in view, don't allow a drop.
			BeginDrag(mouseLoc, inMouseDown.macEvent);
		} 
		else 
		{
			WText::Click(mouseLoc, modifiers, when); 
		}
		
		AdjustImageToText();
		// added timp 9/28/96
		CheckScroll();
		// removed SelView call 10/16/96
	}
}


// ---------------------------------------------------------------------------
//	¥ResizeFrameBy
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

void
WTextView::ResizeFrameBy(
		SInt16 inWidthDelta,
		SInt16 inHeightDelta,
		Boolean inRefresh )
{
	LView::ResizeFrameBy(inWidthDelta, inHeightDelta, Refresh_No);
	
	//now resize the image. This is only needed if word wrap is
	//turned on
	if (mTextAttributes & wasteAttr_WordWrap) 
	{
		ResizeImageTo(mFrameSize.width, mImageSize.height, Refresh_No);	
	}
	AlignWasteViewRects();
	
	LView::OutOfFocus(this);
	
	AdjustImageToText();
	
	if(mTextAttributes & wasteAttr_WordWrap) 
	{
		SelView();
		CheckScroll();
	}
	
	if (inRefresh) 
	{								// It's safe to refresh now that
		Refresh();					//   everything is in synch
	}
	
	// change parameters if inside scroller
	// This is very important if you are inside a scroller and 
	// the frame binding is changing
	if (mInScroller) 
	{
		AdaptToEnclosingScroller();
	}
}

// ---------------------------------------------------------------------------
//	¥MoveBy
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

void
WTextView::MoveBy(
		SInt32 inHorizDelta,
		SInt32 inVertDelta,
		Boolean inRefresh )
{
	LView::MoveBy(inHorizDelta, inVertDelta, inRefresh);
	AlignWasteViewRects();
}

// ---------------------------------------------------------------------------
//	¥ScrollImageBy
// ---------------------------------------------------------------------------
//
// Purpose: Move the location of the Frame by the specified amounts
//
//		inHorizDelta and inVertDelta specify, in pixels, how far to move the
//		Frame (within its surrounding Image). Positive horiz deltas move to
//		the left, negative to the right. Positive vert deltas move down,
//		negative up.
//
// Revision:
//

void
WTextView::ScrollImageBy(
		SInt32 inLeftDelta,
		SInt32 inTopDelta,
		Boolean inRefresh )
{
	// inhibit redraw if refresh is false, otherwise
	//WASTE will redraw.
	SInt16	theDraw;
	if(!inRefresh)
		theDraw = FeatureFlag(weFInhibitRedraw, weBitSet);
	
	FocusDraw();
	// Scroll the destRect
	Scroll(-inLeftDelta, -inTopDelta);
	
	//resynch everything
	CheckScroll();
	
	if (!inRefresh) 
	{
		FeatureFlag(weFInhibitRedraw, theDraw);
	}
}

// ---------------------------------------------------------------------------
//	¥SpendTime
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

void
WTextView::SpendTime(
		const EventRecord& /*inMacEvent*/ )
{
	if ((mTextAttributes & wasteAttr_Selectable) && FocusDraw() && IsVisible()) 
	{
		SInt32 selStart, selEnd;
		GetTextSelection(selStart, selEnd);
		
		if ((selStart == selEnd) && CursorInView(selStart)) {
			// flash the cursor
			UInt32	maxSleep;
			Idle(maxSleep);
		}

#if WASTE_OBJECTS
		// check to see if any memory tied up for sounds
		// can be deallocated
		DoObjectTasks(mWASTEEditH);		
#endif

	}
}

// ---------------------------------------------------------------------------
//	¥DrawSelf
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

void
WTextView::DrawSelf()
{
	// We have to change it to the current port in case we are drawing
	// into a port that is not the owner port. This happens when we are
	// printing or drawing into an offscreen port.
	
	
	GrafPtr savePort;
	ThrowIfOSErr_(GetInfo(wePort, &savePort));

	GrafPtr newPort = UQDGlobals::GetCurrentPort();
	ThrowIfOSErr_(SetInfo(wePort, &newPort));
	
	StRegion updateRgn(GetLocalUpdateRgn(), false);
	
	FocusDraw();
	
	// somehwhat of a hack, if the background color of the view is not white
	// then we need to erase the rgn that is being updated to get it to look 
	// right 9/30/96
	// I removed the color check. This really isn't costing much and it takes 
	// care of the margins 10/18/96
	if (mMargins.v != 0 || mMargins.h != 0) 
	{
		StRegion	marginRgn;
		StRegion	frameRgn;
		Rect		frame, marginFrame;
		CalcLocalFrameRect(frame);
		marginFrame = frame;
		::InsetRect(&marginFrame, -(mMargins.h - 1), -(mMargins.v - 1));
		::RectRgn(frameRgn, &frame); 
		::RectRgn(marginRgn, &marginFrame);
		::DiffRgn(marginRgn, frameRgn, marginRgn);
		::UnionRgn(marginRgn, updateRgn, marginRgn);			
		EraseUpdateArea(marginRgn);
	}
	else 
	{
		EraseUpdateArea(updateRgn);
	}
	// the parameter is the region to be updated. If we pass nil,
	// the viewRect is used as the update region.
	Update(updateRgn);
	
	ThrowIfOSErr_(SetInfo(wePort, &savePort));
}

// ---------------------------------------------------------------------------
//	¥EraseUpdateArea
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

void
WTextView::EraseUpdateArea(
		RgnHandle inEraseRgn )
{
	::EraseRgn(inEraseRgn);
}

// ---------------------------------------------------------------------------
//	¥FocusDraw
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
// 10/17/96 expand the ClipRect a little to allow
// Drawing in the margins. If you don't want this to happen
// make sure the margins are set to 0
//
// 10/22/99 Thanks to Rick Aurbach for a fix. The new code clips to the revealed
// rect for the view and prevents the expansion of the margins overdrawing
// other views.	
//

Boolean
WTextView::FocusDraw(
		LPane* inSubPane )
{
	Boolean focused = LView::FocusDraw(inSubPane );

	if (focused) {
		if (mMargins.h != 0 || mMargins.v != 0) {
			Rect    frame, superRevealed;
            if (CalcPortFrameRect(frame)) {
                ::MacInsetRect(&frame, -mMargins.h, -mMargins.v);
                if (mSuperView != nil) {
                    mSuperView->GetRevealedRect(superRevealed);
                    ::SectRect(&frame, &superRevealed, &frame);
                    PortToLocalPoint(topLeft(frame));
                    PortToLocalPoint(botRight(frame));
                    ::ClipRect(&frame);
                }
            }
		}
		// this is pretty much fixed now. 9/30/96
		ApplyForeAndBackColors();
	}
	
	return focused;
}

// ---------------------------------------------------------------------------
//	¥UserChangedText
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

void
WTextView::UserChangedText()
{
	BroadcastMessage(wTextChanged, this);
}

// ---------------------------------------------------------------------------
//	¥ScrollToPanel
// ---------------------------------------------------------------------------
//
// Purpose: Code for printing. This scrolls the text to the right panel
// that will then be imaged on the printer
//
// Revision:
//

Boolean
WTextView::ScrollToPanel(
		const PanelSpec& inPanel )
{
	Boolean			panelInImage = false;
	LongPt			thePoint;
	SInt32			bottom;
	SDimension16	frameSize = mFrameSize;
	UInt32			horizPanelCount = 1;
	UInt32			vertPanelCount = mPrintPanels->GetCount();
	
	if ((inPanel.horizIndex <= horizPanelCount) &&
		(inPanel.vertIndex <= vertPanelCount)) 
	{
		mPrintPanels->FetchItemAt(inPanel.vertIndex, thePoint.v);
		
		// calculate the bottom of this panel, we use this to set the frame
		SInt32	lineCount = CountLines();
		if (inPanel.vertIndex != vertPanelCount)
			mPrintPanels->FetchItemAt(inPanel.vertIndex + 1, bottom);
		else					
			bottom = GetHeight(0, lineCount);
			
		thePoint.h = 0;
		
		// faster than calling the PPlant ResizeFrameTo
		mFrameSize.height =	 bottom - thePoint.v;
		AlignWasteViewRects();
		ScrollImageTo(thePoint.h, thePoint.v, false);
		panelInImage = true;
	}
	
	return panelInImage;
}

// ---------------------------------------------------------------------------
//	¥CountPanels
// ---------------------------------------------------------------------------
//
// Purpose: Count the total number of panels for printing. Note that this 
// version assumes that there is only one horizontal panel. If you want
// something other than that, override this.
//
// Revision:
//

void
WTextView::CountPanels(
		UInt32& outHorizPanels,
		UInt32& outVertPanels )
{
	// Remove any items in the print record. 
	SInt32 count = mPrintPanels->GetCount();
	if (count > 0)
		mPrintPanels->RemoveItemsAt(count, 1);
	
	SDimension32 imageSize;
	GetImageSize(imageSize);
	
	// note that the frameSize is the size of the page if you have set
	// up your placeholder correctly.
	SDimension16 frameSize;
	GetFrameSize(frameSize);
	
	SInt32	lineNo = 0;
	SInt32	lineCount = CountLines();
	SInt16	panelHeight = 0;
	SInt32	totalHeight = 0;

	outVertPanels = 0;
	
	while( (imageSize.height > totalHeight) && (lineNo < lineCount)) 
	{
		// remember the offset for this panel
		mPrintPanels->InsertItemsAt(1, LArray::index_Last, totalHeight);
		// keep adding lines until we extend past the frame
		// also keep it in the bounds of the total line count
		while((panelHeight < frameSize.height) && (lineNo < lineCount)) {
			panelHeight += GetHeight(lineNo, lineNo + 1);
			lineNo++;
		}
		if (lineNo < lineCount) {
			// get rid of the last line since it is over the frame
			panelHeight -= GetHeight(lineNo - 1, lineNo);
			lineNo--;
		}
		// we have reached a panel end so add to the number of panels
		outVertPanels++;
		// add this panel to the totalHeight
		totalHeight += panelHeight;
		panelHeight = 0;
	}
	// we don't support more than one horizontal panel, override if
	// you want to do this
	outHorizPanels = 1;
}

// ---------------------------------------------------------------------------
//	¥SuperPrintPanel
// ---------------------------------------------------------------------------
//
// Purpose: This over ride allows us to set the port on the WASTE WEReference to the 
// printing port.  This way we won't see the scolling on the screen during printing.
// Need to do this because WTextView's DrawSelf function sets the port to the
// print port for the drawing but sets it back when its done.
//
// Revision:
//

void
WTextView::SuperPrintPanel(
		const PanelSpec& inSuperPanel,
		RgnHandle inSuperPrintRgnH )
{
	//Set the WEReference port to the printer port
    GrafPtr newPort = UQDGlobals::GetCurrentPort();
    ThrowIfOSErr_(SetInfo(wePort, &newPort));

    LView::SuperPrintPanel(inSuperPanel, inSuperPrintRgnH);
}

// ---------------------------------------------------------------------------
//	¥SavePlace
// ---------------------------------------------------------------------------
//
// Purpose: Save the place we were in to the stream.
//
// Revision:
//

void
WTextView::SavePlace(
		LStream* outPlace )
{
	LView::SavePlace(outPlace);

	LongRect theLongViewRect, theLongDestRect;
	GetViewRect(theLongViewRect);
	outPlace->WriteData(&theLongViewRect, sizeof(LongRect));
	GetDestRect(theLongDestRect);
	outPlace->WriteData(&theLongDestRect, sizeof(LongRect));

	GrafPtr savePort;
	ThrowIfOSErr_(GetInfo(wePort, &savePort));
	outPlace->WriteData(&savePort, sizeof(savePort));
}

// ---------------------------------------------------------------------------
//	¥RestorePlace
// ---------------------------------------------------------------------------
//
// Purpose: Restore the place we were in from the stream
//
// Revision:
//

void
WTextView::RestorePlace(
		LStream* inPlace )
{
	LView::RestorePlace(inPlace);

	LongRect theLongViewRect, theLongDestRect;
	inPlace->ReadData(&theLongViewRect, sizeof(LongRect));
	SetViewRect(theLongViewRect);
	inPlace->ReadData(&theLongDestRect, sizeof(LongRect));
	SetDestRect(theLongDestRect);

	GrafPtr oldPort;
	inPlace->ReadData(&oldPort, sizeof(oldPort));
	ThrowIfOSErr_(SetInfo(wePort, &oldPort));
}

// ---------------------------------------------------------------------------
//	¥BeginDrag
// ---------------------------------------------------------------------------
//
// Purpose: This split out to allow override of the data that is put in 
// a drag. The default just uses the WASTE impementation 
//
// Revision:
//

void
WTextView::BeginDrag(
		const Point inMouseLoc,
		const EventRecord& inEventRecord )
{
	WText::Click(inMouseLoc, inEventRecord.modifiers, inEventRecord.when);
}

// ---------------------------------------------------------------------------
//	¥DragIsAcceptable
// ---------------------------------------------------------------------------
//
// Purpose: Check for Waste information in the drag. This and the other
// drag routines are basically pass throughs to Waste.
//
// Revision:
//

Boolean
WTextView::DragIsAcceptable(
		DragReference inDragRef )
{
	if ( (FeatureFlag(weFReadOnly, weBitTest)) || (!FeatureFlag(weFDragAndDrop, weBitTest))) 
	{
		return false;
	} 
	// if we cannot see the end of the text, then don't allow a drop.
	// This only happens when the waste view pane is inside a scrolling
	// view and all the text is scrolled out of view.
	SInt32	offset = GetTextLength();
	if (!CursorInView(offset)) 
	{
		return false;
	}
	
	FocusDraw();
	return WECanAcceptDrag(inDragRef, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥DoDragReceive
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

void
WTextView::DoDragReceive(
		DragReference inDragRef )
{
	if (FeatureFlag(weFReadOnly, weBitTest) || (!FeatureFlag(weFDragAndDrop, weBitTest))) 
	{
		return;
	}
			
	// The next three calls get the pane ready to accept the drop 
	LView::OutOfFocus(nil);
	FocusDraw();
	UnhiliteDropArea(inDragRef);

	OSErr	theErr = WEReceiveDrag(inDragRef, mWasteH);
	
	if (theErr == noErr) {
		AdjustImageToText();
		// added timp 9/28/96
		CheckScroll();
		UserChangedText();
		SetUpdateCommandStatus(true);
	}
	else if (theErr != dragNotAcceptedErr) {
		// ignore the dragNotAcceptErr. This means the text was 
		// dragged back on top of the same selection
		ThrowIfOSErr_(theErr);
	}
}

// ---------------------------------------------------------------------------
//	¥HiliteDropArea
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

void
WTextView::HiliteDropArea(
		DragReference inDragRef )
{
	if(!mIsHilited)
	{
		ApplyForeAndBackColors();
		Rect	dropRect;
		CalcLocalFrameRect(dropRect);
		//::InsetRect(&dropRect, 1, 1);
		StClipRgnState	theClip(dropRect);
		StRegion	dropRgn(dropRect);
		::ShowDragHilite(inDragRef, dropRgn, true);
		mIsHilited = true;
	}
}

// ---------------------------------------------------------------------------
//	¥EnterDropArea
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

void
WTextView::EnterDropArea(
		DragReference inDragRef,
		Boolean /*inDragHasLeftSender*/ )
{
	if (FeatureFlag(weFReadOnly, weBitTest) || (!FeatureFlag(weFDragAndDrop, weBitTest))) 
	{
		return;
	}
	else 
	{
		WETrackDrag(kDragTrackingEnterWindow, inDragRef, mWasteH);
	}
}

// ---------------------------------------------------------------------------
//	¥LeaveDropArea
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

void
WTextView::LeaveDropArea(
		DragReference inDragRef )
{
	if (FeatureFlag(weFReadOnly, weBitTest) || (!FeatureFlag(weFDragAndDrop, weBitTest))) 
	{
		return;
	}
	else 
	{
		FocusDraw();
		WETrackDrag(kDragTrackingLeaveWindow, inDragRef, mWasteH);
		mCanAcceptCurrentDrag = false;
	}
}

// ---------------------------------------------------------------------------
//	¥InsideDropArea
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

void
WTextView::InsideDropArea(
		DragReference inDragRef )
{
	if (FeatureFlag(weFReadOnly, weBitTest) || (!FeatureFlag(weFDragAndDrop, weBitTest))) 
	{
		return;
	}
	else 
	{
		FocusDraw();
		WETrackDrag(kDragTrackingInWindow, inDragRef, mWasteH);
	}
}

// ---------------------------------------------------------------------------
//	¥PointInDropArea
// ---------------------------------------------------------------------------
//
// Purpose: I overrode this to make autoscrolling during drags work better
// The default routine compares the point to the size of the 
// WTextView pane. We want the LDragAndDrop to think it is 
// inside the pane if inPoint is anywhere inside theSuperView.
// (This is often an LScroller Object.)
// This allows the WETrackDrag Function to call the clickLoop
// When it detects that the mouse is outside viewRect but inside
// the superView and we getter better autoscrolling behavior.
//
// Revision:
//

Boolean
WTextView::PointInDropArea(
		Point inPoint )
{
	if (FeatureFlag(weFReadOnly, weBitTest) || (!FeatureFlag(weFDragAndDrop, weBitTest))) 
	{
		return false;
	}
	else 
	{
		Boolean rVal = 0;
		// check to see if theSuperView is a LScroller. If it is, we want to 
		// scroll the view otherwise don't I love RTTI!
		if (mInScroller) {
			// WTextView is in a scroller or some other view.
			// check to see if theSuperView is hit.
			GlobalToPortPoint(inPoint);
			// The messing with the frame location and size is to give a
			// 16 pixel top and left margin so that autoscrolling during
			// a drag will work when the cursor moves outside the top of
			// left of the viewRect but within 16 pixels. This may need
			// to be overridden depending upon your particular WTextView
			// view set up. LPane's method is called to prevent undesired
			// adjustment of subimages. They are not needed here since we
			// immeadiately set it back to it's original size. 

			// mSuperView->
			LPane::ResizeFrameBy(2*sMaxScrollDelta, 2*sMaxScrollDelta, false);
			// mSuperView->
			LPane::MoveBy(-sMaxScrollDelta, -sMaxScrollDelta, false);
			
			rVal =	IsHitBy(inPoint.h, inPoint.v);
			
			// mSuperView->
			LPane::ResizeFrameBy(-2*sMaxScrollDelta, -2*sMaxScrollDelta, false);
			// mSuperView->
			LPane::MoveBy(sMaxScrollDelta, sMaxScrollDelta, false);
		}
		else 
		{
			// The superView is the window object. The default method will
			// define the WTextView area
			rVal = LDragAndDrop::PointInDropArea( inPoint );
		}
		return rVal;
	}
}

// ---------------------------------------------------------------------------
//	¥ScrolledImageReport
// ---------------------------------------------------------------------------
//
// Purpose: Waste and PowerPlant handle scrolling differently. Waste keeps the
// viewRect at 0, 0 and scrolls the destRect. PowerPlant keeps the 
// Image (== destRect) at 0, 0 and moves the frame (== viewRect). We
// have to account for this or really bad things happen. I have choosen
// to let waste win and adjust the frame and image to match the viewRect
// and destRect, respectively.
//
// Revision:
//

void
WTextView::ScrolledImageReport(
		SInt32 inLeftDelta,
		SInt32 inTopDelta )
{
	TArrayIterator<LPane*> iterator(mSubPanes, LArrayIterator::from_Start);
	LPane	*theSub;
	while (iterator.Next(theSub)) {
		// This adapts any panes inside the WasteEdit
		// I don't know if there will be any, but it's
		// good to put this in. It also caused the frame to move
		// added code here to make sure that subviews are updated 
		// properly. I still have problems with holding down of the 
		// mouse on a control, but it is better.
		// 1/15/97 timp
		Rect subFrame;
		Rect superRevealed;
		
		if (theSub->IsVisible() && theSub->CalcPortFrameRect(subFrame)) {
			GetRevealedRect(superRevealed);
			if (::SectRect(&subFrame, &superRevealed, &subFrame)) {
				OffsetRect( &subFrame, inLeftDelta, inTopDelta);
				InvalPortRect(&subFrame);
			}
		}
		theSub->AdaptToSuperScroll(inLeftDelta, inTopDelta);
		theSub->Draw(nil);
	}
	
	if (mSuperView != nil) {
	// This adjusts the scroll bars
		mSuperView->SubImageChanged(this);
	}

}

// ---------------------------------------------------------------------------
//	¥CheckScroll
// ---------------------------------------------------------------------------
//
// Purpose: A routine to handle adjusting the image and frame whenever
//	WASTE might've moved them. If this does happen, reset the image and frame
//	Then notify sub and super views by calling scrolled image report
//	
// Revision:
//

void
WTextView::CheckScroll()
{
	SPoint32	delta;
	LongRect	viewRect, destRect;
	
	GetViewRect(viewRect);
	GetDestRect(destRect);
	
	// find out if we scrolled, the destRect will scroll if WASTE moves it
	// therefore the diff between these two is the delta. If the destRect
	// moves, we need to adjust the image and the frame location. Remember
	// this is anti powerplant. The frame stays put and the image moves
	delta.h = destRect.left - mImageLocation.h - mPortOrigin.h;
	delta.v = destRect.top - mImageLocation.v - mPortOrigin.v; 
	
	if ( (delta.h != 0) || (delta.v != 0) ) {
		mImageLocation.h = destRect.left - mPortOrigin.h;
		mImageLocation.v = destRect.top - mPortOrigin.v;
	
		// Synchronize PowerPlant view, we reset the frame to the 
		// viewRect
		mFrameLocation.v = viewRect.top - mPortOrigin.v;
		mFrameLocation.h = viewRect.left - mPortOrigin.h;
		
		ScrolledImageReport(delta.h, delta.v);
		
		//we just changed the image location relative to the frame.
		//so we need to scroll dude.
		OutOfFocus(this);
		//now refocus it
		FocusDraw();
	}
}

// ---------------------------------------------------------------------------
//	¥CursorInView
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

bool
WTextView::CursorInView(
		SInt32 inPosition )
{
	Boolean retVal = true;
	// This was added for views inside of scrollers where the cursor
	// may scroll out of view, but the view is partially visible.
	if (mInScroller && (mSuperView != mScroller)) {
		LongPt	longPoint;
		Point	shortPoint;
		Rect	superRect;
		short	lineHeight; 
		
		GetPoint(inPosition, 0, longPoint, lineHeight);
		WELongPointToPoint(&longPoint, &shortPoint);
		LocalToPortPoint(shortPoint);
		
		Assert_(mScroller != nil);
		mScroller->CalcPortFrameRect(superRect);
		if (!::PtInRect(shortPoint, &superRect))
			retVal = false;
	}
	return retVal;
}

#pragma mark -WASTE 2.0-
#if WASTE_VERSION > 0x02000000

// ---------------------------------------------------------------------------
//	¥SetRuler
// ---------------------------------------------------------------------------
//
// Purpose: override to set the Ruler alignment using apple events if possible.
//
// Revision:
//


OSErr
WTextView::AESetRuler(
			WERulerMode		inMode,
			WERuler			inRuler,
			bool			inDoAppleEvent) //=true
{
	OSErr	anErr = noErr;
	FocusDraw();
	if(mTextModel != nil && inDoAppleEvent)
	{
		ValidateObj_(mTextModel);
		//always reset the serach pointers before doing a set
		//style
		mTextModel->SetSearchPointers(0, GetTextLength());
		StAEDescriptor	event;
		mTextModel->CreateParaStyle(inMode, inRuler, event);
		UAppleEventsMgr::SendAppleEvent(event);
	}
	else
	{
		anErr = SetRuler(inMode, inRuler);
	}
	
	//the size of the view and dest rect may
	//change after a style call
	AdjustImageToText();
	
	CheckScroll();
	
	UserChangedText();
	
	SetUpdateCommandStatus(true);
	
	return anErr;
}

#endif