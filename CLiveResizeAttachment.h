/*
File:		CLiveResizeAttachment.h
Author:		Richard Buckle, Sailmaker Software Ltd
			<mailto:richardb@sailmaker.co.uk>
			<http://www.sailmaker.co.uk/>
Version:	1.0.1
Purpose:	Attachment to add Carbon live resizing to windows
Status:		Public domain

Change history:
----- 1.0.1 -----
06/07/01	Put Execute and HandleResize inside #if PP_Target_Carbon block.
			Thanks to Bob Bradley <bob@chaoticsoftware.com>.
06/07/01 	HandleResize returns ASAP if the Carbon event
			doesn't have the kWindowBoundsChangeSizeChanged
			attribute.
			Thanks to Bob Bradley <bob@chaoticsoftware.com>.
06/07/01	Back to mWindow->UpdatePort().

----- 1.0 -----
27/6/01		HandleResize calls mWindow->Draw(nil)
			not mWindow->UpdatePort() for better performance.
*/

/*
*********** Licensing  *************
This code is placed "as is" in the public domain in the hope that it may be useful.
You may use and modify it free of charge, however it comes with no warranty whatsoever.
It is your responsibility to determine its fitness for use and no liability, whether 
express or implied, will be accepted.
I would however appreciate being advised of any bug fixes or enhancements that 
you make.

*********** Requirements  *************
PowerPlant as of CW7 early access or later.

*********** Usage *************
IMPORTANT: to use this class, you must first fix a "wart" in LEventHandler.
Replace LEventHandler::Remove() with the following:
--------------------------------------
void
LEventHandler::Remove()
{
	// JRB fixes to allow a handler to be removed before calling its destructor
	if (mHandlerRef != nil) {
		OSStatus err = ::RemoveEventHandler(mHandlerRef);
		Assert_(err == noErr );
		mHandlerRef = nil;
	}
}
--------------------------------------

After that, usage is pretty simple. 
1. Add the following files to your project:
	CLiveResizeAttachment.cp
	LEventHandler.cp
	
2. Copy or alias CLiveResizeAttachment.CTYP to Constructor's "Custom Types" folder.

3. Add the following call to your class registrations:
	RegisterClass_(CLiveResizeAttachment);

4. Either add a CLiveResizeAttachment to your window in Constructor or in your window's
	FinishCreateSelf() override, call:
	AddAttachment( new CLiveResizeAttachment(this) );
	
Hint: if all your windows descend from a common subclass of LWindow, you can use a 
FinishCreateSelf() override to do them all in one fell swoop. 

NB: CLiveResizeAttachment will not do anything to windows that don't have the 
	windAttr_Resizable attribute.
*/

#pragma once

#include <LAttachment.h>
#if PP_Target_Carbon
	#include <TEventHandler.h>
#endif	

class	CLiveResizeAttachment : public LAttachment {
public:
	enum { class_ID = FOUR_CHAR_CODE('RSiz') };

					CLiveResizeAttachment();
					CLiveResizeAttachment( LStream* inStream );
					CLiveResizeAttachment( LWindow* inWindow );
	virtual			~CLiveResizeAttachment();

#if PP_Target_Carbon
	virtual Boolean	Execute(MessageT inMessage, void* ioParam);
#endif

protected:
#if PP_Target_Carbon
	OSStatus		HandleResize(
								EventHandlerCallRef	inCallRef,
								EventRef			inEventRef);
	OSStatus		HandleClose(
								EventHandlerCallRef	inCallRef,
								EventRef			inEventRef);
#endif

private:
	LWindow*	mWindow;
#if PP_Target_Carbon
	TEventHandler<CLiveResizeAttachment>	mResizeHandler;
	TEventHandler<CLiveResizeAttachment>	mCloseHandler;
#endif
	
	// disallowed methods
	CLiveResizeAttachment( CLiveResizeAttachment& );
	CLiveResizeAttachment& operator= (const CLiveResizeAttachment&);
};