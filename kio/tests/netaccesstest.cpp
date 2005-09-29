/*
 *  Copyright (C) 2004 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include <kapplication.h>
#include <kdebug.h>
#include <kurl.h>
#include <kio/netaccess.h>
#include <qfile.h>

int main(int argc, char **argv)
{
  KApplication app( argc, argv, "netaccesstest", true /* it _has_ a GUI ! */);

  KURL srcURL( "ftp://ftp.kde.org/pub/kde/README" );
  KURL tmpURL( "file:/tmp/netaccesstest_README" );

  for ( uint i = 0; i < 4 ; ++i ) {
    kdDebug() << "file_copy" << endl;
    if ( !KIO::NetAccess::file_copy(srcURL, tmpURL, -1, true, false, 0) )
      kdError() << "file_copy failed: " << KIO::NetAccess::lastErrorString() << endl;
    else {
      QFile f( tmpURL.path() );
      if (!f.open(IO_ReadOnly))
        kdFatal() << "Cannot open: " << f.name() << ". The error was: " << f.errorString() << endl;
      else {
        f.close();
      }
    }
  }

  return 0;
}

