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

#ifndef _KDEBUG_H
#define _KDEBUG_H "$Id$"

/**
  * kdebug provides and advanced mechanism for displaying debugging
  * information to the user.  kdebug works like printf does but takes two
  * extra arguments.  If your program is compiled with NDEBUG defined,
  * kdebug is rendered useless and your debugging statements hidden from
  * the end-user's view.
  *
  *
  * The first argument required by kdebug is an "level".  A kdebug level
  * determines how important the message being displayed is.  There are
  * four available levels:
  *     KDEBUG_INFO
  *       for informal messages to the user such as: hey don't press that button
  *     KDEBUG_WARN
  *       for when something is just a little bit more significant happens.
  *     KDEBUG_ERROR
  *       for when a non fatal error has occured.
  *     KDEBUG_FATAL
  *       for when a horrific error has happened (1+1 != 2 for instance)
  * By pressing SHIFT-CTRL-F12 (C-Sh-f12) a small configuration dialog box
  * will pop up and let one assign actions to each debug level on an
  * application by application basis.
  *
  * The second argument is a kdebug "area".  This "area" tells kdebug
  * where the call to kdebug came from.  The "area" is an unsigned number
  * specified in kdebug.areas ($KDEDIR/share/config/kdebug.areas).  If
  * this number is zero, the application name (from KApplication) will be
  * used, and if that is not available, unknown will be used instead.
  *
  * The third (and final) required argument is a format string identical
  * to the kind used by printf.
  */
extern "C" {
void kdebug( unsigned short level, unsigned short area, const char* fmt, ... );
void kdebug_null (unsigned short level, unsigned short area, const char* fmt, ...);
};

#ifdef kdebug
#undef kdebug
#endif
#ifdef NDEBUG
#define kdebug kdebug_null
#endif

#ifndef NDEBUG
#define KASSERT( Cond, Level, Area, String ) { if( !Cond ) kdebug( (Level), (Area), (String) ); }
#define KASSERT1( Cond, Level, Area, String, Par1 ) { if( !Cond ) kdebug( (Level), (Area), (String), (Par1) ); }
#define KASSERT2( Cond, Level, Area, String, Par1, Par2 ) { if( !Cond ) kdebug( (Level), (Area), (String), (Par1), (Par2) ); }
#define KASSERT3( Cond, Level, Area, String, Par1, Par2, Par3 ) { if( !Cond ) kdebug( (Level), (Area), (String), (Par1), (Par2), (Par3) ); }
#define KASSERT4( Cond, Level, Area, String, Par1, Par2, Par3, Par4 ) { if( !Cond ) kdebug( (Level), (Area), (String), (Par1), (Par2), (Par3), (Par4) ); }
#define KASSERT5( Cond, Level, Area, String, Par1, Par2, Par3, Par4, Par5 ) { if( !Cond ) kdebug( (Level), (Area), (String), (Par1), (Par2), (Par3), (Par4), (Par5) ); }
#define KASSERT6( Cond, Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6 ) { if( !Cond ) kdebug( (Level), (Area), (String), (Par1), (Par2), (Par3), (Par4), (Par5), (Par6) ); }
#define KASSERT7( Cond, Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6, Par7 ) { if( !Cond ) kdebug( (Level), (Area), (String), (Par1), (Par2), (Par3), (Par4), (Par5), (Par6), (Par7) ); }
#define KASSERT8( Cond, Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6, Par7, Par8 ) { if( !Cond ) kdebug( (Level), (Area), (String), (Par1), (Par2), (Par3), (Par4), (Par5), (Par6), (Par7), (Par8) ); }
#define KASSERT9( Cond, Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6, Par7, Par8, Par9 ) { if( !Cond ) kdebug( (Level), (Area), (String), (Par1), (Par2), (Par3), (Par4), (Par5), (Par6), (Par7), (Par8), (Par9) ); }
#else
#define KASSERT( Cond, Level, Area, String )
#define KASSERT1( Cond, Level, Area, String, Par1 )
#define KASSERT2( Cond, Level, Area, String, Par1, Par2 )
#define KASSERT3( Cond, Level, Area, String, Par1, Par2, Par3 )
#define KASSERT4( Cond, Level, Area, String, Par1, Par2, Par3, Par4 )
#define KASSERT5( Cond, Level, Area, String, Par1, Par2, Par3, Par4, Par5 )
#define KASSERT6( Cond, Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6 )
#define KASSERT7( Cond, Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6, Par7 )
#define KASSERT8( Cond, Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6, Par7, Par8 )
#define KASSERT9( Cond, Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6, Par7, Par8, Par9 )
#endif

enum DebugLevels {
	KDEBUG_INFO=	0,
	KDEBUG_WARN=	1,
	KDEBUG_ERROR=	2,
	KDEBUG_FATAL=	3
};

#endif
