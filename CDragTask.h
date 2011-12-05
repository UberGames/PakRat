//	===============		Demonstration code for Drag and Drop was created by:
//	== CDragTask.h ==			gdignard@hookup.net (Gilles Dignard)
//	===============			©1994 Gilles Dignard. All Rights Reserved

#pragma once

#include <LDragTask.h>
#include "COutlineItem.h"

class CImage;
class COutlineTable;
class CPict;

//
// A structure which will contain sufficient
// data to build an item which looks and behaves
// just like this one.
//
// We'll define a constant for the FlavorType
// which will be associated with this structure
//
const UInt32 cImageType = 'img2';	// Usable as a FlavorType or an OSType
const UInt32 cPakType = 'pak2';	// Usable as a FlavorType or an OSType
const UInt32 cPakHFSType = 'Fpak';	// Usable as a FlavorType or an OSType

struct ItemData {
	void*		vPointerToSourceObject;
};


class CDragTask : public LDragTask
{
public:
					 CDragTask				(const EventRecord	&inEventRecord,
												 CImage* inImage, CPict *inPict = nil);
					CDragTask				(const EventRecord &inEventRecord, const STableCell &cell, COutlineTable *table, COutlineItem *item);
	virtual			~CDragTask				();
			
protected:
		
	virtual void			AddFlavors (DragReference	inDragRef);
	virtual void			MakeDragRegion (DragReference	inDragRef, RgnHandle inDragRegion);

private:	
		CPict			*_pict;
		CImage			*_image;
		COutlineItem 	*_item;
		COutlineTable 	*_table;
		STableCell		_cell;
};


pascal OSErr MyHandleDragSendData(
	FlavorType		inFlavor,
	void*			inRefCon,
	ItemReference	inItemRef,
	DragReference	inDragRef);
	
