/* 
	CControlPanel.cpp

	Author:			Tom Naughton
	Description:	<describe the CControlPanel class here>
*/


#include <LMultiPanelView.h>
#include <LPageController.h>
#include <LStaticText.h>
#include <LSlider.h>
#include <LProgressBar.h>

#include "CControlPanel.h"
#include "AppConstants.h"

CControlPanel::CControlPanel()
{
	_totalMem = 0;
	_renderingMode = 0;
	
	_floatWindow = LWindow::CreateWindow(PPob_ControlWindow, this);
	if (_floatWindow) {
	
	
		// Get the main multi panel view and load up all the panels
		LMultiPanelView*	mainMPV = dynamic_cast<LMultiPanelView*> (_floatWindow->FindPaneByID(item_ControlMainPanelView));
		ThrowIfNil_(mainMPV);
		mainMPV->CreateAllPanels();
		LPageController*	thePage = dynamic_cast<LPageController*> (_floatWindow->FindPaneByID(item_ControlPageController));
		ThrowIfNil_(thePage);
		thePage->AddListener(mainMPV);
		
		LView*	theRenderingView = dynamic_cast<LView*> (_floatWindow->FindPaneByID(kRenderingView));
		LView*	thePositionView = dynamic_cast<LView*> (_floatWindow->FindPaneByID(kPositionView));
		LView*	theAnimationView = dynamic_cast<LView*> (_floatWindow->FindPaneByID(kAnimationView));

		UReanimator::LinkListenerToControls(this, theRenderingView, PPob_RenderingView);
		UReanimator::LinkListenerToControls(this, thePositionView, PPob_PositionView);
		UReanimator::LinkListenerToControls(this, theAnimationView, PPob_AnimationView);
		
		UpdateValues(true);
		StartIdling();
		_lastDraw = 0;
		_drawInterval = 60; // in Ticks

		// position
		GDHandle dominantDevice = ::GetMainDevice();
		Rect screenRect = (**dominantDevice).gdRect;
		Rect windowBounds;
		_floatWindow->GetGlobalBounds(windowBounds);
		_floatWindow->MoveWindowTo(screenRect.right - (windowBounds.right - windowBounds.left + 13), ::GetMBarHeight() + 23 );
		

		_floatWindow->Show();
	}
}


CControlPanel::~CControlPanel()
{
}

// ---------------------------------------------------------------------------
// SpendTime
// ---------------------------------------------------------------------------
void CControlPanel::SpendTime(const EventRecord	&/*inMacEvent*/)
{
	UInt32 theTick = ::TickCount();
	if (theTick > _lastDraw + _drawInterval) {
		UpdateDebug();
		_lastDraw = theTick;
	}
}


void CControlPanel::UpdateDebug()
{
	long total, contig;
	
	LProgressBar *theBar = dynamic_cast<LProgressBar*> (_floatWindow->FindPaneByID(kMemoryIndicator));
	LStaticText *theCaption = dynamic_cast<LStaticText*> (_floatWindow->FindPaneByID(kMemoryField));
	
	::PurgeSpace(&total, &contig);

	if (_totalMem == 0) {
		_totalMem = total;
	}
	
	if (total != _displayedTotalMem) {
		LStr255 purgespace = LStr255(total/1024);
		LStr255 totalspace = LStr255((long)_totalMem/1024);
		LStr255 message;
		message += purgespace;
		message += "K free of ";
		message += totalspace;
		message += "K";
		theCaption->SetDescriptor((StringPtr)message);
		
		theBar->SetMinValue(0);
		theBar->SetMaxValue(_totalMem/1024);
		theBar->SetValue(_totalMem/1024 - total/1024);
		_displayedTotalMem = total;
	}
}

void CControlPanel::GetRotations(SInt32 &x, SInt32 &y, SInt32 &z)
{
	LSlider *theControl;

	theControl = dynamic_cast<LSlider*> (_floatWindow->FindPaneByID(kRotXSlider));
	x = theControl->GetValue();
	theControl = dynamic_cast<LSlider*> (_floatWindow->FindPaneByID(kRotYSlider));
	y = theControl->GetValue();
	theControl = dynamic_cast<LSlider*> (_floatWindow->FindPaneByID(kRotZSlider));
	z = theControl->GetValue();
}

void CControlPanel::GetTranslations(SInt32 &x, SInt32 &y, SInt32 &z)
{
	LSlider *theControl;

	theControl = dynamic_cast<LSlider*> (_floatWindow->FindPaneByID(kTransXSlider));
	x = theControl->GetValue();
	theControl = dynamic_cast<LSlider*> (_floatWindow->FindPaneByID(kTransYSlider));
	y = theControl->GetValue();
	theControl = dynamic_cast<LSlider*> (_floatWindow->FindPaneByID(kTransZSlider));
	z = theControl->GetValue();
}

void CControlPanel::UpdateValues(Boolean draw)
{
	LStaticText *theCaption;
	LSlider *theControl;
	
	theCaption = dynamic_cast<LStaticText*> (_floatWindow->FindPaneByID(kRotXField));
	theControl = dynamic_cast<LSlider*> (_floatWindow->FindPaneByID(kRotXSlider));
	theCaption->SetValue(theControl->GetValue());
	if (draw) UpdateCaption(theCaption);

	theCaption = dynamic_cast<LStaticText*> (_floatWindow->FindPaneByID(kRotYField));
	theControl = dynamic_cast<LSlider*> (_floatWindow->FindPaneByID(kRotYSlider));
	theCaption->SetValue(theControl->GetValue());
	if (draw) UpdateCaption(theCaption);

	theCaption = dynamic_cast<LStaticText*> (_floatWindow->FindPaneByID(kRotZField));
	theControl = dynamic_cast<LSlider*> (_floatWindow->FindPaneByID(kRotZSlider));
	theCaption->SetValue(theControl->GetValue());
	if (draw) UpdateCaption(theCaption);


	theCaption = dynamic_cast<LStaticText*> (_floatWindow->FindPaneByID(kTransXField));
	theControl = dynamic_cast<LSlider*> (_floatWindow->FindPaneByID(kTransXSlider));
	theCaption->SetValue(theControl->GetValue());
	if (draw) UpdateCaption(theCaption);

	theCaption = dynamic_cast<LStaticText*> (_floatWindow->FindPaneByID(kTransYField));
	theControl = dynamic_cast<LSlider*> (_floatWindow->FindPaneByID(kTransYSlider));
	theCaption->SetValue(theControl->GetValue());
	if (draw) UpdateCaption(theCaption);

	theCaption = dynamic_cast<LStaticText*> (_floatWindow->FindPaneByID(kTransZField));
	theControl = dynamic_cast<LSlider*> (_floatWindow->FindPaneByID(kTransZSlider));
	theCaption->SetValue(theControl->GetValue());
	if (draw) UpdateCaption(theCaption);

}

void CControlPanel::UpdateCaption(LStaticText *theCaption)
{
	if (theCaption->IsVisible()) {
		Rect frame;
		theCaption->FocusDraw();
		theCaption->CalcLocalFrameRect(frame);
		theCaption->ApplyForeAndBackColors();
		::EraseRect(&frame);
		theCaption->Draw(nil);
	}
}

UInt32 CControlPanel::GetRenderingMode()
{
	return _renderingMode;
}

void CControlPanel::ListenToMessage(MessageT	 inMessage, void* ioParam)
{
	LStaticText *theCaption;
	
	switch (inMessage) {
	
		case msg_BroadcasterDied:
			// maybe you should just
			StopListening();
			break;
		
		case msg_RenderModeChanged:
			_renderingMode = *((UInt32*)ioParam);
			BroadcastMessage(msg_RenderModeChanged, ioParam);
			break;
	}
	
	UpdateValues(true);
	BroadcastMessage(msg_ControllerChanged ,nil);
}
