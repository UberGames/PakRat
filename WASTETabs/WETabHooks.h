/*
 *	WETabHooks.h
 *
 *	WASTE TABS PACKAGE
 *	Private (internal) interface
 *
 *	Copyright (c) 1993-1998 Marco Piovanelli
 *	All Rights Reserved
 *
 */


#ifndef _WASTE_
#include "WASTE.h"
#endif

enum {
	kTabSizeTag			=	'tbsz'
};

#ifdef __cplusplus
extern "C" {
#endif

pascal void _WETabDrawText(Ptr, SInt32, Fixed, JustStyleCode, WEReference);
pascal SInt32 _WETabPixelToChar(Ptr, SInt32, Fixed, Fixed *, WEEdge *, JustStyleCode, Fixed, WEReference);
pascal SInt16 _WETabCharToPixel(Ptr, SInt32, Fixed, SInt32, SInt16, JustStyleCode, SInt16, WEReference);
pascal StyledLineBreakCode _WETabLineBreak(Ptr, SInt32, SInt32, SInt32, Fixed *, SInt32 *, WEReference);

#ifdef __cplusplus
}
#endif
