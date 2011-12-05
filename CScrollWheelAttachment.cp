/*
File:		CScrollWheelAttachment.cp
Contact:	Richard Buckle, Sailmaker Software Ltd
			<mailto:richardb@sailmaker.co.uk>
			<http://www.sailmaker.co.uk/>
Version:	1.0.1
Purpose:	Attachment to add Carbon scroll wheel support to windows
Status:		Public domain
*/

// See CScrollWheelAttachment.h for important usage notes

#include "CScrollWheelAttachment.h"
#include <LScrollerView.h>
#include <LScroller.h>

#if PP_Target_Carbon
//	EventHandlerUPP	TEventHandler<CScrollWheelAttachment>::sHandlerUPP = 0;
#endif

CScrollWheelAttachment::CScrollWheelAttachment()
	{
#if PP_Target_Carbon
	mScrollHandler.Install( ::GetApplicationEventTarget(), kEventClassMouse, kEventMouseWheelMoved, this, &HandleScroll );
#endif	
	}
	
CScrollWheelAttachment::~CScrollWheelAttachment()
	{}

#if PP_Target_Carbon
OSStatus
CScrollWheelAttachment::HandleScroll(
								EventHandlerCallRef	/*inCallRef*/,
								EventRef			inEvent)
	{
	EventMouseWheelAxis axis;
	::GetEventParameter( inEvent, kEventParamMouseWheelAxis, typeMouseWheelAxis,
							NULL, sizeof(axis), NULL, &axis );	
	
	SInt32 delta;
	::GetEventParameter( inEvent, kEventParamMouseWheelDelta, typeLongInteger,
							NULL, sizeof(delta), NULL, &delta );	

	Point mouseLoc;
	::GetEventParameter( inEvent, kEventParamMouseLocation, typeQDPoint,
							NULL, sizeof(mouseLoc), NULL, &mouseLoc );	

	DoScrollEvent( axis == kEventMouseWheelAxisY, delta, mouseLoc );

	return noErr;
	}

// We allow the scroll wheel to scroll background windows of our app.
// This is somewhat questionable UI, as it breaks the fundamental principle of
// consistency. "My Mom" might become rather confused by some background windows
// scrolling and others not.
// However, scrolling is such an innocuous, non-destructive process that I've
// decided to go with it.
// Maybe some day both Carbon and Cocoa apps will all support scrolling of 
// background windows.
void
CScrollWheelAttachment::DoScrollEvent(
									Boolean isVertical, 
									SInt32 delta, 
									Point mouseLoc)
	{
	WindowRef 		macHitWindow = nil;
	short			hitPart = ::FindWindow(mouseLoc, &macHitWindow);
	
	LWindow 		*hitWindow = nil;
	if (macHitWindow && (hitPart == inContent))
		{hitWindow = (LWindow::FetchWindowObject(macHitWindow));}
		
	if (hitWindow)
		{
		hitWindow->GlobalToPortPoint(mouseLoc);
		
		LPane*	hitPane = hitWindow->FindDeepSubPaneContaining(mouseLoc.h, mouseLoc.v);
		LView* viewToScroll = nil;
		
		while(hitPane)
			{
			LScroller* scroller = dynamic_cast<LScroller*>(hitPane);
			if (scroller)
				{
				viewToScroll = scroller->GetScrollingView();
				}
			else
				{
				LScrollerView* scrollerView = dynamic_cast<LScrollerView*>(hitPane);
				if (scrollerView)
					{
					viewToScroll = scrollerView->GetScrollingView();
					}
				}
			
			if( viewToScroll )
				{
				SPoint32 scrollUnit;
				viewToScroll->GetScrollUnit(scrollUnit);
				if (isVertical)
					{
					scrollUnit.h = 0;
					scrollUnit.v *= -delta * kScrollWheelFactor;
					}
				else
					{
					scrollUnit.h *= -delta * kScrollWheelFactor;
					scrollUnit.v = 0;
					}
				viewToScroll->ScrollPinnedImageBy(scrollUnit.h, scrollUnit.v, Refresh_Yes);
				break;
				}
			
			hitPane = hitPane->GetSuperView();
			}		
		}
	}
#endif // PP_Target_Carbon
