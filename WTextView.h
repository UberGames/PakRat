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
//=================================================================

#ifndef		__WTextView
#define		__WTextView

#pragma once




#if PP_Uses_Pragma_Import
	#pragma import on
#endif

#include	<LView.h>
#include	<LCommander.h>
#include	<LPeriodical.h>
#include	<LDragAndDrop.h>
#include	"WText.h"
#include	<LBroadcaster.h>

#ifndef __WASTE__
#include "waste.h"
#endif

#ifndef __FONTS__
#include <Fonts.h>
#endif

class WTextModel;
#pragma warn_hidevirtual off
template class TArray<SInt32>;
#pragma warn_hidevirtual reset

enum {
	wDeleteBeforeInsert 	= 1 << 0,
	wRefreshAfterInsert 	= 1 << 1
};

enum {
	wasteAttr_MultiStyle	=	0x8000,
	wasteAttr_Editable		=	0x4000,
	wasteAttr_Selectable	=	0x2000,
	wasteAttr_WordWrap		=	0x1000,
	wasteAttr_TabSupport	=	0x0800,
	wasteAttr_DoMargins		= 	0x0400,
	wasteAttr_ActDeactScroll = 	0x0200
};

const UInt16 WDefaultAttrs =	wasteAttr_MultiStyle	+
								wasteAttr_Editable		+
								wasteAttr_Selectable	+
								wasteAttr_WordWrap;

const UInt32 WDefaultFlags =	weDoAutoScroll		+
								weDoOutlineHilite	+
								weDoUndo			+
								weDoIntCutAndPaste	+
								weDoDragAndDrop		+
								weDoUseTempMem		+
								weDoDrawOffscreen;

const SInt32 WEDefaultImageWidth		=	540;
const SInt32 WDefaultNoWrapWidth		=	4000;
const SInt16 WDefaultFont				=	applFont;
const SInt16 WDefaultSize				=	12;
const Style  WDefaultFace				=	normal;
const RGBColor WDefaultColor			=	{0,0,0};


//const MessageT	cmd_Redo			= FOUR_CHAR_CODE('reDO');
const MessageT	wTextChanged		= 'TVCH';

class	WTextView : 
			public LView, 
			public	LCommander, 
			public	LPeriodical, 
			public	LDragAndDrop, 
			public	WText, 
			public	LBroadcaster
{
	public:
		enum { class_ID = FOUR_CHAR_CODE('NWSt')};
		// methods
							WTextView();

							WTextView(
									const SPaneInfo& inPaneInfo,
									const SViewInfo& inViewInfo,
									UInt16 inTextAttributes = WDefaultAttrs,
									ResIDT inTextTraitsID = 0,
									ResIDT inInitialTextID = 0,
									UInt32 inWasteFlags = 0);
		
							WTextView(
									LStream* inStream);
		
							WTextView(
									const WTextView & inOriginal);
		
		virtual 			~WTextView();
		
		virtual void			InsertText(
									Ptr inTextPtr,
									SInt32 inTextLength,
									StScrpHandle inStylesH = nil,
									Handle inSoupH = nil,
									Handle inParaFormat = nil,
									Handle inRulerScrap = nil,
									UInt32	inFlags = wRefreshAfterInsert);
		
		virtual void			InsertText(
									Handle inTextH,
									StScrpHandle inStylesH = nil,
									Handle inSoupH = nil,
									Handle inParaFormat = nil,
									Handle inRulerScrap = nil,
									UInt32	inFlags = wRefreshAfterInsert);
		
		virtual void			InsertFromNativeH(
										Handle	inTextData,
										UInt32	inFlags = 0);

		virtual Handle			CopyIntoNativeH(
										SInt32	inRangeStart,
										SInt32	inRangeEnd);
		
		virtual OSErr			AESetStyle(
									WEStyleMode 	inMode,
									TextStyle 		inTextStyle,
									bool			inDoAppleEvent = true);
		
		virtual void			StyleCheck(
									TextStyle	inStyle,
									Boolean &			outEnabled,
									Boolean &			outUsesMark,
									UInt16 &			outMark);
								
		
		virtual void			SetTextSelection(
									SInt32		inStart,
									SInt32		inEnd);
		
		virtual void			AdjustImageToText();
		
		virtual void			AlignWasteViewRects();
		
		virtual void			AdaptToNewSurroundings();
		
		virtual void			AdaptToEnclosingScroller();
		
		virtual Boolean			ObeyCommand(
									CommandT inCommand,
									void* ioParam);
		
		virtual void			FindCommandStatus(
									CommandT inCommand,
									Boolean& outEnabled,
									Boolean& outUsesMark,
									UInt16& outMark,
									Str255 outName);
									
		virtual void			SelectAll();
		
		virtual Boolean			HandleKeyPress(
									const EventRecord& inKeyEvent);
		
		virtual void			SetForeAndBackColor(
									const RGBColor * inForeColor = nil,
									const RGBColor * inBackColor = nil);
		
		virtual void			ResizeFrameBy(
									SInt16 inWidthDelta,
									SInt16 inHeightDelta,
									Boolean inRefresh);
		
		virtual void			MoveBy(
									SInt32 inHorizDelta,
									SInt32 inVertDelta,
									Boolean inRefresh);
		
		virtual void			ScrollImageBy(
									SInt32 inLeftDelta,
									SInt32 inTopDelta,
									Boolean inRefresh);
		
		virtual void			SpendTime(
									const EventRecord& inMacEvent);
		
		virtual void			EraseUpdateArea(
									RgnHandle inEraseRgn);
		
		virtual Boolean			FocusDraw(
									LPane* inSubPane = nil);
		
		virtual void			UserChangedText();
		
		virtual Boolean			ScrollToPanel(
									const PanelSpec& inPanel);
		
		virtual void			CountPanels(
									UInt32& outHorizPanels,
									UInt32& outVertPanels);
		
		virtual void			SuperPrintPanel(
									const PanelSpec& inSuperPanel,
									RgnHandle inSuperPrintRgnH);
		
		virtual void			SavePlace(
									LStream* outPlace);
		
		virtual void			RestorePlace(
									LStream* inPlace);
									
		virtual WTextModel *	GetTextModel();
		
		virtual void			SetTextModel(
									WTextModel*	inModel);
		
#if WASTE_VERSION > 0x02000000
	
		virtual OSErr			AESetRuler(
									WERulerMode inMode,
									WERuler		inRuler,
									bool		inDoAppleEvent = true);
#endif	
		
		
	protected:
		// methods
		static pascal OSErr		MyDragHiliteProc(
									DragReference drag,
									Boolean hiliteFlag,
									WEReference hWE);
		
		static pascal Boolean	MyClickLoop(
									WEReference hWE);
		
		virtual Boolean			IsInSelection(
									Point inMouseLoc);
		
		virtual void			ReconcileFrameAndImage(
									Boolean inRefresh);
		
		virtual void			AdjustCursorSelf(
									Point inPortPt,
									const EventRecord& inMacEvent);
		
		virtual void			ApplyForeAndBackColors() const;
		
		virtual void			BeTarget();
		
		virtual void			ClickSelf(
									const SMouseDownEvent& inMouseDown);
		
		virtual void			DontBeTarget();
		
		virtual void			DrawSelf();
		
		virtual void			BeginDrag(
									const Point inMouseLoc,
									const EventRecord& inEventRecord);
		
		virtual Boolean			DragIsAcceptable(
									DragReference inDragRef);
		
		virtual void			DoDragReceive(
									DragReference inDragRef);
		
		virtual void			HiliteDropArea(
									DragReference inDragRef);
		
		virtual void			EnterDropArea(
									DragReference inDragRef,
									Boolean inDragHasLeftSender);
		
		virtual void			LeaveDropArea(
									DragReference inDragRef);
		
		virtual void			InsideDropArea(
									DragReference inDragRef);
		
		virtual Boolean			PointInDropArea(
									Point inPoint);
		
		virtual void			ScrolledImageReport(
									SInt32 inLeftDelta,
									SInt32 inTopDelta);
		
		virtual void			CheckScroll();
		
		
		// attributes
		UInt16					mTextAttributes;
		ResIDT					mTextTraitsID;
		ResIDT					mInitialTextID;
		Point					mMargins;
		Boolean					mInScroller;
		Point					mSavePos;
#pragma warn_hidevirtual off
		TArray<SInt32> 	*		mPrintPanels;
#pragma warn_hidevirtual reset
		RGBColor				mForeColor;
		RGBColor				mBackColor;
		
	private:

		// methods
		void				InitTextView(
									UInt32 inWasteFlags);
		
		bool				CursorInView(
									SInt32 inPosition);
		
		
		// attributes
		static SInt32		sMaxScrollDelta;
		LView*				mScroller;
		WTextModel*			mTextModel;
		bool				mDeleting;	
};


#if PP_Uses_Pragma_Import
	#pragma import reset
#endif

#endif
