// ===========================================================================
// CEditItem.cp				   ©1997-1998 Metrowerks Inc. All rights reserved.
// ===========================================================================
// Original Author: John C. Daub

#include "CEditItem.h"
#include <LInPlaceEditField.h>


CEditItem::CEditItem()
{
	LString::CopyPStr( "\pfoo", mText );
}

CEditItem::~CEditItem()
{
	// nothing
}


void
CEditItem::GetDrawContentsSelf(
	const STableCell&		inCell,
	SOutlineDrawContents&	ioDrawContents)
{

	switch (inCell.col)
	{
		case 1:
		{
			ioDrawContents.outShowSelection = true;
			ioDrawContents.outTextTraits.style = 0;
			ioDrawContents.outCanDoInPlaceEdit = true;
			
			LString::CopyPStr( mText, ioDrawContents.outTextString);
			
			break;
		}
	}
}


void
CEditItem::StartInPlaceEdit(
	const STableCell&	inCell)
{
	LEditableOutlineItem::StartInPlaceEdit( inCell );

	// we need to listen for the edit field to stop editing
	
	mEditField = GetEditField();
	ThrowIfNil_(mEditField);
	
	mEditField->AddListener(this);
	mEditField->SetValueMessage(100);
}

void
CEditItem::ListenToMessage(
	MessageT	inMessage,
	void 		*ioParam )
{
#pragma unused(ioParam)

	switch ( inMessage )
	{
		case 100:
		{
			mEditField->GetDescriptor( mText );
			if ( mText[0] == 0 ) {
				mText = "\p-";
			}
			break;
		}
	}
}
