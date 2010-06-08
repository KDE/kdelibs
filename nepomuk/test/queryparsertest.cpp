/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2009-2010 Sebastian Trueg <trueg@kde.org>

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

#include "queryparsertest.h"
#include "queryparser.h"
#include "query.h"
#include "literalterm.h"
#include "resourceterm.h"
#include "andterm.h"
#include "orterm.h"
#include "negationterm.h"
#include "comparisonterm.h"

#include <QtTest>
#include <qtest_kde.h>

#include "ktempdir.h"

#include <Soprano/LiteralValue>
#include <Soprano/Node>
#include <Soprano/StorageModel>
#include <Soprano/Backend>
#include <Soprano/PluginManager>
#include <Soprano/Vocabulary/NAO>
#include <Soprano/Vocabulary/NRL>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/RDFS>
#include <Soprano/Vocabulary/XMLSchema>

#include "property.h"
#include "resourcemanager.h"

Q_DECLARE_METATYPE( Nepomuk::Query::Query )

using namespace Nepomuk::Query;

void QueryParserTest::initTestCase()
{
    // we need to use a Virtuoso model as tmp model since redland misses important SPARQL features
    // that are used by libnepomuk below
    const Soprano::Backend* backend = Soprano::PluginManager::instance()->discoverBackendByName( "virtuosobackend" );
    QVERIFY( backend );
    m_storageDir = new KTempDir();
    m_model = backend->createModel( Soprano::BackendSettings() << Soprano::BackendSetting(Soprano::BackendOptionStorageDir, m_storageDir->name()) );
    QVERIFY( m_model );

    // we create one fake ontology
    QUrl graph("graph:/onto");
    m_model->addStatement( graph, Soprano::Vocabulary::RDF::type(), Soprano::Vocabulary::NRL::Ontology(), graph );

    m_model->addStatement( QUrl("onto:/label"), Soprano::Vocabulary::RDF::type(), Soprano::Vocabulary::RDF::Property(), graph );
    m_model->addStatement( QUrl("onto:/label"), Soprano::Vocabulary::RDFS::label(), Soprano::LiteralValue::createPlainLiteral("label"), graph );
    m_model->addStatement( QUrl("onto:/label"), Soprano::Vocabulary::RDFS::range(), Soprano::Vocabulary::XMLSchema::string(), graph );

    m_model->addStatement( QUrl("onto:/hasTag"), Soprano::Vocabulary::RDF::type(), Soprano::Vocabulary::RDF::Property(), graph );
    m_model->addStatement( QUrl("onto:/hasTag"), Soprano::Vocabulary::RDFS::label(), Soprano::LiteralValue::createPlainLiteral("has tag"), graph );
    m_model->addStatement( QUrl("onto:/hasTag"), Soprano::Vocabulary::RDFS::range(), QUrl("onto:/Tag"), graph );

    m_model->addStatement( QUrl("onto:/tag"), Soprano::Vocabulary::RDF::type(), Soprano::Vocabulary::RDF::Property(), graph );
    m_model->addStatement( QUrl("onto:/tag"), Soprano::Vocabulary::RDFS::label(), Soprano::LiteralValue::createPlainLiteral("hastag"), graph );
    m_model->addStatement( QUrl("onto:/tag"), Soprano::Vocabulary::RDFS::range(), QUrl("onto:/Tag"), graph );

    m_model->addStatement( QUrl("onto:/Tag"), Soprano::Vocabulary::RDF::type(), Soprano::Vocabulary::RDFS::Class(), graph );

    m_model->addStatement( QUrl("onto:/int"), Soprano::Vocabulary::RDF::type(), Soprano::Vocabulary::RDF::Property(), graph );
    m_model->addStatement( QUrl("onto:/int"), Soprano::Vocabulary::RDFS::label(), Soprano::LiteralValue::createPlainLiteral("int value"), graph );
    m_model->addStatement( QUrl("onto:/int"), Soprano::Vocabulary::RDFS::range(), Soprano::Vocabulary::XMLSchema::integer(), graph );

    Nepomuk::ResourceManager::instance()->setOverrideMainModel( m_model );
}


void QueryParserTest::cleanupTestCase()
{
    Nepomuk::ResourceManager::instance()->setOverrideMainModel( 0 );
    delete m_model;
    delete m_storageDir;
}


void QueryParserTest::testQueryParser_data()
{
    QTest::addColumn<QString>( "queryString" );
    QTest::addColumn<Nepomuk::Query::Query>( "query" );

    // simple literal queries
    QTest::newRow( "simple literal query" ) << QString( "Hello" ) << Query( LiteralTerm( "Hello" ) );
    QTest::newRow( "literal with spaces without quotes" ) << QString( "Hello World" ) << Query( AndTerm( LiteralTerm("Hello"), LiteralTerm("World" ) ) );
    QTest::newRow( "literal with spaces with quotes" ) << QString( "'Hello World'" ) << Query( LiteralTerm( "Hello World" ) );

    // comparison queries
    QTest::newRow( "simple field query" )    << QString( "hastag:nepomuk" )
                                             << Query( OrTerm( ComparisonTerm( QUrl("onto:/hasTag"), LiteralTerm( "nepomuk" ) ),
                                                               ComparisonTerm( QUrl("onto:/tag"), LiteralTerm( "nepomuk" ) ) ) );
    QTest::newRow( "simple property query" ) << QString( "<onto:/hasTag>:nepomuk" )
                                             << Query( ComparisonTerm( QUrl("onto:/hasTag"), LiteralTerm( "nepomuk" ) ) );
#ifdef QUERY_PARSER_SUPPORTS_RESOURCE_VALUES
    QTest::newRow( "resource field query" )  << QString( "hastag:<nepomuk:/Nepomuk>" )
                                             << Query( ComparisonTerm( "hastag", ResourceTerm( QUrl( "nepomuk:/Nepomuk" ) ) ) );
#endif
    QTest::newRow( "nested resource query" ) << QString( "hastag:(label:nepomuk)" )
                                             << Query( OrTerm( ComparisonTerm( QUrl("onto:/hasTag"), ComparisonTerm( QUrl("onto:/label"), LiteralTerm( "nepomuk" ) ) ),
                                                               ComparisonTerm( QUrl("onto:/tag"), ComparisonTerm( QUrl("onto:/label"), LiteralTerm( "nepomuk" ) ) ) ) );
    QTest::newRow( "int property query" ) << QString( "'int value':42" )
                                          << Query( ComparisonTerm( QUrl("onto:/int"), LiteralTerm( 42 ) ) );
    QTest::newRow( "int property query 2" ) << QString( "int:\"42\"" )
                                            << Query( ComparisonTerm( QUrl("onto:/int"), LiteralTerm( 42 ) ) );


    // negation
    QTest::newRow( "simple negation" ) << QString( "-Hello" ) << Query( NegationTerm::negateTerm( LiteralTerm( "Hello" ) ) );
    QTest::newRow( "field negation" ) << QString( "-label:nepomuk" ) << Query( NegationTerm::negateTerm( ComparisonTerm( QUrl("onto:/label"), LiteralTerm( "nepomuk" ) ) ) );

    // and query
    QTest::newRow( "and: two literals" )          << QString( "Hello World" ) << Query( AndTerm( LiteralTerm( "Hello" ), LiteralTerm( "World" ) ) );
    QTest::newRow( "and: two literals with AND" ) << QString( "Hello AND World" ) << Query( AndTerm( LiteralTerm( "Hello" ), LiteralTerm( "World" ) ) );

    // or queries
    QTest::newRow( "or: two literals" )          << QString( "Hello OR World" ) << Query( OrTerm( LiteralTerm( "Hello" ), LiteralTerm( "World" ) ) );
}


void QueryParserTest::testQueryParser()
{
    QFETCH( QString, queryString );
    QFETCH( Nepomuk::Query::Query, query );

    Query q = QueryParser::parseQuery( queryString );

//    qDebug() << "Wanted query:" << query;
//    qDebug() << "Parsed query:" << q;

    QCOMPARE( q, query );
}


void QueryParserTest::testQueryParserWithGlobbing_data()
{
    QTest::addColumn<QString>( "queryString" );
    QTest::addColumn<Nepomuk::Query::Query>( "query" );

    // simple literal queries
    QTest::newRow( "simple literal query" ) << QString( "Hello" ) << Query( LiteralTerm( "Hello*" ) );
    QTest::newRow( "simple literal query" ) << QString( "\"Hello\"" ) << Query( LiteralTerm( "Hello" ) );
    QTest::newRow( "literal with spaces without quotes" ) << QString( "Hello World" ) << Query( AndTerm( LiteralTerm("Hello*"), LiteralTerm("World*" ) ) );
    QTest::newRow( "literal with spaces with quotes" ) << QString( "'Hello World'" ) << Query( LiteralTerm( "Hello World" ) );
}


void QueryParserTest::testQueryParserWithGlobbing()
{
    QFETCH( QString, queryString );
    QFETCH( Nepomuk::Query::Query, query );

    QueryParser p;
    Query q = p.parse( queryString, QueryParser::QueryTermGlobbing );

//     qDebug() << "Wanted query:" << query;
//     qDebug() << "Parsed query:" << q;

    QCOMPARE( q, query );
}

QTEST_KDEMAIN_CORE( QueryParserTest )

#include "queryparsertest.moc"
