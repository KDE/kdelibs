/* This file is part of the KDE libraries
    Copyright (C) 2004, 2005, 2006 Ian Reinhart Geiser <geiseri@kde.org>
    Copyright (C) 2004, 2005, 2006 Matt Broadstone <mbroadst@gmail.com>
    Copyright (C) 2004, 2005, 2006 Richard J. Moore <rich@kde.org>
    Copyright (C) 2004, 2005, 2006 Erik L. Bunce <kde@bunce.us>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kjseglobal.h"

#ifdef QT_ONLY
# include <QObject>
# include <cstdio>
#endif

#if defined(_WIN32) || defined(_WIN64)
# include <windows.h>
# include <fcntl.h>
# include <io.h>
# include <ios>
# include <QFile>
# include <QTextStream>
#endif

static QTextStream *kjsembed_err = 0L;
static QTextStream *kjsembed_in = 0L;
static QTextStream *kjsembed_out = 0L;

#if defined(_WIN32) || defined(_WIN64)
static QFile win32_stdin;
static QFile win32_stdout;
static QFile win32_stderr;

static const WORD MAX_CONSOLE_LINES = 500;

void RedirectIOToConsole() {
   int hConHandle;
   intptr_t lStdHandle;
   CONSOLE_SCREEN_BUFFER_INFO coninfo;
   AllocConsole();
   GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
   coninfo.dwSize.Y = MAX_CONSOLE_LINES;
   SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

   lStdHandle = (intptr_t)GetStdHandle(STD_INPUT_HANDLE);
   hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
   win32_stdin.open(hConHandle,QIODevice::ReadOnly);

   lStdHandle = (intptr_t)GetStdHandle(STD_OUTPUT_HANDLE);
   hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
   win32_stdout.open(hConHandle,QIODevice::WriteOnly);

   lStdHandle = (intptr_t)GetStdHandle(STD_ERROR_HANDLE);
   hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
   win32_stderr.open(hConHandle, QIODevice::WriteOnly);

   std::ios::sync_with_stdio();

}



#endif


QTextStream &consoleOut(  )
{
   return *KJSEmbed::conout();
}

QTextStream &consoleError( )
{
   return *KJSEmbed::conerr();
}

QTextStream &consoleIn( )
{
   return *KJSEmbed::conin();
}

#ifdef QT_ONLY
QTextStream &kdDebug( int area )
{
#ifndef QT_DEBUG
   return consoleError() << "DEBUG: (" << area << ") ";
#else
   return consoleOut();
#endif

}

QTextStream &kdWarning( int area )
{
   return consoleOut() << "WARNING: (" << area << ") ";
}

QString i18n( const char *string )
{
	return QCoreApplication::translate( "KJSEmbed", string, "qjsembed string");
}

#endif

QTextStream *KJSEmbed::conin()
{
   if ( !kjsembed_in ) {
#ifdef _WIN32
	   kjsembed_in = new QTextStream( &win32_stdin );
#else
	   kjsembed_in = new QTextStream( stdin, QIODevice::ReadOnly );
#endif
   }
   return kjsembed_in;
}

QTextStream *KJSEmbed::conout()
{
   if ( !kjsembed_out ) {
#ifdef _WIN32
	   kjsembed_out = new QTextStream( &win32_stdout  );
#else
	   kjsembed_out = new QTextStream( stdout, QIODevice::WriteOnly );
#endif
   }
   return kjsembed_out;

}

QTextStream *KJSEmbed::conerr()
{
   if ( !kjsembed_err ) {
#ifdef _WIN32
	   kjsembed_err = new QTextStream( &win32_stderr  );
#else
	   kjsembed_err = new QTextStream( stderr, QIODevice::WriteOnly );
#endif
   }
   return kjsembed_err;
}

//kate: indent-spaces on; indent-width 4; replace-tabs on; indent-mode cstyle;
