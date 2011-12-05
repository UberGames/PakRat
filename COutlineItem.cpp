/* 
	COutlineItem.cpp

	Author:			Tom Naughton
	Description:	<describe the COutlineItem class here>
*/

#include <LOutlineItem.h>
#include <LOutlineTable.h>
#include "COutlineItem.h"


COutlineItem::COutlineItem(CFileArchive *inPak, string name)
{
	_name = name;
	_pak = inPak;
}


COutlineItem::~COutlineItem()
{
}

void
COutlineItem::TrackDisclosureTriangle(
	const SMouseDownEvent& inMouseDown)
{
	StDeferTableAdjustment defer(mOutlineTable);
	LOutlineItem::TrackDisclosureTriangle(inMouseDown);
}

