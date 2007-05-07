/*
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006 Sebastian Trueg <trueg@kde.org>
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

static const QString s_hasIdentifierUri( "http://semanticdesktop.org/ontologies/2007/03/31/nao#altIdentifier" );

void ResourceTest::testResourceStates()
{
    QString someUri = ResourceManager::instance()->generateUniqueUri();

    Resource r1( someUri );
    Resource r2( someUri );

    QCOMPARE( r1, r2 );

    QVERIFY( r1.isValid() );
    QVERIFY( !r1.isModified() );
    QEXPECT_FAIL("", "Resource::inSyncWithStore() does not work properly due to the kickoffUri and the generated hasIdentifier property.", Continue);
    QVERIFY( r1.inSyncWithStore() );
    QEXPECT_FAIL("", "Resource::isModified() does not work properly after a Resource::inSyncWithStore() call. Same reason.", Continue);
    QVERIFY( !r1.isModified() );

    r1.setProperty( "someinvaliduri", 12 );

    QCOMPARE( r1, r2 );
    QVERIFY( r1.isModified() );
    QVERIFY( r2.isModified() );

    QVERIFY( !r2.inSyncWithStore() );
}


void ResourceTest::testResourceRemoval()
{
    QString someUri = ResourceManager::instance()->generateUniqueUri();

    Resource r1( someUri );
    QVERIFY( !r1.isModified() );
    QVERIFY( !r1.exists() );

    QVERIFY( r1.sync() == 0 );

    QVERIFY( !r1.isModified() );
    QVERIFY( r1.exists() );

    r1.remove();

    QVERIFY( r1.isModified() );
    QVERIFY( r1.exists() );

    QVERIFY( r1.sync() == 0 );

    QVERIFY( !r1.isModified() );
    QVERIFY( !r1.exists() );

    r1.revive();

    QVERIFY( !r1.isModified() );
    QVERIFY( !r1.exists() );

    QVERIFY( r1.sync() == 0 );

    QVERIFY( !r1.isModified() );
    QVERIFY( r1.exists() );

    r1.remove();
    r1.setProperty( "testProp", "hello" );

    QVERIFY( r1.isModified() );

    QVERIFY( r1.sync() == 0 );

    QVERIFY( !r1.isModified() );
    QVERIFY( r1.exists() );

    r1.remove();
    QVERIFY( r1.sync() == 0 );

    QVERIFY( !r1.exists() );
}


void ResourceTest::testResourceIdentifiers()
{
    QString theUri;
    {
        Resource r1( "wurst" );
        Resource r2( "wurst" );

        QVERIFY( r1 == r2 );

        r1.sync();

        QVERIFY( r1.hasProperty( s_hasIdentifierUri ) );

        theUri = r1.uri();

        Resource r3( r1.uri() );

        QVERIFY( r1 == r3 );

        RDFRepository rr( ResourceManager::instance()->serviceRegistry()->discoverRDFRepository() );
        QList<Statement> sl = rr.listStatements( defaultGraph(), Statement( QUrl( r1.uri() ), Node(), Node() ) );

        foreach( const Statement& s, sl )
            qDebug() << s << endl;

        QCOMPARE( sl.count(), 2 );

        QVERIFY( r1.uri() != "wurst" );

        QVERIFY( rr.contains( defaultGraph(),
                              Statement( QUrl( r1.uri() ),
                                         QUrl( s_hasIdentifierUri ),
                                         LiteralValue( "wurst" ) ) ) );
    }

    {
        Resource r1( theUri );
        Resource r2( "wurst" );

        QCOMPARE( r1, r2 );

        r1.remove();
        r1.sync();

        QVERIFY( !r2.exists() );
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

        ResourceManager::instance()->syncAll();
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
