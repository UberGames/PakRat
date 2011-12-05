/* 
	COutlineItem.h

	Author:			Tom Naughton
	Description:	<describe the COutlineItem class here>
*/

#ifndef COutlineItem_H
#define COutlineItem_H

#include <iostream.h>
#include <LString.h>
#include <LOutlineItem.h>

class CFileArchive;
using std::string;

class COutlineItem: public LOutlineItem
{

public:
	COutlineItem(CFileArchive *inPak, string name);
	virtual ~COutlineItem();
	
	virtual void			TrackDisclosureTriangle(
									const SMouseDownEvent&		inMouseDown);

	CFileArchive 	*archive() { return _pak; }
	string 			name() { return _name; }

	CFileArchive 	*_pak;
	string 			_name;

};

#endif	// COutlineItem_H
