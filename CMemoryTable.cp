// ===========================================================================
//	CMemoryTable.cp			   ©1997-1998 Metrowerks Inc. All rights reserved.
// ===========================================================================

#include <LTableView.h>
#include <LTableMonoGeometry.h>
#include <LTableMultiGeometry.h>
#include <LTableSingleSelector.h>
#include <LTableMultiSelector.h>
#include <LTableArrayStorage.h>

#include "utilities.h"
#include "CMemoryTable.h"

const int TABLE_WIDTH = 16;

CMemoryTable::CMemoryTable(
	LStream		*inStream)
		: LTableView(inStream)
{
	mCustomHilite = true;
	
	SetUseDragSelect(true);
	SetTableSelector(new LTableMultiSelector(this));
	SetTableGeometry(new LTableMonoGeometry(this, 18, 16));

}


CMemoryTable::~CMemoryTable()
{
}


void
CMemoryTable::DrawCell(
	const STableCell	&inCell,
	const Rect			&inLocalRect)
{
	TextFont(1);
	TextSize(9);
	TextFace(0);

	SInt32	number;
	UInt32	dataSize = sizeof(number);
	GetCellData(inCell, &number, dataSize);
		
	::MoveTo(inLocalRect.left + 4, inLocalRect.top + 14);
	if (number <  256) {
		unsigned char s[256];
		c2pstrcpy(s, (const char*)byteHexString(number));
		::DrawString(s);	
	}
}


// ---------------------------------------------------------------------------
//		¥ HiliteCellActively
// ---------------------------------------------------------------------------
//	Draw or undraw active hiliting for a Cell

void
CMemoryTable::HiliteCellActively(
	const STableCell	&inCell,
	Boolean				inHilite)
{
//	LTableView::HiliteCellActively(inCell, inHilite);
//	return;

	Rect	cellFrame;
    if (GetLocalCellRect(inCell, cellFrame) && FocusExposed()) {

		TextFont(1);
		TextSize(9);
		TextFace(0);
		
		SInt32	number;
		UInt32	dataSize = sizeof(number);
		GetCellData(inCell, &number, dataSize);
				
		::MoveTo(cellFrame.left + 4, cellFrame.bottom - 4);
		
		if (inHilite) {
			TextFace(bold);
		}
		
		::EraseRect(&cellFrame);
		if (number <  256) {
			unsigned char s[256];
			c2pstrcpy(s, (const char*)byteHexString(number));
			::DrawString(s);	
		}
	}
}


// ---------------------------------------------------------------------------
//		¥ HiliteCellInactively
// ---------------------------------------------------------------------------
//	Draw or undraw inactive hiliting for a Cell

void
CMemoryTable::HiliteCellInactively(
	const STableCell	&inCell,
	Boolean				inHilite)
{
//	LTableView::HiliteCellInactively(inCell, inHilite);
//	return;

	Rect	cellFrame;
	if (GetLocalCellRect(inCell, cellFrame) && FocusExposed()) {

		TextFont(1);
		TextSize(9);
		TextFace(0);

		SInt32	number;
		UInt32	dataSize = sizeof(number);
		GetCellData(inCell, &number, dataSize);
				
		::MoveTo(cellFrame.left + 4, cellFrame.bottom - 4);
		
		if (inHilite) {
			TextFace(italic);
		}
		
		::EraseRect(&cellFrame);
		if (number <  256) {
			unsigned char s[256];
			c2pstrcpy(s, (const char*)byteHexString(number));
			::DrawString(s);	
		}
	}
}



void CMemoryTable::GetCellData(
	const STableCell	&inCell,
	void				*outDataPtr,
	UInt32				&) const
{	
	SInt32 *out = (SInt32*) outDataPtr;
	
	long loc = (inCell.row-1) * TABLE_WIDTH + (inCell.col-1);
	if (loc > _dataSize)
		*out = 1000;
	else
		*out = *(_data + loc);		
}

Boolean CMemoryTable::SetPakItem(CPakStream *inItem)
{
	_dataSize = 0;
	_data = inItem->getData("memory table data");
	if (!_data)
		return false;
	
	_dataSize = inItem->getSize();
	dprintf("CMemoryTable::SetPakItem");
	
	long cols = TABLE_WIDTH;
	long rows = _dataSize/TABLE_WIDTH;
	
		// Add some rows and columns
	InsertRows(rows, 0);
	InsertCols(cols, 0);
	
	return true;
}
