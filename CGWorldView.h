// ===========================================================================
// CGWorldView.h
// ===========================================================================
/* This class implements a view that mirrors the contents of a GWorld. */
	
	
#pragma once
#include <LView.h>
#include <UGWorld.h>

#include "DragAndDrop.h"
#include <LClipboard.h>

const PaneIDT kGWPaneID = 'Gwld';	// PaneID in Example Window
const PaneIDT kGWorldWindowTimed = 20023;
const PaneIDT kGWorldWindowListener = 20024;

class CImage;


class CGWorldView : public LView, public LDragAndDrop, public LCommander
{
	public:
		CGWorldView();
		CGWorldView(LStream *inStream);
		~CGWorldView(void);
		void				SetMyGWorld(LGWorld *inGW);
		void 				SetImage(CImage *inImage);
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
		
		void				DrawSelf();
		LGWorld				*mGWorld;
		
	private:
	
		void				CreateDragEvent				(const SMouseDownEvent &inMouseDown);
		Boolean				CheckForOptionKey			(DragReference	inDragRef);
		Boolean				CheckIfViewIsAlsoSender		(DragReference	inDragRef);
		
		void				PasteItem					();
		
		CImage				*_image;
};

class CGWorldViewTimed : public CGWorldView, public LPeriodical {
	public:
		enum { class_ID = 'CGwT' };
		
		CGWorldViewTimed();
		CGWorldViewTimed(LStream *inStream);
		~CGWorldViewTimed(void);
		void				SetMyGWorld(LGWorld *inGW);
		void				SpendTime(const EventRecord	&inMacEvent);
		void				SetDrawInterval(short inSeconds) {mDrawInterval = inSeconds*60;};
		
	protected:
	
		UInt32				mLastDraw;
		UInt32				mDrawInterval; // in Ticks

};


const MessageT	msg_JustChanged	= 'JtCh';	// this is the Message to send in order to get the Pane to draw a new copy of the GWorld

class CGWorldViewListener : public CGWorldView, public LListener {
	public:
		enum { class_ID = 'CGwL' };
		
		CGWorldViewListener();
		CGWorldViewListener(LStream *inStream);
		~CGWorldViewListener(void);
		void				ListenToMessage(
								MessageT		inMessage,
								void			*ioParam);
};
