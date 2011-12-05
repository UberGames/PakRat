/*
File:		CLiveResizeAttachment.cp
Author:		Richard Buckle, Sailmaker Software Ltd
			<mailto:richardb@sailmaker.co.uk>
			<http://www.sailmaker.co.uk/>
Version:	1.0.1
Purpose:	Attachment to add Carbon live resizing to windows
Status:		Public domain
*/

// IMPORTANT: see CLiveResizeAttachment.h for usage notes

#include "CLiveResizeAttachment.h"

#if PP_Target_Carbon
//	EventHandlerUPP	TEventHandler<CLiveResizeAttachment>::sHandlerUPP = 0;
#endif

//
CLiveResizeAttachment::CLiveResizeAttachment()

	: LAttachment(msg_FinishCreate),	// We only handle this message
	mWindow(0)
{}

//
CLiveResizeAttachment::CLiveResizeAttachment(LWindow* inWindow)
	: LAttachment(msg_FinishCreate),	// We only handle this message
	mWindow(inWindow)
{
	Execute(msg_FinishCreate, mWindow);
}

//
CLiveResizeAttachment::CLiveResizeAttachment(LStream*	inStream)
	: LAttachment(inStream),
	mWindow(0)
{
	mMessage = msg_FinishCreate;	// We only handle this message
}

//
CLiveResizeAttachment::~CLiveResizeAttachment()
{}


#if PP_Target_Carbon
//
Boolean
CLiveResizeAttachment::Execute(
	MessageT	inMessage,
	void*		ioParam)
{
	Boolean		executeHost = true;

	if (inMessage == msg_FinishCreate) 
	{
		mWindow = static_cast<LWindow*>(ioParam);
		if(	mWindow != nil
			&& mWindow->HasAttribute(windAttr_Resizable)
			&& UEnvironment::HasFeature(env_HasAquaTheme)
			)
		{
			WindowRef windowRef = mWindow->GetMacWindow();
			EventTargetRef target = ::GetWindowEventTarget(windowRef);
			mResizeHandler.Install( target, kEventClassWindow, kEventWindowBoundsChanged, this, &HandleResize );
			mCloseHandler.Install( target, kEventClassWindow, kEventWindowClosed, this, &HandleClose );
		
			OSStatus err = ::ChangeWindowAttributes( windowRef, kWindowLiveResizeAttribute, kWindowNoAttributes);
			SignalIfOSStatus_(err);
		}
		executeHost = mExecuteHost;
	}

	return executeHost;
}

//
OSStatus
CLiveResizeAttachment::HandleResize(
	EventHandlerCallRef	/* inCallRef */,
	EventRef			inEventRef)
{
	// We also get this event when the user is moving the window,
	// but we only need to intervene if the window size is changed.
	// If the window is merely moving then we should just return.
	UInt32			attributes;
	OSStatus result = ::GetEventParameter( 
								inEventRef, kEventParamAttributes, 
								typeUInt32, NULL, 
								sizeof( attributes ), NULL, 
								&attributes );

	if( (result == noErr) && (attributes & kWindowBoundsChangeSizeChanged) )
	{
		// OK, we have kWindowBoundsChangeSizeChanged so we need to draw
		Rect newBounds; // global co-ords
		result = ::GetEventParameter( inEventRef,
									  kEventParamCurrentBounds,
									  typeQDRectangle, nil,
									  sizeof(newBounds), nil,
									  &newBounds );
		if( result == noErr && mWindow != nil )
		{
			// there's no need to call mWindow->SendAESetBounds() because
			// LWindow::ClickInGrow() gets called when the resize is complete.
			// i.e the resize is still recordable.
			// Carbon Events are bliss!
			mWindow->DoSetBounds(newBounds);
			mWindow->UpdatePort();
		}
	}
	return result;
}

// Unfortunately, LWindow's destructor runs before LAttachable's, 
// which means that by the time CLiveResizeAttachment's destructor gets called
// ::DisposeWindow() has already been called by ~LWindow and the OS has already
// removed our handlers.
// If we don't deal with this, the destructors of our TEventHandler members
// will remove their handlers a second time, leading to a double-free and
// memory corruption.
// So, we listen for the kEventWindowClosed Carbon event and remove our handlers
// at that point.
// The fix to LEventHandler::Remove() documented in CLiveResizeAttachment.h
// then ensures that the destructors don't try to remove the handlers a second
// time.
//
// James Walker has suggested an alternative way of dealing with this,
// which is in CLiveResizeAttachment::Execute() to detach from the window
// and attach to one of its subpanes. After some thought, I have decided
// to stick with my method because it strikes me as more robust against
// future changes in PowerPlant.
OSStatus
CLiveResizeAttachment::HandleClose(
	EventHandlerCallRef	inCallRef,
	EventRef			inEventRef)
{
	Assert_(mWindow != nil);
	mResizeHandler.Remove();
	mCloseHandler.Remove();
	OSStatus result = ::CallNextEventHandler(inCallRef, inEventRef);
	return result;
}

#endif // PP_Target_Carbon


