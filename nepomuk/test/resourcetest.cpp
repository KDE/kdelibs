/*
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2009 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING.LIB" for the exact licensing terms.
 */

#include "resourcetest.h"
#include "nie.h"

#include "resource.h"
#include "variant.h"
#include "resourcemanager.h"

#include <kdebug.h>
#include <ktemporaryfile.h>
#include <qtest_kde.h>

#include <Soprano/Soprano>

#include <QtCore/QTextStream>

using namespace Soprano;
using namespace Nepomuk;


void ResourceTest::testResourceStates()
{
    QUrl someUri = ResourceManager::instance()->generateUniqueUri( QString() );

    Resource r1( someUri );
    Resource r2( someUri );

    QCOMPARE( r1, r2 );

    QVERIFY( r1.isValid() );
    QVERIFY( !r1.exists() );

    r1.setProperty( QUrl("http://test/something"), 12 );

    QCOMPARE( r1, r2 );
    QVERIFY( r1.exists() );
}


void ResourceTest::testResourceRemoval()
{
    QString testiId( QLatin1String("testi") );

    Resource res( testiId );
    res.setProperty( QUrl("http://nepomuk.test.org/foo/bar"),  "foobar" );

    QUrl testiUri = res.resourceUri();

    QVERIFY( !testiUri.isEmpty() );

    QVERIFY( ResourceManager::instance()->mainModel()->containsAnyStatement( Statement( testiUri, Node(), Node() ) ) );

    res.remove();

    QVERIFY( !res.exists() );

    QVERIFY( !ResourceManager::instance()->mainModel()->containsAnyStatement( Statement( testiUri, Node(), Node() ) ) );

    //
    // test recursive removal
    //
    Resource res2( "testi2" );

    res.setProperty( QUrl("http://nepomuk.test.org/foo/bar2"), res2 );

    QVERIFY( res.exists() );
    QVERIFY( res2.exists() );

    QVERIFY( ResourceManager::instance()->mainModel()->containsAnyStatement( Statement( res.resourceUri(), QUrl("http://nepomuk.test.org/foo/bar2"), Node(res2.resourceUri()) ) ) );

    res2.remove();

    QVERIFY( res.exists() );
    QVERIFY( !res2.exists() );

    QVERIFY( !ResourceManager::instance()->mainModel()->containsAnyStatement( Statement( res.resourceUri(), QUrl("http://nepomuk.test.org/foo/bar2"), Node(res2.resourceUri()) ) ) );

    //
    // Now make sure the relation between id and URI has actually be removed
    //
    Resource res3( testiId );
    QVERIFY( res3.resourceUri() != testiUri );
}


void ResourceTest::testProperties()
{
    QUrl r1Uri, r2Uri;

    {
        Resource r1( "testi" );
        Resource r2( "testi2" );

        r1.setProperty( QUrl("http://nepomuk.test.org/int"), 17 );
        r1.setProperty( QUrl("http://nepomuk.test.org/bool1"), true );
        r1.setProperty( QUrl("http://nepomuk.test.org/bool2"), false );
        r1.setProperty( QUrl("http://nepomuk.test.org/double"), 2.2 );
        r1.setProperty( QUrl("http://nepomuk.test.org/string"), "test" );
        r1.setProperty( QUrl("http://nepomuk.test.org/date"), QDate::currentDate() );
        r1.setProperty( QUrl("http://nepomuk.test.org/Resource"), r2 );

        r1Uri = r1.resourceUri();
        r2Uri = r2.resourceUri();
    }

    QTextStream s(stdout);
    foreach( const Statement& st,     ResourceManager::instance()->mainModel()->listStatements().allStatements() ) {
        s << st << endl;
    }

    {
        Resource r1( r1Uri );
        Resource r2( r2Uri );

        QVERIFY( r1.hasProperty( QUrl("http://nepomuk.test.org/int" ) ) );
        QVERIFY( r1.hasProperty( QUrl("http://nepomuk.test.org/bool1" ) ) );
        QVERIFY( r1.hasProperty( QUrl("http://nepomuk.test.org/bool2" ) ) );
        QVERIFY( r1.hasProperty( QUrl("http://nepomuk.test.org/double" ) ) );
        QVERIFY( r1.hasProperty( QUrl("http://nepomuk.test.org/string" ) ) );
        QVERIFY( r1.hasProperty( QUrl("http://nepomuk.test.org/date" ) ) );
        QVERIFY( r1.hasProperty( QUrl("http://nepomuk.test.org/Resource" ) ) );

        QCOMPARE( r1.property( QUrl("http://nepomuk.test.org/int" ) ).toInt(), 17 );
        QCOMPARE( r1.property( QUrl("http://nepomuk.test.org/bool1" ) ).toBool(), true );
        QCOMPARE( r1.property( QUrl("http://nepomuk.test.org/bool2" ) ).toBool(), false );
        QCOMPARE( r1.property( QUrl("http://nepomuk.test.org/double" ) ).toDouble(), 2.2 );
        QCOMPARE( r1.property( QUrl("http://nepomuk.test.org/string" ) ).toString(), QString("test") );
        QCOMPARE( r1.property( QUrl("http://nepomuk.test.org/date" ) ).toDate(), QDate::currentDate() );
        QCOMPARE( r1.property( QUrl("http://nepomuk.test.org/Resource" ) ).toResource(), r2 );

        QHash<QString, Variant> allProps = r1.allProperties();
        QCOMPARE( allProps.count(), 10 ); // properties + type + identifier + modification date
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
    QUrl theUri;
    {
        Resource r1( "wurst" );
        Resource r2( "wurst" );

        QVERIFY( r1 == r2 );

        QVERIFY( r1.resourceUri() != QUrl("wurst") );

        r1.setProperty( QUrl("http://nepomuk.test.org/foo/bar"), "foobar" );

        theUri = r1.resourceUri();

        QList<Statement> sl
            = ResourceManager::instance()->mainModel()->listStatements( Statement( r1.resourceUri(), Node(), Node() ) ).allStatements();

        QCOMPARE( sl.count(), 4 );

        QVERIFY( ResourceManager::instance()->mainModel()->containsAnyStatement( Statement( r1.resourceUri(),
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
        Resource r1( "res1", QUrl("http://test/mytype" ) );
        Resource r2( "res2", QUrl("http://test/mytype" ) );
        Resource r3( "res3", QUrl("http://test/myothertype" ) );
        Resource r4( "res4", QUrl("http://test/myothertype" ) );
        Resource r5( "res5", QUrl("http://test/myothertype" ) );
        Resource r6( "res6", QUrl("http://test/mythirdtype" ) );

        QList<Resource> rl = ResourceManager::instance()->allResourcesOfType( QUrl("http://test/mytype") );
        QCOMPARE( rl.count(), 2 );
        QVERIFY( rl.contains( r1 ) && rl.contains( r2 ) );

        rl = ResourceManager::instance()->allResourcesOfType( r6.resourceType() );
        QCOMPARE( rl.count(), 1 );
        QCOMPARE( rl.first(), r6 );

        r1.setProperty( QUrl("http://test/prop1"), 42 );
        r3.setProperty( QUrl("http://test/prop1"), 42 );
        r4.setProperty( QUrl("http://test/prop1"), 41 );

        r3.setProperty( QUrl("http://test/prop2"), r6 );
        r4.setProperty( QUrl("http://test/prop2"), r6 );
        r5.setProperty( QUrl("http://test/prop2"), r6 );
        r6.setProperty( QUrl("http://test/prop2"), r1 );

        rl = ResourceManager::instance()->allResourcesWithProperty( QUrl("http://test/prop1"), 42 );
        QCOMPARE( rl.count(), 2 );
        QVERIFY( rl.contains( r1 ) && rl.contains( r3 ) );

        rl = ResourceManager::instance()->allResourcesWithProperty( QUrl("http://test/prop2"), r6 );
        QCOMPARE( rl.count(), 3 );
        QVERIFY( rl.contains( r3 ) && rl.contains( r4 ) && rl.contains( r5 ) );
    }

    {
        Resource r1( "res1", QUrl("http://test/mytype" ) );
        Resource r2( "res2", QUrl("http://test/mytype" ) );
        Resource r3( "res3", QUrl("http://test/myothertype" ) );
        Resource r4( "res4", QUrl("http://test/myothertype" ) );
        Resource r5( "res5", QUrl("http://test/myothertype" ) );
        Resource r6( "res6", QUrl("http://test/mythirdtype" ) );

        QList<Resource> rl = ResourceManager::instance()->allResourcesOfType( QUrl("http://test/mytype" ));

        QCOMPARE( rl.count(), 2 );
        QVERIFY( rl.contains( r1 ) && rl.contains( r2 ) );

        rl = ResourceManager::instance()->allResourcesOfType( r6.resourceType() );
        QCOMPARE( rl.count(), 1 );
        QCOMPARE( rl.first(), r6 );

        rl = ResourceManager::instance()->allResourcesWithProperty( QUrl("http://test/prop1"), 42 );
        QCOMPARE( rl.count(), 2 );
        QVERIFY( rl.contains( r1 ) && rl.contains( r3 ) );

        rl = ResourceManager::instance()->allResourcesWithProperty( QUrl("http://test/prop2"), r6 );
        QCOMPARE( rl.count(), 3 );
        QVERIFY( rl.contains( r3 ) && rl.contains( r4 ) && rl.contains( r5 ) );

        QVERIFY( r1.hasProperty( QUrl("http://test/prop1" ) ) );
        QVERIFY( r3.hasProperty( QUrl("http://test/prop1" ) ) );
        QVERIFY( r4.hasProperty( QUrl("http://test/prop1" ) ) );

        QVERIFY( r3.hasProperty( QUrl("http://test/prop2" ) ) );
        QVERIFY( r4.hasProperty( QUrl("http://test/prop2" ) ) );
        QVERIFY( r5.hasProperty( QUrl("http://test/prop2" ) ) );
        QVERIFY( r6.hasProperty( QUrl("http://test/prop2" ) ) );

        QCOMPARE( r3.property( QUrl("http://test/prop2" )).toResource(), r6 );
        QCOMPARE( r4.property( QUrl("http://test/prop2" )).toResource(), r6 );
        QCOMPARE( r5.property( QUrl("http://test/prop2" )).toResource(), r6 );
        QCOMPARE( r6.property( QUrl("http://test/prop2" )).toResource(), r1 );
    }
}


void ResourceTest::testLocalFileUrls()
{
    // create a testfile
    KTemporaryFile tmpFile1;
    QVERIFY( tmpFile1.open() );

    QUrl tmpFile1ResUri;
    // create a new file resource. Resource should automatically save the nie:url property
    {
        Resource fileRes( KUrl(tmpFile1.fileName()) );
        fileRes.setRating( 4 );

        // make sure the nie:url is saved
        QVERIFY( ResourceManager::instance()->mainModel()->containsAnyStatement( fileRes.resourceUri(), Nepomuk::Vocabulary::NIE::url(), KUrl(tmpFile1.fileName()) ) );

        // make sure a proper nepomuk:/ uri has been created
        QVERIFY( fileRes.resourceUri().scheme() == QLatin1String("nepomuk") );

        // make sure the local resource is reused with the file URL
        Resource fileRes2( KUrl(tmpFile1.fileName()) );
        QCOMPARE( fileRes.resourceUri(), fileRes2.resourceUri() );

        // make sure the local resource is reused with the resource URI
        Resource fileRes3( fileRes.resourceUri() );
        QCOMPARE( fileRes.resourceUri(), fileRes3.resourceUri() );

        tmpFile1ResUri = fileRes.resourceUri();

        // make sure even the string constructor will find the resource again with
        Resource fileRes4( KUrl(tmpFile1ResUri).url() );
        fileRes4.setRating(4);
        QCOMPARE( fileRes4.resourceUri(), tmpFile1ResUri );

        // make sure the resource is reused with the local file path
        Resource fileRes5( tmpFile1.fileName() );
        fileRes4.setRating(5);
        QCOMPARE( fileRes5.resourceUri(), tmpFile1ResUri );
    }

    // clear cache to be sure we call ResourceData::determineUri
    ResourceManager::instance()->clearCache();

    {
        // verify that the resource in question is found again
        Resource fileRes1( KUrl(tmpFile1.fileName()) );
        QCOMPARE( tmpFile1ResUri, fileRes1.resourceUri() );

        // make sure the local resource is reused with the resource URI
        Resource fileRes2( tmpFile1ResUri );
        QCOMPARE( tmpFile1ResUri, fileRes2.resourceUri() );

        // create a second test file
        KTemporaryFile tmpFile2;
        QVERIFY( tmpFile2.open() );

        // make sure the file:/ URL is reused as resource URI
        ResourceManager::instance()->mainModel()->addStatement( KUrl(tmpFile2.fileName()), Nepomuk::Vocabulary::NIE::url(), KUrl(tmpFile2.fileName()) );

        Resource fileRes3( KUrl(tmpFile2.fileName()) );
        fileRes3.setRating( 4 );
        QCOMPARE( KUrl(fileRes3.resourceUri()), KUrl(tmpFile2.fileName()) );

        // create a third test file
        KTemporaryFile tmpFile3;
        QVERIFY( tmpFile3.open() );

        // add a random bit of information about it
        ResourceManager::instance()->mainModel()->addStatement( KUrl(tmpFile3.fileName()), Soprano::Vocabulary::NAO::rating(), Soprano::LiteralValue(4) );

        Resource fileRes4( KUrl(tmpFile3.fileName()) );
        QCOMPARE( KUrl(fileRes4.resourceUri()).url(), KUrl(tmpFile3.fileName()).url() );

        // make sure removing the resource results in us not reusing the URI
        QUrl fileRes1Uri = fileRes1.resourceUri();
        fileRes1.remove();

        Resource fileRes5( KUrl(tmpFile1.fileName()) );
        QVERIFY( fileRes1Uri != fileRes5.resourceUri() );
    }

    // clear cache to be sure we do not reuse the cache
    ResourceManager::instance()->clearCache();
}


void ResourceTest::testKickOffListRemoval()
{
    {
        KTemporaryFile tmpFile1;
        QVERIFY( tmpFile1.open() );

        Resource fileRes( KUrl(tmpFile1.fileName()) );
        fileRes.setRating( 4 );

        // make sure the nie:url is saved
        QVERIFY( ResourceManager::instance()->mainModel()->containsAnyStatement( fileRes.resourceUri(), Nepomuk::Vocabulary::NIE::url(), KUrl(tmpFile1.fileName()) ) );

        // make sure a proper nepomuk:/ uri has been created
        QVERIFY( fileRes.resourceUri().scheme() == QLatin1String("nepomuk") );
        
        // Remove the nie:url
        fileRes.removeProperty( Nepomuk::Vocabulary::NIE::url() );
        
        Resource fileRes2( KUrl(tmpFile1.fileName()) );
        QVERIFY( fileRes.resourceUri() != fileRes2.resourceUri() );
        
        Resource r1( "res1" );
        r1.setProperty( QUrl("http://test/prop1"), 42 );        
        r1.removeProperty( Soprano::Vocabulary::NAO::identifier() );
        
        Resource r2( "res1" );
        r2.setProperty( QUrl("http://test/prop1"), 46 );
        
        QVERIFY( r2.resourceUri() != r1.resourceUri() );
        QVERIFY( r1.property(QUrl("http://test/prop1")) != r2.property(QUrl("http://test/prop1")) );
            
    }
    {
        KTemporaryFile tmpFile;
        QVERIFY( tmpFile.open() );

        Resource fileRes( KUrl(tmpFile.fileName()) );
        fileRes.setRating( 4 );

        // make sure the nie:url is saved
        QVERIFY( ResourceManager::instance()->mainModel()->containsAnyStatement( fileRes.resourceUri(), Nepomuk::Vocabulary::NIE::url(), KUrl(tmpFile.fileName()) ) );

        // make sure a proper nepomuk:/ uri has been created
        QVERIFY( fileRes.resourceUri().scheme() == QLatin1String("nepomuk") );
        
        // Add a different the nie:url
        KTemporaryFile tmpFile2;
        QVERIFY( tmpFile2.open() );
        fileRes.setProperty( Nepomuk::Vocabulary::NIE::url(), KUrl(tmpFile2.fileName()) );
        
        Resource fileRes2( KUrl(tmpFile.fileName()) );
        QVERIFY( fileRes.resourceUri() != fileRes2.resourceUri() );
        
        Resource fileRes3( KUrl(tmpFile2.fileName()) );
        QVERIFY( fileRes3.resourceUri() == fileRes.resourceUri() );
        
        Resource r1( "res1" );
        r1.setProperty( QUrl("http://test/prop1"), 42 );
        
        r1.setProperty( Soprano::Vocabulary::NAO::identifier(), "foo" );
        
        Resource r2( "res1" );
        r2.setProperty( QUrl("http://test/prop1"), 46 );
        
        QVERIFY( r2.resourceUri() != r1.resourceUri() );
        QVERIFY( r1.property(QUrl("http://test/prop1")) != r2.property(QUrl("http://test/prop1")) );
        
        Resource r3( "foo" );
        QVERIFY( r3.resourceUri() == r1.resourceUri() );
    }

}

QTEST_KDEMAIN(ResourceTest, NoGUI)

#include "resourcetest.moc"
