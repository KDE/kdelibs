/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef _KDEBUG_H_
#define _KDEBUG_H_ "$Id$"

#include <qstring.h>

/**
  * kDebug provides and advanced mechanism for displaying debugging
  * information to the user.  kDebug works like printf does but takes one
  * or two extra arguments.  If your program is compiled with NDEBUG defined,
  * the calls are rendered useless and your debugging statements
  * hidden from the end-user's view. This doesn't apply to kDebugWarning,
  * kDebugError and kDebugFatal, since those should always been shown to the
  * user.
  *
  *
  * There are two families of functions. The first one allows variable
  * arguments, much like printf or the previous kdebug, and has the notion of
  * level (see below). The second one does not allow variable arguments
  * and only applies to debug info, but adds the filename and line number
  * before the message printed.
  * You can't have both at the same time, for technical reasons
  * (first faimly is functions, second one is macros, which can't have variable
  * arguments since we support non-gcc compilers)
  *
  *
  * A kDebug level determines how important the message being displayed is.
  * The first family of functions define four functions, one for each level :
  *     kDebugInfo
  *       for debug output
  *     kDebugWarning
  *       for when something strange or unexpected happened.
  *     kDebugError
  *       for when an error has occured, but the program can continue.
  *     kDebugFatal
  *       for when a horrific error has happened and the program must stop.
  *
  * The first (and optional) argument is a debug "area".  This "area" tells
  * kDebug where the call to kDebug came from.  The "area" is an unsigned number
  * specified in kdebug.areas ($KDEDIR/share/config/kdebug.areas).  If
  * this number is zero or unspecified, the instance (e.g. application) name
  * will be used.
  *
  * A separate program with a small configuration dialog box
  * will soon be written, to let one assign actions to each debug level on an
  * area by area basis.
  */

void kDebugInfo( const char* fmt, ... );
void kDebugInfo( unsigned short area, const char* fmt, ... );
void kDebugInfo( bool cond, unsigned short area, const char* fmt, ... );
void kDebugWarning( const char* fmt, ... );
void kDebugWarning( unsigned short area, const char* fmt, ... );
void kDebugWarning( bool cond, unsigned short area, const char* fmt, ... );
void kDebugError( const char* fmt, ... );
void kDebugError( unsigned short area, const char* fmt, ... );
void kDebugError( bool cond, unsigned short area, const char* fmt, ... );
void kDebugFatal( const char* fmt, ... );
void kDebugFatal( unsigned short area, const char* fmt, ... );
void kDebugFatal( bool cond, unsigned short area, const char* fmt, ... );

#ifdef NDEBUG
inline void kDebugInfo( const char* , ... ) {}
inline void kDebugInfo( unsigned short , const char* , ... ) {}
inline void kDebugInfo( bool , unsigned short , const char* , ... ) {}
// All the others remained defined, even with NDEBUG
#endif

/**
 * The second family of functions have more feature for debug output.
 * Those print file and line information, which kDebugInfo can't do.
 * And they also natively support QString.
 *
 * Applications using areas, or libraries :
 * use kDebugArea( area, my_char_* ) and kDebugStringArea( area, my_QString )
 *
 * Applications not using areas :
 * use kDebug( my_char_* ) and kDebugString( my_QString )
 *
 */
#ifdef NDEBUG
#define kDebugArea(area, a) ;
#define kDebugStringArea(area,a) ;
#define kDebug(a) ;
#define kDebugString(a) ;
#else
#define kDebugArea(area, a) kDebugInfo( area, "[%s:%d] %s", __FILE__, __LINE__, a )
#define kDebugStringArea(area, a) kDebugArea( area, a.ascii() )
#define kDebug(a) kDebugArea(0, a)
#define kDebugString(a) kDebugStringArea( 0, a )
#endif

// ----- OLD AND DEPRECATED
extern "C" {
  void kdebug( unsigned short level, unsigned short area, const char* fmt, ... );
}
enum DebugLevels { KDEBUG_INFO= 0, KDEBUG_WARN= 1, KDEBUG_ERROR= 2, KDEBUG_FATAL= 3 };
inline const char* debugString(const QString& a) { if (a.isNull()) return "<null>"; else return a.ascii(); }
#define KASSERT( Cond, Level, Area, String ) { if( !(Cond) ) kdebug( (Level), (Area), (String) ); }
// -----

/*  Script to help porting from kdebug to kDebug* :
#!/bin/sh
perl -pi -e 's/kdebug\([ ]*KDEBUG_INFO,/kDebugInfo\(/' $*
perl -pi -e 's/kdebug\([ ]*0,/kDebugInfo\(/' $*
perl -pi -e 's/kdebug\([ ]*KDEBUG_WARN,/kDebugWarning\(/' $*
perl -pi -e 's/kdebug\([ ]*KDEBUG_ERROR,/kDebugError\(/' $*
perl -pi -e 's/kdebug\([ ]*KDEBUG_FATAL,/kDebugFatal\(/' $*

(Then cvs update -A, check compile and cvs commit)
*/

#endif
