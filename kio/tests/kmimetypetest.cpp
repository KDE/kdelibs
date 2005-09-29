/*
 *  Copyright (C) 2005 David Faure   <faure@kde.org>
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

#include <kmimetype.h>
#include <kinstance.h>
#include <assert.h>
#include <stdlib.h>

static void checkIcon( const KURL& url, const QString& expectedIcon )
{
  QString icon = KMimeType::iconForURL( url );
  if ( icon == expectedIcon )
    qDebug( "icon for %s is %s, OK", url.prettyURL().latin1(), icon.latin1() );
  else {
    qDebug( "ERROR: icon for %s is %s, expected %s!", url.prettyURL().latin1(), icon.latin1(), expectedIcon.latin1() );
    exit(1);
  }
}

int main( int argc, char** argv )
{
  KInstance blah("kmimetypetest");

  // Obviously those tests will need to be fixed if we ever change the name of the icons
  // but at least they unit-test KMimeType::iconForURL.
  checkIcon( "file:/tmp/", "folder" );
  checkIcon( "file:/root/", "folder_locked" );
  checkIcon( "trash:/", "trashcan_full" ); // #100321
  checkIcon( "trash:/foo/", "folder" );

  return 0;
}
