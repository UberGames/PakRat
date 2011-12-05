// ===========================================================================
// CEditTable.h				  ©1997-1998 Metrowerks, Inc. All rights reserved.
// ===========================================================================
// Original Author: John C. Daub
//
// An outline table that demonstrates pane hosting and in-place editing

#pragma once

#include <LOutlineTable.h>
#include <LCommander.h>


class CEditTable : public LOutlineTable,
				   public LCommander {
public:
	enum { class_ID = 'edTB' };
				
						CEditTable( LStream *inStream );
	virtual				~CEditTable();

protected:
	virtual void		FinishCreateSelf();

private:

			// defensive programming
			
						CEditTable();
						CEditTable( const CEditTable &inOriginal );
	CEditTable&			operator=( const CEditTable &inOriginal );

};