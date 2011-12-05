// ===========================================================================
//	UMiscUtils.h			©1997-2000 Metrowerks Inc. All rights reserved.
//	Original Author: John C. Daub
// ===========================================================================

#ifndef _H_UMiscUtils
#define _H_UMiscUtils
#pragma once

#include <PP_Prefix.h>

#if PP_Uses_Pragma_Import
	#pragma import on
#endif

namespace UMiscUtils {

	bool				System7Present();

// The Patch Manager is not available in Carbon
#if !PP_Target_Carbon

	bool				TrapAvailable(
								SInt16				inTrap);
	TrapType			GetTrapType(
								SInt16				inTrap);
	SInt16				NumToolboxTraps();

#endif // !PP_Target_Carbon
		
	bool				MemoryIsLow();
		
	bool				IsFontInstalled(
								ConstStr255Param	inName);	
}


// ===========================================================================
//	¥ StCriticalSection
// ===========================================================================
//	StCriticalSection is a stack-based object that marks a "critical section,"
//	i.e. a section that should not be allowed to fail because memory is
//	running low. Examples of appropriate uses of StCriticalSection are
//	commands that close windows or save files.

class StCriticalSection {

public:
						StCriticalSection()
							{
								mWasInCriticalSection	= sCriticalSection;
								sCriticalSection		= true;
							}
								
	virtual				~StCriticalSection()
							{
								sCriticalSection = mWasInCriticalSection;
							}

	static	bool		InCriticalSection()
							{
								return sCriticalSection;
							}
	

private:

			bool		mWasInCriticalSection;
	static	bool		sCriticalSection;
};


// ===========================================================================
//	¥ StInterruptSection
// ===========================================================================
//	StInterruptSection is a stack-based object that marks an "interrupt
//	section," i.e. a section of code that (could) execute at interrupt time.
//	Useful in memory allocation to prohibit allocation at interrupt time.

class StInterruptSection {

public:
						StInterruptSection()
							{
								mWasInInterruptSection	= sInterruptSection;
								sInterruptSection		= true;
							}
						
	virtual				~StInterruptSection()
							{
								sInterruptSection = mWasInInterruptSection;
							}

	static	bool		InInterruptSection()
							{
								return sInterruptSection;
							}
	

private:
			bool		mWasInInterruptSection;
	static	bool		sInterruptSection;

};


#if PP_Uses_Pragma_Import
	#pragma import reset
#endif

#endif // _H_UMiscUtils