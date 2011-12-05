// ===========================================================================
//	CDirtyText.cp			   ©1994-1999 Metrowerks Inc. All rights reserved.
// ===========================================================================
//
//	Subclass of LTextEditView which keeps track of whether the text is dirty
//	or not (i.e. the user has made changes since last save). Since
//	LTextEditView (should) follows any user change to the text with a call to
//	UserChangedText(), we just need to track that here to set our dirty flag.
//	We also must override SetTextPtr and Insert to flag the dirty changes
//	as these are usually programatic changes of text, not typically directly
//	resulting from a user action (tho they can in your implementations, so
//	just call SetDirty() afterwards).

#include "CDirtyText.h"


CDirtyText::CDirtyText(
	LStream*	inStream)
	
	: LTextEditView(inStream)
{
	mIsDirty = false;
}


void
CDirtyText::UserChangedText()
{
	if (!mIsDirty) {
		SetUpdateCommandStatus(true);
		mIsDirty = true;
	}
}


void
CDirtyText::SetTextPtr(
	Ptr				inTextP,
	SInt32			inTextLen,
	StScrpHandle	inStyleH)
{
	LTextEditView::SetTextPtr(inTextP, inTextLen, inStyleH);
		
	mIsDirty = false;
}


void
CDirtyText::Insert(
	const void*		inText,
	SInt32			inLength,
	StScrpHandle	inStyleH,
	Boolean			inRefresh )
{
	try {
		LTextEditView::Insert( inText, inLength, inStyleH, inRefresh );
	}
	
	catch (...) {
					// We failed for some reason, so let's return to short
					// circut things
		return;
	}
	
	UserChangedText();
}


// these are inlined in the header
#pragma mark CDirtyText::IsDirty
#pragma mark CDirtyText::SetDirty