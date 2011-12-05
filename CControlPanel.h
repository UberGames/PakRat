/* 
	CControlPanel.h

	Author:			Tom Naughton
	Description:	<describe the CControlPanel class here>
*/

#ifndef CControlPanel_H
#define CControlPanel_H

#include <LCommander.h>

class LStaticText;

class CControlPanel : public LCommander, public LListener, public LBroadcaster, public LPeriodical
{
public:
	CControlPanel();
	virtual ~CControlPanel();

	void GetRotations(SInt32 &x, SInt32 &y, SInt32 &z);
	void GetTranslations(SInt32 &x, SInt32 &y, SInt32 &z);
	UInt32 GetRenderingMode();

	void SpendTime(const EventRecord	&/*inMacEvent*/);

	virtual void	ListenToMessage(MessageT inMessage, void* ioParam);
protected:

	LWindow*	_floatWindow;

	void UpdateValues(Boolean draw);
	void UpdateCaption(LStaticText *theCaption);
	void UpdateDebug();

	UInt32				_renderingMode;
	UInt32				_lastDraw;
	UInt32				_drawInterval; // in Ticks
	UInt32 				_totalMem;
	UInt32				_displayedTotalMem;
};

#endif	// CControlPanel_H
