/*
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2007 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING.LIB" for the exact licensing terms.
 */

#include "resourcedatatest.h"

#include "../kmetadata/kmetadata.h"

#include <knepomuk/knepomuk.h>
#include <knepomuk/services/rdfrepository.h>

#include <konto/ontologyloader.h>
#include <konto/ontologymanager.h>
#include <konto/global.h>

#include <kdebug.h>
#include <qtest_kde.h>

using namespace Nepomuk::KMetaData;
using namespace Nepomuk::RDF;
using namespace Nepomuk::Services;
using namespace Soprano;

static const QString s_hasIdentifierUri( "http://semanticdesktop.org/ontologies/2007/03/31/nao#altIdentifier" );


void ResourceDataTest::testResourceData()
{
    QString uri;
    QString id = "id002";

    {
        ResourceData* rd1 = new ResourceData( id );
        QVERIFY( rd1->uri().isEmpty() );
        QCOMPARE( rd1->kickoffUriOrId(), id );
        QVERIFY( rd1->allProperties().isEmpty() );
        QVERIFY( rd1->determineUri() );
        uri = rd1->uri();
        QVERIFY( !rd1->uri().isEmpty() );
        QCOMPARE( rd1->allProperties().count(), 1 );
        QList<Statement> sl = rd1->allStatementsToAdd();

        kDebug() << "(ResourceDataTest) statements to add for ResourceData( " << id << "):" << endl;
        foreach( const Statement& s, sl ) {
            kDebug() << "(ResourceDataTest)     " << s << endl;
        }

        QCOMPARE( sl.count(), 2 );
        QVERIFY( rd1->hasProperty( s_hasIdentifierUri ) );
        QCOMPARE( rd1->property( s_hasIdentifierUri ).toString(), rd1->kickoffUriOrId() );
        QVERIFY( !rd1->exists() );
        QVERIFY( !rd1->inSync() );
        QVERIFY( rd1->save() );
        QVERIFY( rd1->exists() );
        QVERIFY( rd1->inSync() );
        rd1->deleteData();
    }

    {
        ResourceData* rd = new ResourceData( id );
        QVERIFY( rd->determineUri() );
        QCOMPARE( rd->uri(), uri );
        QVERIFY( rd->inSync() );
        rd->deleteData();
    }

    {
        ResourceData* rd = new ResourceData( uri );
        QVERIFY( rd->determineUri() );
        QCOMPARE( rd->uri(), uri );
        QVERIFY( rd->inSync() );
        QVERIFY( rd->exists() );
        rd->deleteData();
    }
}


void ResourceDataTest::testLoad()
{
    RDFRepository rr( ResourceManager::instance()->serviceRegistry()->discoverRDFRepository() );
    if( !rr.listRepositoriyIds().contains( defaultGraph() ) )
        rr.createRepository( defaultGraph() );

    QString resUri = ResourceManager::instance()->generateUniqueUri();
    QString typeUri = "http://nepomuk-kde.semanticdesktop.org/ontology/nkde-0.1#TestType";
    QString identifier = "test365";

    // create some data
    QList<Statement> sl;
    sl.append( Statement( QUrl( resUri ), QUrl( typePredicate() ), QUrl( typeUri ) ) );
    sl.append( Statement( QUrl( resUri ), QUrl( s_hasIdentifierUri ), LiteralValue( identifier ) ) );
    rr.addStatements( defaultGraph(), sl );
    QVERIFY( rr.success() );

    // now load it via ResourceData
    {
        ResourceData* rd = new ResourceData( resUri );
        QVERIFY( rd->determineUri() );
        QCOMPARE( rd->uri(), resUri );
        QVERIFY( rd->load() );
        QCOMPARE( rd->type(), typeUri );
        QCOMPARE( rd->property( s_hasIdentifierUri ).toString(), identifier );
        rd->deleteData();
    }
}


void ResourceDataTest::testPropertyTypes()
{
    Variant vInt( (int)17 );
    Variant vUInt( (unsigned int)12 );
    Variant vLong( (long long)123498670808943LL );
    Variant vULong( (unsigned long long)2709487120951223ULL );
    Variant vDouble( 78879.2324 );
    Variant vDate( QDate( 1977, 8, 9 ) );
    Variant vTime( QTime::currentTime() );
    Variant vDateTime( QDateTime( vDate.toDate(), vTime.toTime() ) );

    {
        ResourceData* r1 = new ResourceData( "something12" );
        r1->init();

        r1->setProperty( "hasInt", vInt );
        r1->setProperty( "hasUInt", vUInt );
        r1->setProperty( "hasLong", vLong );
        r1->setProperty( "hasULong", vULong );
        r1->setProperty( "hasDouble", vDouble );
        r1->setProperty( "hasDate", vDate );
        r1->setProperty( "hasTime", vTime );
        r1->setProperty( "hasDateTime", vDateTime );

        r1->save();
        r1->deleteData();
    }

    {
        ResourceData* r1 = new ResourceData( "something12" );
        r1->init();

        QVERIFY( r1->hasProperty( "hasInt" ) );
        QVERIFY( r1->hasProperty( "hasUInt" ) );
        QVERIFY( r1->hasProperty( "hasLong" ) );
        QVERIFY( r1->hasProperty( "hasULong" ) );
        QVERIFY( r1->hasProperty( "hasDouble" ) );
        QVERIFY( r1->hasProperty( "hasDate" ) );
        QVERIFY( r1->hasProperty( "hasTime" ) );
        QVERIFY( r1->hasProperty( "hasDateTime" ) );

        QCOMPARE( r1->property( "hasInt" ).toInt(), vInt.toInt() );
        QCOMPARE( r1->property( "hasUInt" ).toUnsignedInt(), vUInt.toUnsignedInt() );
        QCOMPARE( r1->property( "hasLong" ).toInt64(), vLong.toInt64() );
        QCOMPARE( r1->property( "hasULong" ).toUnsignedInt64(), vULong.toUnsignedInt64() );
        QCOMPARE( r1->property( "hasDouble" ).toDouble(), vDouble.toDouble() );
        QCOMPARE( r1->property( "hasDate" ).toDate(), vDate.toDate() );
        QCOMPARE( r1->property( "hasTime" ).toTime(), vTime.toTime() );
        QCOMPARE( r1->property( "hasDateTime" ).toDateTime(), vDateTime.toDateTime().toUTC() );

        r1->deleteData();
    }
}


class DummyOntologyLoader : public Konto::OntologyLoader
{
public:
    QList<Soprano::Statement> loadOntology( const QUrl& uri ) {
        // create some dummy onto stuff
        QList<Statement> sl;

        QString ns = uri.toString();
        if ( !ns.endsWith( "#" ) ) {
            ns += '#';
        }

        // one dummy class
        sl.append( Statement( QUrl( ns + "DummyClass" ),
                              QUrl( Konto::rdfNamespace() + "type" ),
                              QUrl( Konto::rdfsNamespace() + "Class" ) ) );
        sl.append( Statement( QUrl( ns + "DummyClass" ),
                              QUrl( Konto::rdfsNamespace() + "label" ),
                              LiteralValue( "A dummy class" ) ) );

        sl.append( Statement( QUrl( ns + "DummySubClass" ),
                              QUrl( Konto::rdfNamespace() + "type" ),
                              QUrl( Konto::rdfsNamespace() + "Class" ) ) );
        sl.append( Statement( QUrl( ns + "DummySubClass" ),
                              QUrl( Konto::rdfsNamespace() + "subClassOf" ),
                              QUrl( ns + "DummyClass" ) ) );

        sl.append( Statement( QUrl( ns + "DummySubSubClass" ),
                              QUrl( Konto::rdfNamespace() + "type" ),
                              QUrl( Konto::rdfsNamespace() + "Class" ) ) );
        sl.append( Statement( QUrl( ns + "DummySubSubClass" ),
                              QUrl( Konto::rdfsNamespace() + "subClassOf" ),
                              QUrl( ns + "DummySubClass" ) ) );

        sl.append( Statement( QUrl( ns + "SomeOtherDummyClass" ),
                              QUrl( Konto::rdfNamespace() + "type" ),
                              QUrl( Konto::rdfsNamespace() + "Class" ) ) );

        return sl;
    }
};


void ResourceDataTest::testTypeHandling()
{
    Konto::OntologyManager::instance()->setOntologyLoader( new DummyOntologyLoader() );

    ResourceData* r1 = ResourceData::data( "test", "test#DummyClass" );
    ResourceData* r2 = ResourceData::data( "test", "test#DummySubClass" );
    ResourceData* r3 = ResourceData::data( "test", "test#DummySubSubClass" );
    ResourceData* r4 = ResourceData::data( "test", "test#SomeOtherDummyClass" );
    ResourceData* r5 = ResourceData::data( "test", "http://www.w3.org/2000/01/rdf-schema#Resource" );

    // let's create one that can be loaded from the store
    Nepomuk::Services::RDFRepository rr( ResourceManager::instance()->serviceRegistry()->discoverRDFRepository() );
    QString uri = ResourceManager::instance()->generateUniqueUri();
    QList<Statement> sl;
    sl.append( Statement( QUrl( uri ),
                          QUrl( typePredicate() ),
                          QUrl( "test#SomeOtherDummyClass" ) ) );
    sl.append( Statement( QUrl( uri ),
                          QUrl( s_hasIdentifierUri ),
                          LiteralValue( "test2" ) ) );
    rr.addStatements( defaultGraph(), sl );

    ResourceData* r6 = ResourceData::data( "test2", "http://www.w3.org/2000/01/rdf-schema#Resource" );

    QCOMPARE( r1, r2 );
    QCOMPARE( r1, r3 );
    QVERIFY( r1 != r4 );
    QVERIFY( r1 == r5 ||
             r4 == r5 );
    QVERIFY( r1 != r6 );
    QVERIFY( r5 != r6 );

    QVERIFY( r1->determineUri() );
    QVERIFY( r2->determineUri() );
    QVERIFY( r3->determineUri() );
    QVERIFY( r4->determineUri() );
    QVERIFY( r5->determineUri() );
    QVERIFY( r6->determineUri() );
    QVERIFY( r6->init() ); // here we need to actually load since determineUri does not read the type for existing resources (hmm... maybe inconsistent)

    // the first three resourcedatas should have the same URI since their type and identifier match
    // the fourth has a completely different type, thus the URI should differ
    QCOMPARE( r1->uri(), r2->uri() );
    QCOMPARE( r1->uri(), r3->uri() );
    QVERIFY( r1->uri() != r4->uri() );
    QVERIFY( r1->uri() == r5->uri() ||
             r4->uri() == r5->uri() );
    QVERIFY( r1->uri() != r6->uri() );
    QVERIFY( r5->uri() != r6->uri() );

    // the type should be set to the most detailed one
    QCOMPARE( r1->type(), QString( "test#DummySubSubClass" ) );
    QCOMPARE( r4->type(), QString( "test#SomeOtherDummyClass" ) );
    QCOMPARE( r6->type(), QString( "test#SomeOtherDummyClass" ) );

    r1->deleteData(); // r1 == r2 == r3
    r4->deleteData(); // r4 == r5
    r6->deleteData();
}


QTEST_KDEMAIN(ResourceDataTest, NoGUI)

#include "resourcedatatest.moc"
