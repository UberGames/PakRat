// ===========================================================================
// CEditItem.h				   ©1997-1998 Metrowerks Inc. All rights reserved.
// ===========================================================================
// Original Author: John C. Daub

#pragma once

#include <LEditableOutlineItem.h>
#include <LInPlaceEditField.h>
#include <LListener.h>
#include <LString.h>


class CEditItem : public LEditableOutlineItem,
				  public LListener {
public:
						CEditItem();
	virtual				~CEditItem();

	virtual	void		StartInPlaceEdit(const STableCell &inCell );

	virtual	void		ListenToMessage(
								MessageT		inMessage,
								void			*ioParam );
		
protected:
	virtual void		GetDrawContentsSelf(
								const STableCell&		inCell,
								SOutlineDrawContents&	ioDrawContents);

	LStr255				mText;
};

