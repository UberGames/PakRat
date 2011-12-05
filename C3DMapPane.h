/* 
	C3DMapPane.h

	Author:			Tom Naughton
	Description:	<describe the C3DMapPane class here>
*/

#ifndef C3DMapPane_H
#define C3DMapPane_H

#include "C3DPane.h"
#include "C3DModel.h"
#include "DragAndDrop.h"
#include "CBsp.h"

class CBsp;

class C3DMapPane: 
	public C3DPane
{
public:
	enum { class_ID = 'MpGL' };
	
	C3DMapPane();
	C3DMapPane(const SPaneInfo&	inPaneInfo,  LStr255& desc);
	C3DMapPane(LStream*	inStream);
	virtual ~C3DMapPane();
	
	// LListener methods
	
	void ListenToMessage(MessageT	 inMessage, void* ioParam);
	
	// LPeriodical methods
	
	void					SpendTime(const EventRecord	&inMacEvent);
	
	// LCommander methods
	
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
	
	void					SetMap(CBsp *bsp);
	void					SetLevelShot(CGLImage *shot);
	
protected:

	virtual void 	PickModel(int x, int y);	
	virtual void	MouseDown(const SMouseDownEvent&	inMouseDown, const Point&);// Mouse down event
	virtual void	MouseDrag(const SMouseDownEvent&	inMouseDown, const Point&);// mouse is being dragged

	virtual void 	SetupRendering();
	virtual void 	resetRendering(); 
	
	virtual void	DrawGL();				// GL Rendering
	virtual void 	DrawHUD();

	
	//keep track of mouse movements
	int 			_m_x, _m_y; 		
	CBsp 			*_map;	
	CGLImage 		*_levelShot;
	float 			_strafeVelocity;
	float 			_moveVelocity;
	float 			_lastTime;
	float 			_dragTime;
	
};

#endif	// C3DMapPane_H
