// ===========================================================================
// CDirectoryItem.h			   ©1997-1998 Metrowerks Inc. All rights reserved.
// ===========================================================================
// Original author: John C. Daub
//
// A concrete LOutlineItem for items of type "directory".

#pragma once

#include "COutlineItem.h"
#include <iostream.h>

class CFileArchive;

class CDirectoryItem : public COutlineItem
{

public:

								CDirectoryItem( CFileArchive *inPak );
		virtual					~CDirectoryItem();
		

		virtual Boolean			CanExpand() const;

protected:
		virtual	void			DetermineFileSize();

		virtual void			GetDrawContentsSelf(
									const STableCell&		inCell,
									SOutlineDrawContents&	ioDrawContents);

		virtual void			DrawRowAdornments(
									const Rect&				inLocalRowRect);
		
		virtual	void			ExpandSelf();


		virtual void			DoubleClick(
									const STableCell&			inCell,
									const SMouseDownEvent&		inMouseDown,
									const SOutlineDrawContents&	inDrawContents,
									Boolean						inHitText);

				LStr255			_sizeStr;
				Handle			_IconH;
				
private:

								CDirectoryItem();
								CDirectoryItem( const CDirectoryItem &inOriginal );
			CDirectoryItem&		operator=( const CDirectoryItem &inOriginal );
};