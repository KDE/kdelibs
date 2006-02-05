//#ifdef    don't do this, this file is supposed to be included
//#define   multiple times

/* Usage:

 If you get compile errors caused by Win32-API includes (the line
 where first error appears contains word like None, Unsorted,
 Below, etc.), put #include <fixwinh.h> in the .cpp file 
 (not .h file!) between the place where Win32-API headers are
 included and the place where the file with compile
 error is included (or the place where the compile error
 in the .cpp file occurs).
 
 This file remaps Win32-API #defines to const variables or
 inline functions. The side effect may be that these
 symbols may now refer to different variables
 (e.g. if Win32-API #defined OPTIONAL, after this file
 is included OPTIONAL would no longer be Win32's
 OPTIONAL, but enum tagStatus OPTIONAL instead). At this time,
 there's no conflict known that could cause problems.

 The original Win32-API symbols are still accessible
 (e.g. for None) as W::OPTIONAL, WOPTIONAL, and also still
 OPTIONAL, unless name lookup finds different OPTIONAL
 first (in the current class, etc.)

 Use 'Unsorted', 'Bool' and 'index' as templates.

*/

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>

#ifdef __cplusplus

namespace W
{

// template --->
// Affects: Should be without side effects.
#ifdef Unsorted
#ifndef FIXX11H_Unsorted
#define FIXX11H_Unsorted
const int XUnsorted = Unsorted;
#undef Unsorted
const int Unsorted = XUnsorted;
#endif
#undef Unsorted
#endif
// template <---

// template --->
// Affects: Should be without side effects.
#ifdef Bool
#ifndef FIXX11H_Bool
#define FIXX11H_Bool
typedef Bool XBool;
#undef Bool
typedef XBool Bool;
#endif
#undef Bool
#endif
// template <---

// template --->
// Affects: Should be without side effects.
#ifdef index
#ifndef FIXX11H_index
#define FIXX11H_index
inline
char* Xindex( const char* s, int c )
    {
    return index( s, c );
    }
#undef index
inline
char* index( const char* s, int c )
    {
    return Xindex( s, c );
    }
#endif
#undef index
#endif
// template <---

#if 0
// OPTIONAL is defined '#define OPTIONAL' - so what to do here?
#ifdef OPTIONAL
#ifndef FIXWINH_OPTIONAL
#define FIXWINH_OPTIONAL
#undef OPTIONAL
#endif
#endif

#ifdef ABSOLUTE
#ifndef FIXWINH_ABSOLUTE
#define FIXWINH_ABSOLUTE
const int WABSOLUTE = ABSOLUTE;
#undef ABSOLUTE
const int ABSOLUTE = WABSOLUTE;
#endif
#undef ABSOLUTE
#endif

#ifdef RELATIVE
#ifndef FIXWINH_RELATIVE
#define FIXWINH_RELATIVE
const int WRELATIVE = RELATIVE;
#undef RELATIVE
const int RELATIVE = WRELATIVE;
#endif
#undef RELATIVE
#endif

#endif	// 0

#ifdef FOCUS_EVENT
# ifndef FIXWINH_FOCUS_EVENT
#  define FIXWINH_FOCUS_EVENT
   const int WFOCUS_EVENT = FOCUS_EVENT;
#  undef FOCUS_EVENT
   const int FOCUS_EVENT = WFOCUS_EVENT;
# endif
# undef FOCUS_EVENT
#endif

#ifdef CreateEvent
# ifndef FIXWINH_CreateEvent
#  define FIXWINH_CreateEvent
   inline
   HANDLE WCreateEvent( LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCTSTR lpName )
   {
    return CreateEvent( lpEventAttributes, bManualReset, bInitialState, lpName );
   }
#  undef CreateEvent
   inline
   HANDLE CreateEvent( LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCTSTR lpName )
   {
    return WCreateEvent( lpEventAttributes, bManualReset, bInitialState, lpName );
   }
# endif	// FIXWINH_CreateEvent
# undef CreateEvent
#endif	// CreateEvent

}
using namespace W;

#endif	// cplusplus
#endif	// defined(_WIN32) || defined(_WIN64)