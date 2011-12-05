//	================		Demonstration code for Drag and Drop was created by:
//	== CDragTask.cp ==			gdignard@hookup.net (Gilles Dignard)
//	================			©1994 Gilles Dignard. All Rights Reserved

#include <Drag.h>

#include "CDragTask.h"
#include "CImage.h"
#include "CPict.h"
#include "COutlineItem.h"
#include "CPakStream.h"


//
// LDragTask gives us general drag functionality and behaviour.  CDragTask exists so
// that we can add behaviour specific to dragging cImageType items.
//
// The behaviour we're most interested in is being able to include a 'PICT' flavor
// in our drag. Using LDragTask allows you to include only one flavor in the drag.
// By subclassing LDragTask and overriding AddFlavors and MakeDragRegion, you can
// (a) include as many flavors of an item as you want and (b) define your own outline
// to replace the generic rectangle that LDragTask provides.
//


CDragTask::CDragTask (const EventRecord &inEventRecord, CImage* inImage, CPict *inPict)
	: LDragTask(inEventRecord)
{	// public
	_pict = inPict;
	_image = inImage;
	_item = 0;
	_table = 0;
}

CDragTask::CDragTask (const EventRecord &inEventRecord, const STableCell &cell, COutlineTable *table, COutlineItem *item)
	: LDragTask(inEventRecord)
{	// public
	_table = table;
	_item = item;
	_cell = cell;
	_image = 0;
}

CDragTask::~CDragTask ()
{	// public, virtual
}

void
CDragTask::AddFlavors (DragReference inDragRef)
{	

	
	//
	// Each drag can contain more than one item, and more than one representation
	// for each item. In this demo, we're only dragging one item at a time, but
	// we would like two representations (flavors) of that item. The first flavor will
	// be the actual data structure that we will need to build a new object just like
	// the current one. The second representation will be a 'PICT' representation so
	// that a clipping file can be kept and viewed at the Finder level.
	//
	// Since the possibility exists of including more than one item, we must specify
	// a unique item reference number for each of our item-specific transactions.
	//
	// The app doesn't support multiple item selections, so we know we only have one
	// item. That makes this part pretty simple.
	//
	ItemReference theItemRef = 1;
	
	if (_item) {
		OSErr err;
		ItemData theFlavorData;
		theFlavorData.vPointerToSourceObject = _item;
	
		err = ::SetDragSendProc( inDragRef,
						   NewDragSendDataUPP(MyHandleDragSendData),
						   _item);
						   
						   
			// add the promise flavor first
		PromiseHFSFlavor phfs;
		phfs.fileType = '????';
		phfs.fileCreator = 'PkRt';
		phfs.fdFlags = 0;
		phfs.promisedFlavor = cPakHFSType;
		err = AddDragItemFlavor(inDragRef, theItemRef, flavorTypePromiseHFS, &phfs, sizeof(phfs), flavorNotSaved); // kDragFlavorTypePromiseHFS
	//		/* add the HFS flavor immediately after the promise */
		err = AddDragItemFlavor(inDragRef, theItemRef, cPakHFSType, NULL, 0, flavorNotSaved);
	
		err = ::AddDragItemFlavor(inDragRef, theItemRef, cPakType,
							&theFlavorData, sizeof(theFlavorData), 0L);
	}
	
	if (_image) {
		//
		// Add the first data flavor (the data structure) to the drag
		//
		ItemData theFlavorData;
		theFlavorData.vPointerToSourceObject = _image;
		::AddDragItemFlavor(inDragRef, theItemRef, cImageType,
							&theFlavorData, sizeof(theFlavorData), 0L);
		
		//
		// Add the second data flavor (the PICT) to the drag
		//
		
		PicHandle thePicH;
		if (!_pict)
			thePicH = _image->CreatePict();
		else
			thePicH = _pict->CreatePict();
					
		::AddDragItemFlavor(inDragRef, theItemRef, 'PICT',
							*thePicH, ::GetHandleSize((Handle) thePicH), 0L);
		::KillPicture(thePicH);
	}
}


void
CDragTask::MakeDragRegion( DragReference inDragRef, RgnHandle inDragRegion)
{

	ItemReference theItemRef = 1;

	if (_item) {
		Rect r;
		
		RgnHandle	outerRgn = ::NewRgn();	// Carve out interior of region so
		
		
		_item->MakeDragRegion(_cell, outerRgn, r);
		
		RgnHandle	innerRgn = ::NewRgn();	// Carve out interior of region so
		::CopyRgn(outerRgn, innerRgn);		//   that it's just a one-pixel thick
		::InsetRgn(innerRgn, 1, 1);			//   outline of the item rectangle
		::DiffRgn(outerRgn, innerRgn, outerRgn);
		::DisposeRgn(innerRgn);
		
											// Accumulate this item in our
											//   total drag region
		::UnionRgn(outerRgn, inDragRegion, inDragRegion);
		
											// Tell Drag Manager about this item
#if PP_Target_Carbon
		Rect	theRect;
		::GetRegionBounds(outerRgn, &theRect);
		::SetDragItemBounds(inDragRef, theItemRef, &theRect);
#else
		::SetDragItemBounds(inDragRef, theItemRef, &(**outerRgn).rgnBBox);
#endif
		
		::DisposeRgn(outerRgn);
		
		return;	
	}
		

	//
	// Parts of this code are cribbed from LDragTask::AddRectDragItem.
	//
	// MD+DDK pages 2-21 through 2-22 also discusses this concept and give
	// another (similar) code example.
	//
	RgnHandle	outerRgn = ::NewRgn();	// Make region containing item
	//
	// First, create a region containing the 'mask' of our object. We'll
	// again use the CDDItem::DoQDInstructions routine to do the QD work for us.
	//
	::OpenRgn();
	Rect theGlobalRect;
	
	if (_image) {
		theGlobalRect = _image->bounds();
	}
	::LocalToGlobal(&topLeft(theGlobalRect));
	::LocalToGlobal(&botRight(theGlobalRect));
	
	// offset to mouse position
	Point mouseLoc;
	::GetMouse(&mouseLoc);
//	::LocalToGlobal(&mouseLoc);
	::OffsetRect(&theGlobalRect, 
		mouseLoc.h - (theGlobalRect.right - theGlobalRect.left)/2, 
		mouseLoc.v - (theGlobalRect.bottom - theGlobalRect.top)/2);
	
	
	::FrameRect(&theGlobalRect);
//	_image->DoQDInstructions(theGlobalRect);
	::CloseRgn(outerRgn);
	//
	// Take that 'mask', and turn it into an outline by making a copy,
	// shrinking it my one pixel, and subtracting the shrunken copy from
	// the original.
	//
	// That outline is then added (via the 'UnionRgn') to the outline of
	// the drag as a whole.
	//
	RgnHandle	innerRgn = ::NewRgn();	// Carve out interior of region so
	::CopyRgn(outerRgn, innerRgn);		//   that it's just a one-pixel thick
	::InsetRgn(innerRgn, 1, 1);			//   outline of the item rectangle
	::DiffRgn(outerRgn, innerRgn, outerRgn);
	::DisposeRgn(innerRgn);
	
										// Accumulate this item in our
										//   total drag region
	::UnionRgn(outerRgn, inDragRegion, inDragRegion);
	
										// Tell Drag Manager about this item
#if PP_Target_Carbon
	Rect	theRect;
	::GetRegionBounds(outerRgn, &theRect);
	::SetDragItemBounds(inDragRef, theItemRef, &theRect);
#else
	::SetDragItemBounds(inDragRef, theItemRef, &(**outerRgn).rgnBBox);
#endif
	
	::DisposeRgn(outerRgn);
}


pascal OSErr MyHandleDragSendData(
	FlavorType		inFlavor,
	void*			inRefCon,
	ItemReference	inItemRef,
	DragReference	inDragRef)
{
#pragma unused inFlavor
#pragma unused inItemRef
	SInt32	theA5 = ::SetCurrentA5();
	OSErr	err   = noErr;
	
	
	// get the file item
//	err = ::GetFlavorFlags(inDragRef, inItemRef, cPakType, &theFlags);
//	err = ::GetFlavorDataSize(inDragRef, inItemRef, cPakType, &theDataSize);
//	err = ::GetFlavorData(inDragRef, inItemRef, cPakType, &theFlavorData, &theDataSize, 0L);
	
	COutlineItem* inItem;
	inItem = (COutlineItem*)inRefCon;

	ItemReference	itemRef;
	StAEDescriptor	dropLocation;	// Get AE descriptor of drop location
	StAEDescriptor	dropSpecDesc;		// Coerce drop location into an FSSpec
	FSSpec theTarget;
	CInfoPBRec cat;
	long destinationDirID;
	Str255 targetName;
	FInfo  dstInfo;
	Boolean destCreated;
	FSSpec destinationDir;
	
	::GetDropLocation(inDragRef, dropLocation);
	::GetDragItemReferenceNumber(inDragRef, 1, &itemRef);
										
	if (::AECoerceDesc(dropLocation, typeFSS, dropSpecDesc) == noErr) {
#if PP_Target_Carbon
		FSSpec	dropSpec;
		err = ::AEGetDescData(dropSpecDesc, &dropSpec, sizeof(FSSpec));
		::BlockMoveData(&dropSpec, &destinationDir, sizeof(FSSpec));
#else
		FSSpec	*dropSpec = (FSSpec*) *dropSpecDesc.mDesc.dataHandle;
		::BlockMoveData(dropSpec, &destinationDir, sizeof(FSSpec));
#endif
		
			// establish the target directory 
		cat.hFileInfo.ioNamePtr = destinationDir.name;
		cat.hFileInfo.ioVRefNum = destinationDir.vRefNum;
		cat.hFileInfo.ioDirID = destinationDir.parID;
		cat.hFileInfo.ioFDirIndex = 0;
		err = ::PBGetCatInfoSync(&cat);
		if (err != noErr) goto bail;
		destinationDirID = cat.hFileInfo.ioDirID;
		
			// construct the target FSSpec, verify the name is free... 
		string path = inItem->archive()->pathName() + inItem->name();
		string filename = inItem->name();
			
		c2pstrcpy((unsigned char*)targetName, (const char*)filename.c_str());

		err = ::FSMakeFSSpec(destinationDir.vRefNum, destinationDirID, targetName, &theTarget);
		if (err == noErr) { err = dupFNErr; goto bail; }
		if (err != fnfErr) goto bail;
			
			// create the destination file.  Unless the promised file is created in
			// the drag send proc, the Finder will not position the icon correctly.  
		err = ::FSpCreate(&theTarget, 'PkRt', '????', smSystemScript);
		if (err != noErr) goto bail;
		destCreated = true;
		
			// set the destintation's flags 
		err = ::FSpGetFInfo(&theTarget, &dstInfo);
		if (err != noErr) goto bail;
		dstInfo.fdFlags = (dstInfo.fdFlags & (~kHasBeenInited));
		err = FSpSetFInfo(&theTarget, &dstInfo);
		if (err != noErr) goto bail;

			// copy the data 
		CPakStream *pakItem = inItem->archive()->itemWithPathName(path.c_str());
		err = pakItem->copyDataToFile(&theTarget);
		delete pakItem;
		if (err != noErr) goto bail;
			
			// return a reference to the new file to the caller 
		err = ::SetDragItemFlavorData(inDragRef, itemRef, cPakHFSType, &theTarget, sizeof(theTarget), 0);
		if (err != noErr) goto bail;
			
		return noErr;
	}
	

bail:
	if (destCreated) ::FSpDelete(&theTarget);
	return err;

	
	::SetA5(theA5);
	
	return err;
}
