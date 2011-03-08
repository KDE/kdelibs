/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2010 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dynamicresourcefacettest.h"
#include "qtest_querytostring.h"

#include "dynamicresourcefacet.h"
#include "dynamicresourcefacet_p.h"
#include "comparisonterm.h"
#include "resourcetypeterm.h"
#include "resourceterm.h"
#include "andterm.h"
#include "orterm.h"

#include "resourcemanager.h"
#include "tag.h"

#include <Soprano/LiteralValue>
#include <Soprano/Node>
#include <Soprano/QueryResultIterator>
#include <Soprano/StorageModel>
#include <Soprano/Backend>
#include <Soprano/PluginManager>
#include <Soprano/Vocabulary/NAO>
#include <Soprano/Vocabulary/NRL>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/RDFS>
#include <Soprano/Vocabulary/XMLSchema>

#include <QtCore/QEventLoop>
#include <QtCore/QTimer>
#include <QSignalSpy>

#include <QtTest>
#include "qtest_kde.h"
#include "kdebug.h"

using namespace Nepomuk;
using namespace Nepomuk::Utils;
using namespace Nepomuk::Query;
using namespace Nepomuk::Types;


class DynamicResourceTestFacet::TestPrivate : public DynamicResourceFacet::Private
{
public:
    void startQuery( const Nepomuk::Query::Query& query ) {
        // we cannot use the query service since that would ignore our custom model
        // thus, we perform a sync query and call _k_newEntries async from there
        Nepomuk::Query::Query ourQuery(query);
        // disable result restrictions since we do not support those in our custom model
        ourQuery.setQueryFlags(Nepomuk::Query::Query::NoResultRestrictions);
        Soprano::QueryResultIterator it = ResourceManager::instance()->mainModel()->executeQuery( ourQuery.toSparqlQuery(), Soprano::Query::QueryLanguageSparql );
        QList<Nepomuk::Query::Result> results;
        while( it.next() ) {
            results << Result( it[0].uri() );
        }
        QMetaObject::invokeMethod( q, "_k_newEntries", Qt::QueuedConnection, Q_ARG(QList<Nepomuk::Query::Result>, results) );
    }
};


DynamicResourceTestFacet::DynamicResourceTestFacet()
    : DynamicResourceFacet() {
    // replace the private thingi with our own implementation
    delete d;
    d = new TestPrivate;
    d->q = this;
}


namespace {

    void waitForSignal( QObject* object, const char* signal, int timeout = 500 )
    {
        QEventLoop loop;
        loop.connect(object, signal, SLOT(quit()));
        QTimer::singleShot(timeout, &loop, SLOT(quit()));
        loop.exec();
    }

    Term buildTagTerm( const Tag& tag ) {
        return Soprano::Vocabulary::NAO::hasTag() == ResourceTerm(tag);
    }

    Term buildTagTermAll( const QList<Tag>& tags ) {
        AndTerm t;
        foreach( const Tag& tag, tags )
            t.addSubTerm( buildTagTerm(tag) );
        return t.optimized();
    }

    Term buildTagTermAny( const QList<Tag>& tags ) {
        OrTerm t;
        foreach( const Tag& tag, tags )
            t.addSubTerm( buildTagTerm(tag) );
        return t.optimized();
    }
}


void DynamicResourceFacetTest::initTestCase()
{
    // We create a dummy model with some simple test data in it
    const Soprano::Backend* backend = Soprano::PluginManager::instance()->discoverBackendByName( "virtuosobackend" );
    QVERIFY( backend );
    m_storageDir = new KTempDir();
    m_model = backend->createModel( Soprano::BackendSettings() << Soprano::BackendSetting(Soprano::BackendOptionStorageDir, m_storageDir->name()) );
    QVERIFY( m_model );

    Nepomuk::ResourceManager::instance()->setOverrideMainModel( m_model );

    // We need basic ontology knowledge for the ResourceTypeTerm queries to work properly
    const QUrl graph = Nepomuk::ResourceManager::instance()->generateUniqueUri(QString());
    m_model->addStatement(graph, Soprano::Vocabulary::RDF::type(), Soprano::Vocabulary::NRL::Ontology(), graph);
    m_model->addStatement(graph, Soprano::Vocabulary::RDF::type(), Soprano::Vocabulary::NRL::InstanceBase(), graph);
    m_model->addStatement(Soprano::Vocabulary::NAO::Tag(), Soprano::Vocabulary::RDFS::subClassOf(), Soprano::Vocabulary::NAO::Tag(), graph);
    m_model->addStatement(Soprano::Vocabulary::NAO::hasTag(), Soprano::Vocabulary::RDF::type(), Soprano::Vocabulary::RDF::Property(), graph);
    m_model->addStatement(Soprano::Vocabulary::NAO::hasTag(), Soprano::Vocabulary::RDFS::range(), Soprano::Vocabulary::NAO::Tag(), graph);

    // now some tags which will be our test resources
    tag1 = Nepomuk::Tag( QLatin1String("Tag 1") );
    tag1.setLabel( QLatin1String("Tag 1") );
    kDebug() << tag1.resourceUri() << tag1.label();
    tag2 = Nepomuk::Tag( QLatin1String("Tag 2") );
    tag2.setLabel( QLatin1String("Tag 2") );
    kDebug() << tag2.resourceUri() << tag2.label();
    tag3 = Nepomuk::Tag( QLatin1String("Tag 3") );
    tag3.setLabel( QLatin1String("Tag 3") );
    kDebug() << tag3.resourceUri() << tag3.label();
    tag4 = Nepomuk::Tag( QLatin1String("Tag 4") );
    tag4.setLabel( QLatin1String("Tag 4") );
    kDebug() << tag4.resourceUri() << tag4.label();

    // and some random resources that we will use for testing setClientQuery
    Resource res1;
    res1.addTag(tag1);
    res1.addTag(tag2);
    res1.addTag(tag3);

    Resource res2;
    res2.addTag(tag2);
    res2.addTag(tag3);

    Resource res3;
    res3.addTag(tag4);

    // nothing is tagged with tag4
}


void DynamicResourceFacetTest::cleanupTestCase()
{
    Nepomuk::ResourceManager::instance()->setOverrideMainModel( 0 );
    delete m_model;
    delete m_storageDir;
}


void DynamicResourceFacetTest::testEmptyFacet()
{
    DynamicResourceFacet f;
    QCOMPARE(f.count(), 0);
    QCOMPARE(f.queryTerm(), Term());
}


void DynamicResourceFacetTest::testSetRelation()
{
    DynamicResourceFacet f;
    f.setRelation(Soprano::Vocabulary::NAO::hasTag());
    QCOMPARE(f.resourceType(), Nepomuk::Types::Class(Soprano::Vocabulary::NAO::Tag()));
}


void DynamicResourceFacetTest::testMatchOne()
{
    DynamicResourceTestFacet f;
    f.setRelation(Soprano::Vocabulary::NAO::hasTag());
    f.setSelectionMode(Facet::MatchOne);

    // wait for the facet to be populated. Since this is happening async
    // and there might be any number of layoutChanged() signals between 1 and 4
    // we can only wait for the number of choices to become 4
    int i = 4;
    while( f.count() < 4 && i-- > 0 ) {
        waitForSignal(&f, SIGNAL(layoutChanged(Nepomuk::Utils::Facet*)));
    }
    QCOMPARE(f.count(), 4);

    // now the facet is initially populated
    // test the matchone selection
    QVERIFY(f.isSelected(0));
    QVERIFY(!f.isSelected(1));
    QVERIFY(!f.isSelected(2));
    QVERIFY(!f.isSelected(3));
    QCOMPARE(f.selectedResources().count(), 1);
    QCOMPARE(f.selectedResources().first(), f.resourceAt(0));
    QCOMPARE(f.queryTerm(), buildTagTerm(f.resourceAt(0)));

    f.setSelected(2);
    QVERIFY(!f.isSelected(0));
    QVERIFY(!f.isSelected(1));
    QVERIFY(f.isSelected(2));
    QVERIFY(!f.isSelected(3));
    QCOMPARE(f.selectedResources().count(), 1);
    QCOMPARE(f.selectedResources().first(), f.resourceAt(2));
    QCOMPARE(f.queryTerm(), buildTagTerm(f.resourceAt(2)));

    f.setSelected(2, false);
    QVERIFY(f.isSelected(0));
    QVERIFY(!f.isSelected(1));
    QVERIFY(!f.isSelected(2));
    QVERIFY(!f.isSelected(3));
    QCOMPARE(f.selectedResources().count(), 1);
    QCOMPARE(f.selectedResources().first(), f.resourceAt(0));
    QCOMPARE(f.queryTerm(), buildTagTerm(f.resourceAt(0)));

    f.setSelected(1);
    QVERIFY(!f.isSelected(0));
    QVERIFY(f.isSelected(1));
    QVERIFY(!f.isSelected(2));
    QVERIFY(!f.isSelected(3));
    QCOMPARE(f.selectedResources().count(), 1);
    QCOMPARE(f.selectedResources().first(), f.resourceAt(1));
    QCOMPARE(f.queryTerm(), buildTagTerm(f.resourceAt(1)));

    f.setSelected(3);
    QVERIFY(!f.isSelected(0));
    QVERIFY(!f.isSelected(1));
    QVERIFY(!f.isSelected(2));
    QVERIFY(f.isSelected(3));
    QCOMPARE(f.selectedResources().count(), 1);
    QCOMPARE(f.selectedResources().first(), f.resourceAt(3));
    QCOMPARE(f.queryTerm(), buildTagTerm(f.resourceAt(3)));

    f.clearSelection();
    QVERIFY(f.isSelected(0));
    QVERIFY(!f.isSelected(1));
    QVERIFY(!f.isSelected(2));
    QVERIFY(!f.isSelected(3));
    QCOMPARE(f.selectedResources().count(), 1);
    QCOMPARE(f.selectedResources().first(), f.resourceAt(0));
    QCOMPARE(f.queryTerm(), buildTagTerm(f.resourceAt(0)));
}


void DynamicResourceFacetTest::testMatchAll()
{
    DynamicResourceTestFacet f;
    f.setRelation(Soprano::Vocabulary::NAO::hasTag());
    f.setSelectionMode(Facet::MatchAll);

    // wait for the facet to be populated. Since this is happening async
    // and there might be any number of layoutChanged() signals between 1 and 4
    // we can only wait for the number of choices to become 4
    int i = 4;
    while( f.count() < 4 && i-- > 0 ) {
        waitForSignal(&f, SIGNAL(layoutChanged(Nepomuk::Utils::Facet*)));
    }
    QCOMPARE(f.count(), 4);

    // now the facet is initially populated
    // test the matchall selection
    QVERIFY(!f.isSelected(0));
    QVERIFY(!f.isSelected(1));
    QVERIFY(!f.isSelected(2));
    QVERIFY(!f.isSelected(3));
    QCOMPARE(f.queryTerm(), Term());
    QCOMPARE(f.selectedResources().count(), 0);

    f.setSelected(2);
    QVERIFY(!f.isSelected(0));
    QVERIFY(!f.isSelected(1));
    QVERIFY(f.isSelected(2));
    QVERIFY(!f.isSelected(3));
    QCOMPARE(f.selectedResources().count(), 1);
    QCOMPARE(f.selectedResources().first(), f.resourceAt(2));
    QCOMPARE(f.queryTerm(), buildTagTerm(f.resourceAt(2)));

    f.setSelected(1);
    QVERIFY(!f.isSelected(0));
    QVERIFY(f.isSelected(1));
    QVERIFY(f.isSelected(2));
    QVERIFY(!f.isSelected(3));
    QCOMPARE(f.selectedResources().count(), 2);
    QCOMPARE(f.queryTerm(), buildTagTermAll(QList<Tag>() << f.resourceAt(1) << f.resourceAt(2)));

    f.setSelected(3);
    QVERIFY(!f.isSelected(0));
    QVERIFY(f.isSelected(1));
    QVERIFY(f.isSelected(2));
    QVERIFY(f.isSelected(3));
    QCOMPARE(f.selectedResources().count(), 3);
    QCOMPARE(f.queryTerm(), buildTagTermAll(QList<Tag>() << f.resourceAt(1) << f.resourceAt(2) << f.resourceAt(3)));

    f.setSelected(2, false);
    QVERIFY(!f.isSelected(0));
    QVERIFY(f.isSelected(1));
    QVERIFY(!f.isSelected(2));
    QVERIFY(f.isSelected(3));
    QCOMPARE(f.selectedResources().count(), 2);
    QCOMPARE(f.queryTerm(), buildTagTermAll(QList<Tag>() << f.resourceAt(1) << f.resourceAt(3)));

    f.clearSelection();
    QVERIFY(!f.isSelected(0));
    QVERIFY(!f.isSelected(1));
    QVERIFY(!f.isSelected(2));
    QVERIFY(!f.isSelected(3));
    QCOMPARE(f.queryTerm(), Term());
    QCOMPARE(f.selectedResources().count(), 0);
}


void DynamicResourceFacetTest::testMatchAny()
{
    DynamicResourceTestFacet f;
    f.setRelation(Soprano::Vocabulary::NAO::hasTag());
    f.setSelectionMode(Facet::MatchAny);

    // wait for the facet to be populated. Since this is happening async
    // and there might be any number of layoutChanged() signals between 1 and 4
    // we can only wait for the number of choices to become 4
    int i = 4;
    while( f.count() < 4 && i-- > 0 ) {
        waitForSignal(&f, SIGNAL(layoutChanged(Nepomuk::Utils::Facet*)));
    }
    QCOMPARE(f.count(), 4);

    // now the facet is initially populated
    // test the matchall selection
    QVERIFY(!f.isSelected(0));
    QVERIFY(!f.isSelected(1));
    QVERIFY(!f.isSelected(2));
    QVERIFY(!f.isSelected(3));
    QCOMPARE(f.queryTerm(), Term());
    QCOMPARE(f.selectedResources().count(), 0);

    f.setSelected(2);
    QVERIFY(!f.isSelected(0));
    QVERIFY(!f.isSelected(1));
    QVERIFY(f.isSelected(2));
    QVERIFY(!f.isSelected(3));
    QCOMPARE(f.selectedResources().count(), 1);
    QCOMPARE(f.selectedResources().first(), f.resourceAt(2));
    QCOMPARE(f.queryTerm(), buildTagTerm(f.resourceAt(2)));

    f.setSelected(1);
    QVERIFY(!f.isSelected(0));
    QVERIFY(f.isSelected(1));
    QVERIFY(f.isSelected(2));
    QVERIFY(!f.isSelected(3));
    QCOMPARE(f.selectedResources().count(), 2);
    QCOMPARE(f.queryTerm(), buildTagTermAny(QList<Tag>() << f.resourceAt(1) << f.resourceAt(2)));

    f.setSelected(3);
    QVERIFY(!f.isSelected(0));
    QVERIFY(f.isSelected(1));
    QVERIFY(f.isSelected(2));
    QVERIFY(f.isSelected(3));
    QCOMPARE(f.selectedResources().count(), 3);
    QCOMPARE(f.queryTerm(), buildTagTermAny(QList<Tag>() << f.resourceAt(1) << f.resourceAt(2) << f.resourceAt(3)));

    f.setSelected(2, false);
    QVERIFY(!f.isSelected(0));
    QVERIFY(f.isSelected(1));
    QVERIFY(!f.isSelected(2));
    QVERIFY(f.isSelected(3));
    QCOMPARE(f.selectedResources().count(), 2);
    QCOMPARE(f.queryTerm(), buildTagTermAny(QList<Tag>() << f.resourceAt(1) << f.resourceAt(3)));

    f.clearSelection();
    QVERIFY(!f.isSelected(0));
    QVERIFY(!f.isSelected(1));
    QVERIFY(!f.isSelected(2));
    QVERIFY(!f.isSelected(3));
    QCOMPARE(f.queryTerm(), Term());
    QCOMPARE(f.selectedResources().count(), 0);
}


void DynamicResourceFacetTest::testSetClientQuery()
{
//    DynamicResourceTestFacet f;
//    f.setRelation(Soprano::Vocabulary::NAO::hasTag());
//    f.setSelectionMode(Facet::MatchAny);

//    // wait for the facet to be populated. Since this is happening async
//    // and there might be any number of layoutChanged() signals between 1 and 4
//    // we can only wait for the number of choices to become 4
//    int i = 4;
//    while( f.count() < 4 && i-- > 0 ) {
//        waitForSignal(&f, SIGNAL(layoutChanged(Nepomuk::Utils::Facet*)));
//    }
//    QCOMPARE(f.count(), 4);
}

QTEST_KDEMAIN_CORE(DynamicResourceFacetTest)
