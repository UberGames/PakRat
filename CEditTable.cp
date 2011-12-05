// ===========================================================================
// CEditTable.cp			  ©1997-1998 Metrowerks, Inc. All rights reserved.
// ===========================================================================
// Original Author: John C. Daub
//
// An outline table that demonstrates pane hosting and in-place editing

#include "CEditTable.h"

#include <LTablePaneHostingGeometry.h>
#include <LOutlineKeySelector.h>
#include <LOutlineRowSelector.h>
#include <UAttachments.h>

#include "CEditItem.h"

#include "OutlineTableConstants.h"



CEditTable::CEditTable(
	LStream *inStream )
		: LOutlineTable( inStream )
{
	SetTableGeometry( new LTablePaneHostingGeometry( this, kColWidth, 20 ) );
	
	// set the table selector
	
	SetTableSelector(new LOutlineRowSelector( this ) );
	
	// and note that we don't set the table storage.... 
	
	// most of the table classes not only maintain the graphical
	// representation of your data but also the data itself. But
	// LOutlineTable doesn't really do this...it mostly handles
	// graphical representation... you need to handle your data
	// maintenance elsewhere by yourself.
	
	// insert a couple columns (name and size)
	
	InsertCols( 1, 0, nil, nil, false );

	// Set up keyboard selection and scrolling.

	AddAttachment(new LOutlineKeySelector(this, msg_AnyMessage));
	AddAttachment(new LKeyScrollAttachment(this));

	// Try to become default commander in the window.

	if (mSuperCommander != nil) {
		mSuperCommander->SetLatentSub(this);
	}	
}



CEditTable::~CEditTable()
{
	// nothing
}


void
CEditTable::FinishCreateSelf()
{

	CEditItem *theItem;
	
	for ( SInt16 i = 1; i <= 5; ++i ) {
	
		theItem = new CEditItem;
		ThrowIfNil_(theItem);
		
		InsertItem( theItem, nil, nil );
	}
}