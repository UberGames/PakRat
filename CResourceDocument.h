// =================================================================================
//	CResourceDocument.h					©1996-1999 Metrowerks Inc. All rights reserved.
// =================================================================================

#ifndef _H_CTextDocument
#define _H_CTextDocument
#pragma once

#include <LSingleDoc.h>
#include <UStandardDialogs.h>
#include <iostream.h>
#include <vector>
#include <string>

class COutlineTable;
class CFileArchive;
class CViewer;

using std::string;

class CResourceDocument : public LSingleDoc {

public:

	
							CResourceDocument(LCommander* inSuper, FSSpec* inFileSpec, string extension);

	virtual					~CResourceDocument();
	
	virtual Boolean			ObeyCommand(
								CommandT			inCommand,
								void*				ioParam = nil);
								
	virtual void			FindCommandStatus(
								CommandT			inCommand,
								Boolean&			outEnabled,
								Boolean&			outUsesMark,
								UInt16&				outMark,
								Str255				outName);					

	CFileArchive 			*getPak()  { return _pak; }
	virtual Boolean			UsesFileSpec(
								const FSSpec&	inFileSpec) const;
	

protected:

	virtual void			OpenFile(FSSpec& inFileSpec, string extension);
	
	PP_StandardDialogs::LFileDesignator*	mFileDesignator;	
	CFileArchive 				*_pak;
	COutlineTable				*_resourceTable;
	
private:

	CResourceDocument();
	CResourceDocument(const CResourceDocument& inOriginal);
	CResourceDocument&	operator=(const CResourceDocument& inRhs);
};

#endif // _H_CTextDocument