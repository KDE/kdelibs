// -*- c++ -*-
/***************************************************************************
 *   Copyright (C) 2004 by Richard Moore                                   *
 *   rich@kde.org                                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/

#ifndef KJSEMBED_GLOBAL_H
#define KJSEMBED_GLOBAL_H

#ifdef WIN32
# ifdef KJSEMBED_DLL
#  ifdef KJSEMBED_MAKE_DLL
#     define KJSEMBED_EXPORT __declspec(dllexport)
#  else
#     define KJSEMBED_EXPORT __declspec(dllimport)
#  endif // KJSEMBED_MAKE_DLL
# else
#  define KJSEMBED_EXPORT
# endif // KJSEMBED_DLL
#else
# define KJSEMBED_EXPORT
#endif // WIN32

#ifdef WIN32
#include <stdlib.h>
KJSEMBED_EXPORT void RedirectIOToConsole();
#endif

#ifndef WIN32
#include <ctype.h>
#include <stdlib.h>
char *itoa(int num, char *str, int radix = 10);
#endif

#include <QTextStream>
#include <kdemacros.h>
namespace KJSEmbed {
KDE_EXPORT QTextStream *conin();
KDE_EXPORT QTextStream *conout();
KDE_EXPORT QTextStream *conerr();
}

#ifndef QT_ONLY

/*
 * These are the normal definitions used when KDE is available.
 */

#include <kdebug.h>
#include <klocale.h>

#else // QT_ONLY

/*
 * These are the custom definitions used when we only have Qt.
 */

KJSEMBED_EXPORT QTextStream &kdDebug( int area=0 );
KJSEMBED_EXPORT QTextStream &kdWarning( int area=0 );

#ifndef NO_I18N
KJSEMBED_EXPORT QString i18n( const char *string );
#else
#define i18n(x) QString(x)
#endif // NO_I18N

#endif // QT_ONLY

#endif // KJSEMBED_GLOBAL_H

