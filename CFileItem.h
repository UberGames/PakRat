// ===========================================================================
// CFileItem.h				   ©1997-1998 Metrowerks Inc. All rights reserved.
// ===========================================================================
// Original author: John C. Daub
//
// A concrete LOutlineItem for items of type "file"

#pragma once

#include <LString.h>
#include <iostream.h>

#include "COutlineItem.h"


class CFileItem : public COutlineItem {
public:
							CFileItem( CFileArchive *inPak, string name, long index );
	virtual					~CFileItem();
		
protected:
	virtual	void			DetermineFileSize();
		
	virtual void			GetDrawContentsSelf(
									const STableCell&		inCell,
									SOutlineDrawContents&	ioDrawContents);

	virtual void			DrawRowAdornments(
									const Rect&				inLocalRowRect);
		

	virtual void			DoubleClick(
									const STableCell&			inCell,
									const SMouseDownEvent&		inMouseDown,
									const SOutlineDrawContents&	inDrawContents,
									Boolean						inHitText);

				Handle			_IconH;
				LStr255			_sizeStr;
				long 			_index;

private:

			// defensive programming
			
							CFileItem();
							CFileItem( const CFileItem &inOriginal );
		CFileItem&			operator=( const CFileItem &inOriginal );
};