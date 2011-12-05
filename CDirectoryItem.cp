// ===========================================================================
// CDirectoryItem.cp		   ©1997-1998 Metrowerks Inc. All rights reserved.
// ===========================================================================
// Original author: John C. Daub
//
// A concrete LOutlineItem for items of type "directory".

#include <LOutlineItem.h>
#include <LOutlineTable.h>
#include <UGAColorRamp.h>


#include "CDirectoryItem.h"
#include "CFileItem.h"
#include "CFileArchive.h"
//#include "SendFinderOpen.h"
#include "OutlineTableConstants.h"
#include "GetFileIcon.h"


CDirectoryItem::CDirectoryItem( CFileArchive *inPak ) : COutlineItem(inPak, inPak->name())
{
	// Note that even tho we are storing data for an item within
	// the outline item, this doesn't provide any sort of persistant
	// storage of data within the table (see comments in COutlineTable's
	// ctor about mTableStorage).
	
	// The reason for this is how the outline table class deals with
	// expansion and collapsing of hierarchical nodes... when you expand
	// the item's ExpandSelf method is called. And as things are expanded
	// and such, items are created.. as collapsed, they are deleted. So
	// there is no persistance of the outline items and their stored data.
	
	// This is also why you might have a deep expansion of nested hierarchies
	// in the table, but you collapse a top-level parent, reexpand that and
	// the expand/nesting hierarchy is not maintained... everything was
	// deleted and has to be recreated.

		
	// try to get the folder's real icon. If it doesn't exist,
	// we'll use our own
	
	OSErr err = ::GetIconSuite( &_IconH, icon_Folder, kSelectorAllAvailableData );
	ThrowIfOSErr_(err);	
	ThrowIfResFail_(_IconH);
	
	::DetachResource(_IconH);

	// and roughly calculate the size of the file, for the second column of
	// the table.
	
	DetermineFileSize();
}


// dtor

CDirectoryItem::~CDirectoryItem()
{
	if ( _IconH != nil ) {
		::DisposeHandle(_IconH);
	}
}

// determine what it is that we're to draw in a given column

void
CDirectoryItem::GetDrawContentsSelf(
	const STableCell&		inCell,
	SOutlineDrawContents&	ioDrawContents)
{

	switch (inCell.col)
	{
		case 1:
		{
			ioDrawContents.outShowSelection = true;
			ioDrawContents.outHasIcon = true;
			ioDrawContents.outIconSuite = _IconH;
			ioDrawContents.outTextTraits.style |= bold;
			
			c2pstrcpy((unsigned char*)ioDrawContents.outTextString, (const char*)_pak->name().c_str());
			
			break;
		}
		
		case 2:
		{
			ioDrawContents.outShowSelection = true;
			ioDrawContents.outHasIcon = false;
			ioDrawContents.outTextTraits.style |= bold;
			
			LString::CopyPStr(_sizeStr, ioDrawContents.outTextString);
			
			break;
		}
	}
}


// by default, rows are not expanders.. but since this is a directory
// and we're showing hierarchy, of course we need to be able to expand.
// So we must make sure the table is aware of that fact.

Boolean
CDirectoryItem::CanExpand() const
{	
	return true;
}


// just to be cute, draw an adornment of a gray background

void
CDirectoryItem::DrawRowAdornments(
	const Rect&		inLocalRowRect )
{
	RGBColor inColor = UGAColorRamp::GetWhiteColor();
	Rect shadeRect = inLocalRowRect;
	shadeRect.bottom += 1;

	mOutlineTable->FocusDraw();
	::RGBForeColor(&inColor);
	::FrameRect(&shadeRect);
}


// this is the magic of what we do when someone tries to
// expand this item. We create outline items for all possible subitems.
// We do not need to destruct these (in CollapseSelf) as that is
// handled for us automatically.. see comments in LOutlineItem.cp for
// more information

void
CDirectoryItem::ExpandSelf()
{
	StDeferTableAdjustment defer(mOutlineTable);
	
	// FIXME - put up memory warning again
	if (LGrowZone::GetGrowZone()->MemoryIsLow()) {
		return;
	}

	// insert the root level of the Pak into the table
		
	LOutlineItem *theItem = nil;
	LOutlineItem *lastItem = nil;

	// add subpackages
	CFileArchive::database_type *database = _pak->getSubpackages();
	CFileArchive::iterator e = database->begin();
		
	while (e != database->end()) {
	
		if (e->second != nil) {
			theItem = new CDirectoryItem( e->second );
			ThrowIfNil_(theItem);
			
			mOutlineTable->InsertItem( theItem, this, lastItem );
			lastItem = theItem;
		}		
		e++;
	}	
	
	// add files
	CFileArchive::file_database_type *filesmap = _pak->getFiles();
	CFileArchive::file_iterator e1 = filesmap->begin();
	
	while (e1 != filesmap->end()) {
	
		theItem = new CFileItem( _pak, e1->first, e1->second );
		ThrowIfNil_(theItem);
		
		mOutlineTable->InsertItem( theItem, this, lastItem );
		lastItem = theItem;

	 	e1++;
	}
	
}

// send an open document AppleEvent to the Finder to open this
// given directory

void
CDirectoryItem::DoubleClick(
	const STableCell&			/* inCell */,
	const SMouseDownEvent&		 inMouseDown,
	const SOutlineDrawContents&	/* inDrawContents */,
	Boolean						/* inHitText */)
{
	StDeferTableAdjustment defer(mOutlineTable);
	Boolean wasExpanded = IsExpanded();
	// Change the expand status.
	
	if (wasExpanded)
		Collapse();
	else if (inMouseDown.macEvent.modifiers & optionKey)
		DeepExpand();
	else
		Expand();

	// Force a redraw.
	RefreshDisclosureTriangle();
	mOutlineTable->UpdatePort();
}

void
CDirectoryItem::DetermineFileSize()
{
	SInt32 fileSize = _pak->size(_name.c_str(), -1);
	
	
	if (fileSize == 0) {
		_sizeStr = "";
	} else if (fileSize < 1024) {
		_sizeStr = fileSize;
		_sizeStr += " bytes ";
	} else if (fileSize < 1024 * 1024) {
		_sizeStr = fileSize/1024 + 1;
		_sizeStr += " K ";
	} else {
		_sizeStr = fileSize/(1024 * 1024) + 1;
		_sizeStr += " MB ";
	}
}
