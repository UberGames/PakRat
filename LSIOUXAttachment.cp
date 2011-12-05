// ===========================================================================
//	LSIOUXAttachment.cp			PowerPlant 2.0		©1995-1999 Metrowerks Inc.
// ===========================================================================
//
//	Attachment for using the SIOUX console window within a PP program

#ifdef PowerPlant_PCH
	#include PowerPlant_PCH
#endif

#include <LSIOUXAttachment.h>
#include <SIOUX.h>

PP_Begin_Namespace_PowerPlant


// ---------------------------------------------------------------------------
//	¥ LSIOUXAttachment						Default Constructor		  [public]
// ---------------------------------------------------------------------------

LSIOUXAttachment::LSIOUXAttachment()

	: LAttachment(msg_Event, true)
{
		// Configure SIOUX to run within a PP program
		
	// position
	GDHandle dominantDevice = ::GetMainDevice();
	Rect screenRect = (**dominantDevice).gdRect;
		
	SIOUXSettings.initializeTB		= false;
	SIOUXSettings.standalone		= false;
	SIOUXSettings.setupmenus		= false;
	SIOUXSettings.autocloseonquit	= true;
	SIOUXSettings.asktosaveonclose	= true;
	
	SIOUXSettings.toppixel = screenRect.bottom - 308;
	SIOUXSettings.leftpixel = 5;	/* The topleft window position (in pixels) ... */

	SIOUXSettings.columns = 64;

	
		// The other SIOUX settings have default values:
		//
		//		Field			Description						Default
		//		-----			-----------						-------
		//		showStatusLine	Draw status line?				false
		//		tabspaces		Number of spaces per tab		4
		//		columns			Char width of window			80
		//		rows			Lines of text in window			24
		//		toppixel		Window position    (0,0)		0
		//		leftpixel		Window position	   centers		0
		//		fontid			Font ID Number					monaco
		//		fontsize		Font size						9
		//		fontface		Font style						normal
		//
		// If you want to change these values, set the fields of the
		// global SIOUXSettings struct in your code before creating
		// the SIOUX Window (normally the before the first call to
		// printf or cout).
}


// ---------------------------------------------------------------------------
//	¥ ExecuteSelf													  [public]
// ---------------------------------------------------------------------------

void
LSIOUXAttachment::ExecuteSelf(
	MessageT	/* inMessage */,
	void*		ioParam)
{
		// Send the Event to SIOUX.
		//
		// If SIOUX doesn't handle the event,
		//		SIOUXHandleOneEvent() returns 0, so mExecuteHost is true,
		//		meaning that the event will be dispatched to PP
		//
		// If SIOUX does handle the event,
		//		SIOUXHandleOneEvent() returns a non-zero value, so
		//		mExecuteHost is false, meaning that the event will not
		//		be passed to PP.

   mExecuteHost = (SIOUXHandleOneEvent((EventRecord*) ioParam) == 0);
}


PP_End_Namespace_PowerPlant
