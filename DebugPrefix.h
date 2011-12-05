// ===========================================================================
//	DebugPrefix.h		©1999 Metrowerks Inc. All rights reserved.
// ===========================================================================

	// Bring in the project's precompiled header for the given target
	
#if __POWERPC__
	#include "DebugPrefixHeadersPPC++"		
#else
	#include "DebugPrefixHeaders68K++"
#endif

#define WE_USE_TABS 1
#define dprintf printf
#define false 0
#define true 1