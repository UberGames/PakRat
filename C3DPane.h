/* 
	C3DPane.h

	Author:			Tom Naughton
	Description:	<describe the C3DPane class here>
*/

#ifndef C3DPane_H
#define C3DPane_H

#include "CShader.h"
#include "GLPane.h"
#include "C3DModel.h"

class CResourceManager;
class CGLImage;
class CModelInstance;

class C3DPane: 
	public GLPane,
	public LPeriodical, 
	public LCommander, 
	public LListener,
	public LDragAndDrop

{
public:
	C3DPane();
	C3DPane(const SPaneInfo&	inPaneInfo,  LStr255& desc);
	C3DPane(LStream*	inStream);
	virtual ~C3DPane();

	CModelInstance			*model() { return _model; };	
	void SetResourceManager(CResourceManager *inResources);
	virtual void MouseDown(const SMouseDownEvent&	inMouseDown, const Point& p);
	renderingAttributes_t *renderingAttributes() { return &_rendering; };
	void ExportScreenDump();
	void SetupScreenDump();
	LGWorld *ScreenDump();

	// LPeriodical methods
	
	virtual void		SpendTime(const EventRecord	&inMacEvent);
	
	// LListener methods
	
	void ListenToMessage(MessageT	 inMessage, void* ioParam);
	
	// LDragAndDrop methods

	virtual Boolean		ItemIsAcceptable			(DragReference	inDragRef,
													 ItemReference	inItemRef);
								
	virtual void		EnterDropArea				(DragReference	inDragRef,
													 Boolean		inDragHasLeftSender);
	
	virtual void		LeaveDropArea				(DragReference	inDragRef);
	
	virtual void		InsideDropArea				(DragReference	inDragRef);
	
	virtual void		ReceiveDragItem				(DragReference	inDragRef,
													 DragAttributes	inDragAttrs,
													 ItemReference	inItemRef,
													 Rect			&inItemBounds);
protected:

	virtual void 	SetupRendering();
	virtual void 	resetRendering(); 
	virtual string 	selectedModelTag(CModelInstance *&selectedModel);
	virtual void 	PickModel(int x, int y);	
	virtual void 	ProcessHits (GLint hits, GLuint buffer[]);
	virtual void 	applyDraggedTexture(CGLImage *texture);


	void 	DrawBitmapString(GLfloat x, GLfloat y, void *font, char *format,...);

	// FPS calculation
	UInt16 	_frameCount;
	UInt32	_frameCountTicks;
	UInt16  _fps;	
	UInt32	_lastDraw;
	UInt32	_drawInterval; // in Ticks
	float	_frameCountTime;
	renderingAttributes_t _rendering;

	CResourceManager 	*_resources;
	CModelInstance 		*_model;

};

#define SELECT_BUFFER_SIZE 512


#endif	// C3DPane_H
