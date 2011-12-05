

#pragma once

#include <LStdControl.h>
#include <LPopupButton.h>
#include "CShader.h"

class WTextView;

class	CShaderMenu : public LPopupButton {
public:
	enum { class_ID = 'popS' };
	static CShaderMenu*	
					CreateShaderMenuStream(LStream *inStream);
					
					CShaderMenu(const SPaneInfo &inPaneInfo,
						MessageT inValueMessage, SInt16 inTitleOptions,
						ResIDT inMENUid, SInt16 inTitleWidth,
						SInt16 inPopupVariation, ResIDT inTextTraitsID,
						Str255 inTitle, OSType inResTypeMENU,
						Str255 inInitialMenuItemText);
					CShaderMenu(LStream *inStream);
	virtual			~CShaderMenu();
	
	void			SetTextView(WTextView *textView);
	void			SetSelectedShader(const char *name) ;
	void			SelectShaderAtIndex(SInt32 index) ;
	void			Refresh() ;
	
	virtual void		HotSpotAction(
								SInt16				inHotSpot,
								Boolean				inCurrInside,
								Boolean				inPrevInside);
	virtual void		HotSpotResult(
								SInt16	 inHotSpot );
	
private:
	WTextView 			*_textView;
	CShaderIndexList 	*_indexInfo;
};



// =================================================================================
//	CShaderMenuAttachment
// =================================================================================

#include <LAttachment.h>

class CShaderMenuAttachment : public LAttachment {
public:
					CShaderMenuAttachment( CShaderMenu *inWindowMenu );
					~CShaderMenuAttachment();
	
protected:
	CShaderMenu *	mShaderMenu;

	void			ExecuteSelf( MessageT inMessage, void *ioParam );
};

