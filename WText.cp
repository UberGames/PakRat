//=================================================================
// ¥ WText
// Copyright© Timothy Paustian All Rights Reserved 1995-1998
//
// <mailto:paustian@bact.wisc.edu>
// <http://www.bact.wisc.edu/WTText/overview.html>
//=================================================================
//
// Purpose:
//
// File Creation Date:		3/5/99 8:42 PM
// Class Creation Date:		2/14/99 12:38 PM
// Class Last Modified:		3/5/99 8:41 PM
// Revision:
//
//=================================================================

#include "WText.h"
#include "Waste.h"

#if	__PowerPlant__ 
									//these macros turn on debugging test for the text engine
	#include <PP_DebugMacros.h>	//The ones I use are in PPlant. WT_Debug contains just defines
									//as empty. You will have to roll your own if you want to use 
#else								//something else. Sorry :-)
#include "W_Debug.h"
#endif

// ---------------------------------------------------------------------------
//	¥Text													[public]
// ---------------------------------------------------------------------------
//
// Purpose: The default constructor to use to create your waste object
//
// Revision:
//

WText::WText()
{
	InitWText();
}

// ---------------------------------------------------------------------------
//	¥Text													[private]
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

WText::WText(
		const WText & /*inOriginal*/ )
{
	InitWText();
}
// ---------------------------------------------------------------------------
//	¥operator=													[private]
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

WText &
WText::operator=(
		WText &  /*inRhs*/ )
{
	InitWText();
	return *this;
}

// ---------------------------------------------------------------------------
//	¥~WText
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

WText::~WText()
{
	
}

// ---------------------------------------------------------------------------
//	¥InitWText
// ---------------------------------------------------------------------------
//
// Purpose: We don't do much here. We let the subclass actually instantiate 
// the Waste instance. This allows them to set the dest and view rect and
// read in features from a stream if desired. This way the class is not tied
// to any framework
//
// Revision:
//

void
WText::InitWText()
{
	mWasteH = nil;
}

// ---------------------------------------------------------------------------
//	¥GetTextHandle
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

inline
Handle
WText::GetText() const
{
	ValidateHandle_((Handle)mWasteH);
	return WEGetText(mWasteH);
}

// ---------------------------------------------------------------------------
//	¥GetWEHandle
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

WEReference
WText::GetWEHandle() const
{
	ValidateHandle_((Handle)mWasteH);
	return mWasteH;
}

// ---------------------------------------------------------------------------
//	¥GetChar
// ---------------------------------------------------------------------------
//
// Purpose: Get a character from the text, this returns two byte. If your 
// character is one byte it will be in the first byte
//
// Revision:
//

inline
SInt16
WText::GetChar(
		SInt32 	inOffset ) const
{
	ValidateHandle_((Handle)mWasteH);
	return WEGetChar(inOffset, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥GetTextLength
// ---------------------------------------------------------------------------
//
// Purpose: return the length of the text
//
// Revision:
//

inline
SInt32
WText::GetTextLength() const
{
	ValidateHandle_((Handle)mWasteH);
	return WEGetTextLength(mWasteH);
}

// ---------------------------------------------------------------------------
//	¥GetHeight
// ---------------------------------------------------------------------------
//
// Purpose: return the height of the text
//
// Revision:
//

inline
SInt32
WText::GetHeight(
		SInt32 startLine,
		SInt32 endLine ) const
{
	ValidateHandle_((Handle)mWasteH);
	return WEGetHeight(startLine, endLine, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥GetTextSelection
// ---------------------------------------------------------------------------
//
// Purpose: Get the current selection.
//
// Revision:
//

inline
void
WText::GetTextSelection(
		SInt32 & selStart,
		SInt32 & selEnd ) const
{
	ValidateHandle_((Handle)mWasteH);
	WEGetSelection(&selStart, &selEnd, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥GetDestRect
// ---------------------------------------------------------------------------
//
// Purpose: Get the dest rect
//
// Revision:
//

inline
void
WText::GetDestRect(
		LongRect	& outDestRect) const
{
	ValidateHandle_((Handle)mWasteH);
	WEGetDestRect(&outDestRect, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥GetDestRect
// ---------------------------------------------------------------------------
//
// Purpose: Get the dest rect
//
// Revision:
//

inline
void
WText::GetViewRect(
		LongRect	& outViewRect) const
{
	ValidateHandle_((Handle)mWasteH);
	WEGetViewRect(&outViewRect, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥IsActive
// ---------------------------------------------------------------------------
//
// Purpose: Is the current view active?
//
// Revision:
//

inline
Boolean
WText::IsActive()
{
	ValidateHandle_((Handle)mWasteH);
	return WEIsActive(mWasteH);
}

// ---------------------------------------------------------------------------
//	¥GetLineRange
// ---------------------------------------------------------------------------
//
// Purpose: Given an offset, find the line it is on.
//
// Revision:
//

inline
SInt32
WText::OffsetToLine(
			SInt32	inOffset) const
{		
	ValidateHandle_((Handle)mWasteH);
	return WEOffsetToLine(inOffset, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥GetLineRange
// ---------------------------------------------------------------------------
//
// Purpose: Given a line, get the start and end of it
//
// Revision:
//
inline
void
WText::GetLineRange (	
			SInt32				inLineIndex,
			SInt32 &			outLineStart,
			SInt32 &			outLineEnd) const
{
	ValidateHandle_((Handle)mWasteH);
	WEGetLineRange(inLineIndex, &outLineStart, &outLineEnd, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥CountLines
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

inline
SInt32
WText::CountLines() const
{
	ValidateHandle_((Handle)mWasteH);
	return WECountLines(mWasteH);
}

// ---------------------------------------------------------------------------
//	¥OffsetToRun
// ---------------------------------------------------------------------------
//
// Purpose: Given a character offset in the text, return the corresponding run
//
// Revision:
//

inline
SInt32
WText::OffsetToRun(
			SInt32		inOffset) const
{
	ValidateHandle_((Handle)mWasteH);
	return WEOffsetToRun(inOffset, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥GetRunRange
// ---------------------------------------------------------------------------
//
// Purpose: Given a run index, return the character offsets that it spans
//
// Revision:
//

inline
void
WText::GetRunRange(
			SInt32		inRunIndex,
			SInt32	&	outRunStart,
			SInt32	&	outRunEnd) const
{
	ValidateHandle_((Handle)mWasteH);
	WEGetRunRange(inRunIndex, &outRunStart, &outRunEnd, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥CountRuns
// ---------------------------------------------------------------------------
//
// Purpose: Count the number of style runs
//
// Revision:
//

inline
SInt32
WText::CountRuns() const
{
	ValidateHandle_((Handle)mWasteH);
	return WECountRuns(mWasteH);
}

// ---------------------------------------------------------------------------
//	¥GetClickCount
// ---------------------------------------------------------------------------
//
// Purpose: Return the number of clicks in a multiclick action
//
// Revision:
//

inline
UInt16
WText::GetClickCount() const
{
	ValidateHandle_((Handle)mWasteH);
	return WEGetClickCount(mWasteH);
}


// ---------------------------------------------------------------------------
//	¥SetTextSelection
// ---------------------------------------------------------------------------
//
// Purpose: Set the selection.
//
// Revision:
//

inline
void
WText::SetTextSelection(
		SInt32 inSelStart,
		SInt32 inSelEnd )
{
	ValidateHandle_((Handle)mWasteH);
	WESetSelection(inSelStart, inSelEnd, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥SetDestRect
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

inline
void
WText::SetDestRect(
			const LongRect	& inDestRect)
{
	ValidateHandle_((Handle)mWasteH);
	WESetDestRect(&inDestRect, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥SetViewRect
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

inline
void
WText::SetViewRect(
			const LongRect	& inViewRect)
{
	ValidateHandle_((Handle)mWasteH);
	WESetViewRect(&inViewRect, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥ContinuousStyle
// ---------------------------------------------------------------------------
//
// Purpose: Find if a style is continuous across the selection.
//
// Revision:
//

inline
Boolean
WText::ContinuousStyle(
		WEStyleMode & ioMode,
		TextStyle	& outTextStyle ) const
{
	ValidateHandle_((Handle)mWasteH);
	return WEContinuousStyle(&ioMode, &outTextStyle, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥GetRunInfo
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

inline
void
WText::GetRunInfo(
		SInt32 		inOffset,
		WERunInfo & outRunInfo ) const
{
	ValidateHandle_((Handle)mWasteH);
	WEGetRunInfo(inOffset, &outRunInfo, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥GetRunDirection
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

inline
Boolean
WText::GetRunDirection(
		SInt32		inOffset) const
{
	ValidateHandle_((Handle)mWasteH);
	return WEGetRunDirection(inOffset, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥GetOffset
// ---------------------------------------------------------------------------
//
// Purpose: Givien a x,y location in local coordinates, find the offset in 
// the text.
//
// Revision:
//

inline
SInt32
WText::GetOffset(
		const LongPt &	inPoint,
		signed char & 	inEdge ) const
{
	ValidateHandle_((Handle)mWasteH);
	return WEGetOffset(&inPoint, &inEdge, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥GetPoint
// ---------------------------------------------------------------------------
//
// Purpose: Given an offset, find the x, y location.
//
// Revision:
//

inline
void
WText::GetPoint(
		SInt32 		inOffset,
		SInt16 		inDirection,
		LongPt & 	outPoint,
		SInt16	&	outLineHeight ) const
{
	ValidateHandle_((Handle)mWasteH);
	WEGetPoint(inOffset, inDirection, &outPoint, &outLineHeight, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥FindWord
// ---------------------------------------------------------------------------
//
// Purpose: Find the borders of the word in the text that is closest to the 
// selection
//
// Revision:
//

inline
void
WText::FindWord(
		SInt32 		inOffset,
		WEEdge	 	inEdge,
		SInt32 & 	outWordStart,
		SInt32 & 	outWordEnd ) const
{
	ValidateHandle_((Handle)mWasteH);
	WEFindWord(inOffset, inEdge, &outWordStart, &outWordEnd, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥FindLine
// ---------------------------------------------------------------------------
//
// Purpose: Find the borders of the line in the text that is closest to the 
// selection
//
// Revision:
//

inline
void
WText::FindLine(
		SInt32 		inOffset,
		WEEdge	 	inEdge,
		SInt32 & 	outLineStart,
		SInt32 & 	outLineEnd ) const
{
	ValidateHandle_((Handle)mWasteH);
	WEFindLine(inOffset, inEdge, &outLineStart, &outLineEnd, mWasteH);

}

// ---------------------------------------------------------------------------
//	¥FindParagraph
// ---------------------------------------------------------------------------
//
// Purpose:Find the borders of the paragraph in the text that is closest to the 
// selection
//
// Revision:
//

inline
void
WText::FindParagraph(
		SInt32 		inOffset,
		WEEdge	 	inEdge,
		SInt32 & 	outParagraphStart,
		SInt32 & 	outParagraphEnd ) const
{
	ValidateHandle_((Handle)mWasteH);
	WEFindParagraph(inOffset, inEdge, &outParagraphStart, &outParagraphEnd, mWasteH);

}

// ---------------------------------------------------------------------------
//	¥CopyRange
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

OSErr
WText::CopyRange(
			SInt32 		inRangeStart,
			SInt32		inRangeEnd,
			Handle		outTextH,
			StScrpHandle outStylesH,
			Handle 		outSOUPH,
			Handle		outRulerH,
			Handle		outParaH) const
{
	OSErr	anErr = noErr;
	ValidateHandle_((Handle)mWasteH);
	
	anErr = WECopyRange(inRangeStart, inRangeEnd, outTextH, outStylesH, outSOUPH,mWasteH);
	if(anErr != noErr)
		return anErr;
		
	#if WASTE_VERSION > 0x02000000
	//now copy out the ruler and paragraph info if requested.
	if(outRulerH == nil)
	{
		anErr = StreamRange(inRangeStart, inRangeEnd, 'WEru', 0, outRulerH);
		if(anErr != noErr)
		return anErr;
	
	}
	
	//finally copy the paragraph data if requested
	if(outParaH == nil)
	{
		anErr = StreamRange(inRangeStart, inRangeEnd, 'WEpf', 0, outParaH);
		if(anErr != noErr)
			return anErr;
	}
	#else
	#pragma unused(outRulerH)
	#pragma unused(outParaH)
	#endif
	
	return anErr;
}

// ---------------------------------------------------------------------------
//	¥SetAlignment
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

void
WText::SetAlignment(
		WEAlignment inAlignment )
{
	ValidateHandle_((Handle)mWasteH);
	WESetAlignment(inAlignment, mWasteH);
	UserChangedText();
}

// ---------------------------------------------------------------------------
//	¥GetAlignment
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

inline
WEAlignment
WText::GetAlignment() const
{
	ValidateHandle_((Handle)mWasteH);
	return WEGetAlignment(mWasteH);
}

// ---------------------------------------------------------------------------
//	¥GetDirection
// ---------------------------------------------------------------------------
//
// Purpose: Get the dominant direction for the text
//
// Revision:
//

inline
WEDirection
WText::GetDirection () const
{
	ValidateHandle_((Handle)mWasteH);
	return WEGetDirection(mWasteH);
}


void
WText::SetDirection (			
			WEDirection		inDirection)
{
	ValidateHandle_((Handle)mWasteH);
	WESetDirection(inDirection, mWasteH);
	UserChangedText();
}

// ---------------------------------------------------------------------------
//	¥CalText
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

inline
void
WText::CalText()
{
	ValidateHandle_((Handle)mWasteH);
	WECalText(mWasteH);
}

// ---------------------------------------------------------------------------
//	¥Update
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

inline
void
WText::Update(
		RgnHandle	inUpdateRgn)
{
	ValidateHandle_((Handle)mWasteH);
	WEUpdate(inUpdateRgn, mWasteH);
}


// ---------------------------------------------------------------------------
//	¥Scroll
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

inline
void
WText::Scroll(
		SInt32		inHorizontalOffset,
		SInt32		inVerticalOffset)
{
	ValidateHandle_((Handle)mWasteH);
	WEScroll(inHorizontalOffset, inVerticalOffset, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥SelView
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

inline
void
WText::SelView()
{
	ValidateHandle_((Handle)mWasteH);
	WESelView(mWasteH);
}

// ---------------------------------------------------------------------------
//	¥Activate
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

inline
void
WText::Activate()
{
	ValidateHandle_((Handle)mWasteH);
	WEActivate(mWasteH);
}

// ---------------------------------------------------------------------------
//	¥Deactivate
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

inline
void
WText::Deactivate()
{
	ValidateHandle_((Handle)mWasteH);
	WEDeactivate(mWasteH);
}

// ---------------------------------------------------------------------------
//	¥Key
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

inline
void
WText::Key(
		CharParameter		inKey,
		EventModifiers		inModifiers)
{
	ValidateHandle_((Handle)mWasteH);
	WEKey(inKey, inModifiers, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥Click
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

inline
void
WText::Click(
		Point				inHitPoint,
		EventModifiers		inModifiers,
		UInt32				inClickTime)
{
	ValidateHandle_((Handle)mWasteH);
	WEClick(inHitPoint, inModifiers, inClickTime, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥AdjustCursor
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

inline
Boolean
WText::AdjustCursor(
			Point		inMouseLoc,
			RgnHandle	ioMouseRgn)
{
	ValidateHandle_((Handle)mWasteH);
	return WEAdjustCursor(inMouseLoc, ioMouseRgn, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥Idle
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

inline
void
WText::Idle(
		UInt32 &		outMaxSleep)
{
	ValidateHandle_((Handle)mWasteH);
	WEIdle(&outMaxSleep, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥Delete
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

inline
void
WText::Delete()
{
	ValidateHandle_((Handle)mWasteH);
	WEDelete(mWasteH);
}

// ---------------------------------------------------------------------------
//	¥SetStyle
// ---------------------------------------------------------------------------
//
// Purpose: Set the style of the selection
//
// Revision:
//

OSErr
WText::SetStyle(
		WEStyleMode			inMode,
		const TextStyle		inTextStyle)
{
	ValidateHandle_((Handle)mWasteH);
	OSErr anErr = WESetStyle(inMode, &inTextStyle, mWasteH);
	UserChangedText();
	return anErr;
}


// ---------------------------------------------------------------------------
//	¥UseStyleScrap
// ---------------------------------------------------------------------------
//
// Purpose: Set the style scrap for the object
//
// Revision:
//

OSErr
WText::UseStyleScrap(
			StScrpHandle		inStyles)
{
	ValidateHandle_((Handle)mWasteH);
	
	OSErr anErr = WEUseStyleScrap(inStyles, mWasteH);
	UserChangedText();
	return anErr;
}

// ---------------------------------------------------------------------------
//	¥UseText
// ---------------------------------------------------------------------------
//
// Purpose: Set the text for the object
//
// Revision:
//

OSErr
WText::UseText(
		Handle		inText)
{
	ValidateHandle_((Handle)mWasteH);
	OSErr anErr = WEUseText(inText, mWasteH);
	UserChangedText();
	return anErr;
}

// ---------------------------------------------------------------------------
//	¥Undo
// ---------------------------------------------------------------------------
//
// Purpose: Undo the last action
//
// Revision:
//

inline
OSErr
WText::Undo()
{
	ValidateHandle_((Handle)mWasteH);
	return WEUndo(mWasteH);
}

// ---------------------------------------------------------------------------
//	¥ClearUndo
// ---------------------------------------------------------------------------
//
// Purpose: Clear the Undo stack
//
// Revision:
//

inline
void
WText::ClearUndo()
{
	ValidateHandle_((Handle)mWasteH);
	WEClearUndo(mWasteH);
}

// ---------------------------------------------------------------------------
//	¥GetUndoInfo
// ---------------------------------------------------------------------------
//
// Purpose: Get the undo information.
//
// Revision:
//

inline
WEActionKind
WText::GetUndoInfo(
		Boolean	&	outRedoFlag) const
{
	ValidateHandle_((Handle)mWasteH);
	return WEGetUndoInfo(&outRedoFlag, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥IsTyping
// ---------------------------------------------------------------------------
//
// Purpose: Check to see if we are typing
//
// Revision:
//

inline
Boolean
WText::IsTyping() const
{
	ValidateHandle_((Handle)mWasteH);
	return WEIsTyping(mWasteH);
}

// ---------------------------------------------------------------------------
//	¥SetRuler
// ---------------------------------------------------------------------------
//
// Purpose: Notify waste that a multistep sequence is starting that should 
// be compiled into one action.
//
// Revision:
//

inline
OSErr
WText::BeginAction()
{
	ValidateHandle_((Handle)mWasteH);
	return WEBeginAction(mWasteH);
}

// ---------------------------------------------------------------------------
//	¥SetRuler
// ---------------------------------------------------------------------------
//
// Purpose: Set the ruler of the selection
//
// Revision:
//

inline
OSErr
WText::EndAction(
			WEActionKind	inActionKind)
{
	ValidateHandle_((Handle)mWasteH);
	return WEEndAction(inActionKind, mWasteH);
}


// ---------------------------------------------------------------------------
//	¥GetModCount
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

inline
UInt32
WText::GetModCount() const
{
	ValidateHandle_((Handle)mWasteH);
	return WEGetModCount(mWasteH);
}

// ---------------------------------------------------------------------------
//	¥ResetModCount
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

inline
void
WText::ResetModCount()
{
	ValidateHandle_((Handle)mWasteH);
	WEResetModCount(mWasteH);
}

// ---------------------------------------------------------------------------
//	¥Cut
// ---------------------------------------------------------------------------
//
// Purpose: Cut the selection
//
// Revision:
//

inline
OSErr
WText::Cut()
{
	ValidateHandle_((Handle)mWasteH);
	return WECut(mWasteH);
}

// ---------------------------------------------------------------------------
//	¥Copy
// ---------------------------------------------------------------------------
//
// Purpose: Copy the selection
//
// Revision:
//

inline
OSErr
WText::Copy()
{
	ValidateHandle_((Handle)mWasteH);
	return WECopy(mWasteH);
}

// ---------------------------------------------------------------------------
//	¥Paste
// ---------------------------------------------------------------------------
//
// Purpose: Paste the selection
//
// Revision:
//

inline
OSErr
WText::Paste()
{
	ValidateHandle_((Handle)mWasteH);
	return WEPaste(mWasteH);
}

// ---------------------------------------------------------------------------
//	¥CanPaste
// ---------------------------------------------------------------------------
//
// Purpose: CanPaste the selection
//
// Revision:
//

inline
Boolean
WText::CanPaste()
{
	ValidateHandle_((Handle)mWasteH);
	return WECanPaste(mWasteH);
}

// ---------------------------------------------------------------------------
//	¥GetHiliteRgn
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

inline
RgnHandle
WText::GetHiliteRgn(
		SInt32 inRangeStart,
		SInt32 inRangeEnd ) const
{
	ValidateHandle_((Handle)mWasteH);
	return WEGetHiliteRgn(inRangeStart, inRangeEnd, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥GetInfo
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

inline
OSErr
WText::GetInfo(
		WESelector 	inSelector,
		void *	outInfo ) const
{
	ValidateHandle_((Handle)mWasteH);
	return WEGetInfo(inSelector, outInfo, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥SetInfo
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

inline
OSErr
WText::SetInfo(
		OSType 		inSelector,
		const void* inInfo)
{
	ValidateHandle_((Handle)mWasteH);
	return WESetInfo(inSelector, inInfo, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥GetUserInfo
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

inline
OSErr
WText::GetUserInfo(
			WESelector			inUserTag,
			SInt32 &			outUserInfo) const
{
	ValidateHandle_((Handle)mWasteH);
	return WEGetUserInfo(inUserTag, &outUserInfo, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥SetUserInfo
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

inline
OSErr
WText::SetUserInfo(
			WESelector			inUserTag,
			SInt32				inUserInfo)
{
	ValidateHandle_((Handle)mWasteH);
	return WESetUserInfo(inUserTag, inUserInfo, mWasteH);
}

#pragma mark ---WASTE 2.0 ---

// ---------------------------------------------------------------------------
//	¥InsertFormattedText		
// ---------------------------------------------------------------------------
//
// Purpose: This must be overridden
//
// Revision:
//

inline
OSErr
WText::InsertFormattedText(
		const void *		inTextPtr,
		SInt32				inTextLength,
		StScrpHandle		inStyles,
		WESoupHandle		inSoup,
		Handle				inParaFormat,
		Handle				inRulerScraps)
{
	ValidateHandle_((Handle)mWasteH);
	
	#if WASTE_VERSION > 0x02000000
	return WEInsertFormattedText(inTextPtr, inTextLength,
						inStyles, inSoup, inParaFormat, inRulerScraps, mWasteH);
	#else
	
	#pragma unused(inParaFormat)
	#pragma unused(inRulerScraps)
	
	return WEInsert(inTextPtr, inTextLength, inStyles, inSoup, mWasteH);
	#endif
}


// ---------------------------------------------------------------------------
//	¥StreamRange
// ---------------------------------------------------------------------------
//
// Purpose:grab the requested type in the range and send it back to the caller
// in the handle
//
// Revision:
//

inline
OSErr
WText::StreamRange(
			SInt32				inRangeStart,
			SInt32				inRangeEnd,
			FlavorType			inRequestedType,
			OptionBits			inFlags,
			Handle				outData) const
{
	ValidateHandle_((Handle)mWasteH);
	
	OSErr	retErr = noErr;
	#if WASTE_VERSION > 0x02000000
	retErr = WEStreamRange(inRangeStart, inRangeEnd, inRequestedType, inFlags, outData, mWasteH);
	#else
	
	#pragma unused(inFlags)
	
	switch(inRequestedType)
	{
		case 'TEXT':
			retErr = CopyRange(inRangeStart, inRangeEnd, outData, nil, nil);
			break;
			
		case 'styl':
			retErr = CopyRange(inRangeStart, inRangeEnd, nil, StScrpHandle(outData), nil);
			break;
		case 'SOUP':
			retErr = CopyRange(inRangeStart, inRangeEnd, nil, nil, outData);
			break;
	}
	#endif
	
	return retErr;
}



#if WASTE_VERSION > 0x02000000
// ---------------------------------------------------------------------------
//	¥GetRunInfoByIndex
// ---------------------------------------------------------------------------
//
// Purpose: Get run info by its run index. 
//
// Revision:
//
inline
void
WText::GetRunInfoByIndex(
		SInt32		inIndex,
		WERunInfo &	outRunInfo) const
{
	ValidateHandle_((Handle)mWasteH);
	#if WT_DEBUG == 1
	SInt32	numRuns = CountRuns();
	
	//sanity checks
	Assert_(inIndex >= 0);
	Assert_(inIndex <= numRuns);
	#endif
	
	WEGetIndRunInfo(inIndex, &outRunInfo, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥GetIndUndoInfo
// ---------------------------------------------------------------------------
//
// Purpose: Get undo information
//
// Revision:
//

inline
WEActionKind
WText::GetIndUndoInfo(
			SInt32		inUndoLevel) const
{
	ValidateHandle_((Handle)mWasteH);
	return WEGetIndUndoInfo(inUndoLevel, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥Redo
// ---------------------------------------------------------------------------
//
// Purpose: Redo the last action
//
// Revision:
//

inline
OSErr
WText::Redo()
{
	ValidateHandle_((Handle)mWasteH);
	return WERedo(mWasteH);
}

// ---------------------------------------------------------------------------
//	¥ChangeCase
// ---------------------------------------------------------------------------
//
// Purpose: Set the ruler of the selection
//
// Revision:
//

inline
OSErr
WText::ChangeCase(
			SInt16		inCase)
{
	ValidateHandle_((Handle)mWasteH);
	return WEChangeCase(inCase, mWasteH);
}

// ---------------------------------------------------------------------------
//	¥SetRuler
// ---------------------------------------------------------------------------
//
// Purpose: Set the ruler of the selection
//
// Revision:
//

OSErr
WText::SetRuler(
		WERulerMode			inMode,
		const WERuler		inRuler)
{
	ValidateHandle_((Handle)mWasteH);
	OSErr anErr = WESetRuler(inMode, &inRuler, mWasteH);
	UserChangedText();
	return anErr;
}


// ---------------------------------------------------------------------------
//	¥PinScroll
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

inline
void
WText::PinScroll(
		SInt32		inHorizontalOffset,
		SInt32		inVerticalOffset)
{
	ValidateHandle_((Handle)mWasteH);
	WEPinScroll(inHorizontalOffset, inVerticalOffset, mWasteH);
}


// ---------------------------------------------------------------------------
//	¥GetParaInfo
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

inline
void
WText::GetParaInfo(
		SInt32 		inOffset,
		WEParaInfo & outParaInfo ) const
{
	ValidateHandle_((Handle)mWasteH);
	WEGetParaInfo(inOffset, &outParaInfo, mWasteH);
}


// ---------------------------------------------------------------------------
//	¥ContinuousRuler
// ---------------------------------------------------------------------------
//
// Purpose:
//
// Revision:
//

inline
Boolean
WText::ContinuousRuler(
		WERulerMode & 	ioRulerMode,
		WERuler& 	inRuler ) const
{
	ValidateHandle_((Handle)mWasteH);
	return WEContinuousRuler(&ioRulerMode, &inRuler, mWasteH);
}

#endif


inline
SInt16
WText::FeatureFlag(
			SInt16	inFeature,
			SInt16	inAction) const
{
	ValidateHandle_((Handle)mWasteH);
	return WEFeatureFlag(inFeature, inAction, mWasteH);
}

#pragma mark ---StFeatureFlag---

// ---------------------------------------------------------------------------
//	¥StFeatureFlag
// ---------------------------------------------------------------------------
//
// Purpose: A stack class to make temporarily setting or clearing a feature
// simple. The constructor sets/clears the feature the destructor clears/resets
// the feature.
//
// Revision:
//

StFeatureFlag::StFeatureFlag(
					SInt16		inFeature,
					SInt16		inAction,
					WEReference	inWE)
:
	mFeature(inFeature),
	mWE(inWE)
{
	ValidateHandle_((Handle)inWE);
	mAction = WEFeatureFlag(mFeature, inAction, mWE);
}

// ---------------------------------------------------------------------------
//	¥StFeatureFlag
// ---------------------------------------------------------------------------
//
// Purpose: You can also call this class using a pointer to a WTextView object.
//
// Revision:
//
StFeatureFlag::StFeatureFlag(
					SInt16		inFeature,
					SInt16		inAction,
					WText*		inTextView)
: mFeature(inFeature)
{
	ValidateObject_(inTextView);
	mWE = inTextView->GetWEHandle();
	ValidateHandle_((Handle)mWE);
	mAction = WEFeatureFlag(mFeature, inAction, mWE);
}

StFeatureFlag::~StFeatureFlag()
{
	WEFeatureFlag(mFeature, mAction, mWE);
}



#pragma mark ---StSetSelectionForRoutine---

// ---------------------------------------------------------------------------
//	¥StSetSelectionForRoutine
// ---------------------------------------------------------------------------
//
// Purpose: A class to temporarily set the seleciton to the input parameters.
// The constructor remembers the old selection, preventss drawing and then
// sets the new selection. The destructor resets the old selection and
// resets the drawing flag.
//
// Revision:
//

StSetSelectionForRoutine::StSetSelectionForRoutine(
							SInt32		inStart,
							SInt32		inEnd,
							WEReference	inWE)
:
	mWE(inWE)
{
	ValidateHandle_((Handle)mWE);
	mPrevDrawState = WEFeatureFlag(weFInhibitRedraw, weBitSet, mWE);
	WEGetSelection(&mOldStart, &mOldEnd, mWE);
	WESetSelection(inStart, inEnd, mWE); 
}

StSetSelectionForRoutine::~StSetSelectionForRoutine()
{
	WESetSelection(mOldStart, mOldEnd, mWE);
	WEFeatureFlag(weFInhibitRedraw, mPrevDrawState, mWE);
}

