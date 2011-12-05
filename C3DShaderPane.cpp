/* 
	C3DShaderPane.cpp

	Author:			Tom Naughton
	Description:	<describe the C3DShaderPane class here>
*/


#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <UStandardDialogs.h>
#include <glut.h>
#include <gl.h>
#include <glu.h>

#include <LDragAndDrop.h>

#include "CDragTask.h" 
#include "C3DShaderPane.h"
#include "utilities.h"
#include "CShader.h"
#include "CGLImage.h"
#include "AppConstants.h"
#include "CPakRatApp.h"
#include "CPreferences.h"
#include "CResourceManager.h"
#include "CTextures.h"
#include "CPict.h"
#include "CDragTask.h" 

static float mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static float mat_shininess[] = { 20.0f };    
static float light_ambient[] = { 0.9f, 0.9f, 0.9f, 1.0f };
static float light0_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static float light0_position[] = { 55.0f, -50.0f, -5.0f, 0.0f };
static float light1_diffuse[] = { 0.5f, 0.5f, 1.0f, 1.0f };
static float light1_position[] = { -50.0f, 45.0f, 15.0f, 0.0f };


// ---------------------------------------------------------------------------
//	¥ C3DShaderPane						Parameterized Constructor [public]
// ---------------------------------------------------------------------------
// desc is a string of pixel format attributes

C3DShaderPane::C3DShaderPane( const SPaneInfo&	inPaneInfo, LStr255& desc) 
	: C3DPane(inPaneInfo, desc)
{
	_retainedShader = false;
	_image = nil;
	_shader = nil;
	dprintf("C3DShaderPane::C3DShaderPane( const SPaneInfo&	inPaneInfo, LStr255& desc) : GLPane(inPaneInfo, desc)\n");
}


// ---------------------------------------------------------------------------
//	¥ C3DShaderPane						Stream Constructor		  [public]
// ---------------------------------------------------------------------------

C3DShaderPane::C3DShaderPane( LStream*	inStream) 
	: C3DPane(inStream)
{
	_retainedShader = false;
	_image = nil;
	_shader = nil;
	float theTime = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0;
	dprintf("C3DShaderPane::C3DShaderPane( LStream*	inStream) : GLPane(inStream)\n");
	
}

C3DShaderPane::C3DShaderPane() : C3DPane()
{
	_retainedShader = false;
	_shader = nil;
	_image = nil;
}


C3DShaderPane::~C3DShaderPane()
{
	aglSetCurrentContext(mCtx);
	if (_retainedShader) {
		if (_shader)
			delete _shader; // shader will delete the image
	} else {
		if (_image)
			delete _image;
		StopIdling();	
	}
		
	dprintf("~C3DShaderPane\n");
}

#pragma mark -


void C3DShaderPane::SetupRendering()
{

	FocusDraw();
	aglSetCurrentContext(mCtx);

    glClearDepth(1.0);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);    

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  
    glEnable(GL_BLEND);    
    glEnable(GL_NORMALIZE);
  
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
    
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
    glLightfv(GL_LIGHT1, GL_POSITION, light1_position);   
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHTING);

 	glEnable(GL_LINE_SMOOTH);
 	glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
    glEnable(GL_POLYGON_SMOOTH);    
 	glHint(GL_POLYGON_SMOOTH_HINT, GL_DONT_CARE);
    glShadeModel(GL_SMOOTH);   

}


void C3DShaderPane::SetShader(CPakStream *item, const char* name, int modifiers)
{
	Boolean commandDown = (modifiers & cmdKey) != 0;
	Boolean shiftDown = (modifiers & shiftKey) != 0;
	Boolean optionDown = (modifiers & optionKey) != 0;

	FocusDraw();
	//aglSetCurrentContext(mCtx);

	// load the image, check it's size
	_image = CTextures::loadTexture(item);
	
	// resize window
	Rect bounds, size;
	bounds = _image->bounds();
	size.left = 0;
	size.right = bounds.right - bounds.left;
	size.top = 0;
	size.bottom = bounds.bottom - bounds.top;
	
#warning broken for Carbon
#if PP_Target_Carbon
	LWindow *myWindow = LWindow::FetchWindowObject(UQDGlobals::GetCurrentWindowPort());
#else
	GrafPtr myPort = GetMacPort();
	LWindow *myWindow = LWindow::FetchWindowObject(myPort);
#endif
	SDimension16 dims;
	dims.width = size.right;
	dims.height = size.bottom;
	myWindow->ResizeWindowTo(dims.width, dims.height);
//	myWindow->SetStandardSize(dims);
	
	// get rid of the file extension
	string strippedName = stripExtension(lowerString(fixSlashes(name))); 
	_shaderName = strippedName;
	
	// if the item doesn't belong to an archive, make a shader for it
	if (!item->rootArchive() || optionDown) {
		_image->uploadGLImage();
		_shader = new CShader(_resources, strippedName.c_str(), SHADER_MD3, _image); 
		_retainedShader = true;
	} else {
		_shader = _resources->shaderWithName((const char*)strippedName.c_str());
		_resources->parseShaders();
		StartIdling();
	}
}



	
#pragma mark -
	
// ---------------------------------------------------------------------------
// SpendTime
// ---------------------------------------------------------------------------
void
C3DShaderPane::SpendTime(const EventRecord	&inMacEvent)
{
	CWav *teleport = nil;
	C3DPane::SpendTime(inMacEvent);
	Boolean optionDown = (inMacEvent.modifiers & optionKey) != 0;
	Boolean commandDown = (inMacEvent.modifiers & cmdKey) != 0;
		
	Redraw();
	_drawInterval = 0; // in Ticks 
}




void C3DShaderPane::ReshapeGL(int width, int height)
{
	FocusDraw();
	//aglSetCurrentContext(mCtx);
    glViewport(0, 0, width, height);    
	aglUpdateContext(mCtx);
//	dprintf("ReshapeGL error: %s\n",aglErrorString(aglGetError()));
}

void C3DShaderPane::DrawGL()	
{
	FocusDraw();
	float theTime = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    if (CShader::_animateShaders) g_frametime = (double)glutGet(GLUT_ELAPSED_TIME) / 1000.0 - g_starttime;
	
	// set up drawing
	//prepare the canvas for rendering
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    if (_shaderName.length()) {

		float z= -1.0;
	
		renderingAttributes_t rendering;
	    memset(&rendering, 0, sizeof(renderingAttributes_t));
		rendering._textured = true;
		rendering._renderOpaque = true;
		rendering._renderBlend = true;
		
		
		int *outElems = gShaderArrays.elems;
		float *outVerts = (float*)gShaderArrays.verts;
		float *outTexvecs = (float*)gShaderArrays.tex_st;
		float *outNormals = (float*)gShaderArrays.norms;
	    gShaderArrays.numcolours = gShaderArrays.numverts = gShaderArrays.numelems = 0;
	    
	    int gridSize = 20;
	    float unit = 2.0 / gridSize;
	    for (int y = 0; y <= gridSize; y++)
	    	for (int x = 0; x <= gridSize; x++) {
	    	
				*outTexvecs++ = 1.0 - unit*x/2.0; 
				*outTexvecs++ = unit*y/2.0; 
				
				*outVerts++ = 1.0 - unit*x; 
				*outVerts++ = 1.0 - unit*y; 
				*outVerts++ = z;
				
				*outNormals++ = 0.0 ;  	  
				*outNormals++ = 0.0 ;
				*outNormals++ = 0.0 ;

				gShaderArrays.numverts++;
			}
			
		gridSize++;
	    for (int y = 0; y < gridSize-1; y++)
	    	for (int x = 0; x < gridSize-1; x++) {
				*outElems++ =  (gridSize * y ) + x; 
				*outElems++ =  (gridSize * y ) + gridSize  + x; 
				*outElems++ =  (gridSize * y ) + x + 1; 
				gShaderArrays.numelems += 3;

				*outElems++ = (gridSize * y ) + x + 1; 
				*outElems++ = (gridSize * y ) + gridSize  + x; 
				*outElems++ = (gridSize * y ) + gridSize  + x + 1; 
				gShaderArrays.numelems += 3;
			}
	    
		glEnable( GL_TEXTURE_2D );
	    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		if (_shader)
			_shader->renderFlush(&rendering, 0);
		
	}
}





#pragma mark -
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
C3DShaderPane::PutOnDuty(
	LCommander*	 inNewTarget)
{
#pragma unused (inNewTarget)
}

// ---------------------------------------------------------------------------
//	¥ TakeOffDuty
// ---------------------------------------------------------------------------
//	A Commander is going off duty
//
//	Subclasses should override this function if they wish to behave
//	differently when on duty than when off duty

void
C3DShaderPane::TakeOffDuty()
{
}

// ---------------------------------------------------------------------------
//	¥ ObeyCommand													  [public]
// ---------------------------------------------------------------------------
//	Respond to commands

Boolean
C3DShaderPane::ObeyCommand(
	CommandT	inCommand,
	void*		ioParam)
{
	Boolean		cmdHandled = true;
	
	switch (inCommand) {
	
						
		default:
			cmdHandled = LCommander::ObeyCommand(inCommand, ioParam);
			break;
	}
	
	return cmdHandled;
}


// ---------------------------------------------------------------------------
//	¥ FindCommandStatus												  [public]
// ---------------------------------------------------------------------------
//	Pass back whether a Command is enabled and/or marked (in a Menu)

void
C3DShaderPane::FindCommandStatus(
	CommandT	inCommand,
	Boolean&	outEnabled,
	Boolean&	outUsesMark,
	UInt16&		outMark,
	Str255		outName)
{
	Boolean isMd3 = true;
	CPreferences *prefs = gApplication->preferences();
	
	switch (inCommand) {
	

		default:
			LCommander::FindCommandStatus(inCommand, outEnabled,
									outUsesMark, outMark, outName);
			break;
	}
}


// ---------------------------------------------------------------------------
//	¥ HandleKeyPress												  [public]
// ---------------------------------------------------------------------------

Boolean
C3DShaderPane::HandleKeyPress(
	const EventRecord	&inKeyEvent)
{	
#pragma unused (inKeyEvent)
	return false;
}


#pragma mark -
void C3DShaderPane::PasteItem()
{	// private
	LClipboard* theClipboard = LClipboard::GetClipboard();
	SignalIf_(theClipboard == nil);
	
	Handle theDataH = ::NewHandle(0);
	theClipboard->GetData(cImageType, theDataH);
	
//	CDDItem* thePastedItem = new CDDItem(this, *(ItemData*) *theDataH);
//	ThrowIf_(thePastedItem == nil);
	
//	SwitchTarget(thePastedItem);
}


void C3DShaderPane::Click(SMouseDownEvent& inMouseDown)
{
	inMouseDown.delaySelect = false;
	C3DPane::Click(inMouseDown);
	if (sClickCount == 2) {
		CShader *shader = _resources->selectedShader();
	//	dprintf("shader name %s in file %s\n", &shader->name, &shader->filename);

		string file = (char*)&_shader->filename;
		string item = (char*)&_shader->name;
		if (file.length() > 0)
			gApplication->OpenEditorForItem(_resources->pak(), file.c_str(), item.c_str(), cmdKey);
	}
	inMouseDown.delaySelect = true;
}


void
C3DShaderPane::ClickSelf(const SMouseDownEvent& inMouseDown )
{	// protected, virtual

	//SwitchTarget(this);		// Make this item active
	//
	// Track item long enough to distinguish between a click to
	// select, and the beginning of a drag
	//
	Boolean isDrag = ::WaitMouseMoved(inMouseDown.macEvent.where);
	
	// no dragging in low memory situations
	if (LGrowZone::GetGrowZone()->MemoryIsLow())
		return;

	if (isDrag) {
		//
		// If we leave the window, the drag manager will be changing thePort,
		// so we'll make sure thePort remains properly set.
		//
		mSuperView->FocusDraw();
		ApplyForeAndBackColors();
		CreateDragEvent(inMouseDown);
		mSuperView->OutOfFocus(nil);
	}
}

void C3DShaderPane::HiliteDropArea(DragReference inDragRef)
{
	//mPane->
	ApplyForeAndBackColors();

	Rect	dropRect;
	//mPane->
	CalcLocalFrameRect(dropRect);
	::MacInsetRect(&dropRect, -1, -1);
	StRegion	dropRgn(dropRect);
	
	::ShowDragHilite(inDragRef, dropRgn, true);
	//LDropArea::HiliteDropArea(inDragRef);
}

void C3DShaderPane::CreateDragEvent(const SMouseDownEvent &inMouseDown)
{	// private
	Rect theRect = _image->bounds();
	CalcLocalFrameRect(theRect);
	//
	// Build a structure to contain the data we'll be passing to the drag event.
	//
	ItemData theFlavorData;
	theFlavorData.vPointerToSourceObject = _image;
	//
	// Begin the drag task
	//
	
	//
	// Create a new drag task
	//
	CDragTask theDragTask (inMouseDown.macEvent, _image);
	theDragTask.DoDrag();

	//if (theDragTask.DropLocationIsFinderTrash()) {
	//	DeleteSelf();
	//}
}


Boolean
C3DShaderPane::ItemIsAcceptable(DragReference inDragRef, ItemReference inItemRef)
//
//	ItemIsAcceptable will be called whenever the Drag Manager wants to know if
//	the item the user is currently dragging contains any information that we
//	can accept.
//
//	In our case, the only thing we'll accept are cImageType items.
//
{
#pragma unused (inDragRef, inItemRef )
//	FlavorFlags		theFlags;
//	if (GetFlavorFlags(inDragRef, inItemRef, 'PICT', &theFlags) == noErr)
//		return true;
	return false;
}


void
C3DShaderPane::EnterDropArea(DragReference inDragRef, Boolean inDragHasLeftSender)
//
// The cursor has just entered our area.
//
{	// protected, virtual
	// Let LDragAndDrop do its thing (hilight the area)
	LDragAndDrop::EnterDropArea(inDragRef, inDragHasLeftSender);
	
	// And we'll do ours.
	//PlaySound(gPlayEnterSound, rsrc_EnterSound);
}


void
C3DShaderPane::LeaveDropArea (DragReference inDragRef)
//
// The cursor has just left the building. I repeat, the cursor has left the building.
//
{	// protected, virtual
	// Let LDragAndDrop do its thing (removes the hilighting)
	LDragAndDrop::LeaveDropArea (inDragRef);
	
	// And we'll do ours.
	//PlaySound(gPlayExitSound, rsrc_ExitSound);
}

		
void
C3DShaderPane::InsideDropArea (DragReference inDragRef)
//
// The cursor is still in our area.
//
{	// protected, virtual
	
//	dprintf("C3DShaderPane::InsideDropArea\n");
	//
	// Let LDragAndDrop do its thing - this is not really necessary, since
	//		the inherited version doesn't do anything. But it's safer this
	//		way because someday it might.
	//
	LDragAndDrop::InsideDropArea(inDragRef);
	
	//
	// And we'll do ours - we'll just read the mouse coordinates, but for this
	// demo we won't do anything with them.
	//
	// The mouse location is where the mouse actually is on the screen. The
	// alternative is the pinned location, which is _usually_ the same location,
	// but can be different if the cursor is being constrained by a tracking handler.
	// This is useful when you want an area within a view to be 'off-limits' to
	// the ongoing drag.
	//
	// If we did want to do something based on where the cursor currently is in
	// our area (such as indicating an insertion point or something), it would
	// usually be best to use the pinned location for that work.
	//
	// Both mouse locations are returned in global screen coordinates
	//
	Point	theMouseLocation;
	Point	thePinnedLocation;
	::GetDragMouse(inDragRef, &theMouseLocation, &thePinnedLocation);
}


void
C3DShaderPane::ReceiveDragItem(
	DragReference	inDragRef,
	DragAttributes	/* inDragAttrs */,
	ItemReference	inItemRef,
	Rect			&inItemBounds)	// In Local coordinates
//
// The user has dropped something in this view.
//
{
#pragma unused (inItemBounds)
	// FIXME - put up memory warning
	if (LGrowZone::GetGrowZone()->MemoryIsLow())
		return;

#warning removed for Carbon		
#if PP_Target_Carbon
#else
	::InvalRect(&inItemBounds);
#endif
	
	
	//
	// Information about the drag contents we'll be needing.
	//
	
	FlavorFlags		theFlags;		// We actually only use the flags to see if a flavor exists
	Size			theDataSize;	// How much data there is for us.
	OSErr			err;

	err = ::GetFlavorFlags(inDragRef, inItemRef, 'PICT', &theFlags);
	if (err == noErr) {
		err = ::GetFlavorDataSize(inDragRef, inItemRef, 'PICT', &theDataSize);
		if (err == noErr) {
			if (theDataSize) {

				PicHandle thePicH = (PicHandle) ::NewHandle(theDataSize);
				err = ::GetFlavorData(inDragRef, inItemRef, 'PICT', *thePicH, &theDataSize, 0L);
				CPict* inImage = new CPict(thePicH);
			//	SetMyGWorld(inImage->CreateGWorld());
			}
		}
	}
}


Boolean
C3DShaderPane::CheckForOptionKey(DragReference inDragRef)
{	// private
	//
	// We'll check whether the option key was down at either the beginning _or_ the
	// end of the drag, since (a) it's the preferred behaviour and (b) its so easy to do.
	//
	SInt16 theModifiersNow;			// The state of the modifier keys right now
	SInt16 theModifiersAtMouseDown;	// The state of the modifier keys when the drag began
	SInt16 theModifiersAtMouseUp;	// The state of the modifier keys when the drag ended
	::GetDragModifiers(inDragRef, &theModifiersNow, &theModifiersAtMouseDown, &theModifiersAtMouseUp);
	
	return ((theModifiersAtMouseDown & optionKey) || (theModifiersAtMouseUp & optionKey));
}


Boolean
C3DShaderPane::CheckIfViewIsAlsoSender(DragReference inDragRef)
{	// private
	//
	// Just a note: While we are using the drag attributes only at the end of the
	// drag, they are also available to you during the drag.
	//
	// Drag Attributes are described in MD+DDK, page 2-31.
	//
	DragAttributes theDragAttributes;
	::GetDragAttributes(inDragRef, &theDragAttributes);
	
	return (theDragAttributes & kDragInsideSenderWindow);
}


