/* 
	C3DModelPane.cpp

	Author:			Tom Naughton
	Description:	<describe the C3DModelPane class here>
*/


#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <UStandardDialogs.h>

#include "C3DModelPane.h"
#include <glut.h>
#include "utilities.h"
#include "CModelController.h"
#include "CModelInstance.h"
#include "CShader.h"
#include "CGLImage.h"
#include "AppConstants.h"
#include "CDragTask.h"
#include "CPict.h"
#include "CPakRatApp.h"
#include "CAnimationMenu.h"
#include "CPreferences.h"
#include "CResourceManager.h"
#include <gl.h>
#include <glu.h>
#include "CWav.h"
#include "CMdl.h"
#include "CMd2.h"
#include "CMd3.h"
#include "CTextures.h"

const float MOUSE_ROT_SCALE = 0.5f;
const float MOUSE_PAN_SCALE = 0.1f;
const float MOUSE_ZPOS_SCALE = 0.1f;

float mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float mat_shininess[] = { 20.0f };    
float light_ambient[] = { 0.9f, 0.9f, 0.9f, 1.0f };
float light0_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
float light0_position[] = { 55.0f, -50.0f, -5.0f, 0.0f };
float light1_diffuse[] = { 0.5f, 0.5f, 1.0f, 1.0f };
float light1_position[] = { -50.0f, 45.0f, 15.0f, 0.0f };


// ---------------------------------------------------------------------------
//	¥ C3DModelPane						Parameterized Constructor [public]
// ---------------------------------------------------------------------------
// desc is a string of pixel format attributes

C3DModelPane::C3DModelPane( const SPaneInfo&	inPaneInfo, LStr255& desc) 
	: C3DPane(inPaneInfo, desc)
{
	dprintf("C3DModelPane::C3DModelPane( const SPaneInfo&	inPaneInfo, LStr255& desc) : GLPane(inPaneInfo, desc)\n");
}


// ---------------------------------------------------------------------------
//	¥ C3DModelPane						Stream Constructor		  [public]
// ---------------------------------------------------------------------------

C3DModelPane::C3DModelPane( LStream*	inStream) 
	: C3DPane(inStream)
{
	float theTime = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0;
	
	// FIXME: this stuff should go in a common init method
	dprintf("C3DModelPane::C3DModelPane( LStream*	inStream) : GLPane(inStream)\n");
	_model = nil;
	_modelController = nil;
	_rendering._rotCorrectionX = 0.0f;
	_rendering._rotCorrectionY = 0.0f;
	_rendering._rotCorrectionZ = 0.0f;
	_playedIntroSound = false;
	_playedAnnounceSound = false;
	_announceSound = nil;
	
}

C3DModelPane::C3DModelPane() : C3DPane()
{
}


C3DModelPane::~C3DModelPane()
{
	StopIdling();	
	
	if (_model) 
		delete _model;	
	if (_modelController)
		delete _modelController;
	dprintf("~C3DModelPane\n");
}

#pragma mark -


void C3DModelPane::SetupRendering()
{

	FocusDraw();
	//aglSetCurrentContext(mCtx);

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



  /**
   * <p>Reset all user manipulations (scaling + panning + rotating) of this model canvas.
   */
void C3DModelPane::resetRendering() 
{
	_rendering._textureMap = nil;
	_rendering._showBoneFrameBox = false;
	_rendering._showNormals = false;	
	_rendering._lighting = false;	
	_rendering._textured = true;	
	_rendering._wireframe = false;	
	_rendering._renderTexturePreview = false;
	_rendering._renderTextureCoordinates = false;

	_rendering._pickShader = false;
	_rendering._pickTag = false;
	_rendering._pickedName = 0;
	_rendering._pickedImage = 0;	

	_rendering._showMissingTextures = gApplication->preferences()->booleanForKey("showMissingTextures");

	SDimension16 l_PortDim;
	GetFrameSize(l_PortDim);
	ReshapeGL(l_PortDim.width, l_PortDim.height);

	_rendering._rotAngleX  = 0.0;
	_rendering._rotAngleY = 0.0;
	_rendering._rotAngleZ = 0.0;
}

void C3DModelPane::SetRotationCorrection(float x, float y, float z)
{
	_rendering._rotCorrectionX = x;
	_rendering._rotCorrectionY = y;
	_rendering._rotCorrectionZ = z;
}

void C3DModelPane::SetPosition(float x, float y, float z)
{
	_rendering._xPos = x;
	_rendering._yPos = y;
	_rendering._zPos = z;
}


void C3DModelPane::SetModelController(CModelController *inController)
{
	_modelController = inController;
	StartIdling();
	SetupRendering();
	resetRendering();
}


void C3DModelPane::SetModel(CModelInstance *inModel) 
{ 
	_modelScaleFactor = 0.0;
	if (_model != inModel)
		delete _model;

	_model = inModel; 
}



	
#pragma mark -
	
// ---------------------------------------------------------------------------
// SpendTime
// ---------------------------------------------------------------------------
void
C3DModelPane::SpendTime(const EventRecord	&inMacEvent)
{
	CWav *teleport = nil;
	C3DPane::SpendTime(inMacEvent);
	Boolean optionDown = (inMacEvent.modifiers & optionKey) != 0;
	Boolean commandDown = (inMacEvent.modifiers & cmdKey) != 0;
	string path = _model->modelClass()->pathName();
	string package, file, extension;
	decomposeEntryName(path,  package, file, extension);

	if (_modelController && !optionDown && !commandDown)
		_modelController->Animate();
		
	Redraw();
	_drawInterval = 0; // in Ticks 
	
	
	if (!_playedIntroSound 
		&& _model 
		&& gApplication->preferences()->booleanForKey("playSounds") 
		&& gApplication->preferences()->booleanForKey("playTeleportSound")) {
		
		// load teleport sound
		teleport = _resources->soundWithName("/sound/world/telein.wav");
		if (!teleport)
			teleport = _resources->soundWithName("/sound/world/transin.wav");
		
		// load player announcement sound
		string playerName = playerNameFromPath(path);
		string announcePath = "sound/player/announce/" + playerName + ".wav";
		_announceSound = _resources->soundWithName(announcePath.c_str());		
			
		// play teleporter sound for players
		if (teleport) {
			dprintf("playedIntroSound path %s\n", path.c_str());
			if (gApplication->preferences()->booleanForKey("playSounds") 
				&& gApplication->preferences()->booleanForKey("playTeleportSound")
				&& path.find("players") != -1) {
				teleport->play();
			}
		}
			
		if (path.find("players") != -1) {
			// play teleporter sound for players
			if (teleport) teleport->play();
		}
		_playedIntroSound = true;
	}
}


string C3DModelPane::selectedModelTag(CModelInstance *&selectedModel)
{
	selectedModel = _model;
	return _model->selectedModelTag(&_rendering, selectedModel);
}


void C3DModelPane::ReshapeGL(int width, int height)
{
	FocusDraw();
	//aglSetCurrentContext(mCtx);
    glViewport(0, 0, width, height);    
    glMatrixMode(GL_PROJECTION); 
    glLoadIdentity();
    _width = width;
    _height = height;
    if (_height > 0)
      gluPerspective((double)60, (double)_width/(double)_height, NEAR_GL_PLANE, FAR_GL_PLANE);
    glMatrixMode(GL_MODELVIEW);
	aglUpdateContext(mCtx);

//	dprintf("ReshapeGL error: %s\n",aglErrorString(aglGetError()));
}

void C3DModelPane::DrawGL()	
{
	float theTime = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0;
	if (CShader::_animateShaders) g_frametime = (double)glutGet(GLUT_ELAPSED_TIME) / 1000.0 - g_starttime;
	
	if (_modelScaleFactor == 0.0) {
		_modelScaleStartTime = theTime;
	}
    // count fps
	if (theTime - _frameCountTime >= 1.0) {
		_frameCountTime = theTime;
		_fps = _frameCount;
		_frameCount = 0;
	}
	_frameCount++;

	//prepare the canvas for rendering
	glDepthMask(GL_TRUE);
	
	CPreferences *prefs = gApplication->preferences();
	string backgroundColor = prefs->valueForKey("backgroundColor");
	int rgbIndex = backgroundColor.find("rgb");
	if (backgroundColor == "black") {
		glClearColor(0.0, 0.0, 0.0, 1.0);
	} else if (backgroundColor == "white") {
		glClearColor(1.0, 1.0, 1.0, 1.0);
	} else if (rgbIndex >= 0) {
		RGBColor color = stringToColor(backgroundColor);
		glClearColor(color.red / 65535.0, color.green / 65535.0, color.blue / 65535.0, 1.0);
	} else {
		glClearColor(0.75, 0.75, 0.75, 1.0);
	}
	
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_MODELVIEW);
	
	/*
	// draw lights
	glPushMatrix();
	glColor3fv(light0_diffuse);
	glTranslatef(light0_position[0], light0_position[1], light0_position[2]);
	auxWireSphere(0.15f);
	glPopMatrix();
	
	glPushMatrix();
	glColor3fv(light1_diffuse);
	glTranslatef(light1_position[0], light1_position[1], light1_position[2]);
	auxWireSphere(0.15f);
	glPopMatrix();
	*/

	// this will be called to draw before being fully initialized
	if (_resources) 
		_resources->setSelectedShader(nil);
	if (_modelController && _model) {
			
		// position model
		glPushMatrix();
		glTranslatef(_rendering._xPos, _rendering._yPos, _rendering._zPos);
		
		_modelScaleFactor = (theTime - _modelScaleStartTime) * 3 + 0.00000000001;
		if(_modelScaleFactor > 1.0) {
			_modelScaleFactor = 1.0;
			if (!_playedAnnounceSound) {
				if (_model
					&& gApplication->preferences()->booleanForKey("playSounds") 
					&& gApplication->preferences()->booleanForKey("playTeleportSound")) {
					string path = _model->modelClass()->pathName();
					if (_announceSound && path.find("players") != -1) _announceSound->play();
				}
				_playedAnnounceSound = true;
			}
		}
		glScalef( _modelScaleFactor * 0.05 , _modelScaleFactor * 0.05, _modelScaleFactor * 0.05 );
		glRotatef( _rendering._rotAngleX , 1.0f ,  0.0f , 0.0f );
		glRotatef( _rendering._rotAngleY, 0.0f ,  1.0f , 0.0f );  
		
		glRotatef( _rendering._rotCorrectionX, 1.0f ,  0.0f , 0.0f );
		glRotatef( _rendering._rotCorrectionY, 0.0f ,  1.0f , 0.0f );
		glRotatef( _rendering._rotCorrectionZ, 0.0f ,  0.0f , 1.0f );
		
		
		// setup environment mapping
		Mat4 transform = identity;
		transform *= identity.rotX( _rendering._rotAngleX * DEGTORAD_CONST);
		transform *= identity.rotY( _rendering._rotAngleY * DEGTORAD_CONST);
		transform *= identity.rotZ( _rendering._rotAngleZ * DEGTORAD_CONST);
		transform *= identity.rotX( _rendering._rotCorrectionX * DEGTORAD_CONST);
		transform *= identity.rotY( _rendering._rotCorrectionY * DEGTORAD_CONST);
		transform *= identity.rotZ( _rendering._rotCorrectionZ * DEGTORAD_CONST);
		vec *mv = transform.raw();
		MAT(mv,0,3) = _rendering._xPos;
		MAT(mv,1,3) = _rendering._yPos;
		MAT(mv,2,3) = _rendering._zPos;
		_rendering._environmentTransform = transform;
	
		// draw solids
		_rendering._renderTextureCoordinates = false;
		_rendering._renderOpaque = true;
		_rendering._renderBlend = false;
		_modelController->Draw(&_rendering);

		// draw blended textures (without depth mask)
		_rendering._renderOpaque = false;
		_rendering._renderBlend = true;
		_modelController->Draw(&_rendering);
		glPopMatrix();
			
		// draw texture preview
		if (_rendering._pickedName && _rendering._pickShader)
			DrawTexturePreview();
		else
			_lastPickedName = 0;
		
		// draw overlay
		DrawHUD();
	}
	_rendering._renderTextureCoordinates = false;
	_rendering._renderTexturePreview = false;
	
	if (!(_rendering._pickTag || _rendering._pickShader))  {
		_rendering._pickedName = 0;	
		_rendering._pickedImage = 0;	
	}
}

void C3DModelPane::DrawTexturePreview()
{
	float theTime = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0;

	glDisable( GL_TEXTURE_2D );
	glDisable( GL_LIGHTING );
	glDisable( GL_BLEND);
	
	SDimension16 l_PortDim;
	GetFrameSize(l_PortDim);
	
	
	if (_lastPickedName == _rendering._pickedName) {
	
		// show it
		_textureScaleFactor = (theTime - _textureScaleStartTime) * 4;
		if(_textureScaleFactor > 1.0)
			_textureScaleFactor = 1.0;
			
		glPushMatrix();

		glTranslatef( 0.0,  0.0,  -1.0);
		glScalef( _textureScaleFactor * 0.8 , _textureScaleFactor * 0.8, _textureScaleFactor * 0.8 );

		// draw texture overlay
		_rendering._renderOpaque = false;
		_rendering._renderBlend = false;
		_rendering._renderTexturePreview = true;
		_modelController->Draw(&_rendering);

		if (_rendering._pickTextureMap) {
			// draw texture coordinate overlay
			_rendering._renderOpaque = false;
			_rendering._renderBlend = false;
			_rendering._renderTexturePreview = false;
			_rendering._renderTextureCoordinates = true;
			_modelController->Draw(&_rendering);
		}
		
		glPopMatrix();
			
	} else {
	
		// don't show it, start to show it
		_textureScaleStartTime = theTime;
		_textureScaleFactor = 0.0;
	}
	_lastPickedName = _rendering._pickedName;

}

void C3DModelPane::DrawHUD()
{	
	CPreferences *prefs = gApplication->preferences();
	if (!prefs->booleanForKey("showHUD"))
		return;
		
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_LIGHTING );
	glDisable( GL_DEPTH_TEST);
	glDisable( GL_BLEND);

	// choose color
	string backgroundColor = prefs->valueForKey("backgroundColor");
	if (backgroundColor == "white") {
		glColor3f(0.0, 0.0, 0.0);
	} else {
		glColor3f(1,1,1);
	}

	// switch to 2d orthographic mode for drawing text
	SDimension16 l_PortDim;
	GetFrameSize(l_PortDim);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, l_PortDim.width, l_PortDim.height, 0);
	glMatrixMode(GL_MODELVIEW);
	
	int lineHeight = 13;
	int x = 8;
	int y = l_PortDim.height - lineHeight * 2 - 5;
	
	glRasterPos2f(x, y);
	CShader *shader = _resources->selectedShader();
	if (shader)
		glPrint("%s", &shader->name);
	y += lineHeight;
	
	// animation info
	glRasterPos2f(x, y);
	if (_modelController) {
		const char *displayString = _modelController->DisplayString();
		if (displayString) {
			glRasterPos2f(x, y);
			glPrint(displayString);
		}
	}
	y += lineHeight;

	glRasterPos2f(x, y);
	glPrint("%d fps", _fps);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

}


#pragma mark -

// handle mouse down
void C3DModelPane::MouseDown(const SMouseDownEvent&	inMouseDown, const Point& p)
{
	C3DPane::MouseDown(inMouseDown, p);

	_m_x=p.h;
	_m_y=p.v;
	string tagName;
		
	if (_rendering._pickTag) {
		PickModel(_m_x, _m_y);
		
		CModelInstance *selectedModel = _model;
		tagName = _model->selectedModelTag(&_rendering, selectedModel);
		if (tagName.length() > 0) {
			selectedModel->detachModelAtTag(tagName);
			_modelController->Reset();
		}
	}
}

// handle mouse drag
void C3DModelPane::MouseDrag(const SMouseDownEvent&	inMouseDown, const Point& p)
{
	Boolean controlDown = (inMouseDown.macEvent.modifiers & controlKey) != 0;
	Boolean shiftDown = (inMouseDown.macEvent.modifiers & shiftKey) != 0;
	Boolean commandDown = (inMouseDown.macEvent.modifiers & cmdKey) != 0;

	int x=p.h, y=p.v;

	if (shiftDown) { 
	
		//scale model
		if (_model != nil && y != _m_y) {
			_rendering._zPos += (((float)(y - _m_y)) / 10.0f) * MOUSE_ZPOS_SCALE;
			if (_rendering._zPos < -1000.0f) _rendering._zPos=-1000.0f;
			if (_rendering._zPos > 1000.0f) _rendering._zPos=1000.0f;
		}

	} else if (controlDown) {
	
		//pan model
		if (_model != nil && ((x != _m_x) || (y != _m_y))) {
			_rendering._xPos += (((float)(x - _m_x))/10.0f) * MOUSE_PAN_SCALE;
			_rendering._yPos += -1.0f * (((float)(y - _m_y))/10.0f) * MOUSE_PAN_SCALE; //switch y orientation
		//	dprintf("_xPos %f, _yPos %f\n", _xPos, _yPos);
		}
	
	} else if (commandDown) {

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

	} else if (!controlDown) {

		//rotate model
		if (_model != nil && ((x != _m_x) || (y != _m_y))) {
			_rendering._rotAngleX += ((float)(y - _m_y)) * MOUSE_ROT_SCALE;
			_rendering._rotAngleY += ((float)(x - _m_x)) * MOUSE_ROT_SCALE;
			if (_rendering._rotAngleX > 360.0f) _rendering._rotAngleX -=360.0f;
			if (_rendering._rotAngleX <-360.0f) _rendering._rotAngleX +=360.0f;
			if (_rendering._rotAngleY> 360.0f) _rendering._rotAngleY-=360.0f;
			if (_rendering._rotAngleY<-360.0f) _rendering._rotAngleY+=360.0f;
		}
	}
		
	_m_x=x;
	_m_y=y;
	
	//dprintf("_rotAngleX  %f _rotAngleY %f _xPos %f _yPos %f _zPos %f\n", _rotAngleX , _rotAngleY, _xPos, _yPos, _zPos);

	if (_modelController)
		_modelController->Animate();
	Redraw();
}

void C3DModelPane::CreateDragEvent(const SMouseDownEvent &inMouseDown)
{	
	Boolean optionDown = (inMouseDown.macEvent.modifiers & optionKey) != 0;
	Boolean commandDown = (inMouseDown.macEvent.modifiers & cmdKey) != 0;

	if (_modelController && _model) {

		CTextureMap *map = new CTextureMap();
		_rendering._textureMap = map;
		DrawTexturePreview();
		map->EndMap();
		_rendering._textureMap = nil;

		if (_rendering._pickedName && _rendering._pickShader) {
			if (optionDown) {

				if (map->picture()) {
					_rendering._pickedName = _rendering._pickShader = false;
					CPict *pict = new CPict(map->picture());

					Rect theRect = map->bounds();
					CalcLocalFrameRect(theRect);

					LGWorld *gWorld = pict->CreateGWorld();
					CGLImage *glimage = new CGLImage(gWorld);
					

					CDragTask theDragTask (inMouseDown.macEvent, glimage, pict);
					theDragTask.DoDrag();
					delete glimage;
					delete gWorld;
					delete pict;
				}

			} else {
				_rendering._pickTag 
				= _rendering._pickTextureMap 
				= _rendering._pickShader = false;
				
				if (_rendering._pickedImage) {
					CDragTask theDragTask (inMouseDown.macEvent, _rendering._pickedImage);
					theDragTask.DoDrag();
				}
			}
		}
		delete map;
	}
}


void C3DModelPane::PickModel(int x, int y)	
{

   GLuint selectBuf[SELECT_BUFFER_SIZE];
   GLint hits;
   GLint viewport[4];
   
	if (!_model)
		return;

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
	gluPickMatrix ((GLdouble) x, (GLdouble) (viewport[3] - y),  10.0, 10.0, viewport);
	if (_height > 0)
		gluPerspective((double)60, (double)_width/(double)_height, NEAR_GL_PLANE, FAR_GL_PLANE);

	// position and render model
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix ();
	glTranslatef(_rendering._xPos, _rendering._yPos, _rendering._zPos);
	glScalef( (float)0.05 , (float)0.05, (float)0.05 );
	glRotatef( _rendering._rotAngleX , 1.0f ,  0.0f , 0.0f );
	glRotatef( _rendering._rotAngleY, 0.0f ,  1.0f , 0.0f );  
	
	glRotatef( _rendering._rotCorrectionX, 1.0f ,  0.0f , 0.0f );
	glRotatef( _rendering._rotCorrectionY, 0.0f ,  1.0f , 0.0f );
	glRotatef( _rendering._rotCorrectionZ, 0.0f ,  0.0f , 1.0f );
	
	glDepthMask( GL_TRUE);
	_rendering._renderOpaque = true;
	_rendering._renderBlend = true;
//glRotatef( -90.0f , 1.0f ,  0.0f , 0.0f );
	if (_modelController)
		_modelController->Draw(&_rendering);
		
	// clean up
	glPopMatrix ();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix ();
	glFlush ();
	glMatrixMode(GL_MODELVIEW);
	hits = glRenderMode (GL_RENDER);
	
	ProcessHits (hits, selectBuf);

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
C3DModelPane::PutOnDuty(
	LCommander*	 inNewTarget)
{
#pragma unused (inNewTarget)
	if (_modelController) {
		StringList names;
		_modelController->appendAnimationNames(&names);
		gAnimationMenu->SetItems(&names, _modelController);
	}
}

// ---------------------------------------------------------------------------
//	¥ TakeOffDuty
// ---------------------------------------------------------------------------
//	A Commander is going off duty
//
//	Subclasses should override this function if they wish to behave
//	differently when on duty than when off duty

void
C3DModelPane::TakeOffDuty()
{
	gAnimationMenu->RemoveAll();
}

// ---------------------------------------------------------------------------
//	¥ ObeyCommand													  [public]
// ---------------------------------------------------------------------------
//	Respond to commands

Boolean
C3DModelPane::ObeyCommand(
	CommandT	inCommand,
	void*		ioParam)
{
	Boolean		cmdHandled = true;
	
	switch (inCommand) {
	
		case cmd_lighting:
			_rendering._lighting = !_rendering._lighting;
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
		case cmd_previousskin:
			break;
		case cmd_nextskin:	
			break;
	
		case cmd_exportobj:	
			exportFormat(WAVEFRONT_OBJ_FORMAT);
			break;
			
		case cmd_exportdxf:	
			exportFormat(AUTOCAD_DXF_FORMAT);
			break;
		
		/* doesn't work	
		case cmd_exportPICT:
			C3DPane::ExportScreenDump();
			break;
		*/
						
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
C3DModelPane::FindCommandStatus(
	CommandT	inCommand,
	Boolean&	outEnabled,
	Boolean&	outUsesMark,
	UInt16&		outMark,
	Str255		outName)
{
	Boolean isMd3 = true;
	CPreferences *prefs = gApplication->preferences();
	
	switch (inCommand) {
	
	// export
		case cmd_exportmenu:
			outEnabled = _model && _model->modelClass()->canExportFormat(WAVEFRONT_OBJ_FORMAT);
			outEnabled |= _model && _model->modelClass()->canExportFormat(AUTOCAD_DXF_FORMAT);
			outUsesMark = false;
			break;
			
		case cmd_exportobj:	
			outEnabled = _model && _model->modelClass()->canExportFormat(WAVEFRONT_OBJ_FORMAT);
			outUsesMark = false;
			break;
			
		case cmd_exportdxf:	
			outEnabled = _model && _model->modelClass()->canExportFormat(AUTOCAD_DXF_FORMAT);
			outUsesMark = false;
			break;

	// rendering settings
		case cmd_lighting:
			outEnabled = true;
			outUsesMark = true;
			outMark = _rendering._lighting ? checkMark : noMark;
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
			outEnabled = isMd3;
			outUsesMark = isMd3;
			outMark = _rendering._showNormals ? checkMark : noMark;
			break;
		case cmd_showbox:	
			outEnabled = isMd3;
			outUsesMark = isMd3;
			outMark = _rendering._showBoneFrameBox ? checkMark : noMark;
			break;
		case cmd_previousskin:
			outEnabled = true;
			break;
		case cmd_nextskin:	
			outEnabled = true;
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

Boolean
C3DModelPane::HandleKeyPress(
	const EventRecord	&inKeyEvent)
{
	// force menu update
	LCommander::SetUpdateCommandStatus(true);
	
	// force redraw
	_drawInterval = 0;
	
	// setup GL context for commands that load images
	FocusDraw();
	
	if(_modelController)
		if(_modelController->HandleKeyPress(inKeyEvent)) 
			return true;
			
	CPreferences *prefs = gApplication->preferences();
	string command = prefs->commandForKeyEvent(inKeyEvent);

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
		_rendering._lighting = !_rendering._lighting;
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

	
	// scaling
	} else if (command == "sizeup") {
		_rendering._zPos += 0.1f;
		return true;
	} else if (command == "sizedown") {
		_rendering._zPos -= 0.1f;
		return true;
	
	// see how fast this thing really goes
	} else if (command == "hogCPU") {
		while(!Button()) {
			
			Boolean animating;
			if (_modelController)
				animating = _modelController->Animate();

			if (animating) 
				Redraw();
		}
		return true;
	}
	
	return false;
}


void C3DModelPane::exportFormat(ExportFormatT format)
{
	Boolean		saveOK = false;

	PP_StandardDialogs::LFileDesignator*	designator =
								new PP_StandardDialogs::LFileDesignator;
								

	LStr255 title;
	switch (format) {
		case WAVEFRONT_OBJ_FORMAT:
			title = "out.obj";
			break;
		case AUTOCAD_DXF_FORMAT:
			title = "out.dxf";
			break;
	}

	if (designator->AskDesignateFile(title)) {
		FSSpec outFSSpec;
		
		designator->GetFileSpec(outFSSpec);
		// designator->IsReplacing() && UsesFileSpec(outFSSpec)
		if (_model)
			_model->exportFormatToFile(format, outFSSpec);
	}
	delete designator;
} 

