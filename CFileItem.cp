// ===========================================================================
// CFileItem.cp				   ©1997-1998 Metrowerks Inc. All rights reserved.
// ===========================================================================
// Original author: John C. Daub
//
// A concrete LOutlineItem for items of type "file"

#include <LOutlineItem.h>
#include <LOutlineTable.h>
#include <UGAColorRamp.h>


#include "CFileItem.h"
#include "OutlineTableConstants.h"
#include "GetFileIcon.h"
#include "CFileArchive.h"
#include "CFSArchive.h"
#include "CPakStream.h"
#include "CPakRatApp.h"
#include "CTypeRegistry.h"
#include "utilities.h"
#include "AppConstants.h"

// constructor

CFileItem::CFileItem( CFileArchive *inPak, string name, long index ) : COutlineItem(inPak, name)
{
	
	// Note that even tho we are storing data for an item within
	// the outline item, this doesn't provide any sort of persistant
	// storage of data within the table (see comments in COutlineTable's
	// ctor about mTableStorage).
	
	// The reason for this is how the outline table class deals with
	// expansion and collapsing of hierarchical nodes... when you expand
	// the item's ExpandSelf method is called (see CDirectoryItem for this
	// as file items of course can't be expanded). And as things are expanded
	// and such, items are created.. as collapsed, they are deleted. So
	// there is no persistance of the outline items and their stored data.
	
	// This is also why you might have a deep expansion of nested hierarchies
	// in the table, but you collapse a top-level parent, reexpand that and
	// the expand/nesting hierarchy is not maintained... everything was
	// deleted and has to be recreated.
	
	_index = index;
	
	OSErr err;
	string package, file, extension;
	decomposeEntryName(name,  package, file, extension);
	extension = lowerString(extension);
	resource_type_t resType = gRegistry->extensionToType(extension.c_str());
	ResIDT	icon = gRegistry->iconID(resType);
	// get the icon for this file
	err = ::GetIconSuite( &_IconH, icon, kSelectorAllAvailableData );
	

	ThrowIfOSErr_(err);	
	ThrowIfResFail_(_IconH);
	
	//::DetachResource(_IconH);
	
	// and roughly calculate the size of the file, for the second column of
	// the table.
	
	DetermineFileSize();
}


// dtor

CFileItem::~CFileItem()
{
	if ( _IconH != nil ) {
		::DisposeHandle(_IconH);
	}
}

// this is the routine called to know what to draw within the
// table cell. See the comments in LOutlineItem.cp for more info.

void
CFileItem::GetDrawContentsSelf(
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
			ioDrawContents.outTextTraits.style = 0;
						
			c2pstrcpy((unsigned char*)ioDrawContents.outTextString, (const char*)_name.c_str());
			
			break;
		}

		case 2:
		{
			ioDrawContents.outShowSelection = true;
			ioDrawContents.outHasIcon = false;
			ioDrawContents.outTextTraits.style = 0;
			
			LString::CopyPStr(_sizeStr, ioDrawContents.outTextString);
			
			break;
		}

	}
}

// just to be cute, we'll draw an adornment (again, see the LOutlineItem.cp
// comments for more information). We'll draw a groovy gray background

void
CFileItem::DrawRowAdornments(
	const Rect&		inLocalRowRect )
{
	RGBColor inColor = UGAColorRamp::GetWhiteColor();
	Rect shadeRect = inLocalRowRect;
	shadeRect.bottom += 1;

	mOutlineTable->FocusDraw();
	::RGBForeColor(&inColor);
	::FrameRect(&shadeRect);
}


// just to be cute, when a double-click occurs, we'll send the Finder
// and open document AppleEvent to open our selected item... just to do
// something.

void
CFileItem::DoubleClick(
	const STableCell&			/* inCell */,
	const SMouseDownEvent&		 inMouseDown ,
	const SOutlineDrawContents&	/* inDrawContents */,
	Boolean						/* inHitText */)
{
	Boolean commandDown = (inMouseDown.macEvent.modifiers & cmdKey) != 0;
	Boolean shiftDown = (inMouseDown.macEvent.modifiers & shiftKey) != 0;
	Boolean optionDown = (inMouseDown.macEvent.modifiers & optionKey) != 0;

	// what? do they want to crash?!
	// FIXME - put up memory warning again
	if (LGrowZone::GetGrowZone()->MemoryIsLow()) {
		return;
	}
	
	// see if it's an archive in a CFSArchive
	CFSArchive *theFSArchive = dynamic_cast<CFSArchive *> (_pak->rootArchive());
	if (theFSArchive) {
		string package, file, extension;
		decomposeEntryName(_pak->pathName() + _name,  package, file, extension);
		extension = lowerString(extension);

		if (extension == "pak" 
			|| extension == "wad" 
			|| extension == "zip" 
			|| extension == "pk3") {

			if (theFSArchive) {
				CFileArchive *archive = theFSArchive->packageWithName(package.c_str(), false);
				long pakIndex = archive->indexOfItemWithName(_name.c_str());
				
				FSSpec spec = theFSArchive->fsspec();
				spec.parID = pakIndex;
				c2pstrcpy((unsigned char*)spec.name, (const char*)_name.c_str());
				gApplication->OpenDocument(&spec);
				return;
			}
		}
	}

	string theItem = _pak->pathName() + _name;
	gApplication->OpenEditorForItem(_pak->rootArchive(), theItem.c_str(), "", inMouseDown.macEvent.modifiers);
}

// see how big the file actually is, and make a size string for printing
// in the second column

void
CFileItem::DetermineFileSize()
{
	SInt32 fileSize = _pak->size(_name.c_str(), _index);
	
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