// ===========================================================================
//	CMemoryTable.h			   ©1996-1998 Metrowerks Inc. All rights reserved.
// ===========================================================================

#ifndef _H_CMemoryTable
#define _H_CMemoryTable

#include "CPakStream.h"

#pragma once

#include <LTableView.h>

#if PP_Uses_Pragma_Import
	#pragma import on
#endif

class CPakStream;

class	CMemoryTable : public LTableView {
public:
	enum				{ class_ID = 'MemT' };
	
						CMemoryTable(
								LStream				*inStream);
	virtual				~CMemoryTable();
	Boolean SetPakItem(CPakStream *inItem);
	
	void GetCellData(
		const STableCell	&inCell,
		void				*outDataPtr,
		UInt32				&ioDataSize) const;
		
protected:
	char 				*_data;
	long				_dataSize;
	
	virtual void		DrawCell(
								const STableCell	&inCell,
								const Rect			&inLocalRect);
									
	virtual void		HiliteCellActively(
								const STableCell	&inCell,
								Boolean				inHilite);
	virtual void		HiliteCellInactively(
								const STableCell	&inCell,
								Boolean				inHilite);
};

#if PP_Uses_Pragma_Import
	#pragma import reset
#endif

#endif