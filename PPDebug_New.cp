// ===========================================================================
//	PPDebug_New.cp		©1997-2000 Metrowerks Inc. All rights reserved
//	Portions ©1998-2000 John C. Daub. All rights reserved. <mailto:hsoi@hsoi.com>
//	Original Author: John C. Daub
// ===========================================================================
// Overrides for C++ memory allocation mechanisms, to work better with
// PowerPlant. Includes use of DebugNew and Spotlight as well (via the
// Debugging Classes).
//
// Contains support for: new, new[], new(nothrow), new[](nothrow), delete,
// and delete[]. Also contains support for the DebugNew versions of new
// that take file and line arguments. Both new_handler and terminate_handler
// implementations are provided as well.
//
// Hsoi 06-JUL-2000 - These implementations are not interrupt-safe. I don't
// know what the state of Metrowerks' runtimes are with regards to interrupt
// safety (mostly the propagation of exceptions if you were in an interrupt),
// but regardless, you probably shouldn't be allocating memory at interrupt
// time anyways (these don't pool, like LReentrantMemoryPool might allow you).
//
// NB: In the first 2 new implementations (new, and new(nothrow)... the "typical"
// ones) I explain what's going on in copious notes. Throughout all the 
// implementations, the only thing that varies is how failure/exceptions are
// handled/returned/propagated. So I figure it's best to explain them once,
// and you keep that around as reference. Keeps costs down. :-)

#include <PP_Debug.h>
#include <UOnyx.h>

#include "UMiscUtils.h"

#include <cstdlib>		// malloc/free


// ===========================================================================
//	¥ MW standard operator new configuration
// ===========================================================================

#undef new
#include <new>


// ===========================================================================
//	¥ DebugNew configuration
// ===========================================================================

#if PP_Debug && PP_DebugNew_Support
	#define DEBUG_NEW_NO_GLOBAL_OPERATORS 1		// suppress DebugNew's global
												// new/delete operators
	#include "DebugNew.cp"
#endif


// ===========================================================================


// ---------------------------------------------------------------------------
//	¥ AllowAllocation								[static]
// ---------------------------------------------------------------------------
//	Determines if it's ok to allocate memory or not. Typical use would
//	be to check the GrowZone, check for critical sections, etc. Returns
//	true if it's ok to proceed with the allocation.
//
//	Hsoi 30-JUN-2000 - Gave exception specification that it will never throw.

static bool	AllowAllocation() throw();

static bool
AllowAllocation() throw()
{
	return (not StInterruptSection::InInterruptSection() &&
				(StCriticalSection::InCriticalSection() ||
				not UMiscUtils::MemoryIsLow())
			);
}


#pragma mark -
#pragma mark === operator new ===

// ---------------------------------------------------------------------------
//	¥ operator new(size_t) throw(std::bad_alloc)
// ---------------------------------------------------------------------------
//	Overriden to test for low-memory conditions, and other things (like
//	helps you exist nicely with Spotlight, is "PowerPlant-savvy", etc.).
//
//	Hsoi 30-JUN-2000 - If this function returns normally, then you are
//	guaranteed (as much as *I* can guarantee) to have a valid block (of the
//	kind/size that you requested). If there are any errors for any reason,
//	this function will throw std::bad_alloc().
//
//	Do not call at interrupt time. This function is not interrupt safe.

void*
operator new(
	PP_STD::size_t	size) throw(PP_STD::bad_alloc)
{
		// Spotlight doesn't like things that DebugNew does, so we'll
		// temporarily disable Spotlight. If there's a problem, DebugNew
		// ought to catch/report it.
	StSpotlightDisable_();
	
		// Start with a nil pointer. 
	void* ptr = nil;
	
		// We check to see if we can allocate right now (i.e. not in a critical
		// section, not in a interrupt, and if we aren't low on memory). If
		// we can't, ptr will remain nil and hit the final validation-else-throw
		// at the end. If we can, then we do all our work within the if statement.
		// This is how we can strive to guarantee the return of a valid ptr else
		// throw.
		
	if (AllowAllocation()) {
	
			// The allocation function must throw std::bad_alloc if
			// there's a failure, so no need for us to handle that here...just
			// let it propagate normally.
			//
			// That is, if there is a failure in the allocation function, it's
			// ok -- in fact it's required -- for it to throw std::bad_alloc().
			// You cannot throw any other type of exception, nor can you report
			// errors any other way.
		
			// We'll use DebugNew, if conditions are right, else we'll do all
			// the allocations ourselves (this code is taken straight from
			// the CW Pro 6 DebugNew).
	#if PP_Debug && PP_DebugNew_Support && (DEBUG_NEW >= DEBUG_NEW_BASIC)
		ptr = DebugNewDoAllocate(size, 0, 0, PP_STD::malloc, false);
	#else
		{
			while (true) {
				ptr = (char*)PP_STD::malloc(size);
				if (ptr != 0) {
					break;
				}
				PP_STD::new_handler f = PP_STD::set_new_handler(0);
				PP_STD::set_new_handler(f);
				if (!f) {
					throw PP_STD::bad_alloc();
				}
				f();
			}
		}
	#endif
	}

		// If we've gotten this far, ptr should be valid (non-nil). If ptr is
		// nil, then we have one of 2 situations: 1. AllowAllocation() returned
		// false, 2. the allocation function returned NULL instead of throwing.
		//
		// 2 is easy: programmer error. 1 can be tricky. The purpose of
		// AllowAllocation() is to ensure we're allowed to allocate... not only
		// can this mean "do we have enough space for allocation" (i.e. not low
		// on memory), but it can also mean if we can do allocations in the first
		// place, like being in a critical section or interrupt.
		//
		// It's a limitation of this implementation that you can't allocate
		// in those sections... but still, we should handle the situation gracefully
		// should the situation arise. As noted in the preface comments of this
		// file, I don't know the state of MW's runtimes wrt exceptions at
		// interrupt time... but I'm going to assume it's safe, so we're going
		// to do what we're supposed to in the case of failure: throw std::bad_alloc.

	if (ptr == nil) {
		throw PP_STD::bad_alloc();
	}

		// If we get here, we know we have a valid pointer (well, as valid as we can
		// guarantee), and so we can rest assured in our return that we can proceed.

	return ptr;
}


// ---------------------------------------------------------------------------
//	¥ operator new(size_t, nothrow_t&) throw()
// ---------------------------------------------------------------------------
//	Overriden to test for low-memory conditions, and other things (like
//	helps you exist nicely with Spotlight, is "PowerPlant-savvy", etc.).
//
//	This is the nothrow_t variant.
//
//	Hsoi 30-JUN-2000 - If this function returns normally, then you are
//	guaranteed (as much as *I* can guarantee) to have a valid block (of the
//	kind/size that you requested). If there are any errors for any reason,
//	this function will throw std::bad_alloc().
//
//	Do not call at interrupt time. This function is not interrupt safe.


void*
operator new(
	PP_STD::size_t				size,
	const PP_STD::nothrow_t& 	nt) throw()
{
#pragma unused(nt)	// Keeps the compiler quiet.

		// Spotlight doesn't like things that DebugNew does, so we'll
		// temporarily disable Spotlight. If there's a problem, DebugNew
		// ought to catch/report it.
	StSpotlightDisable_();
	
		// Start with a nil pointer. 
	void* ptr = nil;
	
		// We check to see if we can allocate right now (i.e. not in a critical
		// section, not in a interrupt, and if we aren't low on memory). If
		// we can't, ptr will remain nil and hit the final validation-else-throw
		// at the end. If we can, then we do all our work within the if statement.
		// This is how we can strive to guarantee the return of a valid ptr else
		// throw.
	if (AllowAllocation()) {
	
			// The allocation function must throw std::bad_alloc if
			// there's a failure, so no need for us to handle that here...just
			// let it propagate normally.
			//
			// That is, if there is a failure in the allocation function, it's
			// ok -- in fact it's required -- for it to throw std::bad_alloc().
			// You cannot throw any other type of exception, nor can you report
			// errors any other way.
			
			// We'll use DebugNew, if conditions are right, else we'll do all
			// the allocations ourselves (this code is taken straight from
			// the CW Pro 6 DebugNew).
		
			// Here we must use a try/catch block. Because in the "throw" variant of
			// this function, we encourage the throwing of exceptions to signal
			// failure/error. So if we happen to get an exception, we swallow it
			// and return 0.
			
		try {
		#if PP_Debug && PP_DebugNew_Support && (DEBUG_NEW >= DEBUG_NEW_BASIC)
			ptr = DebugNewDoAllocate(size, 0, 0, PP_STD::malloc, false);
		#else
			{
				while (true) {
					ptr = (char*)PP_STD::malloc(size);
					if (ptr != 0) {
						break;
					}
					PP_STD::new_handler f = PP_STD::set_new_handler(0);
					PP_STD::set_new_handler(f);
					if (!f) {
						return 0;
					}
					try {
						f();
					} catch (...) {
						return 0;
					}
				}
			}
		#endif
		} catch (...) {
			return 0;
		}
	}

		// If something went wrong, that's ok... ptr will be nil, and we want
		// nil to be returned if there's failure.
	
	return ptr;
}


// ---------------------------------------------------------------------------
//	¥ operator new[](size_t) throw(bad_alloc)
// ---------------------------------------------------------------------------
//	See above versions for comments.

void*
operator new[](
	PP_STD::size_t	size) throw(PP_STD::bad_alloc)
{
	StSpotlightDisable_();
	
	void* ptr = nil;
	if (AllowAllocation()) {
		
	#if PP_Debug && PP_DebugNew_Support && (DEBUG_NEW >= DEBUG_NEW_BASIC)
		ptr = DebugNewDoAllocate(size, 0, 0, PP_STD::malloc, true);
	#else
		{
			while (true) {
				ptr = (char*)PP_STD::malloc(size);
				if (ptr != 0) {
					break;
				}
				PP_STD::new_handler f = PP_STD::set_new_handler(0);
				PP_STD::set_new_handler(f);
				if (!f) {
					throw PP_STD::bad_alloc();
				}
				f();
			}
		}
	#endif
	}

	if (ptr == nil) {
		throw PP_STD::bad_alloc();
	}
	
	return ptr;
}


// ---------------------------------------------------------------------------
//	¥ operator new[](size_t, nothrow_t&) throw()
// ---------------------------------------------------------------------------
//	See above versions for comments.

void*
operator new[](
	PP_STD::size_t				size,
	const PP_STD::nothrow_t&	nt) throw()
{
#pragma unused(nt)	// Keeps the compiler quiet

	StSpotlightDisable_();
	
	void* ptr = nil;
	if (AllowAllocation()) {
		try {
		#if PP_Debug && PP_DebugNew_Support && (DEBUG_NEW >= DEBUG_NEW_BASIC)
			ptr = DebugNewDoAllocate(size, 0, 0, PP_STD::malloc, true);
		#else
			{
				while (true) {
					ptr = (char*)PP_STD::malloc(size);
					if (ptr != 0) {
						break;
					}
					PP_STD::new_handler f = PP_STD::set_new_handler(0);
					PP_STD::set_new_handler(f);
					if (!f) {
						return 0;
					}
					try {
						f();
					} catch (...) {
						return 0;
					}
				}
			}
		#endif
		} catch (...) {
			return 0;
		}
	}
	
	return ptr;
}


#pragma mark === leaks new ===

#if PP_Debug && PP_DebugNew_Support && (DEBUG_NEW == DEBUG_NEW_LEAKS)

// Prototypes
void*	operator new(PP_STD::size_t size, const char* file, int line) throw(PP_STD::bad_alloc);
void*	operator new(PP_STD::size_t size, const PP_STD::nothrow_t& nt, const char* file, int line) throw();
void*	operator new[](PP_STD::size_t size, const char* file, int line) throw(PP_STD::bad_alloc);
void*	operator new[](PP_STD::size_t size, const PP_STD::nothrow_t& nt, const char* file, int line) throw();


// ---------------------------------------------------------------------------
//	¥ operator new(size_t, char*, int) throw(bad_alloc)
// ---------------------------------------------------------------------------
//	Only used when DebugNew is active. Same as new(size_t) but records
//	the location of the allocation.
//
//	See above versions for comments.

void*
operator new(
	PP_STD::size_t		size,
	const char*			file,
	int					line) throw(PP_STD::bad_alloc)
{
#pragma unused (file, line)	// Quiet down the non-debug builds.

	StSpotlightDisable_();
	
	void* ptr = nil;
	if (AllowAllocation()) {
		
	#if PP_Debug && PP_DebugNew_Support && (DEBUG_NEW >= DEBUG_NEW_BASIC)
		ptr = DebugNewDoAllocate(size, file, line, PP_STD::malloc, false);
	#else
		{
			while (true) {
				ptr = (char*)PP_STD::malloc(size);
				if (ptr != 0) {
					break;
				}
				PP_STD::new_handler f = PP_STD::set_new_handler(0);
				PP_STD::set_new_handler(f);
				if (!f) {
					throw PP_STD::bad_alloc();
				}
				f();
			}
		}
	#endif
	}

	if (ptr == nil) {
		throw PP_STD::bad_alloc();
	}
	
	return ptr;
}


// ---------------------------------------------------------------------------
//	¥ operator new(size_t, char*, int) throw()
// ---------------------------------------------------------------------------
//	Only used when DebugNew is active. Same as new(size_t) nothrow but records
//	the location of the allocation. You'll need to use the DebugNew NEW_NOTHROW
//	macro to invoke this method (instead of the usual NEW). See DebugNew.h
//	for more information.
//
//	See above versions for comments.


void*
operator new(
	PP_STD::size_t				size,
	const PP_STD::nothrow_t&	nt,
	const char*					file,
	int							line) throw()
{
#pragma unused (nt, file, line)	// Quiet down the non-debug builds.

	StSpotlightDisable_();
	
	void* ptr = nil;
	if (AllowAllocation()) {
		try {		
		#if PP_Debug && PP_DebugNew_Support && (DEBUG_NEW >= DEBUG_NEW_BASIC)
			ptr = DebugNewDoAllocate(size, file, line, PP_STD::malloc, false);
		#else
			{
				while (true) {
					ptr = (char*)PP_STD::malloc(size);
					if (ptr != 0) {
						break;
					}
					PP_STD::new_handler f = PP_STD::set_new_handler(0);
					PP_STD::set_new_handler(f);
					if (!f) {
						return 0;
					}
					try {
						f();
					} catch (...) {
						return 0;
					}
				}
			}
		#endif
		} catch (...) {
			return 0;
		}
	}
	
	return ptr;
}


// ---------------------------------------------------------------------------
//	¥ operator new[](size_t, char*, int) throw(bad_alloc)
// ---------------------------------------------------------------------------
//	Only used when DebugNew is active. Same as new[](size_t) but records
//	the location of the allocation.
//
//	See above versions for comments.


void*
operator new[](
	PP_STD::size_t	size,
	const char*		file,
	int				line) throw(PP_STD::bad_alloc)
{
#pragma unused (file, line)	// Quiet down the non-debug builds.

	StSpotlightDisable_();
	
	void* ptr = nil;
	if (AllowAllocation()) {

	#if PP_Debug && PP_DebugNew_Support && (DEBUG_NEW >= DEBUG_NEW_BASIC)
		ptr = DebugNewDoAllocate(size, file, line, PP_STD::malloc, true);
	#else
		{
			while (true) {
				ptr = (char*)PP_STD::malloc(size);
				if (ptr != 0) {
					break;
				}
				PP_STD::new_handler f = PP_STD::set_new_handler(0);
				PP_STD::set_new_handler(f);
				if (!f) {
					throw PP_STD::bad_alloc();
				}
				f();
			}
		}
	#endif
	}

	if (ptr == nil) {
		throw PP_STD::bad_alloc();
	}

	return ptr;
}


// ---------------------------------------------------------------------------
//	¥ operator new[](size_t, char*, int) throw()
// ---------------------------------------------------------------------------
//	Only used when DebugNew is active. Same as new[](size_t) nothrow but records
//	the location of the allocation. You'll need to use the DebugNew NEW_NOTHROW
//	macro to invoke this method (instead of the usual NEW). See DebugNew.h
//	for more information.
//
//	See above versions for comments.


void*
operator new[](
	PP_STD::size_t				size,
	const PP_STD::nothrow_t&	nt,
	const char*					file,
	int							line) throw()
{
#pragma unused (nt, file, line)	// Quiet down the non-debug builds.

	StSpotlightDisable_();
	
	void* ptr = nil;
	if (AllowAllocation()) {
	
		try {		
		#if PP_Debug && PP_DebugNew_Support && (DEBUG_NEW >= DEBUG_NEW_BASIC)
			ptr = DebugNewDoAllocate(size, file, line, PP_STD::malloc, true);
		#else
			{
				while (true) {
					ptr = (char*)PP_STD::malloc(size);
					if (ptr != 0) {
						break;
					}
					PP_STD::new_handler f = PP_STD::set_new_handler(0);
					PP_STD::set_new_handler(f);
					if (!f) {
						return 0;
					}
					try {
						f();
					} catch (...) {
						return 0;
					}
				}
			}
		#endif
		} catch (...) {
			return 0;
		}
	}

	return ptr;
}


#endif // PP_Debug && PP_DebugNew_Support && (DEBUG_NEW == DEBUG_NEW_LEAKS)


#pragma mark -
#pragma mark === operator delete ===

// ---------------------------------------------------------------------------
//	¥ operator delete
// ---------------------------------------------------------------------------
//	Overridden to disable Spotlight memory checks.

void
operator delete(
	void*	ptr) throw()
{
#if PP_Debug && PP_DebugNew_Support && (DEBUG_NEW >= DEBUG_NEW_BASIC)

	StSpotlightDisable_();
	DebugNewDoFree(ptr, PP_STD::free, false);

#else

	PP_STD::free(ptr);

#endif
}


// ---------------------------------------------------------------------------
//	¥ operator delete[]
// ---------------------------------------------------------------------------
//	Overridden to disable Spotlight memory checks.

void
operator delete[](
	void*	ptr) throw()
{
#if PP_Debug && PP_DebugNew_Support && (DEBUG_NEW >= DEBUG_NEW_BASIC)

	StSpotlightDisable_();
	DebugNewDoFree(ptr, PP_STD::free, true);

#else

	PP_STD::free(ptr);

#endif
}


// ---------------------------------------------------------------------------
//	¥ operator delete
// ---------------------------------------------------------------------------
//	Overridden to disable Spotlight memory checks.

void
operator delete(
	void*	ptr, const char*, int) throw()
{
#if PP_Debug && PP_DebugNew_Support && (DEBUG_NEW >= DEBUG_NEW_BASIC)

	StSpotlightDisable_();
	DebugNewDoFree(ptr, PP_STD::free, false);

#else

	PP_STD::free(ptr);

#endif
}


// ---------------------------------------------------------------------------
//	¥ operator delete[]
// ---------------------------------------------------------------------------
//	Overridden to disable Spotlight memory checks.

void
operator delete[](
	void*	ptr, const char*, int) throw()
{
#if PP_Debug && PP_DebugNew_Support && (DEBUG_NEW >= DEBUG_NEW_BASIC)

	StSpotlightDisable_();
	DebugNewDoFree(ptr, PP_STD::free, true);

#else

	PP_STD::free(ptr);

#endif
}


// ---------------------------------------------------------------------------
//	¥ operator delete
// ---------------------------------------------------------------------------
//	Overridden to disable Spotlight memory checks.

void
operator delete(
	void*						ptr,
	const PP_STD::nothrow_t&	nt) throw()
{
#pragma unused(nt)
#if PP_Debug && PP_DebugNew_Support && (DEBUG_NEW >= DEBUG_NEW_BASIC)

	StSpotlightDisable_();
	DebugNewDoFree(ptr, PP_STD::free, false);

#else

	PP_STD::free(ptr);

#endif
}


// ---------------------------------------------------------------------------
//	¥ operator delete[]
// ---------------------------------------------------------------------------
//	Overridden to disable Spotlight memory checks.

void
operator delete[](
	void*						ptr,
	const PP_STD::nothrow_t&	nt) throw()
{
#pragma unused(nt)
#if PP_Debug && PP_DebugNew_Support && (DEBUG_NEW >= DEBUG_NEW_BASIC)

	StSpotlightDisable_();
	DebugNewDoFree(ptr, PP_STD::free, true);

#else

	PP_STD::free(ptr);

#endif
}


#pragma mark -
#pragma mark === new_handler ===

// ---------------------------------------------------------------------------
//	¥ PP_NewHandler()
// ---------------------------------------------------------------------------
//	This is a new_handler function that attempts to free up memory
//	when new fails to find enough memory. See section 18.4.2.2 of the C++
//	standard, or section 14.4.5 in Stroustrup's 3rd edition.
//
//	It is not installed automatically. You must manually install it via
//	set_new_handler().

#include <LGrowZone.h>

void PP_NewHandler() throw(PP_STD::bad_alloc);	// prototype

void PP_NewHandler() throw(PP_STD::bad_alloc)
{
		// Ask the GrowZone to free up memory. Since at this point we
		// have no idea how much memory we need, consider any freed
		// memory as a positive result and return so new can try again.
		// If it failed to free any memory or there is no GrowZone at
		// all, throw bad_alloc to signal the failure.
		//
		// This does assume that once DoGrowZone() has been called and
		// if it returns a positive value, that subsequent calls to
		// DoGrowZone() should fail to return a positive value (at least
		// until after this allocation attempt has completed). Not only
		// does this assumption affect this function, but the method
		// used by new to allocate memory (malloc, my_alloc (New.cp),
		// NewPtr, etc.) could potentially call into the system (NewPtr)
		// and cause the GrowZone to again trigger. However the structuring
		// of the implementation of new (at least in the Metrowerks runtimes,
		// see New.cp) should utimately still end up in failure and none
		// of this should truly be an issue.
		//
		// Besides, if you hit a situation like that, you're probably in
		// deep kimchee anyways. ;-)
		
	LGrowZone*	theGZ = LGrowZone::GetGrowZone();
	if (theGZ != nil) {
		if (theGZ->DoGrowZone(max_Int32) > 0) {
			return;
		}
	}
	
	throw PP_STD::bad_alloc();
}