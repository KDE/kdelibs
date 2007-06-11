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

#include "resourcetest.h"
#include <kmetadata/kmetadata.h>

#include <knepomuk/knepomuk.h>
#include <knepomuk/services/rdfrepository.h>

#include <kdebug.h>
#include <qtest_kde.h>

using namespace Nepomuk::KMetaData;
using namespace Soprano;
using namespace Nepomuk::Services;


void ResourceTest::testResourceStates()
{
    QString someUri = ResourceManager::instance()->generateUniqueUri();

    Resource r1( someUri );
    Resource r2( someUri );

    QCOMPARE( r1, r2 );

    QVERIFY( r1.isValid() );
    QVERIFY( !r1.exists() );

    r1.setProperty( "someinvaliduri", 12 );

    QCOMPARE( r1, r2 );
    QVERIFY( r1.exists() );
}


void ResourceTest::testResourceRemoval()
{
    Resource res( "testi" );
    res.setProperty( "http://nepomuk.test.org/foo/bar",  "foobar" );

    QVERIFY( !res.uri().isEmpty() );

    Nepomuk::Services::RDFRepository rr( ResourceManager::instance()->serviceRegistry()->discoverRDFRepository() );

    QVERIFY( rr.contains( defaultGraph(), Statement( QUrl(res.uri()), Node(), Node() ) ) );

    res.remove();

    QVERIFY( !res.exists() );

    QVERIFY( !rr.contains( defaultGraph(), Statement( QUrl(res.uri()), Node(), Node() ) ) );

    //
    // test recursive removal
    //
    Resource res2( "testi2" );

    res.setProperty( "http://nepomuk.test.org/foo/bar2", res2 );

    QVERIFY( res.exists() );
    QVERIFY( res2.exists() );

    QVERIFY( rr.contains( defaultGraph(), Statement( QUrl(res.uri()), QUrl("http://nepomuk.test.org/foo/bar2"), Node(QUrl(res2.uri())) ) ) );

    res2.remove();

    QVERIFY( res.exists() );
    QVERIFY( !res2.exists() );

    QVERIFY( !rr.contains( defaultGraph(), Statement( QUrl(res.uri()), QUrl("http://nepomuk.test.org/foo/bar2"), Node(QUrl(res2.uri())) ) ) );
}


void ResourceTest::testProperties()
{
    RDFRepository rr( ResourceManager::instance()->serviceRegistry()->discoverRDFRepository() );
    QString r1Uri, r2Uri;

    {
        Resource r1( "testi" );
        Resource r2( "testi2" );

        r1.setProperty( "http://nepomuk.test.org/int", 17 );
        r1.setProperty( "http://nepomuk.test.org/bool1", true );
        r1.setProperty( "http://nepomuk.test.org/bool2", false );
        r1.setProperty( "http://nepomuk.test.org/double", 2.2 );
        r1.setProperty( "http://nepomuk.test.org/string", "test" );
        r1.setProperty( "http://nepomuk.test.org/date", QDate::currentDate() );
        r1.setProperty( "http://nepomuk.test.org/Resource", r2 );

        r1Uri = r1.uri();
        r2Uri = r2.uri();
    }

    {
        Resource r1( r1Uri );
        Resource r2( r2Uri );

        QVERIFY( r1.hasProperty( "http://nepomuk.test.org/int" ) );
        QVERIFY( r1.hasProperty( "http://nepomuk.test.org/bool1" ) );
        QVERIFY( r1.hasProperty( "http://nepomuk.test.org/bool2" ) );
        QVERIFY( r1.hasProperty( "http://nepomuk.test.org/double" ) );
        QVERIFY( r1.hasProperty( "http://nepomuk.test.org/string" ) );
        QVERIFY( r1.hasProperty( "http://nepomuk.test.org/date" ) );
        QVERIFY( r1.hasProperty( "http://nepomuk.test.org/Resource" ) );

        QCOMPARE( r1.property( "http://nepomuk.test.org/int" ).toInt(), 17 );
        QCOMPARE( r1.property( "http://nepomuk.test.org/bool1" ).toBool(), true );
        QCOMPARE( r1.property( "http://nepomuk.test.org/bool2" ).toBool(), false );
        QCOMPARE( r1.property( "http://nepomuk.test.org/double" ).toDouble(), 2.2 );
        QCOMPARE( r1.property( "http://nepomuk.test.org/string" ).toString(), QString("test") );
        QCOMPARE( r1.property( "http://nepomuk.test.org/date" ).toDate(), QDate::currentDate() );
        QCOMPARE( r1.property( "http://nepomuk.test.org/Resource" ).toResource(), r2 );

        QHash<QString, Variant> allProps = r1.allProperties();
        QCOMPARE( allProps.count(), 9 ); // properties + type + identifier
        QVERIFY( allProps.keys().contains( "http://nepomuk.test.org/int" ) );
        QVERIFY( allProps.keys().contains( "http://nepomuk.test.org/bool1" ) );
        QVERIFY( allProps.keys().contains( "http://nepomuk.test.org/bool2" ) );
        QVERIFY( allProps.keys().contains( "http://nepomuk.test.org/double" ) );
        QVERIFY( allProps.keys().contains( "http://nepomuk.test.org/string" ) );
        QVERIFY( allProps.keys().contains( "http://nepomuk.test.org/date" ) );
        QVERIFY( allProps.keys().contains( "http://nepomuk.test.org/Resource" ) );
    }
}


void ResourceTest::testResourceIdentifiers()
{
    QString theUri;
    {
        Resource r1( "wurst" );
        Resource r2( "wurst" );

        QVERIFY( r1 == r2 );

        theUri = r1.uri();

        Resource r3( r1.uri() );

        QVERIFY( r1 == r3 );

        QVERIFY( r1.uri() != "wurst" );

        r1.setProperty( "http://nepomuk.test.org/foo/bar", "foobar" );

        RDFRepository rr( ResourceManager::instance()->serviceRegistry()->discoverRDFRepository() );

        QList<Statement> sl = rr.listStatements( defaultGraph(), Statement( QUrl( r1.uri() ), Node(), Node() ) );

        foreach( const Statement& s, sl )
            qDebug() << s << endl;

        QCOMPARE( sl.count(), 3 );

        QVERIFY( rr.contains( defaultGraph(),
                              Statement( QUrl( r1.uri() ),
                                         QUrl( Resource::identifierUri() ),
                                         LiteralValue( "wurst" ) ) ) );
    }

    {
        Resource r1( theUri );
        Resource r2( "wurst" );

        QCOMPARE( r1, r2 );
    }
}


void ResourceTest::testResourceManager()
{
    {
        Resource r1( "res1", "mytype" );
        Resource r2( "res2", "mytype" );
        Resource r3( "res3", "myothertype" );
        Resource r4( "res4", "myothertype" );
        Resource r5( "res5", "myothertype" );
        Resource r6( "res6", "mythirdtype" );

        QList<Resource> rl = ResourceManager::instance()->allResourcesOfType( "mytype" );
        QCOMPARE( rl.count(), 2 );
        QVERIFY( rl.contains( r1 ) && rl.contains( r2 ) );

        rl = ResourceManager::instance()->allResourcesOfType( r6.type() );
        QCOMPARE( rl.count(), 1 );
        QCOMPARE( rl.first(), r6 );

        r1.setProperty( "prop1", 42 );
        r3.setProperty( "prop1", 42 );
        r4.setProperty( "prop1", 41 );

        r3.setProperty( "prop2", r6 );
        r4.setProperty( "prop2", r6 );
        r5.setProperty( "prop2", r6 );
        r6.setProperty( "prop2", r1 );

        rl = ResourceManager::instance()->allResourcesWithProperty( "prop1", 42 );
        QCOMPARE( rl.count(), 2 );
        QVERIFY( rl.contains( r1 ) && rl.contains( r3 ) );

        rl = ResourceManager::instance()->allResourcesWithProperty( "prop2", r6 );
        QCOMPARE( rl.count(), 3 );
        QVERIFY( rl.contains( r3 ) && rl.contains( r4 ) && rl.contains( r5 ) );
    }

    {
        Resource r1( "res1", "mytype" );
        Resource r2( "res2", "mytype" );
        Resource r3( "res3", "myothertype" );
        Resource r4( "res4", "myothertype" );
        Resource r5( "res5", "myothertype" );
        Resource r6( "res6", "mythirdtype" );

        QList<Resource> rl = ResourceManager::instance()->allResourcesOfType( "mytype" );
        QCOMPARE( rl.count(), 2 );
        QVERIFY( rl.contains( r1 ) && rl.contains( r2 ) );

        rl = ResourceManager::instance()->allResourcesOfType( r6.type() );
        QCOMPARE( rl.count(), 1 );
        QCOMPARE( rl.first(), r6 );

        rl = ResourceManager::instance()->allResourcesWithProperty( "prop1", 42 );
        QCOMPARE( rl.count(), 2 );
        QVERIFY( rl.contains( r1 ) && rl.contains( r3 ) );

        rl = ResourceManager::instance()->allResourcesWithProperty( "prop2", r6 );
        QCOMPARE( rl.count(), 3 );
        QVERIFY( rl.contains( r3 ) && rl.contains( r4 ) && rl.contains( r5 ) );

        QVERIFY( r1.hasProperty( "prop1" ) );
        QVERIFY( r3.hasProperty( "prop1" ) );
        QVERIFY( r4.hasProperty( "prop1" ) );

        QVERIFY( r3.hasProperty( "prop2" ) );
        QVERIFY( r4.hasProperty( "prop2" ) );
        QVERIFY( r5.hasProperty( "prop2" ) );
        QVERIFY( r6.hasProperty( "prop2" ) );

        QCOMPARE( r3.property( "prop2" ).toResource(), r6 );
        QCOMPARE( r4.property( "prop2" ).toResource(), r6 );
        QCOMPARE( r5.property( "prop2" ).toResource(), r6 );
        QCOMPARE( r6.property( "prop2" ).toResource(), r1 );
    }
}


QTEST_KDEMAIN(ResourceTest, NoGUI)

#include "resourcetest.moc"
