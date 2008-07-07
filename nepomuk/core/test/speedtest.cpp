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
#include "../resourcemanager.h"
#include "../resource.h"
#include "../variant.h"

#include <Soprano/Model>
#include <Soprano/Statement>
#include <Soprano/StatementIterator>
#include <Soprano/Node>
#include <Soprano/LiteralValue>
#include <Soprano/QueryResultIterator>

#include <QtDBus/QtDBus>

#include <qtest_kde.h>

using namespace Soprano;
using namespace Nepomuk;


void SpeedTest::testKMetaData()
{
    return;
    const int TESTCNT = 10;

    Soprano::Model* rr = ResourceManager::instance()->mainModel();

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
    foreach( const Resource &r, rl ) {
        QVERIFY( rr->containsAnyStatement( Statement( r.resourceUri(), Node(), Node() ) ) );
    }

    t.start();

    for( int i = 0; i < TESTCNT; ++i ) {
        Resource( ns + QString("test") + QString::number(i) ).remove();
    }

    qDebug( "Removed %d resources in %d ms", TESTCNT, t.elapsed() );

    // check that there actually is nothing left
    foreach( const Resource &r, rl ) {
        QVERIFY( !rr->containsAnyStatement( Statement( r.resourceUri(), Node(), Node() ) ) );
    }
}


void SpeedTest::compareToPlain_data()
{
    QTest::addColumn<QUrl>( "uri" );
    QTest::addColumn<QString>( "identifier" );

    QTest::newRow( "simple" ) << QUrl( "http://test.kde.org/A" ) << "testA";
    QTest::newRow( "file" ) << QUrl( "file:///tmp" ) << "/tmp";
}

void SpeedTest::compareToPlain()
{
    QFETCH( QUrl, uri );
    QFETCH( QString,  identifier );

    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.nepomuk.services.nepomukstorage")) {
        QSKIP("nepomuk not running", SkipAll);
    }

    // first we add some data
    Soprano::Model* rr = ResourceManager::instance()->mainModel();

    QVERIFY( rr->addStatement( Statement( uri, QUrl(Resource::identifierUri()), LiteralValue(identifier) ) ) == Soprano::Error::ErrorNone );
    QVERIFY( rr->addStatement( Statement( uri, QUrl("http://test.kde.org/hasSomething"), LiteralValue(17) ) ) == Soprano::Error::ErrorNone );

    QTime t;
    t.start();

    // now read the thing with Nepomuk
    {
        Resource res( identifier );
        Variant val = res.property( "http://test.kde.org/hasSomething" );

        qDebug( "Read property with nepomuk and identifier in %d ms", t.elapsed() );

        QCOMPARE( val.toInt(), 17 );
    }

    t.start();

    {
        Resource res2( uri.toString() );
        Variant val2 = res2.property( "http://test.kde.org/hasSomething" );

        qDebug( "Read property with nepomuk and URI in %d ms", t.elapsed() );

        QCOMPARE( val2.toInt(), 17 );
    }

    // now read it with plain Soprano
    t.start();

    StatementIterator it = rr->listStatements( Statement( uri, QUrl("http://test.kde.org/hasSomething"), Node() ) );
    qDebug( "Read property with Soprano and URI in %d ms", t.elapsed() );

    QVERIFY( it.next() );
    QCOMPARE( it.current().object().literal().toInt(), 17 );


    t.start();
    QueryResultIterator qr = rr->executeQuery( QString( "select ?x where { ?r <%1> \"%2\"^^<http://www.w3.org/2001/XMLSchema#string> . "
                                                        "?r <http://test.kde.org/hasSomething> ?x . }" )
                                               .arg( Resource::identifierUri() )
                                               .arg( identifier ),
                                               Soprano::Query::QUERY_LANGUAGE_SPARQL );
    qDebug( "Read property with Soprano and identifier in %d ms", t.elapsed() );
    QVERIFY( qr.next() );
    QCOMPARE( qr.binding( "x" ).literal().toInt(),  17 );
}


QTEST_KDEMAIN(SpeedTest, NoGUI)

#include "speedtest.moc"
