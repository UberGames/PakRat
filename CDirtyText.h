// ===========================================================================
//	CDirtyText.h			   ©1994-1999 Metrowerks Inc. All rights reserved.
// ===========================================================================

#pragma once

#include <LTextEditView.h>

class CDirtyText : public LTextEditView {
public:
	enum { class_ID = FOUR_CHAR_CODE('Dtxt') };
	 
 						CDirtyText(
 								LStream*		inStream);
 						
	virtual void		UserChangedText();
	
	bool				IsDirty() const { return mIsDirty; }
	
	void				SetDirty(
								bool				inDirty)
							{
								mIsDirty = inDirty;
							}
	
	virtual	void		SetTextPtr(
								Ptr					inTextP,
								SInt32				inTextLen,
								StScrpHandle		inStyleH = nil);
	
	virtual	void		Insert(	const void*			inText,
								SInt32				inLength,
								StScrpHandle		inStyleH = nil,
								Boolean				inRefresh = Refresh_No);
	
protected:
			bool		mIsDirty;
};
