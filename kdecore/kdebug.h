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
#define _KDEBUG_H

#ifdef kdebug
#undef kdebug
#endif
#ifdef NDEBUG
#define kdebug kdebug_null
#endif

void kdebug( unsigned short level, unsigned short area, const char* fmt, ... );
void kdebug_null (unsigned short level, unsigned short area, const char* fmt, ...);

#ifndef NDEBUG
#define KDEBUG( Level, Area, String ) kdebug( (Level), (Area), (String) );
#define KDEBUG1( Level, Area, String, Par1 ) kdebug( (Level), (Area), (String), (Par1) );
#define KDEBUG2( Level, Area, String, Par1, Par2 ) kdebug( (Level), (Area), (String), (Par1), (Par2) );
#define KDEBUG3( Level, Area, String, Par1, Par2, Par3 ) kdebug( (Level), (Area), (String), (Par1), (Par2), (Par3) );
#define KDEBUG4( Level, Area, String, Par1, Par2, Par3, Par4 ) kdebug( (Level), (Area), (String), (Par1), (Par2), (Par3), (Par4) );
#define KDEBUG5( Level, Area, String, Par1, Par2, Par3, Par4, Par5 ) kdebug( (Level), (Area), (String), (Par1), (Par2), (Par3), (Par4), (Par5) );
#define KDEBUG6( Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6 ) kdebug( (Level), (Area), (String), (Par1), (Par2), (Par3), (Par4), (Par5), (Par6) );
#define KDEBUG7( Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6, Par7 ) kdebug( (Level), (Area), (String), (Par1), (Par2), (Par3), (Par4), (Par5), (Par6), (Par7) );
#define KDEBUG8( Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6, Par7, Par8 ) kdebug( (Level), (Area), (String), (Par1), (Par2), (Par3), (Par4), (Par5), (Par6), (Par7), (Par8) );
#define KDEBUG9( Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6, Par7, Par8, Par9 ) kdebug( (Level), (Area), (String), (Par1), (Par2), (Par3), (Par4), (Par5), (Par6), (Par7), (Par8), (Par9) );
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
#define KDEBUG( Level, Area, String )
#define KDEBUG1( Level, Area, String, Par1 )
#define KDEBUG2( Level, Area, String, Par1, Par2 )
#define KDEBUG3( Level, Area, String, Par1, Par2, Par3 )
#define KDEBUG4( Level, Area, String, Par1, Par2, Par3, Par4 )
#define KDEBUG5( Level, Area, String, Par1, Par2, Par3, Par4, Par5 )
#define KDEBUG6( Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6 )
#define KDEBUG7( Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6, Par7 )
#define KDEBUG8( Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6, Par7, Par8 )
#define KDEBUG9( Level, Area, String, Par1, Par2, Par3, Par4, Par5, Par6, Par7, Par8, Par9 )
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
