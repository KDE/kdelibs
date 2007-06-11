/*
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING.LIB" for the exact licensing terms.
 */

#include "speedtest.h"
#include "../kmetadata/kmetadata.h"

#include <knepomuk/knepomuk.h>
#include <knepomuk/services/rdfrepository.h>

#include <qtest_kde.h>

using namespace Nepomuk::Services;
using namespace Soprano;
using namespace Nepomuk::KMetaData;



void SpeedTest::testKMetaData()
{
    const int TESTCNT = 1000;

    RDFRepository rr( ResourceManager::instance()->serviceRegistry()->discoverRDFRepository() );

    QTime t;
    t.start();

    QList<Resource> rl;
    QString ns = "http://nepomuk-kde.semanticdesktop.org/testdata#";
    for( int i = 0; i < TESTCNT; ++i ) {
        Resource r( ns + QString("test") + QString::number(i) );
        r.setProperty( ns + QString("wurstprop"), 10 );
        rl.append( r );
    }

    qDebug( "Added %d resources in %d ms", TESTCNT, t.elapsed() );

    // check that everything has been saved properly
    foreach( Resource r, rl ) {
        QVERIFY( !rr.listStatements( defaultGraph(), Statement( QUrl( r.uri() ), Node(), Node() ) ).isEmpty() );
    }

    t.start();

    for( int i = 0; i < TESTCNT; ++i ) {
        Resource( ns + QString("test") + QString::number(i) ).remove();
    }

    qDebug( "Removed %d resources in %d ms", TESTCNT, t.elapsed() );

    // check that there actually is nothing left
    foreach( Resource r, rl ) {
        QVERIFY( rr.listStatements( defaultGraph(), Statement( QUrl( r.uri() ), Node(), Node() ) ).isEmpty() );
    }
}

QTEST_KDEMAIN(SpeedTest, NoGUI)

#include "speedtest.moc"
