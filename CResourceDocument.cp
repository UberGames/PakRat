

#include <LFile.h>
#include <LPlaceHolder.h>
#include <LPrintout.h>
#include <LString.h>
#include <LWindow.h>
#include <PP_Messages.h>
#include <UMemoryMgr.h>
#include <UResourceMgr.h>
#include <UWindows.h>

#include "CResourceDocument.h"
#include "COutlineTable.h"
#include "AppConstants.h"
#include "CFileArchive.h"
#include "CPak.h"
#include "CWad.h"
#include "CPk3.h"
#include "CFSArchive.h"
#include "CPakRatApp.h"
#include "CWindowMenu.h"
#include "CRecentFilesMenu.h"
#include "CTypeRegistry.h"
#include "CLiveResizeAttachment.h"

extern "C" {
	#include "MoreFiles.h"
	#include "MoreFilesExtras.h"
}


// ---------------------------------------------------------------------------------
//	¥ CTextDocument										[public]
// ---------------------------------------------------------------------------------
//	Constructor

CResourceDocument::CResourceDocument(
	LCommander*		inSuper,
	FSSpec*			inFileSpec,
	string extension)

	: LSingleDoc(inSuper)
{
		// Create window for our document.
	mWindow = LWindow::CreateWindow(PPob_ResourceWindow, this );
	ThrowIfNil_(mWindow);
	
	// gotta have that live window resizing 
	mWindow->AddAttachment(new CLiveResizeAttachment(mWindow));


	_resourceTable = dynamic_cast<COutlineTable*>(mWindow->FindPaneByID(kResourceTable));
	
	mFileDesignator = nil;
	_pak = nil;
	
	OpenFile(*inFileSpec, extension);		// Display contents of file in window.
	
	// position
	GDHandle dominantDevice = ::GetMainDevice();
	Rect screenRect = (**dominantDevice).gdRect;
	Rect windowBounds;
	mWindow->GetGlobalBounds(windowBounds);
	
	static UInt32 stagger = 0;
	int h = screenRect.right - (windowBounds.right - windowBounds.left + 8) - stagger;
	int v = ::GetMBarHeight() + 23 + stagger;
	
	// make window positioning wrap
	while (v + 40> (screenRect.bottom - screenRect.top)) {
		v = v - (screenRect.bottom - screenRect.top) + ::GetMBarHeight() + 23 + 40;
	}
	
	while (h < 0) {
		h = h + (screenRect.right - screenRect.left) - (windowBounds.right - windowBounds.left + 8);
	}
		
	mWindow->MoveWindowTo(h, v);
	stagger += 16;

	
	// Make the window visible.
	mWindow->Show();
}


// ---------------------------------------------------------------------------------
//	¥ ~CResourceDocument									[public, virtual]
// ---------------------------------------------------------------------------------
//	Destructor

CResourceDocument::~CResourceDocument()
{
	gWindowMenu->RemoveWindow( mWindow );
	
	dprintf("~CResourceDocument\n");
	try {
		TakeOffDuty();
		
		if ((mFile != nil) && (mFileDesignator != nil)) {
			mFile->CloseDataFork();
			mFile->CloseResourceFork();
			mFileDesignator->CompleteSave();
		}
	}
		
	catch (...) { }

	if (mFileDesignator)
		delete mFileDesignator;
	if (_pak)
		delete _pak;
}



// ---------------------------------------------------------------------------------
//	¥ OpenFile											[protected, virtual]
// ---------------------------------------------------------------------------------
//	Open the specified file

void
CResourceDocument::OpenFile(FSSpec&	inFileSpec, string extension)
{
	// create a file for the benefit of cDirectoryPopupAttachment
	mFile = new LFile(inFileSpec); 
	string docPath = fileSpecToPath(inFileSpec);
	gRecentFilesMenu->addRecentFile(docPath);
	dprintf("CResourceDocument::OpenFile: %s\n", docPath.c_str()); 
	mWindow->SetDescriptor(inFileSpec.name);
	resource_type_t resType = gRegistry->extensionToType(extension.c_str());
	
		// Create a new File object, read the entire File contents,
		//  put the contents into the text view, and set the Window
		//  title to the name of the File.
		// We don't close the file (until mFile is deleted) so we
		//  can prevent others from modifying the file.
		
	try {
	
		if (extension == "pak") {
			_pak = new CPak(inFileSpec);
			if (!_pak->init()) {
				delete _pak;
				_pak = nil;
				goto fail;
			}
		} else if (extension == "wad") {
			_pak = new CWad(inFileSpec);
			if (!_pak->init()) {
				delete _pak;
				_pak = nil;
				goto fail;
			}
		} else if (extension == "pk3") {
			_pak = new CPk3(inFileSpec);
			if (!_pak->init()) {
				delete _pak;
				_pak = nil;
				goto fail;
			}
		} else if (extension == "zip") {
			_pak = new CPk3(inFileSpec);
			if (!_pak->init()) {
				delete _pak;
				_pak = nil;
				goto fail;
			}
			
		// opening a File System archive for an item	
		} else if (gRegistry->isModelType(resType) || resType == skin_type || gRegistry->isMapType(resType)) {
		
			// choose a good root directory
			if (extension == "bsp")
				ancestorDirectoryWithName("maps", inFileSpec);
			else if (resType == md3_type || resType == mdr_type || resType == skin_type)
				ancestorDirectoryWithName("models", inFileSpec);
			
			// get path of parent directory
			long directory;
			FSSpec parentSpec;
			
			GetParentID(inFileSpec.vRefNum, 
				inFileSpec.parID, 
				inFileSpec.name,
				&directory);
			OSErr anErr = FSMakeFSSpec(inFileSpec.vRefNum, directory, "\p", &parentSpec);

			Str255 windowtitle;
			string windowname = fileSpecToPath(parentSpec);
			c2pstrcpy((unsigned char*)windowtitle, (const char*)windowname.c_str());

			mWindow->SetDescriptor(windowtitle);			
			_pak = new CFSArchive(inFileSpec);
			if (!_pak->init()) {
				delete _pak;
				_pak = nil;
				goto fail;
			}
		}
		if (_pak) {
			_pak->registerArchive();
			_resourceTable->SetPak(_pak);
			_pak->setDocument(this);
		}
		
		gWindowMenu->InsertWindow( mWindow );
		mIsSpecified = true;
		
	}
	
	catch (LException& inErr) {
		goto fail;
	}
	
	#if 0
			gApplication->OpenEditorForItem(_pak, string("models/monsters/bitch/tris.md2"));
			gApplication->OpenEditorForItem(_pak, string("models/players/anarki/lower.md3"));
			gApplication->OpenEditorForItem(_pak, string("progs/dog.mdl"));
	#endif
	
	return;
	
fail:
	
	ThrowOSErr_(0);
	return;
}



// ---------------------------------------------------------------------------
//	¥ FindCommandStatus									[public, virtual]
// ---------------------------------------------------------------------------
//	Override provided here for convenience.

void
CResourceDocument::FindCommandStatus(
	CommandT		inCommand,
	Boolean&		outEnabled,
	Boolean&		outUsesMark,
	UInt16&			outMark,
	Str255			outName)
{
	LSingleDoc::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);	
}


// ---------------------------------------------------------------------------
//	¥ ObeyCommand										[public, virtual]
// ---------------------------------------------------------------------------
//	Override provided here for convenience.

Boolean
CResourceDocument::ObeyCommand(
	CommandT		inCommand,
	void*			ioParam)
{
	Boolean	cmdHandled = LSingleDoc::ObeyCommand(inCommand, ioParam);

	return cmdHandled;
}

// ---------------------------------------------------------------------------
//	¥ UsesFileSpec													  [public]
// ---------------------------------------------------------------------------
//	Returns whether the Document's File has the given FSSpec

Boolean
CResourceDocument::UsesFileSpec(
	const FSSpec&	inFileSpec) const
{
	if (_pak)
		return _pak->UsesFileSpec(inFileSpec);
	return false;
}

