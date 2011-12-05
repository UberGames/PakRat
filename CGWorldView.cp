// ===========================================================================
// CGWorldView.cp
// ===========================================================================
// CGWorldView - Base class only
// CGWorldViewTimed - Draws the GWorld in the Idle cycle, every mDrawInterval ticks
// CGWorldViewListener - Draws the GWorld in response to a message

#include <LStream.h>
#include <UDrawingState.h>


#include "CGWorldView.h"
#include "utilities.h"
#include "CPict.h"
#include "CDragTask.h" 

// ---------------------------------------------------------------------------
// Default Constructor
// ---------------------------------------------------------------------------
CGWorldView::CGWorldView()
#if PP_Target_Carbon
	: LView(),  LDragAndDrop(UQDGlobals::GetCurrentWindowPort(), this)
#else
	: LView(),  LDragAndDrop(UQDGlobals::GetCurrentPort(), this)
#endif
{
	_image = nil;
	mGWorld = nil;
};

// ---------------------------------------------------------------------------
// Stream Constructor
// ---------------------------------------------------------------------------
CGWorldView::CGWorldView(LStream *inStream)
#if PP_Target_Carbon
	: LView(inStream),  LDragAndDrop(UQDGlobals::GetCurrentWindowPort(), this)
#else
	: LView(inStream),  LDragAndDrop(UQDGlobals::GetCurrentPort(), this)
#endif
{
	_image = nil;
	mGWorld = nil;
};

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
CGWorldView::~CGWorldView(void)
{
	if (mGWorld)
		delete mGWorld;
	if (_image)
		delete _image;
};

void CGWorldView::SetImage(CImage *inImage)
{
	if (_image)
		delete _image;
	_image = inImage;
	SetMyGWorld(_image->CreateGWorld());
}

// ---------------------------------------------------------------------------
// SetMyGWorld
// ---------------------------------------------------------------------------
void
CGWorldView::SetMyGWorld(LGWorld *inGW)
{
	if (inGW) {
		Rect bounds, size;
		inGW->GetBounds(bounds);
		size.left = 10;
		size.right = bounds.right - bounds.left;
		size.top = 10;
		size.bottom = bounds.bottom - bounds.top;
		ResizeImageTo(size.right, size.bottom, true);
#warning broken for Carbon
#if PP_Target_Carbon
	//	LWindow *myWindow = LWindow::FetchWindowObject(::FrontWindow());
		LWindow *myWindow = LWindow::FetchWindowObject(UQDGlobals::GetCurrentWindowPort());
#else
		GrafPtr myPort = GetMacPort();
		LWindow *myWindow = LWindow::FetchWindowObject(myPort);
#endif
		SDimension16 dims;
		dims.width = size.right + 14;
		dims.height = size.bottom + 14;
		myWindow->ResizeWindowTo(dims.width, dims.height);
		myWindow->SetStandardSize(dims);
	}
	mGWorld = inGW;
}

// ---------------------------------------------------------------------------
// DrawSelf
// ---------------------------------------------------------------------------
void
CGWorldView::DrawSelf(void)
{
	if(!mGWorld) return;
	Rect theRect;
	mGWorld->GetBounds(theRect);
	FocusExposed(true);
	
	SDimension16 outSize;
	GetFrameSize(outSize);
	scaleRectPreservingAspectRation(theRect, outSize.width, outSize.height);

	PixMapHandle myPixMap;
	myPixMap = ::GetGWorldPixMap(mGWorld->GetMacGWorld());
	::LockPixels(myPixMap);
	mGWorld->CopyImage(GetMacPort(), theRect, srcCopy);
	::FrameRect(&theRect);
	::UnlockPixels(myPixMap);
}

#pragma mark -

// ---------------------------------------------------------------------------
// Default Constructor
// ---------------------------------------------------------------------------
CGWorldViewTimed::CGWorldViewTimed()
	: CGWorldView(), LPeriodical()
{
	mLastDraw = 0;
	mDrawInterval = 60; // in Ticks
};

// ---------------------------------------------------------------------------
// Stream Constructor
// ---------------------------------------------------------------------------
CGWorldViewTimed::CGWorldViewTimed(LStream *inStream)
	: CGWorldView(inStream)
{
	UInt8	redrawSecs;
	inStream->ReadData(&redrawSecs, sizeof(UInt8));
	mLastDraw = 0;
	mDrawInterval = redrawSecs*60;
};


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
CGWorldViewTimed::~CGWorldViewTimed(void)
{
};

// ---------------------------------------------------------------------------
// SetMyGWorld
// ---------------------------------------------------------------------------
void
CGWorldViewTimed::SetMyGWorld(LGWorld *inGW)
{
	CGWorldView::SetMyGWorld(inGW);	// call inherited
	
	if (!inGW) {
		StopIdling();
	} else {
		StartIdling();
	}
}

// ---------------------------------------------------------------------------
// SpendTime
// ---------------------------------------------------------------------------
void
CGWorldViewTimed::SpendTime(const EventRecord	&/*inMacEvent*/)
{
	dprintf("CGWorldViewTimed::SpendTime");
	UInt32 theTick = ::TickCount();
	if (theTick > mLastDraw + mDrawInterval) {
		Refresh();
		mLastDraw = theTick;
	}
}



#pragma mark -
// ---------------------------------------------------------------------------
// Default Constructor
// ---------------------------------------------------------------------------
CGWorldViewListener::CGWorldViewListener()
	: CGWorldView(), LListener()
{
};

// ---------------------------------------------------------------------------
// Stream Constructor
// ---------------------------------------------------------------------------
CGWorldViewListener::CGWorldViewListener(LStream *inStream)
	: CGWorldView(inStream)
{
};


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
CGWorldViewListener::~CGWorldViewListener(void)
{
};

// ---------------------------------------------------------------------------
// ListenToMessage
// ---------------------------------------------------------------------------
void
CGWorldViewListener::ListenToMessage(
								MessageT		inMessage,
								void			*)
{
	if (inMessage == msg_JustChanged)
		Refresh();
}

#pragma mark -
void CGWorldView::PasteItem()
{	// private
	LClipboard* theClipboard = LClipboard::GetClipboard();
	SignalIf_(theClipboard == nil);
	
	Handle theDataH = ::NewHandle(0);
	theClipboard->GetData(cImageType, theDataH);
	
//	CDDItem* thePastedItem = new CDDItem(this, *(ItemData*) *theDataH);
//	ThrowIf_(thePastedItem == nil);
	
//	SwitchTarget(thePastedItem);
}


void CGWorldView::Click(SMouseDownEvent& inMouseDown)
{
	inMouseDown.delaySelect = false;
	LView::Click(inMouseDown);
	inMouseDown.delaySelect = true;
}


void
CGWorldView::ClickSelf(const SMouseDownEvent& inMouseDown )
{	// protected, virtual

	//SwitchTarget(this);		// Make this item active
	//
	// Track item long enough to distinguish between a click to
	// select, and the beginning of a drag
	//
	Boolean isDrag = ::WaitMouseMoved(inMouseDown.macEvent.where);
	
	// no dragging in low memory situations
	if (LGrowZone::GetGrowZone()->MemoryIsLow())
		return;

	if (isDrag) {
		//
		// If we leave the window, the drag manager will be changing thePort,
		// so we'll make sure thePort remains properly set.
		//
		mSuperView->FocusDraw();
		ApplyForeAndBackColors();
		CreateDragEvent(inMouseDown);
		mSuperView->OutOfFocus(nil);
	}
}

void CGWorldView::HiliteDropArea(DragReference inDragRef)
{
	mPane->ApplyForeAndBackColors();

	Rect	dropRect;
	mPane->CalcLocalFrameRect(dropRect);
	::MacInsetRect(&dropRect, -1, -1);
	StRegion	dropRgn(dropRect);
	
	::ShowDragHilite(inDragRef, dropRgn, true);
	//LDropArea::HiliteDropArea(inDragRef);
}

void CGWorldView::CreateDragEvent(const SMouseDownEvent &inMouseDown)
{	// private
	Rect theRect = _image->bounds();
	CalcLocalFrameRect(theRect);
	//
	// Build a structure to contain the data we'll be passing to the drag event.
	//
	ItemData theFlavorData;
	theFlavorData.vPointerToSourceObject = _image;
	//
	// Begin the drag task
	//
	
	//
	// Create a new drag task
	//
	CDragTask theDragTask (inMouseDown.macEvent, _image);
	theDragTask.DoDrag();

	//if (theDragTask.DropLocationIsFinderTrash()) {
	//	DeleteSelf();
	//}
}


Boolean
CGWorldView::ItemIsAcceptable(DragReference inDragRef, ItemReference inItemRef)
//
//	ItemIsAcceptable will be called whenever the Drag Manager wants to know if
//	the item the user is currently dragging contains any information that we
//	can accept.
//
//	In our case, the only thing we'll accept are cImageType items.
//
{
#pragma unused (inDragRef, inItemRef)
//	FlavorFlags		theFlags;
//	if (GetFlavorFlags(inDragRef, inItemRef, 'PICT', &theFlags) == noErr)
//		return true;
	return false;
}


void
CGWorldView::EnterDropArea(DragReference inDragRef, Boolean inDragHasLeftSender)
//
// The cursor has just entered our area.
//
{	// protected, virtual
	// Let LDragAndDrop do its thing (hilight the area)
	LDragAndDrop::EnterDropArea(inDragRef, inDragHasLeftSender);
	
	// And we'll do ours.
	//PlaySound(gPlayEnterSound, rsrc_EnterSound);
}


void
CGWorldView::LeaveDropArea (DragReference inDragRef)
//
// The cursor has just left the building. I repeat, the cursor has left the building.
//
{	// protected, virtual
	// Let LDragAndDrop do its thing (removes the hilighting)
	LDragAndDrop::LeaveDropArea (inDragRef);
	
	// And we'll do ours.
	//PlaySound(gPlayExitSound, rsrc_ExitSound);
}

		
void
CGWorldView::InsideDropArea (DragReference inDragRef)
//
// The cursor is still in our area.
//
{	// protected, virtual
	
//	dprintf("CGWorldView::InsideDropArea\n");
	//
	// Let LDragAndDrop do its thing - this is not really necessary, since
	//		the inherited version doesn't do anything. But it's safer this
	//		way because someday it might.
	//
	LDragAndDrop::InsideDropArea(inDragRef);
	
	//
	// And we'll do ours - we'll just read the mouse coordinates, but for this
	// demo we won't do anything with them.
	//
	// The mouse location is where the mouse actually is on the screen. The
	// alternative is the pinned location, which is _usually_ the same location,
	// but can be different if the cursor is being constrained by a tracking handler.
	// This is useful when you want an area within a view to be 'off-limits' to
	// the ongoing drag.
	//
	// If we did want to do something based on where the cursor currently is in
	// our area (such as indicating an insertion point or something), it would
	// usually be best to use the pinned location for that work.
	//
	// Both mouse locations are returned in global screen coordinates
	//
	Point	theMouseLocation;
	Point	thePinnedLocation;
	::GetDragMouse(inDragRef, &theMouseLocation, &thePinnedLocation);
}


void
CGWorldView::ReceiveDragItem(
	DragReference	inDragRef,
	DragAttributes	/* inDragAttrs */,
	ItemReference	inItemRef,
	Rect			&inItemBounds)	// In Local coordinates
//
// The user has dropped something in this view.
//
{
#pragma unused (inItemBounds)
	// FIXME - put up memory warning
	if (LGrowZone::GetGrowZone()->MemoryIsLow())
		return;

#warning removed for Carbon		
#if PP_Target_Carbon
#else
	::InvalRect(&inItemBounds);
#endif
	
	
	//
	// Information about the drag contents we'll be needing.
	//
	
	FlavorFlags		theFlags;		// We actually only use the flags to see if a flavor exists
	Size			theDataSize;	// How much data there is for us.
	OSErr			err;

	err = ::GetFlavorFlags(inDragRef, inItemRef, 'PICT', &theFlags);
	if (err == noErr) {
		err = ::GetFlavorDataSize(inDragRef, inItemRef, 'PICT', &theDataSize);
		if (err == noErr) {
			if (theDataSize) {

				PicHandle thePicH = (PicHandle) ::NewHandle(theDataSize);
				err = ::GetFlavorData(inDragRef, inItemRef, 'PICT', *thePicH, &theDataSize, 0L);
				CPict* inImage = new CPict(thePicH);
				SetMyGWorld(inImage->CreateGWorld());
			}
		}
	}
}


Boolean
CGWorldView::CheckForOptionKey(DragReference inDragRef)
{	// private
	//
	// We'll check whether the option key was down at either the beginning _or_ the
	// end of the drag, since (a) it's the preferred behaviour and (b) its so easy to do.
	//
	SInt16 theModifiersNow;			// The state of the modifier keys right now
	SInt16 theModifiersAtMouseDown;	// The state of the modifier keys when the drag began
	SInt16 theModifiersAtMouseUp;	// The state of the modifier keys when the drag ended
	::GetDragModifiers(inDragRef, &theModifiersNow, &theModifiersAtMouseDown, &theModifiersAtMouseUp);
	
	return ((theModifiersAtMouseDown & optionKey) || (theModifiersAtMouseUp & optionKey));
}


Boolean
CGWorldView::CheckIfViewIsAlsoSender(DragReference inDragRef)
{	// private
	//
	// Just a note: While we are using the drag attributes only at the end of the
	// drag, they are also available to you during the drag.
	//
	// Drag Attributes are described in MD+DDK, page 2-31.
	//
	DragAttributes theDragAttributes;
	::GetDragAttributes(inDragRef, &theDragAttributes);
	
	return (theDragAttributes & kDragInsideSenderWindow);
}
