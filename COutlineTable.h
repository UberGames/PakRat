// ===========================================================================
// COutlineTable.h			   ©1997-1998 Metrowerks Inc. All rights reserved.
// ===========================================================================
// Original Author: John C. Daub
//
//	Subclass and concrete instantiation of LOutlineTable

#pragma once

#include <LOutlineTable.h>
#include <LCommander.h>
#include <iostream.h>

class CFileArchive;

class COutlineTable : public LOutlineTable,
	public LDragAndDrop, 
	public LCommander 
{
public:
	enum { class_ID = 'Cout' };

						COutlineTable( LStream *inStream );
	virtual				~COutlineTable();

	virtual void 		SetPak(CFileArchive *inPak);

protected:
	virtual	void		FinishCreateSelf();

	virtual void		TrackDrag(
								const STableCell&			inCell,
								const SMouseDownEvent&		inMouseDown);
	virtual void		ReceiveDragItem(
								DragReference		inDragRef,
								DragAttributes		inDragAttrs,
								ItemReference		inItemRef,
								Rect&				inItemBounds);
								
	virtual void		DoDragSendData(
								FlavorType			inFlavor,
								ItemReference		inItemRef,
								DragReference		inDragRef);

private:
	
	CFileArchive	*_pak;
	virtual void 	PopulateFromPak(CFileArchive *pak);
	void 			CreateDragEvent(
						const STableCell&		inCell,
						const SMouseDownEvent&	inMouseDown);

				// defensive programming
				
						COutlineTable();
						COutlineTable( const COutlineTable &inOriginal );
	COutlineTable&		operator=( const COutlineTable &inOriginal );

	friend class LOutlineItem;

};