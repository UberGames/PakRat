// File Object Handler Utilities for the WASTE Text Engine
// Part of the WASTE Object Handler Library by Michael Kamprath, kamprath@kagi.com
// maintenance by John C. Daub
//


/* ------------------ GetFileIcon.h ------------------- */

#ifndef __GETFILEICON__
#define __GETFILEICON__

#ifndef __ICONS__
#include <Icons.h>
#endif
#ifndef __FINDER__
#include <Finder.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

pascal OSErr GetFileIcon( FSSpec *thing, IconSelectorValue iconSelector, Handle *theSuite);
Boolean IsSuiteEmpty( Handle theSuite );

#ifdef __cplusplus
}
#endif

#endif // __GETFILEICON__
