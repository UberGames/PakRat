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
//=================================================================

#ifndef		__WText
#define		__WText
#pragma once


#if __PowerPlant__ && PP_Uses_Pragma_Import
	#pragma import on
#endif

#ifndef __WASTE__
#include "waste.h"
#endif

#ifndef __climits__
#include <climits>
#endif

#if WASTE_VERSION < 0x02000000
enum
{
	kTypeText					=	FOUR_CHAR_CODE('TEXT'),	/* raw text */
	kTypeStyles 				=	FOUR_CHAR_CODE('styl'),	/* TextEdit-compatible style scrap */
	kTypeSoup 					=	FOUR_CHAR_CODE('SOUP')	/* "soup" of embedded objects */
};
#endif

class	WText
{
	public:
		// methods
								WText();
		
		virtual 				~WText();
		
		virtual Handle			GetText() const;
		
		virtual WEReference		GetWEHandle() const;
		
		virtual SInt16			GetChar(
									SInt32 	inOffset ) const;
	
		virtual SInt32			GetTextLength() const;

		virtual	SInt32			GetHeight(
									SInt32 startLine,
									SInt32 endLine ) const;
		
		virtual void			GetTextSelection(
									SInt32 & selStart,
									SInt32 & selEnd ) const;

		virtual void			GetDestRect(
									LongRect	& outDestRect) const;
		
		virtual void			GetViewRect(
									LongRect	& outViewRect) const;
		
		virtual Boolean			IsActive();

		virtual	SInt32			OffsetToLine(
									SInt32	inOffset) const;
		
		virtual void			GetLineRange (	
									SInt32		inLineIndex,
									SInt32 &	outLineStart,
									SInt32 &	outLineEnd) const;
		
		virtual SInt32			CountLines() const;

		virtual SInt32			OffsetToRun(
									SInt32		inOffset) const;

		virtual void			GetRunRange(
									SInt32		inRunIndex,
									SInt32	&	outRunStart,
									SInt32	&	outRunEnd) const;

		virtual	SInt32			CountRuns() const;

		virtual UInt16			GetClickCount() const;

		virtual void			SetTextSelection(
									SInt32 inSelStart,
									SInt32 inSelEnd );
									
		virtual void			SetDestRect(
									const LongRect	& inDestRect);

		virtual	void			SetViewRect(
									const LongRect	& inViewRect);

		virtual Boolean			ContinuousStyle(
									WEStyleMode & ioMode,
									TextStyle	& outTextStyle ) const;

		virtual void			GetRunInfo(
										SInt32 		inOffset,
										WERunInfo & outRunInfo ) const;

		virtual Boolean			GetRunDirection(
										SInt32		inOffset) const;

		virtual	SInt32			GetOffset(
										const LongPt &	inPoint,
										signed char & 	inEdge) const;

		virtual	void			GetPoint(
										SInt32 		inOffset,
										SInt16 		inDirection,
										LongPt & 	outPoint,
										SInt16	&	outLineHeight ) const;

		virtual void			FindWord(
										SInt32 		inOffset,
										WEEdge	 	inEdge,
										SInt32 & 	outWordStart,
										SInt32 & 	outWordEnd ) const;

		virtual void			FindLine(
										SInt32 		inOffset,
										WEEdge	 	inEdge,
										SInt32 & 	outLineStart,
										SInt32 & 	outLineEnd ) const;

		virtual void			FindParagraph(
										SInt32 		inOffset,
										WEEdge	 	inEdge,
										SInt32 & 	outParagraphStart,
										SInt32 & 	outParagraphEnd ) const;
	
		virtual OSErr			CopyRange(
										SInt32 		inRangeStart,
										SInt32		inRangeEnd,
										Handle		outTextH,
										StScrpHandle outStylesH,
										Handle		 outSOUPH,
										Handle		outRulerH = nil,
										Handle		outParaH = nil) const;

		virtual	void			SetAlignment(
										WEAlignment inAlignment );

		virtual WEAlignment		GetAlignment() const;

		virtual WEDirection		GetDirection () const;

		virtual void			SetDirection (			
										WEDirection		inDirection);

		virtual void			CalText();

		virtual void			Update(
										RgnHandle	inUpdateRgn);

		virtual void			Scroll(
										SInt32		inHorizontalOffset,
										SInt32		inVerticalOffset);
	
		virtual void			SelView();

		virtual void			Activate();
		
		virtual void			Deactivate();

		virtual void			Key(
										CharParameter		inKey,
										EventModifiers		inModifiers);

		virtual void			Click(
										Point				inHitPoint,
										EventModifiers		inModifiers,
										UInt32				inClickTime);
										

		virtual Boolean			AdjustCursor(
										Point		inMouseLoc,
										RgnHandle	ioMouseRgn);

		virtual void			Idle(
										UInt32 &		outMaxSleep);

		virtual void			Delete();

		virtual OSErr			SetStyle(
										WEStyleMode			inMode,
										const TextStyle		inTextStyle);

		virtual OSErr			UseStyleScrap(
										StScrpHandle		inStyles);

		virtual OSErr			UseText(
										Handle		inText);

		virtual OSErr			Undo();

		virtual void			ClearUndo();

		virtual WEActionKind	GetUndoInfo(
										Boolean	&	outRedoFlag) const;

		virtual Boolean			IsTyping() const;

		virtual OSErr			BeginAction();

		virtual OSErr			EndAction(
										WEActionKind	inActionKind);

		virtual UInt32			GetModCount() const;

		virtual void			ResetModCount();

		virtual OSErr			Cut();

		virtual	OSErr			Copy();

		virtual OSErr			Paste();

		virtual Boolean			CanPaste();

		virtual RgnHandle		GetHiliteRgn(
										SInt32 inRangeStart,
										SInt32 inRangeEnd ) const;

		virtual OSErr			GetInfo(
										WESelector 	inSelector,
										void *	outInfo ) const;

		virtual OSErr			SetInfo(
										OSType 		inSelector,
										const void* inInfo);

		virtual OSErr			GetUserInfo(
										WESelector			inUserTag,
										SInt32 &			outUserInfo) const;

		virtual OSErr			SetUserInfo(
										WESelector			inUserTag,
										SInt32				inUserInfo);
												
		virtual void			UserChangedText() = 0 {}
			
		virtual SInt16			FeatureFlag(
									SInt16	inFeature,
									SInt16	inAction) const;

//version 2.0 features


		virtual OSErr			InsertFormattedText(
										const void *		inTextPtr,
										SInt32				inTextLength,
										StScrpHandle		inStyles,
										WESoupHandle		inSoup,
										Handle				inParaFormat,
										Handle				inRulerScrap);
		
		virtual OSErr			StreamRange(
										SInt32				inRangeStart,
										SInt32				inRangeEnd,
										FlavorType			inRequestedType,
										OptionBits			inFlags,
										Handle				outData) const;
		
			 	
#if WASTE_VERSION > 0x02000000
		virtual void			GetRunInfoByIndex(
										SInt32		inIndex,
										WERunInfo &	outRunInfo) const;
		
		virtual WEActionKind	GetIndUndoInfo(
										SInt32		inUndoLevel) const;
		virtual OSErr			Redo();

		virtual OSErr			ChangeCase(
										SInt16		inCase);
		
		virtual OSErr			SetRuler(
										WERulerMode			inMode,
										const WERuler		inRuler);
		
		
		virtual void			PinScroll(
										SInt32		inHorizontalOffset,
										SInt32		inVerticalOffset);
	
		
		virtual void			GetParaInfo(
										SInt32 		inOffset,
										WEParaInfo & outParaInfo ) const;

										
		virtual Boolean			ContinuousRuler(
										WERulerMode & 	ioRulerMode,
										WERuler& 	inRuler ) const;
#endif

	protected:
		// methods
		virtual void			CheckScroll() = 0 {}
		
		
		// attributes
		WEReference				mWasteH;
		
	private:
		// methods
								WText(
									const WText& inOriginal);
		
		WText &					operator=(
									WText &  inRhs);
		
		void					InitWText();
		
		
};

class	StFeatureFlag {

		private:
				StFeatureFlag() {}
				
				StFeatureFlag(
						StFeatureFlag	& /*inOriginal*/) {}
						
	StFeatureFlag&		operator=(
						StFeatureFlag	& /*inRhs*/){return *this;}
				
		public:
				StFeatureFlag(
							SInt16		inFeature,
							SInt16		inAction,
							WEReference	inWE);

				StFeatureFlag(
					SInt16		inFeature,
					SInt16		inAction,
					WText*		inTextView);
					
				~StFeatureFlag();
		
		private:
			SInt16		mFeature;
			SInt16		mAction;
			WEReference	mWE;
};


// a stack based class for setting the selection to something else
// before calling a WASTE routine that can only query the selection
// The class sets this without drawing anything and then sets it 
// back. 

class	StSetSelectionForRoutine {

		private:
				StSetSelectionForRoutine();
				
				StSetSelectionForRoutine(
						StSetSelectionForRoutine	& /*inOriginal*/);
						
	StSetSelectionForRoutine&		operator=(
						StSetSelectionForRoutine	& /*inRhs*/);
				
		public:
				StSetSelectionForRoutine(
							SInt32		inStart,
							SInt32		inEnd,
							WEReference	inWE);
							
				~StSetSelectionForRoutine();
		
		private:
			SInt32		mPrevDrawState;
			SInt32		mOldStart;
			SInt32		mOldEnd;
			WEReference	mWE;
};

#if WASTE_VERSION < 0x02000000
//we need to make this public.
pascal void _WEGetIndStyle(SInt32 runIndex, WERunInfo *info, WEHandle hWE);
#endif

#if __PowerPlant__ && PP_Uses_Pragma_Import
	#pragma import reset
#endif

#endif
