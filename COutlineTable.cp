// ===========================================================================
// COutlineTable.cp			   ©1997-1998 Metrowerks Inc. All rights reserved.
// ===========================================================================
// Original Author: John C. Daub
//
//	Subclass and concrete instantiation of LOutlineTable

#include "COutlineTable.h"

#include <LTableMultiGeometry.h>
#include <LOutlineKeySelector.h>
#include <LOutlineRowSelector.h>
#include <LOutlineItem.h>
#include <UAttachments.h>

#include "CDirectoryItem.h"
#include "CFileItem.h"
#include "CFileArchive.h"
#include "CDragTask.h"

#include "OutlineTableConstants.h"

extern SInt16 gVolIndex;	// defined in COutlineTableApp.cp


// Constructor

COutlineTable::COutlineTable(
	LStream *inStream )
#if PP_Target_Carbon
		: LOutlineTable( inStream ), LDragAndDrop(UQDGlobals::GetCurrentWindowPort(), this)
#else
		: LOutlineTable( inStream ), LDragAndDrop(UQDGlobals::GetCurrentPort(), this)
#endif
{
	// set the table geometry
	
	SetTableGeometry(new LTableMultiGeometry(this, kColWidth, 20));
	
	// set the table selector
	
	SetTableSelector(new LOutlineRowSelector( this ) );
	
	// and note that we don't set the table storage....
	
	// most of the table classes not only maintain the graphical
	// representation of your data but also the data itself. But
	// LOutlineTable doesn't really do this...it mostly handles
	// graphical representation... you need to handle your data
	// maintenance elsewhere by yourself.
	
	// insert a couple columns (name and size)
	
	InsertCols( 2, 0, nil, nil, false );
	SetColWidth(70, 2, 2);

	// Set up keyboard selection and scrolling.

	AddAttachment(new LOutlineKeySelector(this, msg_AnyMessage));
	AddAttachment(new LKeyScrollAttachment(this));

	// Try to become default commander in the window.

	if (mSuperCommander != nil) {
		mSuperCommander->SetLatentSub(this);
	}
	
}

// dtor

COutlineTable::~COutlineTable()
{
}

void COutlineTable::SetPak(CFileArchive *inPak)
{
 	_pak = inPak;
 	PopulateFromPak(_pak);
}

void COutlineTable::PopulateFromPak(CFileArchive *pak)
{

	// insert the root level of the Pak into the table
		
	LOutlineItem *theItem = nil;
	LOutlineItem *lastItem = nil;

	// add subpackages
	CFileArchive::database_type *database = pak->getSubpackages();
	CFileArchive::iterator e = database->begin();
		
	while (e != database->end()) {
	
		theItem = new CDirectoryItem( e->second );
		ThrowIfNil_(theItem);
		
		InsertItem( theItem, nil, lastItem );
		lastItem = theItem;
		e++;
	}	
	
	// add files
	CFileArchive::file_database_type *filesmap = pak->getFiles();
	CFileArchive::file_iterator e1 = filesmap->begin();
	
	while (e1 != filesmap->end()) {
	
		theItem = new CFileItem( pak, e1->first, e1->second );
		ThrowIfNil_(theItem);
		
		InsertItem( theItem, nil, lastItem );
		lastItem = theItem;
	 	e1++;
	}
	
}

void COutlineTable::FinishCreateSelf()
{
}

void COutlineTable::TrackDrag(
	const STableCell&		inCell,
	const SMouseDownEvent&	inMouseDown)
{
	// Track item long enough to distinguish between a click to
	// select, and the beginning of a drag
	//
	Boolean isDrag = ::WaitMouseMoved(inMouseDown.macEvent.where);
	
	// no dragging in low memory situations
//	if (LGrowZone::GetGrowZone()->MemoryIsLow())
//		return;

	if (isDrag) {
		//
		// If we leave the window, the drag manager will be changing thePort,
		// so we'll make sure thePort remains properly set.
		//
		mSuperView->FocusDraw();
		ApplyForeAndBackColors();
		CreateDragEvent(inCell, inMouseDown);
		mSuperView->OutOfFocus(nil);
	}
}

void COutlineTable::CreateDragEvent(
	const STableCell&		inCell,
	const SMouseDownEvent&	inMouseDown)
{	// private
	
	COutlineItem* outlineItem = (COutlineItem*)FindItemForRow(inCell.row);
	
	// can't drag directories, yet
	CFileItem *item = dynamic_cast<CFileItem *> (outlineItem);	
	
	if (item) {
	
		// Build a structure to contain the data we'll be passing to the drag event.
		//
		ItemData theFlavorData;
		theFlavorData.vPointerToSourceObject = nil;
		//
		// Begin the drag task
		//
		
		//
		// Create a new drag task
		//
		CDragTask theDragTask (inMouseDown.macEvent, inCell, this, item);
		
		theDragTask.DoDrag();

		if (theDragTask.DropLocationIsFinderTrash()) {
			// delete			
		}
	}
}


// ---------------------------------------------------------------------------
//	¥ ReceiveDragItem												  [public]
// ---------------------------------------------------------------------------
//	Process an Item which has been dragged into a DropArea
//
//	This function gets called once for each Item contained in a completed
//	Drag. The Item will have returned true from ItemIsAcceptable().
//
//	The DropArea is focused upon entry and inItemBounds is specified
//	in the local coordinates of the DropArea.
//
//	Override this function if the DropArea can accept dropped Items.
//	You may want to call GetFlavorData and GetFlavorDataSize if there
//	is information associated with the dragged Item.

void
COutlineTable::ReceiveDragItem(
	DragReference	/* inDragRef */,
	DragAttributes	/* inDragAttrs */,
	ItemReference	/* inItemRef */,
	Rect&			/* inItemBounds */)	// In Local coordinates
{
	dprintf("COutlineTable::ReceiveDragItem\n");
}


// ---------------------------------------------------------------------------
//	¥ DoDragSendData												  [public]
// ---------------------------------------------------------------------------
//	Send the data associated with a particular drag item
//
//	This function gets called if you installed the optional DragSendDataProc
//	for this DropArea. In which case you should override this function
//	to provide the requested data by calling SetDragItemFlavorData.

void
COutlineTable::DoDragSendData(
	FlavorType		/* inFlavor */,
	ItemReference	/* inItemRef */,
	DragReference	/* inDragRef */)
{
	dprintf("COutlineTable::DoDragSendData\n");
}
