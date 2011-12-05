/* 
	CViewer.cpp

	Author:			Tom Naughton
	Description:	<describe the CViewer class here>
*/

#include <LWindow.h> 
#include <UStandardDialogs.h>
#include <LScrollerView.h>
#include <LPushButton.h>
#include <LPopupButton.h>
#include <LWindowEventHandlers.h>

#include "CPakRatApp.h"
#include "CPreferences.h"
#include "CViewer.h"
#include "AppConstants.h"
#include "CMemoryTable.h"
#include "C3DShaderPane.h"
#include "CMdl.h"
#include "CMd2.h"
#include "CMd3.h"
#include "CBsp.h"
#include "CWav.h"
#include "CResourceDocument.h"
#include "C3DModelPane.h"
#include "CModelInstance.h"
#include "C3DMapPane.h"
#include "CPakStream.h"
#include "CMd3Controller.h"
#include "CHLMdlController.h"
#include "CWindowMenu.h"
#include "CShader.h"
#include "CShaderMenu.h"
#include "CResourceManager.h"
#include "CTypeRegistry.h"
#include "utilities.h"
#include "WETabs.h"
#include "WTextView.h"
#include "WTextModel.h"
#include "CLiveResizeAttachment.h"

#define kStandardTranslucencyThreshold	0x7FFFFFFF	//	infinity

CViewer::viewerList CViewer::_viewerList;


CViewer::CViewer(LCommander* inSuper) : LCommander(inSuper)
{
	_viewerList.push_front(this);
}


CViewer::~CViewer()
{
	dprintf("~CViewer\n");
	gWindowMenu->RemoveWindow( _window );
	_viewerList.remove(this);
}

// ---------------------------------------------------------------------------
// ListenToMessage
// ---------------------------------------------------------------------------
void
CViewer::ListenToMessage(
								MessageT		inMessage,
								void			*)
{
#warning this code is duplicated in ObeyCommand

	switch(inMessage) {
		case cmd_reload:
			dprintf("CViewer::ListenToMessage cmd_reload\n");
			LPushButton *compileButton = dynamic_cast<LPushButton *> (_window->FindPaneByID( kCompileButton ));
			
			// get the text from the editor
			WTextView *textView = dynamic_cast<WTextView *> (_window->FindPaneByID( kTextEditView )); // kTextEditView
			ThrowIfNil_(textView);
			
			if (_extension == "shader" || _extension == "low")
				CResourceManager::updateShaders((char*)*textView->GetText(), textView->GetTextLength());
			else if (_extension == "skin")
				gApplication->updateSkin(_path.c_str(), (char*)*textView->GetText(), textView->GetTextLength());
			else if (_file == "animation"  && _extension == "cfg" )
				CResourceManager::updateAnimationInfo(_path.c_str(), (char*)*textView->GetText(), textView->GetTextLength());
			break;
			
		case cmd_chooseShader:
			dprintf("CViewer::ListenToMessage cmd_chooseShader\n");
 			break;
			
		default:
			dprintf("CViewer::ListenToMessage %d\n", inMessage);
	}	
}

// ---------------------------------------------------------------------------
//	¥ FindCommandStatus									[public, virtual]
// ---------------------------------------------------------------------------
//	Override provided here for convenience.

void
CViewer::FindCommandStatus(
	CommandT		inCommand,
	Boolean&		outEnabled,
	Boolean&		outUsesMark,
	UInt16&			outMark,
	Str255			outName)
{
	switch (inCommand) {
		resource_type_t resType = gRegistry->extensionToType(_extension.c_str());
	
		case cmd_reload:
			outEnabled = (gRegistry->isModelType(resType) || gRegistry->isScriptType(resType));
			break;

		case cmd_Close:
		case cmd_SaveAs:
		case cmd_PageSetup:
		case cmd_Print:
		case cmd_PrintOne:
			outEnabled = true;
			break;
			
		case cmd_Save:
			outEnabled = true;
			break;
						
		default:
			LCommander::FindCommandStatus(inCommand, outEnabled,
									outUsesMark, outMark, outName);
			break;
	}
}

// ---------------------------------------------------------------------------
//	¥ ObeyCommand									[public, virtual]
// ---------------------------------------------------------------------------
//	Respond to Commands. Returns true if the Command was handled, false if not.


Boolean
CViewer::ObeyCommand(
	CommandT	inCommand,
	void*		ioParam)
{
	Boolean		cmdHandled = true;	// Assume we'll handle the command
	resource_type_t resType = gRegistry->extensionToType(_extension.c_str());

	dprintf("CViewer::ObeyCommand inCommand %d\n", inCommand);
	
	switch (inCommand) {
	
		case cmd_SaveAs:
		case cmd_Save:
			AskSaveAs();
			break;
		case cmd_reload:
			if (gRegistry->isModelType(resType)) {
				C3DModelPane *modelPane = dynamic_cast<C3DModelPane *> (_window->FindPaneByID( kModelView ));
				ThrowIfNil_(modelPane);
				
				// setup correct GL context for texture uploading
				modelPane->FocusDraw();
				Reload();
			} else if (gRegistry->isScriptType(resType)) {
				LPushButton *compileButton = dynamic_cast<LPushButton *> (_window->FindPaneByID( kCompileButton ));
				compileButton->SimulateHotSpotClick(true);
				// get the text from the editor
				WTextView *textView = dynamic_cast<WTextView *> (_window->FindPaneByID( kTextEditView )); // kTextEditView
				ThrowIfNil_(textView);
				
				CResourceManager::updateShaders((char*)*textView->GetText(), textView->GetTextLength());
				compileButton->SimulateHotSpotClick(false);
			}
			break;

		default: {
			Boolean	cmdHandled = LCommander::ObeyCommand(inCommand, ioParam);
			break;
		}
	}
	
	return cmdHandled;
}



// ---------------------------------------------------------------------------
//	¥ PutOnDuty
// ---------------------------------------------------------------------------
//	A Commander is going on duty.
//
//	inNewTarget is the Commander that is becoming the Target, which is
//	this Commander or one of its SubCommanders.
//
//	Subclasses should override this function if they wish to behave
//	differently when on duty than when off duty

void
CViewer::PutOnDuty(
	LCommander*	 inNewTarget)
{
	_viewerList.remove(this);
	_viewerList.push_front(this);

	dprintf("CViewer::PutOnDuty %s\n", pathName().c_str());
	LCommander::PutOnDuty(inNewTarget);
}


// ---------------------------------------------------------------------------
//	¥ AllowSubRemoval												  [public]
// ---------------------------------------------------------------------------

Boolean
CViewer::AllowSubRemoval(
	LCommander*		inSub)
{
	if (inSub == _window) {
	
		// Check if the current AppleEvent is a "close" event
		// sent to the Window. If so, we handle it as if the
		// "close" event were sent to the Document
		 
		
		Close();			// A non-AppleEvent close
		return false;

	} else {
		return true;
	}
}


// ---------------------------------------------------------------------------
//	¥ Close															  [public]
// ---------------------------------------------------------------------------
//	Close a Document

void
CViewer::Close()
{
	if ((mSuperCommander == nil) || mSuperCommander->AllowSubRemoval(this)) {
		delete this;
	}
}

void CViewer::Reload()
{
	LoadPakResource(_pak, _path.c_str(), _itemname.c_str());
	dprintf("reload\n");
}

StringPtr CViewer::itemTitle()
{
	static char _pTitle[1024];	
	c2pstrcpy((unsigned char*)_pTitle, (const char*)_path.c_str());
	return (StringPtr) _pTitle;
}

Boolean CViewer::SetupViewer(CFileArchive *pak, const char *path, const char *itemname, int modifiers)
{
	Boolean commandDown = (modifiers & cmdKey) != 0;
	Boolean shiftDown = (modifiers & shiftKey) != 0;
	Boolean optionDown = (modifiers & optionKey) != 0;

	_path = lowerString(fixSlashes(path)); 
	_pak = pak;
	_itemname = itemname;
	string package, file, extension;
	decomposeEntryName(_path,  package, file, extension);
	_extension = extension;
	_file = file;
	resource_type_t resType = gRegistry->extensionToType(_extension.c_str());
	
	if (gRegistry->isScriptType(resType)) {
		
		_window = LWindow::CreateWindow(PPob_CompileWindow, this ); // PPob_TextWindow
		ThrowIfNil_(_window);
		LPushButton *compileButton = dynamic_cast<LPushButton *> (_window->FindPaneByID( kCompileButton ));
		ThrowIfNil_(compileButton);
		compileButton->AddListener(this);
		WTextView *textView = dynamic_cast<WTextView *> (_window->FindPaneByID( kTextEditView )); // kTextEditView
		ThrowIfNil_(textView);
		CShaderMenu *shaderPopup = dynamic_cast<CShaderMenu *> (_window->FindPaneByID( kShaderPopup ));
		ThrowIfNil_(shaderPopup);
		shaderPopup->AddListener(this);
		shaderPopup->SetTextView(textView);
		_window->SetDescriptor(itemTitle());
		
	} else if (gRegistry->isTextType(resType)) {
		
		_window = LWindow::CreateWindow(PPob_TextWindow, this ); // PPob_TextWindow
		ThrowIfNil_(_window);
		_window->SetDescriptor(itemTitle());

	} else if (gRegistry->isImageType(resType)) {
	
		_window = LWindow::CreateWindow(PPob_ShaderWindow, this );
		ThrowIfNil_(_window);
		_window->SetDescriptor(itemTitle());

	} else if (gRegistry->isMapType(resType)) {
	
		_window = LWindow::CreateWindow(PPob_MapWindow, this );
		ThrowIfNil_(_window);
		C3DMapPane *mapPane = dynamic_cast<C3DMapPane *> (_window->FindPaneByID( kMapView ));
		ThrowIfNil_(mapPane);
		_window->SetDescriptor(itemTitle());
		_window->Show();

	} else if (gRegistry->isModelType(resType)) {
	
		_window = LWindow::CreateWindow(PPob_ModelWindow, this );
		ThrowIfNil_(_window);
		C3DModelPane *modelPane = dynamic_cast<C3DModelPane *> (_window->FindPaneByID( kModelView ));
		ThrowIfNil_(modelPane);
		_window->SetDescriptor(itemTitle());
		_window->Show();
		modelPane->FocusDraw();
		modelPane->Draw(nil);
		
		if (resType == md3_type || resType == mdr_type) {
			modelPane->SetRotationCorrection(-90.0f, 0.0f, -90.0f);
			modelPane->SetPosition(0.04f, -0.36f, -3.09f);
		} else {
			modelPane->SetRotationCorrection(-90.0f, 0.0f, -90.0f);
			modelPane->SetPosition(0.04f, -0.19f, -3.09f);
		}

	} else if (gRegistry->isSoundType(resType)) {

		_window = LWindow::CreateWindow(PPob_SoundWindow, this );
		ThrowIfNil_(_window);
		_window->SetDescriptor(itemTitle());
		

	} else {

		// memory editor
		_window = LWindow::CreateWindow(PPob_MemoryWindow, this );
		ThrowIfNil_(_window);
		_window->SetDescriptor(itemTitle());		
		CMemoryTable *memoryTable = dynamic_cast<CMemoryTable *> (_window->FindPaneByID( kMemoryTable ));
		ThrowIfNil_(memoryTable);
		_window->Show();
		
		// FIXME failure?
	}
	
	// gotta have that live window resizing 
	_window->AddAttachment(new CLiveResizeAttachment(_window));
	
	gWindowMenu->InsertWindow( _window );
	Boolean loaded = LoadPakResource(_pak, _path.c_str(), _itemname.c_str(), modifiers);
	return loaded;
}

Boolean CViewer::LoadPakResource(CFileArchive *pak, const char *path, const char *itemname, int modifiers)
{
	CPakStream *inItem = nil;
	resource_type_t resType = gRegistry->extensionToType(_extension.c_str());
	
	if (pak) {
		inItem = pak->itemWithPathName(path);
	} else {
		FSSpec spec;
		pathToFSSpec(path, spec); 
		inItem = new CPakStream(spec);
	}
	
	if (!inItem)
		goto fail;

	// if loading a resource caused a low memory situation...
	if (LGrowZone::GetGrowZone()->MemoryIsLow()) {
		dprintf("Memory is too low\n");
		goto fail;
	}

	if (gRegistry->isTextType(resType)) {

		WTextView *textView = dynamic_cast<WTextView *> (_window->FindPaneByID( kTextEditView )); // kTextEditView
		ThrowIfNil_(textView);
		

		// convert line feeds
		long size = inItem->getSize();
		char *textData = (char*) inItem->getData("text resource");
		char *p = textData;
		
		if (p) {
			for(int i = 0; i < size; i++, p++) {
				if (*p == '\n' && *(p + 1) == '\r') {
					*p =  ' ';
				} else if (*p == '\r' && *(p + 1) == '\n') {
					*p =  ' ';
				} else if (*p == '\n') {
					*p = '\r';
				}
			}
			
			textView->InsertText(textData, size);
			textView->Refresh();
			textView->SetTextSelection(0, 0);
			
			WESetTabSize(20, textView->GetWEHandle());
			textView->ClearUndo();
			
			
			// translucent dragging
			SInt32 translucencyThreshold = 0 ;
			WEReference we = textView->GetWEHandle();
			if ( WEGetInfo ( weTranslucencyThreshold, & translucencyThreshold, we ) == noErr )
			{
				translucencyThreshold = kStandardTranslucencyThreshold - translucencyThreshold ;
				WESetInfo ( weTranslucencyThreshold, & translucencyThreshold, we ) ;
			}
			
			/* -- this doesn't work yet
			//create the text model
			LModelObject *theModel = dynamic_cast<LModelObject *>(this);
			WTextModel	*theAppleScriptTextModel = new WTextModel(theModel, textView);
			textView->SetTextModel(theAppleScriptTextModel);
			
			LMultiUndoer *undoer = new LMultiUndoer(20);
			textView->AddAttachment(undoer);
			*/
			
			_window->SetLatentSub(textView);
			_window->Show();

			CShaderMenu *shaderPopup = dynamic_cast<CShaderMenu *> (_window->FindPaneByID( kShaderPopup ));
			if (shaderPopup && _extension != "shader" && _extension != "low") {
				delete shaderPopup;
			}
			
			CMemoryTracker::safeFree(textData);			
		} else {
			goto fail;
		}


	} else if (gRegistry->isImageType(resType)) {
		
		C3DShaderPane *shaderView = dynamic_cast<C3DShaderPane *> (_window->FindPaneByID( kShaderView ));
		ThrowIfNil_(shaderView);

	 	CResourceManager *resources = new CResourceManager(shaderView, _pak);
	 	resources->initShaders(0, 200, (shaderref_t*) 0);
		shaderView->SetResourceManager(resources);		
		shaderView->SetShader(inItem, (const char*)_path.c_str(), modifiers);
		_window->Show();
		
	} else if (gRegistry->isMapType(resType)) {
	
		string shotName = "levelshots/" + _file + ".jpg";
		C3DMapPane *mapPane = dynamic_cast<C3DMapPane *> (_window->FindPaneByID( kMapView ));
		ThrowIfNil_(mapPane);
	 	CResourceManager *resources = new CResourceManager(mapPane, _pak);
		mapPane->FocusDraw();
		mapPane->SetResourceManager(resources);
		mapPane->SetLevelShot(resources->textureWithName(shotName.c_str()));
		mapPane->Draw(nil);

 		// have to load the model after the gl context is created 
		// so it can upload its textures
		CBsp *map = new CBsp(resources);
		if(map->init(inItem, mapPane->renderingAttributes())) {
			mapPane->SetMap(map);
		} else {
			delete map;
			goto fail;
		}
		
		_window->SetLatentSub(mapPane);
	
	} else if (gRegistry->isModelType(resType)) {

		Boolean assemble = gApplication->preferences()->booleanForKey("assembleModels");
		C3DModelPane *modelPane = dynamic_cast<C3DModelPane *> (_window->FindPaneByID( kModelView ));
		ThrowIfNil_(modelPane);

		modelPane->FocusDraw();
		
	 	CResourceManager *resources = new CResourceManager(modelPane, _pak);
	 	resources->initShaders(0, 200, (shaderref_t*) 0);
		CModelInstance *instance = resources->modelInstanceWithClassName(path);

			

		if (instance) {
				
		 	resources->parseShaders();
			modelPane->SetModel(instance);
			modelPane->SetResourceManager(resources);
			if (resType == md3_type || resType == mdr_type) {
				CMd3Controller *controller = new CMd3Controller(instance);
				modelPane->SetModelController(controller);
			} else if (resType == hl_mdl_type) {
				CHLMdlController *controller = new CHLMdlController(instance);
				modelPane->SetModelController(controller);
            } else {
				CModelController *controller = new CModelController(instance);
				modelPane->SetModelController(controller);
			}

		} else {
			delete resources;
			goto fail;
		}
		_window->SetLatentSub(modelPane);

	} else if (gRegistry->isSoundType(resType)) { 

		
		SPaneInfo pinfo;
		pinfo.paneID = 0;
		pinfo.width = 280;
		pinfo.height = 40;
		pinfo.visible = true;
		pinfo.enabled = true;
		pinfo.bindings.left = true;
		pinfo.bindings.top = true;
		pinfo.bindings.right = true;
		pinfo.bindings.bottom = true;
		pinfo.left = 0;
		pinfo.top = 0;
		pinfo.userCon = 0;
		pinfo.superView = _window;
		
		CWav *wav = new CWav(inItem);
		wav->play();
		CMovieController *movieController = new CMovieController(pinfo, wav);  
		movieController->FinishCreate();	
		movieController->PutInside(_window);
		_window->Show();

		// FIXME failure?

	} else {

		CMemoryTable *memoryTable = dynamic_cast<CMemoryTable *> (_window->FindPaneByID( kMemoryTable ));
		ThrowIfNil_(memoryTable);
		if (!memoryTable->SetPakItem(inItem))
			goto fail;
		_window->Show();
	}
	
	delete inItem;
	
	selectItemNamed(itemname);

	return true;
	
fail:
	dprintf("CViewer::LoadPakResource failed!\n");
	if (inItem)
		delete inItem;
	return false;
}

void CViewer::selectItemNamed(const char *itemname)
{
	resource_type_t resType = gRegistry->extensionToType(_extension.c_str());
	_itemname = itemname;
	
	// select the correct shader
	if (gRegistry->isShaderType(resType)) {
		CShaderMenu *shaderPopup = dynamic_cast<CShaderMenu *> (_window->FindPaneByID( kShaderPopup ));
		ThrowIfNil_(shaderPopup);
		 shaderPopup->Refresh();
		 if (_itemname.length() > 0) {
		 	shaderPopup->SetSelectedShader(_itemname.c_str()) ;
		 }
	}
}


CViewer	*CViewer::viewerWithPath(const char *inPath)
{
	// check all viewers
	viewerList_iterator e = _viewerList.begin();
	while (e != _viewerList.end()) {
	 	CViewer *viewer = *e;
	 	dprintf("%s == %s\n", viewer->pathName(), inPath);
	 	if (viewer->pathName() == inPath) {
	 		dprintf("found viewer!\n");
	 		return viewer;
	 	}
	 	e++;
	}
	return nil;
}


CViewer	*CViewer::viewerForTypeWithPath(const char *type, const char *inPath)
{
	CViewer *bestviewer = nil;
	
	// check all viewers
	viewerList_iterator e = _viewerList.begin();
	while (e != _viewerList.end()) {
	 	CViewer *viewer = *e;
	 	string viewerPath = viewer->pathName();
	 	if (fileExtension(viewerPath) == type && directoriesMatch(viewerPath, inPath)) {
	 		if (!bestviewer || viewer->IsOnDuty())
	 			bestviewer = viewer;
	 	}
	 	e++;
	}
	return bestviewer;
}

CModelInstance *CViewer::model()
{
	C3DModelPane *modelPane = dynamic_cast<C3DModelPane *> (_window->FindPaneByID( kModelView ));
	if (modelPane)
		return modelPane->model();
	return 0;
}

C3DModelPane *CViewer::modelPane()
{
	C3DModelPane *modelPane = dynamic_cast<C3DModelPane *> (_window->FindPaneByID( kModelView ));
	return modelPane;
}

// call this to setup the current GL context before uploading textures and such
void CViewer::focusGLPane()
{
	C3DModelPane *modelPane = dynamic_cast<C3DModelPane *> (_window->FindPaneByID( kModelView ));
	if (modelPane) {
		modelPane->FocusDraw();
	}

	C3DMapPane *mapPane = dynamic_cast<C3DMapPane *> (_window->FindPaneByID( kMapView ));
	if (mapPane) {
		mapPane->FocusDraw();
	}
}



// ---------------------------------------------------------------------------
//	¥ GetFileType										[public, virtual]
// ---------------------------------------------------------------------------
//	Return the type (four character code) of the file used for saving
//	the Document. Subclasses should override if they support saving files.

OSType
CViewer::GetFileType() const
{
	return ResType_Text;
}

// ---------------------------------------------------------------------------
//	¥ AskSaveAs														  [public]
// ---------------------------------------------------------------------------
//	Ask the user to save a Document and give it a name
//
//	Returns false if the user cancels the operation

Boolean
CViewer::AskSaveAs()
{
/*
	Boolean		saveOK = false;
	FSSpec 		outFSSpec;

	PP_StandardDialogs::LFileDesignator*	designator =
								new PP_StandardDialogs::LFileDesignator;
								
	designator->SetFileType( GetFileType() );
	
	Str255	defaultName;
	strcpy((char*)defaultName, inItem->name().c_str());
	c2pstr((char*)defaultName);
	
	if (designator->AskDesignateFile(defaultName)) {
		
		designator->GetFileSpec(outFSSpec);
		

		if (designator->IsReplacing()) {
			ThrowIfOSErr_(::FSpDelete(&outFSSpec));
		}
		
								// Save data to new file. This also
								//    closes the original file
		DoSave(outFSSpec);
		saveOK = true;
	}
	
	return saveOK;
*/
	return false;
}


// ---------------------------------------------------------------------------------
//	¥ DoSave											[public, virtual]
// ---------------------------------------------------------------------------------
//	Save the entire Document to its associated File, which must already exist

void
CViewer::DoSave( FSSpec&  outFSSpec)
{
#pragma unused (outFSSpec)

/*
	LFile *file = new LFile(outFSSpec);
	file->CreateNewDataFile('????', '????', smSystemScript);
	file->OpenDataFork(fsWrPerm);
	file->WriteDataFork(CMemoryTracker::safeAlloc(1, inItem->getSize(), "fixme", false), inItem->getSize());
	file->CloseDataFork();
*/
}



// ---------------------------------------------------------------------------
//	¥ CMovieController						Constructor				  [public]
// ---------------------------------------------------------------------------

CMovieController::CMovieController(
	const SPaneInfo	&inPaneInfo,
	CWav *wav)
	
	: LMovieController(inPaneInfo, wav->getMovie())
{
	_wav = wav;
}


// ---------------------------------------------------------------------------
//	¥ ~CMovieController						Destructor				  [public]
// ---------------------------------------------------------------------------

CMovieController::~CMovieController()
{
	_wav->stop();
	::DisposeMovieController(mMovieController);
	delete _wav;
}
