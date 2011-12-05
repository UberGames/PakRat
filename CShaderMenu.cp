// ===========================================================================
//	CShaderMenu.cp		  
// ===========================================================================
//

#ifdef PowerPlant_PCH
#include PowerPlant_PCH
#endif

#include <LControlImp.h>

#include "WTextView.h"
#include "CShaderMenu.h"
#include "CShaderParser.h"



// ===========================================================================
// ¥ CShaderMenu										CShaderMenu ¥
// ===========================================================================

CShaderMenu*
CShaderMenu::CreateShaderMenuStream(
	LStream	*inStream)
{
	return (new CShaderMenu(inStream));
}


// ---------------------------------------------------------------------------
//		¥ CShaderMenu
// ---------------------------------------------------------------------------
//	Construct from input parameters

CShaderMenu::CShaderMenu(
	const SPaneInfo	&inPaneInfo,
	MessageT		inValueMessage,
	SInt16			inTitleOptions,
	ResIDT			inMENUid,
	SInt16			inTitleWidth,
	SInt16			inPopupVariation,
	ResIDT			inTextTraitsID,
	Str255			inTitle,
	OSType			inResTypeMENU,
	Str255			inInitialMenuItemText)
		: LPopupButton(inPaneInfo, inValueMessage, inTitleOptions,
				inMENUid, inTitleWidth, inPopupVariation, inTextTraitsID,
				inTitle, inResTypeMENU, 1)
{
#pragma unused (inInitialMenuItemText)
 
 	_textView = nil;
	_indexInfo = nil;
	AddAttachment(new CShaderMenuAttachment(this));
}


// ---------------------------------------------------------------------------
//		¥ CShaderMenu(LStream*)
// ---------------------------------------------------------------------------
//	Construct from data in a Stream

CShaderMenu::CShaderMenu(
	LStream	*inStream)
		: LPopupButton(inStream)
{		
	_textView = nil;
	_indexInfo = nil;
	AddAttachment(new CShaderMenuAttachment(this));
}


CShaderMenu::~CShaderMenu()
{
}


void
CShaderMenu::HotSpotAction(
	SInt16		inHotSpot,
	Boolean		inCurrInside,
	Boolean		inPrevInside)
{
	LControlPane::HotSpotAction(inHotSpot, inCurrInside, inPrevInside);
	Refresh();
}


// ---------------------------------------------------------------------------
//	¥ HotSpotResult													  [public]
// ---------------------------------------------------------------------------
//	Perform result of clicking and releasing mouse inside a HotSpot
//
//	Subclasses should usually override this function to implement
//	behavior associated with clicking in a Control HotSpot.

void
CShaderMenu::HotSpotResult(
	SInt16	 inHotSpot )
{
	LControlPane::HotSpotResult(inHotSpot);
	int selectedItem = GetCurrentMenuItem();
	SelectShaderAtIndex(selectedItem);
}


void
CShaderMenu::SetTextView(WTextView *textView) 
{
	 _textView = textView;
}

void
CShaderMenu::Refresh()
{	
	// get the menu handle
	MenuHandle theMenuH = GetMacMenuH();
	SInt32 savedSelection = GetCurrentMenuItem();
	
	
	dprintf("CShaderMenu::Refresh current item %d \n", savedSelection);
	
	if (_textView) {

		CShaderParser parser;
		if (_indexInfo)
			delete _indexInfo;
		_indexInfo = parser.index(*_textView->GetText(), _textView->GetTextLength());
		
		// remove excess menu items
		int itemcount;
		while (itemcount = ::CountMenuItems(theMenuH) > _indexInfo->size()) {
			::DeleteMenuItem(mMenuH, ::CountMenuItems(theMenuH));
			//dprintf("deleting item %d (%d)\n", itemcount, _indexInfo->size());
		}
		
		// add missing items
		while (itemcount = ::CountMenuItems(theMenuH) < _indexInfo->size()) {
			::MacInsertMenuItem(mMenuH, "\pinserted", ::CountMenuItems(theMenuH));
			//dprintf("inserting item %d (%d)\n", itemcount, _indexInfo->size());
		}
		

		int index = 1;
		CShaderIndexList_iterator e1 = _indexInfo->begin();
		while (e1 != _indexInfo->end()) {
			CShaderIndexInfo info = *e1;
			
			unsigned char s[256];
			c2pstrcpy(s, (const char*)info.name);
			SetMenuItemText(index, s);
			e1++;
			index++;
		}
		SetMenuMinMax();
		SetCurrentMenuItem(savedSelection);
	}
}

void			
CShaderMenu::SelectShaderAtIndex(SInt32 index) 
{
	if (!_indexInfo) 
		Refresh();
		
	if (_indexInfo) {
		CShaderIndexList_iterator e1 = _indexInfo->begin();
		for (int i = 1; i < index && e1 != _indexInfo->end(); i++, e1++) 
			;
		CShaderIndexInfo info = *e1;
		_textView->FocusDraw();
		_textView->SetTextSelection(info.startIndex, info.startIndex); // scroll top of selection into view
		_textView->SetTextSelection(info.startIndex, info.endIndex);
		SetCurrentMenuItem(index);
		Draw(nil);
	}
}


void			
CShaderMenu::SetSelectedShader(const char *name) 
{
	string theItem = lowerString(name);
	SInt32 index = 1;
	if (!_indexInfo) 
		Refresh();
		
	if (_indexInfo) {
	
		CShaderIndexList_iterator e1 = _indexInfo->begin();
		while (e1 != _indexInfo->end()) {
			CShaderIndexInfo info = *e1;
			if (lowerString(info.name) == theItem) {
				SelectShaderAtIndex(index);
				break;
			}
			e1++;
			index++;
		}
	}

}


#pragma mark


// =================================================================================
//	CShaderMenuAttachment
// =================================================================================

// ---------------------------------------------------------------------------------
//		¥ CShaderMenuAttachment
// ---------------------------------------------------------------------------------

CShaderMenuAttachment::CShaderMenuAttachment(
	CShaderMenu	*inShaderMenu )
		: LAttachment( msg_AnyMessage, true ), mShaderMenu( inShaderMenu )
{
}


// ---------------------------------------------------------------------------------
//		¥ ~CShaderMenuAttachment
// ---------------------------------------------------------------------------------

CShaderMenuAttachment::~CShaderMenuAttachment()
{
}


// ---------------------------------------------------------------------------------
//		¥ ExecuteSelf
// ---------------------------------------------------------------------------------

void
CShaderMenuAttachment::ExecuteSelf(
	MessageT	inMessage,
	void		*ioParam )
{
#pragma unused (ioParam)
	// Turn on host execution by default.
	mExecuteHost = true;

	if ( inMessage == msg_CommandStatus ) {
	//	dprintf("CShaderMenuAttachment::ExecuteSelf msg_CommandStatus\n");	
	} else if ( inMessage == msg_Click) {
		mShaderMenu->Refresh();
	//	dprintf("CShaderMenuAttachment::ExecuteSelf inMessage == msg_Click || inMessage == msg_DrawOrPrint\n");	
	} else {
	//	dprintf("CShaderMenuAttachment::ExecuteSelf %d\n", inMessage);	
	}
}
