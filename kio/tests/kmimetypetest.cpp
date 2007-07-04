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
#include <ktempdir.h>
#include <kprotocolinfo.h>
#include <qdir.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

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
  KURL url;

  // safely check a "regular" folder
  url.setPath( QDir::homeDirPath() );
  checkIcon( url, "folder" );
  
  // safely check a non-readable folder
  if (0 != geteuid()) { // can't do this test if we're root
    KTempDir tmp( QString::null, 0 );
    tmp.setAutoDelete( true );
    url.setPath( tmp.name() );
    checkIcon( url, "folder_locked" );
    chmod( QFile::encodeName( tmp.name() ), 0500 ); // so we can 'rm -rf' it
  }

  // safely check the trash folder
  if ( KProtocolInfo::isKnownProtocol( QString("trash") ) ) {
    checkIcon( "trash:/", "trashcan_full" ); // #100321
    checkIcon( "trash:/foo/", "folder" );
  }

  QString pdf;
  KMimeType::diagnoseFileName("foo.pdf", pdf);
  qDebug("extension: '%s'", pdf.latin1());
  assert(pdf == QString("*.pdf"));
  QString ps;
  KMimeType::diagnoseFileName("foo.ps", ps);
  qDebug("extension: '%s'", ps.latin1());
  assert(ps == QString("*.ps"));

  return 0;
}
