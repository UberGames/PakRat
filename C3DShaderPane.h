/* 
	C3DShaderPane.h

	Author:			Tom Naughton
	Description:	<describe the C3DShaderPane class here>
*/

#ifndef C3DShaderPane_H
#define C3DShaderPane_H

#include "C3DPane.h"
#include "CModelInstance.h"
#include "DragAndDrop.h"

class CModelController;
class CGLImage;
class CImage;
class CShader;

class C3DShaderPane: 
	public C3DPane
{
public:
	enum { class_ID = 'SdrP' };
	
	C3DShaderPane();
	C3DShaderPane(const SPaneInfo&	inPaneInfo,  LStr255& desc);
	C3DShaderPane(LStream*	inStream);
	virtual ~C3DShaderPane();

	void 				SetShader(CPakStream *item, const char *inShader, int modifiers);
	
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
	virtual void		Click(	SMouseDownEvent 	&inMouseDown);
	

protected:

		virtual void		ClickSelf					(const SMouseDownEvent &inMouseDown);
		
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
		
		void 				HiliteDropArea(DragReference inDragRef);
		void				CreateDragEvent				(const SMouseDownEvent &inMouseDown);
		Boolean				CheckForOptionKey			(DragReference	inDragRef);
		Boolean				CheckIfViewIsAlsoSender		(DragReference	inDragRef);
		
		void				PasteItem					();
		CGLImage			*_image;
		CShader				*_shader;
		Boolean				_retainedShader;
		string 				_shaderName;

	virtual void 	SetupRendering();
	
	virtual void			DrawGL();				// GL Rendering

};

#endif	// C3DShaderPane_H

