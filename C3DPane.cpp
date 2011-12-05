/* 
	C3DPane.cpp

	Author:			Tom Naughton
	Description:	<describe the C3DPane class here>
*/

#include <stdarg.h>
#include <agl.h>
#include <gl.h>
#include <glu.h>
#include <glut.h>
#include <drag.h>

#include <UGWorld.h>
#include <UStandardDialogs.h>

#include "C3DPane.h"
#include "CPakRatApp.h"
#include "CDragTask.h"
#include "CPreferences.h"
#include "CFileArchive.h"
#include "CResourceManager.h"
#include "CGLImage.h"
#include "CPict.h"
#include "CModelInstance.h"
#include "CTextures.h"
#include "CTypeRegistry.h"


//	¥ C3DPane						Parameterized Constructor [public]
// ---------------------------------------------------------------------------
// desc is a string of pixel format attributes

C3DPane::C3DPane( const SPaneInfo&	inPaneInfo, LStr255& desc) 
#if PP_Target_Carbon
	: GLPane(inPaneInfo, desc),  LDragAndDrop(UQDGlobals::GetCurrentWindowPort(), this)
#else
	: GLPane(inPaneInfo, desc),  LDragAndDrop(UQDGlobals::GetCurrentPort(), this)
#endif
{
	_frameCount = 0;
	_fps = 0;
	_frameCountTime = 0.0;
	_drawInterval = 0;
	_resources = nil;
	_model = nil;
	
	_rendering._rotCorrectionX = 0.0f;
	_rendering._rotCorrectionY = 0.0f;
	_rendering._rotCorrectionZ = 0.0f;
	
	_rendering._rotAngleX  = 0.0;
	_rendering._rotAngleY = 0.0;
	_rendering._rotAngleZ = 0.0;
}


// ---------------------------------------------------------------------------
//	¥ C3DPane						Stream Constructor		  [public]
// ---------------------------------------------------------------------------

C3DPane::C3DPane( LStream*	inStream) 
#if PP_Target_Carbon
	: GLPane(inStream),  LDragAndDrop(UQDGlobals::GetCurrentWindowPort(), this)
#else
	: GLPane(inStream),  LDragAndDrop(UQDGlobals::GetCurrentPort(), this)
#endif
{
	_frameCount = 0;
	_fps = 0;
	_frameCountTime = 0.0;
	_drawInterval = 0;
	_resources = nil;
	_model = nil;
	
	_rendering._rotCorrectionX = 0.0f;
	_rendering._rotCorrectionY = 0.0f;
	_rendering._rotCorrectionZ = 0.0f;
	
	_rendering._rotAngleX  = 0.0;
	_rendering._rotAngleY = 0.0;
	_rendering._rotAngleZ = 0.0;
}

#if PP_Target_Carbon
C3DPane::C3DPane() : GLPane(),  LDragAndDrop(UQDGlobals::GetCurrentWindowPort(), this)
#else
C3DPane::C3DPane() : GLPane(),  LDragAndDrop(UQDGlobals::GetCurrentPort(), this)
#endif
{
	_frameCount = 0;
	_fps = 0;
	_frameCountTime = 0.0;
	_drawInterval = 0;
	_resources = nil;
	_model = nil;
}


C3DPane::~C3DPane()
{
	StopIdling();
	
	// delete textures from the correct context
	FocusDraw();
	//aglSetCurrentContext(mCtx);
	if (_resources)
		delete _resources;
	dprintf("~C3DPane\n");
}


void C3DPane::SetupRendering()
{
	_rendering._textureMap = nil;
	_rendering._lighting = false;
	_rendering._lightmap = true;
	_rendering._textured = true;
	_rendering._wireframe = false;

	_rendering._showBoneFrameBox = false;
	_rendering._showNormals = false;	
	_rendering._pickShader = false;
	_rendering._pickTag = false;
	_rendering._showMissingTextures = gApplication->preferences()->booleanForKey("showMissingTextures");

	_rendering._renderOpaque = true;
	_rendering._renderBlend = true;
	
	SDimension16 l_PortDim;
	GetFrameSize(l_PortDim);
	ReshapeGL(l_PortDim.width, l_PortDim.height);

	resetRendering();
}

  /**
   * <p>Reset all user manipulations (scaling + panning + rotating) of this model canvas.
   */
void C3DPane::resetRendering() 
{

	_rendering._showBoneFrameBox = false;
	_rendering._showNormals = false;	
	_rendering._lighting = false;	
	_rendering._textured = true;	
	_rendering._wireframe = false;	

	_rendering._pickShader = false;
	_rendering._pickTag = false;
	_rendering._showMissingTextures = gApplication->preferences()->booleanForKey("showMissingTextures");

	SDimension16 l_PortDim;
	GetFrameSize(l_PortDim);
	ReshapeGL(l_PortDim.width, l_PortDim.height);


}

void C3DPane::ListenToMessage(MessageT	 inMessage, void* ioParam)
{
#pragma unused (inMessage, ioParam)
}


// ---------------------------------------------------------------------------
// SpendTime
// ---------------------------------------------------------------------------
void
C3DPane::SpendTime(const EventRecord	&inMacEvent)
{
	if (IsOnDuty()) {
		Boolean commandDown = (inMacEvent.modifiers & cmdKey) != 0;
		Boolean optionDown = (inMacEvent.modifiers & optionKey) != 0;
		_rendering._pickTag = optionDown && !commandDown;
		_rendering._pickTextureMap = optionDown && commandDown;
		_rendering._pickShader = commandDown;
		
		if (optionDown || commandDown) {
			FocusDraw();
			Point theMouseLocation;
			::GetMouse(&theMouseLocation);
			PickModel(theMouseLocation.h, theMouseLocation.v);
		}
	} else {
		_rendering._pickTag = false;
		_rendering._pickTextureMap = false;
		_rendering._pickShader = false;
	}
	MoviesTask(nil, 50);
}


void C3DPane::SetResourceManager(CResourceManager *inResources)
{
	if (_resources)
		delete _resources;
	_resources = inResources;
}

#pragma mark -

void C3DPane::EnterDropArea(DragReference inDragRef, Boolean inDragHasLeftSender)
//
// The cursor has just entered our area.
//
{	// protected, virtual
	dprintf("C3DPane::EnterDropArea\n");
	// Let LDragAndDrop do its thing (hilight the area)
	LDragAndDrop::EnterDropArea(inDragRef, inDragHasLeftSender);
	
}


void C3DPane::LeaveDropArea (DragReference inDragRef)
//
// The cursor has just left the building. I repeat, the cursor has left the building.
//
{	// protected, virtual
	// Let LDragAndDrop do its thing (removes the hilighting)
	dprintf("C3DPane::LeaveDropArea\n");
	LDragAndDrop::LeaveDropArea (inDragRef);
	Redraw();
#warning removed for Carbon
//	InvalPortRgn(GetMacPort()->visRgn);
	
	_rendering._pickShader = false;
	_rendering._pickTag = false;
	Redraw();	
}

		
Boolean C3DPane::ItemIsAcceptable(DragReference inDragRef, ItemReference inItemRef)
//
//	ItemIsAcceptable will be called whenever the Drag Manager wants to know if
//	the item the user is currently dragging contains any information that we
//	can accept.
//
//
{		
	OSErr err;
	DragAttributes attributes;
	err = ::GetDragAttributes(inDragRef, &attributes);
	if (err == noErr && attributes & kDragInsideSenderWindow) {
		return true;
	}

	FlavorFlags		theFlags;
	if (::GetFlavorFlags(inDragRef, inItemRef, cImageType, &theFlags) == noErr) {
		_rendering._pickShader = true;
		return true;
	}
	
	if (::GetFlavorFlags(inDragRef, inItemRef, 'PICT', &theFlags) == noErr) {
		_rendering._pickShader = true;
		return true;
	}
	
	if (::GetFlavorFlags(inDragRef, inItemRef, cPakType , &theFlags) == noErr) {
		
		Size			theDataSize;	// How much data there is for us.
		ItemData		theFlavorData;	// Where we will put that data.
		OSErr			err;
		
		err = ::GetFlavorDataSize(inDragRef, inItemRef, cPakType, &theDataSize);
		if (!err && theDataSize) {
			::GetFlavorData(inDragRef, inItemRef, cPakType, &theFlavorData, &theDataSize, 0L);
			
			COutlineItem* inItem;
			inItem = (COutlineItem*)theFlavorData.vPointerToSourceObject;
			string itempath = inItem->archive()->pathName() + inItem->name();
			string path, file, extension;
			decomposeEntryName(itempath, path, file, extension);
			resource_type_t resType = gRegistry->extensionToType(extension.c_str());
			
			if (gRegistry->isModelType(resType)) {
				_rendering._pickTag = true;
			} else {
				_rendering._pickShader = true;
			}	
			return true;
		}
	}	
	return false;
}

void C3DPane::InsideDropArea (DragReference inDragRef)
//
// The cursor is still in our area.
//
{	// protected, virtual
	FocusDraw();
	//aglSetCurrentContext(mCtx);
	
	// handle mesh and tag selection
	static Point lastMouse;
	Point	theMouseLocation;
	Point	thePinnedLocation;
	OSErr anErr = ::GetDragMouse(inDragRef, &theMouseLocation, &thePinnedLocation);
//	if (lastMouse.h != theMouseLocation.h || lastMouse.v != theMouseLocation.v) {
		lastMouse = theMouseLocation;
		::GlobalToLocal(&theMouseLocation);
		PickModel(theMouseLocation.h, theMouseLocation.v);
#warning removed for Carbon			
//		anErr = ::UpdateDragHilite(inDragRef, GetMacPort()->visRgn);
		Redraw();	
		FocusDraw();
		// wait a few ticks
		long now = ::TickCount();
		while(::TickCount() < now + 3)
			;	
//	}
	LDragAndDrop::InsideDropArea(inDragRef);
}


string C3DPane::selectedModelTag(CModelInstance *&selectedModel)
{
	selectedModel = nil;
	return "";
}

void C3DPane::applyDraggedTexture(CGLImage *texture)
{
	CShader *selectedShader = _resources->selectedShader();
	if (selectedShader) {
		dprintf("applyDraggedTexture: %s\n", &selectedShader->name);
		selectedShader->setTexture(texture);
	} else if (_model) {
		// FIXME - textures never go away
		_model->modelClass()->addTexture(texture);
		_model->setTextureIndex(_model->modelClass()->textureCount()-1);
	}
	_resources->setSelectedShader(nil);
}

void C3DPane::ReceiveDragItem(
	DragReference	inDragRef,
	DragAttributes	/* inDragAttrs */,
	ItemReference	inItemRef,
	Rect			&inItemBounds)	// In Local coordinates
{
#pragma unused (inItemBounds)
	//
	// Information about the drag contents we'll be needing.
	//
	FlavorFlags		theFlags;		// We actually only use the flags to see if a flavor exists
	Size			theDataSize;	// How much data there is for us.
	ItemData		theFlavorData;	// Where we will put that data.
	OSErr			err;

	string tagName;

	// FIXME - put up memory warning
	if (LGrowZone::GetGrowZone()->MemoryIsLow())
		return;
		
#warning removed for carbon
//	::InvalRect(&inItemBounds);


	// see if the drag originated in this applicatoin
	// if not, flavors containing pointers won't work
	
	DragAttributes attributes;
	err = ::GetDragAttributes(inDragRef, &attributes);
	if (err == noErr && (attributes & kDragInsideSenderApplication)) {
			
		//
		// Check to see if the drag contains a cPakType item.
		//
		
		err = ::GetFlavorFlags(inDragRef, inItemRef, cPakType, &theFlags);
		if (err == noErr) {
			err = ::GetFlavorDataSize(inDragRef, inItemRef, cPakType, &theDataSize);
			if (!err && theDataSize) {
				err = ::GetFlavorData(inDragRef, inItemRef, cPakType, &theFlavorData, &theDataSize, 0L);
				if (err == noErr) {
					
					COutlineItem* inItem;
					inItem = (COutlineItem*)theFlavorData.vPointerToSourceObject;
					string itempath = inItem->archive()->pathName() + inItem->name();
					string path, file, extension;
					decomposeEntryName(itempath, path, file, extension);
					resource_type_t resType = gRegistry->extensionToType(extension.c_str());
					
					if (gRegistry->isModelType(resType)) {
						
						
						CModelInstance *selectedModel = nil;
						tagName = selectedModelTag(selectedModel);
						if (tagName.length() > 0 && selectedModel) {
							selectedModel->detachModelAtTag(tagName);
							
							CModelInstance *instance = selectedModel->resources()->modelInstanceWithClassName(itempath.c_str());
							selectedModel->addLinkedModel(instance, tagName.c_str());
							_resources->parseShaders();
						}
						
					} else if (extension == "skin") {
					
						// skin for md3
						if (_model) {
							_model->applySkin(inItem->archive(), itempath);
							_model->resources()->parseShaders();
						}
							
					} else {
					
						// texture
						CGLImage *glimage = _resources->textureWithName(itempath.c_str(), 0);
						if (glimage) {
						
							// FIXME: can't remember why I copy here
							CGLImage* copyImage = glimage->copy();
							if (copyImage) {
								copyImage->uploadGLImage();
								applyDraggedTexture(copyImage);
							}
						}
					}
				}
				return;
			}
		}
		
		
		
		//
		// Check to see if the drag contains a cImageType item.
		//
		err = ::GetFlavorFlags(inDragRef, inItemRef, cImageType, &theFlags);
		if (err == noErr) {
			err = ::GetFlavorDataSize(inDragRef, inItemRef, cImageType, &theDataSize);
			if (!err && theDataSize) {
				ThrowIf_(theDataSize != sizeof(ItemData));	// sanity check
				//
				// Get the data about the image we are receiving.
				//
				err == ::GetFlavorData(inDragRef, inItemRef, cImageType, &theFlavorData, &theDataSize, 0L);
				if (err == noErr) {
					CGLImage* inImage;
					inImage = (CGLImage*)theFlavorData.vPointerToSourceObject;
					CGLImage* copyImage = inImage->copy();
					copyImage->uploadGLImage();
					applyDraggedTexture(copyImage);
					return;
				}
			}
		}
	}

	//
	// Check to see if the drag contains a 'PICT' item.
	//
	
	err = ::GetFlavorFlags(inDragRef, inItemRef, 'PICT', &theFlags);
	if (err == noErr) {
		err = ::GetFlavorDataSize(inDragRef, inItemRef, 'PICT', &theDataSize);
		if (!err && theDataSize) {

			PicHandle thePicH = (PicHandle) ::NewHandle(theDataSize);
			err = ::GetFlavorData(inDragRef, inItemRef, 'PICT', *thePicH, &theDataSize, 0L);

			if (err == noErr) {
				CPict* inImage = new CPict(thePicH);
				LGWorld *gWorld = inImage->CreateGWorld();
				CGLImage *glimage = new CGLImage(gWorld);
				glimage->uploadGLImage();
				applyDraggedTexture(glimage);
				delete gWorld;
				delete inImage;
				return;
			} else {
				::DisposeHandle((Handle)thePicH);
			}
		}
	}
}

#pragma mark -

// handle mouse down
void C3DPane::MouseDown(const SMouseDownEvent&	inMouseDown, const Point& p)
{
#pragma unused ( p)
		
	dprintf("sClickCount %d what %d message %d modifiers %d\n", sClickCount, inMouseDown.macEvent.what,  inMouseDown.macEvent.message, inMouseDown.macEvent.modifiers);
	if (sClickCount == 2 && _rendering._pickShader) {
		CShader *shader = _resources->selectedShader();
		dprintf("shader name %s in file %s\n", &shader->name, &shader->filename);

		string file = (char*)&shader->filename;
		string item = (char*)&shader->name;
		if (file.length() > 0)
			gApplication->OpenEditorForItem(_resources->pak(), file.c_str(), item.c_str(), cmdKey);
	}
}

void C3DPane::PickModel(int x, int y)	
{
#pragma unused (x, y)
}


/*  processHits prints out the contents of the 
 *  selection array.
 */
void C3DPane::ProcessHits (GLint hits, GLuint buffer[])
{
	UInt32 i, j;
	GLuint  names, *ptr;
	float z1,z2;
	float smallest = 999999999999.9;

	_rendering._pickedName = 0;
	//dprintf ("hits = %d\n", hits);
	ptr = (GLuint *) buffer;
	for (i = 0; i < hits; i++) { // for each hit 
	
		names = *ptr++;
		z1 = (float) *ptr++/0x7fffffff;
		z2 = (float) *ptr++/0x7fffffff;

		//dprintf ("number of names for hit %d = %d (z1 %g z2 %g)\n", i, names,z1 , z2); 
		if (names) {
			//dprintf ("   names are ");
			for (j = 0; j < names; j++, ptr++) { //  for each name
				if (z1 < smallest) {
					smallest = z1;
					_rendering._pickedName = *ptr;
				}
			//	dprintf ("%d ", *ptr);
			}
			//dprintf ("\n");
		}
	}
}

#pragma mark -


void C3DPane::ExportScreenDump()
{
	Boolean		saveOK = false;

	PP_StandardDialogs::LFileDesignator*	designator =
								new PP_StandardDialogs::LFileDesignator;
								
//	designator->SetFileType( GetFileType() );
//	Str255	defaultName;

	LStr255 title;
	title = "out.pict";
	
	if (designator->AskDesignateFile(title)) {
		FSSpec outFSSpec;
		
		designator->GetFileSpec(outFSSpec);
		// designator->IsReplacing() && UsesFileSpec(outFSSpec)
		
			SetupScreenDump();
			DrawGL();
			FocusDraw();
			SwapBuffers();
			LGWorld *world = ScreenDump();
			if (world) {
				CPict *pict = new CPict(world);
				if (pict) {
					pict->SaveToFile(outFSSpec);
					delete pict;
				}
				delete world;
			}
	}
	delete designator;
}

CGrafPtr	currentPort;
GDHandle	currentDevice;
AGLDrawable currentDrawable;
PixMapHandle pixMap;
GWorldPtr worldPtr;

void C3DPane::SetupScreenDump()
{
	FocusDraw();
	currentDrawable = aglGetDrawable ( mCtx );

	// create a GWorld to render into
	Rect bounds;
	::SetRect(&bounds,0,0, 480, 640);
	
	QDErr err = ::NewGWorld(&worldPtr, 32, &bounds, 0, 0, 0);
//	if (err != noErr)
//		return nil;
	
//	pixMap = ::GetGWorldPixMap( worldPtr );
//	::LockPixels(pixMap);

	/*
	// prepare gworld for drawing
	::GetGWorld( &currentPort, &currentDevice );
	::SetGWorld( worldPtr, nil );
	pixMap = ::GetGWorldPixMap( worldPtr );
	::LockPixels(pixMap);
	::EraseRect( &(**pixMap).bounds );
	::SetGWorld( currentPort, currentDevice );
	*/
	
	::GetGWorld( &currentPort, &currentDevice );
	pixMap = ::GetGWorldPixMap( worldPtr );
	::LockPixels(pixMap);
	::SetGWorld( worldPtr, nil );
	GLboolean result = aglSetDrawable (mCtx, worldPtr );
	result = aglUpdateContext ( mCtx );
	dprintf("SetupScreenDump aglGetError %d\n", aglGetError());
}

LGWorld *C3DPane::ScreenDump()
{
	
//	::UnlockPixels( pixMap );
	::SetGWorld( currentPort, currentDevice );
	Boolean result = aglSetDrawable (mCtx, currentDrawable );

	// finished drawing in gworld	
	return new LGWorld(worldPtr);
}

#pragma mark -


/*
 	Function:	DrawBitmapString
 	Usage:		DrawBitmapString(x, y, GLUT_BITMAP_8_BY_13, "Test"
 	---------------------------------------------------------------------------
 	Draws a string using a bitmap font.  See GLUT documentation for the names
 	of fonts.
 */
 
void C3DPane::DrawBitmapString(GLfloat x, GLfloat y, void *font, char *format,...)
{
	int len, i;
	va_list args;
	char string[256];

	// special C stuff to interpret a dynamic set of arguments specified by "..."
	va_start(args, format);
	vsprintf(string, format, args);
	va_end(args);

	glRasterPos2f(x, y);
	len = (int) strlen(string);
	
	
	
	for (i = 0; i < len; i++)
	{
		glutBitmapCharacter(font, string[i]);
	}
}

