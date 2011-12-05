/* 
	C3DMapPane.cpp

	Author:			Tom Naughton
	Description:	<describe the C3DMapPane class here>
*/

#include <stdio.h>
#include <gl.h>
#include <glu.h>
#include <glut.h>
#include <stdarg.h>

#include "C3DMapPane.h"
#include "CGLImage.h"
#include "CPakRatApp.h"
#include "CPreferences.h"
#include "AppConstants.h"

// FIXME move this
GLvoid glPrint(const char *fmt, ...);					// Custom GL "Print" Routine


// ---------------------------------------------------------------------------
//	¥ C3DMapPane						Parameterized Constructor [public]
// ---------------------------------------------------------------------------
// desc is a string of pixel format attributes

C3DMapPane::C3DMapPane( const SPaneInfo&	inPaneInfo, LStr255& desc) 
	: C3DPane(inPaneInfo, desc)
{
	_frameCount = 0;
	_frameCountTicks = 0;
	_fps = 0;
	_drawInterval = 0;
	_resources = 0;
	_map = 0;
	_levelShot = 0;
	dprintf("C3DMapPane::C3DMapPane( const SPaneInfo&	inPaneInfo, LStr255& desc) : GLPane(inPaneInfo, desc)\n");
}


// ---------------------------------------------------------------------------
//	¥ C3DMapPane						Stream Constructor		  [public]
// ---------------------------------------------------------------------------

C3DMapPane::C3DMapPane( LStream*	inStream) 
	: C3DPane(inStream)
{

	// FIXME: this stuff should go in a common init method
	dprintf("C3DMapPane::C3DMapPane( LStream*	inStream) : GLPane(inStream)\n");

	
	_frameCount = 0;
	_frameCountTicks = 0;
	_fps = 0;
	_drawInterval = 0;
	_resources = 0;
	_map = 0;
	_levelShot = 0;
	_moveVelocity = 0.0f;
	_strafeVelocity = 0.0f;
	_dragTime = 0.0f;

	resetRendering() ;
}

C3DMapPane::C3DMapPane() : C3DPane()
{
}


C3DMapPane::~C3DMapPane()
{
	// map will delete its textures from the current context
	if (_map)
		delete _map;
	dprintf("~C3DMapPane\n");
}

void C3DMapPane::SetLevelShot(CGLImage *shot) 
{ 
	_levelShot = shot;
}

void C3DMapPane::SetMap(CBsp *bsp) 
{ 
	if (_map)
		delete _map;

	_lastDraw = 0;
	_map = bsp; 
	
	SDimension16 l_PortDim;
	GetFrameSize(l_PortDim);
	ReshapeGL(l_PortDim.width, l_PortDim.height);

	StartIdling();
	SetupRendering();
}

  /**
   * <p>Reset all user manipulations (scaling + panning + rotating) of this model canvas.
   */
void C3DMapPane::resetRendering() 
{

	_rendering.r_lockpvs = 0;
	_rendering.r_eyesep=2.0;
	_rendering.r_focallength=100;
    _rendering.r_eyefov = 90.0;
	_rendering._textureMap = nil;
	_rendering._renderTexturePreview = false;
	_rendering._renderTextureCoordinates = false;

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
	
	_moveVelocity = 0.0f;
	_strafeVelocity = 0.0f;
	_dragTime = 0.0f;


	SDimension16 l_PortDim;
	GetFrameSize(l_PortDim);
	ReshapeGL(l_PortDim.width, l_PortDim.height);
}

// ---------------------------------------------------------------------------
// SpendTime
// ---------------------------------------------------------------------------
void
C3DMapPane::SpendTime(const EventRecord	&inMacEvent)
{
	C3DPane::SpendTime(inMacEvent);
	float theTime = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0;
	float timeInterval = theTime - _lastTime;
	
	// constrain speed
	#define SPEED_INC 0.09f
	#define MAX_SPEED 2.5f
	#define SPEED_DRAG 0.3f

	if (_moveVelocity > MAX_SPEED) _moveVelocity = MAX_SPEED;
	if (_moveVelocity < -MAX_SPEED) _moveVelocity = -MAX_SPEED;
	
	if (_strafeVelocity > MAX_SPEED) _strafeVelocity = MAX_SPEED;
	if (_strafeVelocity < -MAX_SPEED) _strafeVelocity = -MAX_SPEED;


	// move
	if (_map) {
		_map->move_eye(&_rendering, _moveVelocity * timeInterval, 1, 0);
		_map->move_eye(&_rendering, _strafeVelocity * timeInterval, 0, 1);
	}
	
	// slow down
	if (theTime > _dragTime) {
		_moveVelocity = _moveVelocity * SPEED_DRAG;
		_strafeVelocity = _strafeVelocity * SPEED_DRAG;
	}

	Redraw();
	_lastTime = theTime;
}

void C3DMapPane::ReshapeGL(int width, int height)
{
	FocusDraw();
	//aglSetCurrentContext(mCtx);
    glViewport(0, 0, width, height);    
    glMatrixMode(GL_PROJECTION); 
    glLoadIdentity();
    _width = width;
    _height = height;
    if (_map)
    	_map->reshape(&_rendering,width,height);
	aglUpdateContext(mCtx);
	
//	dprintf("ReshapeGL error: %s\n",aglErrorString(aglGetError()));
}

void C3DMapPane::PickModel(int x, int y)	
{

   GLuint selectBuf[SELECT_BUFFER_SIZE];
   GLint hits;
   GLint viewport[4];
   
	FocusDraw();
	//aglSetCurrentContext(mCtx);
	glDepthMask(GL_TRUE);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable(GL_DEPTH_TEST);

	glGetIntegerv (GL_VIEWPORT, viewport);
	glSelectBuffer (SELECT_BUFFER_SIZE, selectBuf);
	glRenderMode (GL_SELECT);
	glInitNames();

	//  create 5x5 pixel picking region near cursor location
	glMatrixMode (GL_PROJECTION);
	glPushMatrix ();
	glLoadIdentity ();
	gluPickMatrix ((GLdouble) x, (GLdouble) (viewport[3] - y),  5.0, 5.0, viewport);
	if (_height > 0 && _map)
		gluPerspective(_map->calc_fov(_rendering.r_eyefov, _width, _height), (double)_width/(double)_height, NEAR_GL_PLANE, FAR_GL_PLANE);

	// position and render model
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix ();
	glDepthMask( GL_TRUE);
	_rendering._renderOpaque = true;
	_rendering._renderBlend = true;
	
	if (_map)
		_map->Draw(&_rendering);
		
	// clean up
	glPopMatrix ();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix ();
	glFlush ();
	glMatrixMode(GL_MODELVIEW);
	hits = glRenderMode (GL_RENDER);
	
	ProcessHits (hits, selectBuf);
}



void C3DMapPane::DrawGL()	
{
    // count fps
	if (::TickCount() > _frameCountTicks + 60) {
		_frameCountTicks = ::TickCount();
		_fps = _frameCount;
		_frameCount = 0;
	}
	_frameCount++;


	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	if (_resources)
		_resources->setSelectedShader(nil);
		
	if(_map) {
		_map->Draw(&_rendering);
	} else {
	
		float x= 1.0;
		float y= 1.0;
		float z= 0.0;

		if (_levelShot)
			_levelShot->bindTexture();
		glDisable( GL_LIGHTING );
		glEnable( GL_TEXTURE_2D );
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); 
		glVertex3f(-x, y, z);
		glTexCoord2f(1.0f, 0.0f); 
		glVertex3f( x, y, z);
		glTexCoord2f(1.0f, 1.0f); 
		glVertex3f( x, -y, z);
		glTexCoord2f(0.0f, 1.0f); 
		glVertex3f(-x, -y, z);
		glEnd();	
	
	}
		
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glPushMatrix();
	DrawHUD();
	glPopMatrix();
}

void C3DMapPane::DrawHUD()
{
	CPreferences *prefs = gApplication->preferences();
	if (!prefs->booleanForKey("showHUD"))
		return;
		
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_LIGHTING );
	glDisable( GL_DEPTH_TEST);
	glDisable( GL_BLEND);

	// switch to 2d orthographic mode for drawing text
	SDimension16 l_PortDim;
	GetFrameSize(l_PortDim);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, l_PortDim.width, l_PortDim.height, 0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	int lineHeight = 13;
	int x = 8;
	int y = l_PortDim.height - lineHeight * 2 - 5;
		
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	y += lineHeight;
	glRasterPos2f(x, y);
	
	CShader *shader = _resources->selectedShader();
	if (shader)
		glPrint("%s", &shader->name);

	y += lineHeight;
	glRasterPos2f(x, y);
	glPrint("%d fps", _fps);
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glEnable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST);
}


// handle mouse down
void C3DMapPane::MouseDown(const SMouseDownEvent&	inMouseDown, const Point& p)
{
	_moveVelocity = 0.0;
	_strafeVelocity = 0.0;
	C3DPane::MouseDown(inMouseDown, p);
	int x=p.h, y=p.v;
	if (_map)
		_map->mouse_down(&_rendering,x,y);
}

// handle mouse drag
void C3DMapPane::MouseDrag(const SMouseDownEvent&	inMouseDown, const Point& p)
{
	Boolean shiftDown = (inMouseDown.macEvent.modifiers & shiftKey) != 0;

	int x=p.h, y=p.v;

	if (shiftDown) { 
	
	
	}
	
	_m_x=x;
	_m_y=y;
	
	if (_map)
		_map->mouse_motion(&_rendering,_m_x, _m_y);
	
	Redraw();
}


// ---------------------------------------------------------------------------
//	¥ ObeyCommand													  [public]
// ---------------------------------------------------------------------------
//	Respond to commands

Boolean
C3DMapPane::ObeyCommand(
	CommandT	inCommand,
	void*		ioParam)
{
	Boolean		cmdHandled = true;
	
	switch (inCommand) {
	
		case cmd_lighting:
			_rendering._lightmap = !_rendering._lightmap;
			break;
		case cmd_wireframe:
			_rendering._wireframe = !_rendering._wireframe;
			break;
		case cmd_texture:
			_rendering._textured = !_rendering._textured;
			break;
		case cmd_shownormals:	
			_rendering._showNormals = !_rendering._showNormals;
			break;
		case cmd_showbox:	
			_rendering._showBoneFrameBox = !_rendering._showBoneFrameBox;
			break;
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
C3DMapPane::FindCommandStatus(
	CommandT	inCommand,
	Boolean&	outEnabled,
	Boolean&	outUsesMark,
	UInt16&		outMark,
	Str255		outName)
{
	CPreferences *prefs = gApplication->preferences();
	switch (inCommand) {
	

	// rendering settings
		case cmd_lighting:
			outEnabled = true;
			outUsesMark = true;
			outMark = _rendering._lightmap ? checkMark : noMark;
			break;
		case cmd_wireframe:
			outEnabled = true;
			outUsesMark = true;
			outMark = _rendering._wireframe ? checkMark : noMark;
			break;
		case cmd_texture:
			outEnabled = true;
			outUsesMark = true;
			outMark = _rendering._textured ? checkMark : noMark;
			break;
		case cmd_shownormals:	
			outEnabled = true;
			outUsesMark = true;
			outMark = _rendering._showNormals ? checkMark : noMark;
			break;
		case cmd_showbox:	
			outEnabled = true;
			outUsesMark = true;
			outMark = _rendering._showBoneFrameBox ? checkMark : noMark;
			break;
		default:
			LCommander::FindCommandStatus(inCommand, outEnabled,
									outUsesMark, outMark, outName);
			break;
	}
}


// ---------------------------------------------------------------------------
//	¥ HandleKeyPress												  [public]
// ---------------------------------------------------------------------------
//	Tab switches the Target to the next item in the TabGroup.
//	Shift-Tab to the previous item.
//	All other keystrokes (and Tabs with modifiers other than Shift)
//		get passed up.

Boolean
C3DMapPane::HandleKeyPress(
	const EventRecord	&inKeyEvent)
{
	// force menu update
	LCommander::SetUpdateCommandStatus(true);
	Boolean controlDown = (inKeyEvent.modifiers & controlKey) != 0;
	Boolean shiftDown = (inKeyEvent.modifiers & shiftKey) != 0;
	float theTime = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0;
	
	// start slowing down in .5 second
	float dragTime = theTime + 0.5;

	
	// force redraw
	_drawInterval = 0;
	
			
	CPreferences *prefs = gApplication->preferences();
	string command = prefs->commandForKeyEvent(inKeyEvent);

	// movement
	if (command == "incrementLower") {
		_moveVelocity += SPEED_INC;
		_dragTime = dragTime;
	} else if (command == "decrementLower") {
		_moveVelocity -= SPEED_INC;
		_dragTime = dragTime;
		
	// strafe	
	} else if (command == "incrementUpper") {
		_strafeVelocity += SPEED_INC;
		_dragTime = dragTime;
	} else if (command == "decrementUpper") {
		_strafeVelocity -= SPEED_INC;
		_dragTime = dragTime;
	}	
	
	// rendering modes
	if (command == "toggleVertexNormals") {
		_rendering._showNormals = !_rendering._showNormals;
		SetupRendering();
		return true;
	} else if (command == "toggleBoneFrameBox") {
		_rendering._showBoneFrameBox = !_rendering._showBoneFrameBox;
		SetupRendering();
		return true;
	} else if (command == "toggleLight") {
		_rendering._lightmap = !_rendering._lightmap;
		SetupRendering();
		return true;
	} else if (command == "toggleWireframe") {
		_rendering._wireframe = !_rendering._wireframe;
		SetupRendering();
		return true;
	} else if (command == "toggleTextured") {
		_rendering._textured = !_rendering._textured;
		SetupRendering();
		return true;
	} else if (command == "toggleHUD") {
		Boolean hud = prefs->booleanForKey("showHUD");
		prefs->setBooleanForKey(!hud, "showHUD");
		return true;
	} else if (command == "resetRendering") {
		resetRendering();
		return true;
	
	
	// see how fast this thing really goes
	} else if (command == "hogCPU") {
		while(!Button()) {
			Redraw();
		}

	// find good things
	} else if (command == "findGoodie") {
		if (_map) {
			if (controlDown && shiftDown)
				_map->find_goodie(&_rendering,"weapon_bfg");
			else if (controlDown)
				_map->find_goodie(&_rendering,"weapon_");
			else 
				_map->find_goodie(&_rendering,nil);
			
		}
	}
	return false;
}


void C3DMapPane::SetupRendering()
{
}

void C3DMapPane::ListenToMessage(MessageT	 inMessage, void* ioParam)
{
#pragma unused (inMessage, ioParam)
	
}

// glPrint -----------------------------------------------------------------

GLvoid glPrint(const char *fmt, ...)					// Custom GL "Print" Routine
{
	char            text[256];                          // Holds Our String
    va_list         ap;                                 // Pointer To List Of Arguments
        
	if (text == NULL)									// If There's No Text
		return;											// Do Nothing

	va_start(ap, fmt);                                      // Parses The String For Variables
        vsprintf(text, fmt, ap);                            // And Converts Symbols To Actual Numbers
    va_end(ap);                                             // Results Are Stored In Text
      
      
	int len, i;
	len = (int) strlen(text);
	for (i = 0; i < len; i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, text[i]);
	}
        
    /*    
	glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
	glListBase(base - 32);								// Sets The Base Character to 32
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
	glPopAttrib();										// Pops The Display List Bits
	*/
}
	
