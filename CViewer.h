/* 
	CViewer.h

	Author:			Tom Naughton
	Description:	<describe the CViewer class here>
*/

#ifndef CViewer_H
#define CViewer_H

#include <list>
#include <UQuickTime.h>
#include <LCommander.h>
#include <LListener.h>
#include <LWindow.h>

#include "CPakStream.h"

class C3DModelPane;
class CModelInstance;
class CMd3;
class CImage;
class CWav;
class CFileArchive;
using std::list;

class CViewer : public LCommander,
				public LListener,
				public LModelObject 
{
public:
	CViewer(LCommander* inSuper);
	virtual ~CViewer();
	
	Boolean 			SetupViewer(CFileArchive *pak, const char *path, const char *itemname = "", int modifiers = 0);
	Boolean 			LoadPakResource(CFileArchive *pak, const char *path, const char *itemname = "", int modifiers = 0);
	void 				selectItemNamed(const char *itemname);

	void	 			DisposePakResource();
	void 				Reload();
	StringPtr			itemTitle();
	string				pathName() { return _path.c_str(); }		
	LWindow 			*window() { return _window; }
	CModelInstance		*model();	
	C3DModelPane		*modelPane();	
	void 				focusGLPane();
	
	// find model to skin
	static CViewer		*viewerForTypeWithPath(const char *type, const char *path);
	static CViewer		*viewerWithPath(const char *inPath);

	// LListener
	virtual void		ListenToMessage(
							MessageT		inMessage,
							void*			ioParam);


	// LCommander 
	virtual void		FindCommandStatus(
								CommandT			inCommand,
								Boolean&			outEnabled,
								Boolean&			outUsesMark,
								UInt16&				outMark,
								Str255				outName);

	virtual Boolean		ObeyCommand(
								CommandT			inCommand,
								void*				ioParam = nil);
						
	virtual void		PutOnDuty(
								LCommander*			inNewTarget);

	virtual Boolean		AllowSubRemoval(
								LCommander*		inSub);
								
	virtual void		Close();

protected:

	OSType 		GetFileType() const;
	Boolean 	AskSaveAs();
	void		DoSave(FSSpec&  outFSSpec);
	Movie 		MakeSoundMovie(Ptr waveDataPtr, long waveDataSize, const char *inType);

	LWindow 			*_window;
	string 				_path;
	string 				_file;
	string 				_extension;
	string 				_itemname;
	CFileArchive 		*_pak;
	
	typedef list<CViewer*> viewerList;
	typedef viewerList::iterator viewerList_iterator;

	static viewerList _viewerList;

};


class	CMovieController : public LMovieController
{
public:
						CMovieController(
								const SPaneInfo &inPaneInfo,
								CWav *wav);
								
						~CMovieController(); // NON VIRTUAL!

private:

	CWav *_wav;
	
};


#endif	// CViewer_H
