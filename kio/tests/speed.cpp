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
#include <QDebug>
#include <QApplication>
#include <QUrl>
#include <time.h>
#include "speed.h"
#include <kio/job.h>
#include <QtCore/QDir>
#include <kio/global.h>
#include <kmountpoint.h>

using namespace KIO;

SpeedTest::SpeedTest( const QUrl & url )
    : QObject(0)
{
    setObjectName( "speed" );
    Job *job = listRecursive( url );
    connect(job, SIGNAL(result(KJob*)),
	    SLOT(finished(KJob*)));
    /*connect(job, SIGNAL(entries(KIO::Job*,KIO::UDSEntryList)),
	    SLOT(entries(KIO::Job*,KIO::UDSEntryList)));
    */
}

void SpeedTest::entries(KIO::Job*, const UDSEntryList& list) {

    UDSEntryList::ConstIterator it = list.begin();
    const UDSEntryList::ConstIterator end = list.end();
    for (; it != end; ++it)
        qDebug() << (*it).stringValue( UDSEntry::UDS_NAME );
}


void SpeedTest::finished(KJob*) {
    qDebug() << "job finished";
    qApp->quit();
}

int main(int argc, char **argv) {

    // "A KIO::listRecursive testing tool"

    //KCmdLineOptions options;
    //options.add("+[URL]", qi18n("the URL to list"));

    QApplication app(argc, argv);
// This is the real "speed test"
    // SpeedTest test( url );
    // app.exec();

// This is a test for KMountPoint and KIO::probably_slow_mounted etc.
    // TODO: SPLIT OUT!

    QUrl url;
    if (argc > 1)
      url = QUrl::fromUserInput(argv[1]);
    else
      url = QUrl::fromLocalFile(QDir::currentPath());

    const KMountPoint::List mountPoints = KMountPoint::currentMountPoints();

    KMountPoint::Ptr mp = mountPoints.findByDevice(url.toLocalFile());
    if (!mp) {
        qDebug() << "no mount point for device " << url << " found\n";
    } else
        qDebug() << mp->mountPoint() << " is the mount point for device " << url;

    mp = mountPoints.findByPath(url.toLocalFile());
    if (!mp) {
        qDebug() << "no mount point for path " << url << " found\n";
    } else {
        qDebug() << mp->mountPoint() << " is the mount point for path " << url;
        qDebug() << url << " is probably " << (mp->probablySlow() ? "slow" : "normal") << " mounted\n";
    }

    url.setPath(QDir::homePath());

    mp = mountPoints.findByPath(url.toLocalFile());
    if (!mp) {
        qDebug() << "no mount point for path " << url << " found\n";
    } else
        qDebug() << mp->mountPoint() << " is the mount point for path " << url;
}

