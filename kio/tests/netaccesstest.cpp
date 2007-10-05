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
#include <kio/job.h>
#include <QtCore/QFile>
#ifdef Q_WS_WIN
#include <QDir>
#endif
#include <kcmdlineargs.h>

int main(int argc, char **argv)
{
  KCmdLineArgs::init(argc,argv, "netaccesstest", 0, ki18n("netaccesstest"), 0);
  KApplication app;
  KUrl srcURL( "ftp://ftp.kde.org/pub/kde/README" );
#ifdef Q_WS_WIN
  KUrl tmpURL( "file://" + QDir::tempPath() + "/netaccesstest_README" );
#else
  KUrl tmpURL( "file:/tmp/netaccesstest_README" );
#endif

  for ( uint i = 0; i < 4 ; ++i ) {
    kDebug() << "file_copy";
    KIO::Job* job = KIO::file_copy(srcURL, tmpURL, -1, KIO::Overwrite);
    if ( !KIO::NetAccess::synchronousRun(job, 0) )
      kError() << "file_copy failed: " << KIO::NetAccess::lastErrorString() << endl;
    else {
      QFile f( tmpURL.path() );
      if (!f.open(QIODevice::ReadOnly))
        kFatal() << "Cannot open: " << f.fileName() << ". The error was: " << f.errorString();
      else {
        f.close();
      }
    }
  }

  return 0;
}

