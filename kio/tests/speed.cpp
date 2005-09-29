/*
 *  Copyright (C) 2002, 2003 Stephan Kulow <coolo@kde.org>
 *  Copyright (C) 2003       David Faure   <faure@kde.org>
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
#include <kdebug.h>
#include <kapplication.h>
#include <time.h>
#include "speed.h"
#include <kio/job.h>
#include <kcmdlineargs.h>
#include <qdir.h>
#include <kio/global.h>

using namespace KIO;

SpeedTest::SpeedTest( const KURL & url )
    : QObject(0, "speed")
{
    Job *job = listRecursive( url );
    //Job *job = del( KURL("file:" + QDir::currentDirPath()) ); DANGEROUS !
    connect(job, SIGNAL( result( KIO::Job*)),
	    SLOT( finished( KIO::Job* ) ));
    /*connect(job, SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList&)),
	    SLOT( entries( KIO::Job*, const KIO::UDSEntryList&)));
    */
}

void SpeedTest::entries(KIO::Job*, const UDSEntryList& list) {

    UDSEntryListConstIterator it=list.begin();
    for (; it != list.end(); ++it) {
      UDSEntry::ConstIterator it2 = (*it).begin();
        for( ; it2 != (*it).end(); it2++ ) {
            if ((*it2).m_uds == UDS_NAME)
              kdDebug() << ( *it2 ).m_str << endl;
        }
    }
}


void SpeedTest::finished(Job*) {
    kdDebug() << "job finished" << endl;
    kapp->quit();
}

static KCmdLineOptions options[] =
{
  { "+[URL]", "the URL to list", 0 },
  KCmdLineLastOption
};

int main(int argc, char **argv) {

    KCmdLineArgs::init( argc, argv, "speedapp", "A KIO::listRecursive testing tool", "0.0" );

    KCmdLineArgs::addCmdLineOptions( options );

    KApplication app;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KURL url;
    if ( args->count() == 1 )
      url = args->url(0);
    else
      url = "file:" + QDir::currentDirPath();

    kdDebug() << url.url() << " is probably " << (KIO::probably_slow_mounted(url.path()) ? "slow" : "normal") << " mounted\n";
    kdDebug() << url.url() << " is " << (KIO::manually_mounted(url.path()) ? "manually" : "system") << " mounted\n";
    QString mp = KIO::findDeviceMountPoint(url.path());
    if (mp.isEmpty()) {
        kdDebug() << "no mount point for device " << url.url() << " found\n";
    } else
        kdDebug() << mp << " is the mount point for device " << url.url() << endl;

    mp = KIO::findPathMountPoint(url.path());
    if (mp.isEmpty()) {
        kdDebug() << "no mount point for path " << url.url() << " found\n";
    } else
        kdDebug() << mp << " is the mount point for path " << url.url() << endl;
    // SpeedTest test( url );
    // app.exec();

    mp = KIO::findPathMountPoint(url.path());
    if (mp.isEmpty()) {
        kdDebug() << "no mount point for path " << url.url() << " found\n";
    } else
        kdDebug() << mp << " is the mount point for path " << url.url() << endl;
    // SpeedTest test( url );
    // app.exec();

    url.setPath(QDir::homeDirPath());

    mp = KIO::findPathMountPoint(url.path());
    if (mp.isEmpty()) {
        kdDebug() << "no mount point for path " << url.url() << " found\n";
    } else
        kdDebug() << mp << " is the mount point for path " << url.url() << endl;
    // SpeedTest test( url );
    // app.exec();

    mp = KIO::findPathMountPoint(url.path());
    if (mp.isEmpty()) {
        kdDebug() << "no mount point for path " << url.url() << " found\n";
    } else
        kdDebug() << mp << " is the mount point for path " << url.url() << endl;
    // SpeedTest test( url );
    // app.exec();

    if ( args->count() == 1 )
      url = args->url(0);
    else
      url = "file:" + QDir::currentDirPath();

    mp = KIO::findPathMountPoint(url.path());
    if (mp.isEmpty()) {
        kdDebug() << "no mount point for path " << url.url() << " found\n";
    } else
        kdDebug() << mp << " is the mount point for path " << url.url() << endl;
    // SpeedTest test( url );
    // app.exec();

}

#include "speed.moc"
