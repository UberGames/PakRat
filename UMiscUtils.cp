// ===========================================================================
//	UMiscUtils.cp		©1997-2000 Metrowerks Inc. All rights reserved.
//	Original Author: John C. Daub
// ===========================================================================
//	A collection of various utilities.


#include "UMiscUtils.h"

#include <LGrowZone.h>

#include <Traps.h>
#include <Gestalt.h>
#include <Patches.h>
#include <Fonts.h>

bool StCriticalSection::sCriticalSection = false;
bool StInterruptSection::sInterruptSection = false;


// ---------------------------------------------------------------------------
//	¥ System7Present								[public]
// ---------------------------------------------------------------------------
//	This routine will determine if you're running at least System 7.0.0
//	It will also check to make sure that the Gestalt trap is available
//	(Gestalt came along with System 6.0.4).

bool
UMiscUtils::System7Present()
{
	bool	has7 = false;
	SInt32	sysVersion;

#if PP_Target_Carbon
	if ((::Gestalt(gestaltSystemVersion, &sysVersion) == noErr) &&
		(sysVersion >= 0x0700)) {
			has7 = true;
	}
#else	
	if (UMiscUtils::TrapAvailable(_Gestalt) &&
		(::Gestalt(gestaltSystemVersion, &sysVersion) == noErr) &&
		(sysVersion >= 0x0700)) {
			has7 = true;
	}
#endif
	
	return has7;
}


// The Patch Manager is not available in Carbon
#if !PP_Target_Carbon

// ---------------------------------------------------------------------------
//	¥ TrapAvailable									[public]
// ---------------------------------------------------------------------------
//	Given a certain trap, see if it actually exists

bool
UMiscUtils::TrapAvailable(
	SInt16	inTrap)
{
	TrapType type = UMiscUtils::GetTrapType(inTrap);
	
	if (type == ToolTrap) {
		inTrap &= 0x07FF;
		
		if (inTrap >= UMiscUtils::NumToolboxTraps()) {
			inTrap = _Unimplemented;
		}
	}
	
	return (::NGetTrapAddress(static_cast<UInt16>(inTrap), type) != 
				::NGetTrapAddress(_Unimplemented, ToolTrap));
}


// ---------------------------------------------------------------------------
//	¥ GetTrapType									[public]
// ---------------------------------------------------------------------------
//	Given a trap, see if it's a toolbox trap or an OS trap (this
//	information is stored in bit 11.  If set, toolbox, else OS)

TrapType
UMiscUtils::GetTrapType(
	SInt16	inTrap )
{
	return (((inTrap & 0x0800) > 0) ? ToolTrap : OSTrap);
}


// ---------------------------------------------------------------------------
//	¥ NumToolboxTraps								[public]
// ---------------------------------------------------------------------------
//	See how many traps we have

SInt16
UMiscUtils::NumToolboxTraps()
{
	if (::NGetTrapAddress(_InitGraf, ToolTrap) == 
			::NGetTrapAddress(0xAA6E, ToolTrap)) {
		return (0x0200);
	} else {
		return (0x0400);
	}
}

#endif // !PP_Target_Carbon

// ---------------------------------------------------------------------------
//	¥ MemoryIsLow									[public]
// ---------------------------------------------------------------------------
//	Determines if memory is low by checking to see if our growzone has
//	fired off or not. Performs the added check of ensuring we even have a
//	grow zone.

bool
UMiscUtils::MemoryIsLow()
{
	PP_PowerPlant::LGrowZone* growZone = PP_PowerPlant::LGrowZone::GetGrowZone();
	if (growZone == nil) {
		return false;
	}
	
		// Ask the grow zone if memory is low.
	return growZone->MemoryIsLow();
}


// ---------------------------------------------------------------------------
//	¥ IsFontInstalled								[public]
// ---------------------------------------------------------------------------
// Given a particular font name to look for, make sure it's
// installed.

bool
UMiscUtils::IsFontInstalled(
	ConstStr255Param	inFontName)
{
	bool	isInstalled = false; // Assume failure
	
		// Get the font number
	SInt16	theFontNum;
	::GetFNum(inFontName, &theFontNum);
	
		// Chances are pretty good that if theFontNum is non-zero that
		// it's installed, so short-circuit here and return as such
	
	if (theFontNum != 0) {
		isInstalled = true;
	} else {

			// The font is zero, that could mean it's not installed
			// and could also mean that the font is the system font.
			// Check for this by comparing the font strings.
		Str255 sysFontName;
		::GetFontName(0, sysFontName);
		
			// To be safe, check for an empty string. If this happens
			// to be the case, this could be a Bad Thing.
		if (sysFontName[0] == 0) {
			SignalStringLiteral_( "No system font found. This is probably bad..." );
		}
		
		isInstalled = ::EqualString(inFontName, sysFontName, false, false);
	}
	
	return isInstalled;
}
