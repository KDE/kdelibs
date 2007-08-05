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
#include <QtCore/QDir>
#include <kio/global.h>
#include <kmountpoint.h>

using namespace KIO;

SpeedTest::SpeedTest( const KUrl & url )
    : QObject(0)
{
    setObjectName( "speed" );
    Job *job = listRecursive( url );
    //Job *job = del( KUrl("file:" + QDir::currentPath()) ); DANGEROUS !
    connect(job, SIGNAL( result( KJob*)),
	    SLOT( finished( KJob* ) ));
    /*connect(job, SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList&)),
	    SLOT( entries( KIO::Job*, const KIO::UDSEntryList&)));
    */
}

void SpeedTest::entries(KIO::Job*, const UDSEntryList& list) {

    UDSEntryList::ConstIterator it = list.begin();
    const UDSEntryList::ConstIterator end = list.end();
    for (; it != end; ++it)
        kDebug() << (*it).stringValue( UDSEntry::UDS_NAME );
}


void SpeedTest::finished(KJob*) {
    kDebug() << "job finished";
    qApp->quit();
}

int main(int argc, char **argv) {

    KCmdLineArgs::init( argc, argv, "speedapp", 0, ki18n("SpeedApp"), "0.0", ki18n("A KIO::listRecursive testing tool"));

    KCmdLineOptions options;
    options.add("+[URL]", ki18n("the URL to list"));

    KCmdLineArgs::addCmdLineOptions( options );

    KApplication app;
// This is the real "speed test"
    // SpeedTest test( url );
    // app.exec();

// This is a test for KMountPoint and KIO::probably_slow_mounted etc.

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KUrl url;
    if ( args->count() == 1 )
      url = args->url(0);
    else
      url = QDir::currentPath();

    const KMountPoint::List mountPoints = KMountPoint::currentMountPoints();

    KMountPoint::Ptr mp = mountPoints.findByDevice(url.path());
    if (!mp) {
        kDebug() << "no mount point for device " << url << " found\n";
    } else
        kDebug() << mp->mountPoint() << " is the mount point for device " << url;

    mp = mountPoints.findByPath(url.path());
    if (!mp) {
        kDebug() << "no mount point for path " << url << " found\n";
    } else {
        kDebug() << mp->mountPoint() << " is the mount point for path " << url;
        kDebug() << url << " is probably " << (mp->probablySlow() ? "slow" : "normal") << " mounted\n";
    }

    url.setPath(QDir::homePath());

    mp = mountPoints.findByPath(url.path());
    if (!mp) {
        kDebug() << "no mount point for path " << url << " found\n";
    } else
        kDebug() << mp->mountPoint() << " is the mount point for path " << url;
}

#include "speed.moc"
