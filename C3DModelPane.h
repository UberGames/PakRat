/* 
	C3DModelPane.h

	Author:			Tom Naughton
	Description:	<describe the C3DModelPane class here>
*/

#ifndef C3DModelPane_H
#define C3DModelPane_H

#include "C3DPane.h"
#include "CModelInstance.h"
#include "DragAndDrop.h"

class CModelController;
class CGLImage;

class C3DModelPane: 
	public C3DPane 
{
public:
	enum { class_ID = 'MdlP' };
	
	C3DModelPane();
	C3DModelPane(const SPaneInfo&	inPaneInfo,  LStr255& desc);
	C3DModelPane(LStream*	inStream);
	virtual ~C3DModelPane();
	
	void SetModel(CModelInstance *inModel);
	void SetModelController(CModelController *inController);
	void SetRotationCorrection(float x, float y, float z);
	void SetPosition(float x, float y, float z);
	void DrawTexturePreview();
	void CreateDragEvent(const SMouseDownEvent &inMouseDown);
	
	
	// LPeriodical methods
	
	virtual void		SpendTime(const EventRecord	&inMacEvent);
	
	// LCommander methods
	
	virtual void		PutOnDuty(
								LCommander*			inNewTarget);
								
	virtual void		TakeOffDuty();

	virtual Boolean		ObeyCommand(
							CommandT			inCommand,
							void*				ioParam = nil);
							
	virtual Boolean		HandleKeyPress(
							const EventRecord&	inKeyEvent);

	virtual void		FindCommandStatus(
							CommandT			inCommand,
							Boolean&			outEnabled,
							Boolean&			outUsesMark,
							UInt16&				outMark,
							Str255				outName);
	
	virtual void			ReshapeGL(int,int);		// change size
	
	void 					exportFormat(ExportFormatT format);

protected:

	virtual void 	PickModel(int x, int y);	
	virtual string 	selectedModelTag(CModelInstance *&selectedModel);
//	void 	ProcessHits (GLint hits, GLuint buffer[]);


	virtual void			MouseDown(const SMouseDownEvent&	inMouseDown, const Point&);// Mouse down event
	virtual void			MouseDrag(const SMouseDownEvent&	inMouseDown, const Point&);// mouse is being dragged

	virtual void 	SetupRendering();
	virtual void 	resetRendering(); 
	
	virtual void			DrawGL();				// GL Rendering
	virtual void 			DrawHUD();

	CModelController 	*_modelController;
	UInt32 				_lastPickedName;
	float 				_textureScaleFactor;
	float				_textureScaleStartTime;
	float 				_modelScaleFactor;
	float				_modelScaleStartTime;
	Boolean				_playedIntroSound;
	Boolean				_playedAnnounceSound;
	CWav				*_announceSound;
	
	int _m_x, _m_y; 			//keep track of mouse movements
};

#endif	// C3DModelPane_H


/*
	float _rotAngleX, _rotAngleY; //rotation angles
//	Point _mousePos;
*/

GLvoid glPrint(const char *fmt, ...);					// Custom GL "Print" Routine

