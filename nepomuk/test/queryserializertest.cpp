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

#include "queryserializertest.h"

#include "filequery.h"
#include "literalterm.h"
#include "resourceterm.h"
#include "andterm.h"
#include "orterm.h"
#include "negationterm.h"
#include "comparisonterm.h"
#include "resourcetypeterm.h"
#include "optionalterm.h"
#include "nie.h"
#include "nfo.h"
#include "nco.h"
#include "pimo.h"
#include "property.h"

#include <QtTest>

#include <Soprano/LiteralValue>
#include <Soprano/Node>
#include <Soprano/Vocabulary/NAO>
#include <Soprano/Vocabulary/RDFS>
#include <Soprano/Vocabulary/XMLSchema>

#include <kdebug.h>
#include <qtest_kde.h>

Q_DECLARE_METATYPE( Nepomuk::Query::Query )

using namespace Nepomuk::Query;


void QuerySerializerTest::testSerialization_data()
{
    QTest::addColumn<Nepomuk::Query::Query>( "query" );

    QTest::newRow( "simple literal query" )
        << Query( LiteralTerm( "Hello" ) );

    QTest::newRow( "simple literal query with space" )
        << Query( LiteralTerm( "Hello World" ) );
    QTest::newRow( "simple literal query with space and quotes" )
        << Query( LiteralTerm( "'Hello World'" ) );
    QTest::newRow( "simple literal query with space and quotes" )
        << Query( LiteralTerm( "\"Hello World\"" ) );

    QTest::newRow( "literal query with depth 2" )
        << Query( AndTerm( LiteralTerm("foo"), ComparisonTerm( Soprano::Vocabulary::NAO::hasTag(), ComparisonTerm( Soprano::Vocabulary::NAO::prefLabel(), LiteralTerm("bar") ) ) ) );

    QTest::newRow( "type query" )
        << Query( ResourceTypeTerm( Soprano::Vocabulary::NAO::Tag() ) );

    QTest::newRow( "negated resource type" )
        << Query( NegationTerm::negateTerm( ResourceTypeTerm( Soprano::Vocabulary::NAO::Tag() ) ) );

    QDateTime now = QDateTime::currentDateTime();
    QTest::newRow( "nie:lastModified" )
        << Query( ComparisonTerm( Nepomuk::Vocabulary::NIE::lastModified(), LiteralTerm( now ), ComparisonTerm::GreaterOrEqual ) );

    QTest::newRow( "hastag with literal term" )
        << Query( ComparisonTerm( Soprano::Vocabulary::NAO::hasTag(), LiteralTerm( QLatin1String("nepomuk")) ) );

    QTest::newRow( "hastag with resource" )
        << Query( ComparisonTerm( Soprano::Vocabulary::NAO::hasTag(), ResourceTerm( QUrl("nepomuk:/res/foobar") ) ));

    QTest::newRow( "negated hastag with resource" )
        << Query( NegationTerm::negateTerm(ComparisonTerm( Soprano::Vocabulary::NAO::hasTag(), ResourceTerm( QUrl("nepomuk:/res/foobar") ) )));

    QTest::newRow( "comparators <" )
        << Query( ComparisonTerm( Soprano::Vocabulary::NAO::numericRating(), LiteralTerm(4), ComparisonTerm::Smaller ) );

    QTest::newRow( "comparators <=" )
        << Query( ComparisonTerm( Soprano::Vocabulary::NAO::numericRating(), LiteralTerm(4), ComparisonTerm::SmallerOrEqual ) );

    QTest::newRow( "comparators >" )
        << Query( ComparisonTerm( Soprano::Vocabulary::NAO::numericRating(), LiteralTerm(4), ComparisonTerm::Greater ) );

    QTest::newRow( "comparators >=" )
        << Query( ComparisonTerm( Soprano::Vocabulary::NAO::numericRating(), LiteralTerm(4), ComparisonTerm::GreaterOrEqual ) );

    QTest::newRow( "inverted comparisonterm" )
        << Query( ComparisonTerm( Soprano::Vocabulary::NAO::hasTag(), ResourceTerm( QUrl("nepomuk:/res/foobar") ) ).inverted() );

    QTest::newRow( "optional term" )
        << Query(OptionalTerm::optionalizeTerm(ComparisonTerm( Soprano::Vocabulary::NAO::hasTag(), ResourceTerm( QUrl("nepomuk:/res/foobar") ) )));

    QTest::newRow( "and term" )
        << Query( AndTerm( ComparisonTerm( Soprano::Vocabulary::NAO::numericRating(), LiteralTerm(4), ComparisonTerm::Greater ),
                           ComparisonTerm( Soprano::Vocabulary::NAO::hasTag(), ResourceTerm(QUrl("nepomuk:/test")) ) ) );

    ComparisonTerm setVarNameTerm1( Soprano::Vocabulary::NAO::hasTag(), ResourceTypeTerm( Soprano::Vocabulary::NAO::Tag() ) );
    setVarNameTerm1.setVariableName("myvar");
    QTest::newRow( "set variable name 1" )
        << Query( setVarNameTerm1 );

    ComparisonTerm setVarNameTerm2( Soprano::Vocabulary::NAO::hasTag(), LiteralTerm( "nepomuk" ) );
    setVarNameTerm2.setVariableName( "myvar" );
    QTest::newRow( "set variable name 2" )
        << Query( setVarNameTerm2 );

    ComparisonTerm setVarNameTerm3( Soprano::Vocabulary::NAO::numericRating(), LiteralTerm(4), ComparisonTerm::Smaller );
    setVarNameTerm3.setVariableName("myvar");
    QTest::newRow( "set variable name 3" )
        << Query( setVarNameTerm3 );

    ComparisonTerm setVarNameTerm4( Soprano::Vocabulary::NAO::numericRating(), LiteralTerm(4), ComparisonTerm::Smaller );
    setVarNameTerm3.setVariableName("myvar" );
    setVarNameTerm3.setAggregateFunction(ComparisonTerm::Count);
    QTest::newRow( "set variable name 4 (with aggregate function count)" )
        << Query( setVarNameTerm3 );

    ComparisonTerm orderByTerm1( Soprano::Vocabulary::NAO::numericRating(), LiteralTerm(4), ComparisonTerm::Smaller );
    orderByTerm1.setSortWeight( 1 );
    QTest::newRow( "order by 1" )
        << Query( orderByTerm1 );

    orderByTerm1.setSortWeight( 1, Qt::DescendingOrder );
    QTest::newRow( "order by 2" )
        << Query( orderByTerm1 );

    ComparisonTerm orderByTerm2( Soprano::Vocabulary::NAO::prefLabel(), LiteralTerm("hello") );
    orderByTerm2.setSortWeight( 2 );

    QTest::newRow( "order by 3" )
        << Query( AndTerm( orderByTerm1, orderByTerm2 ) );

    orderByTerm1.setVariableName("myvar");
    QTest::newRow( "order by 4" )
        << Query( orderByTerm1 );

    QTest::newRow( "ComparisonTerm with invalid property" )
        << Query( ComparisonTerm( Nepomuk::Types::Property(), ResourceTerm( QUrl("nepomuk:/res/foobar") ) ));

    QTest::newRow( "ComparisonTerm with invalid subterm" )
        << Query( ComparisonTerm( Soprano::Vocabulary::NAO::hasTag(), Term() ) );

    QTest::newRow( "ComparisonTerm with invalid property and subterm" )
        << Query( ComparisonTerm( Nepomuk::Types::Property(), Term() ) );

    ComparisonTerm orderByTerm5( Soprano::Vocabulary::NAO::numericRating(), Term() );
    orderByTerm5.setSortWeight( 1 );
    orderByTerm5.setAggregateFunction( ComparisonTerm::Max );
    QTest::newRow( "order by 5 (with aggregate function and invalid subterm)" )
        << Query( orderByTerm5 );

    orderByTerm5.setVariableName( "myvar" );
    QTest::newRow( "order by 5 (with aggregate function and invalid subterm and varname)" )
        << Query( orderByTerm5 );

    orderByTerm5.setVariableName( QString() );
    orderByTerm5.setAggregateFunction( ComparisonTerm::DistinctCount );
    QTest::newRow( "order by 5 (with aggregate function and invalid subterm and varname)" )
        << Query( orderByTerm5 );


    FileQuery fileQuery( ComparisonTerm( Soprano::Vocabulary::NAO::hasTag(), ResourceTerm(QUrl("nepomuk:/res/foobar")) ) );
    QTest::newRow( "file query" )
        << Query(fileQuery);

    fileQuery.setFileMode(FileQuery::QueryFiles);
    QTest::newRow( "file query (only files)" )
        << Query(fileQuery);

    fileQuery.setFileMode(FileQuery::QueryFolders);
    QTest::newRow( "file query (only folders)" )
        << Query(fileQuery);

    //
    // A more complex example
    //
    QUrl res("nepomuk:/res/foobar");
    AndTerm mainTerm;
    OrTerm typeOr;
    typeOr.addSubTerm( ResourceTypeTerm( Nepomuk::Vocabulary::NFO::RasterImage() ) );
    typeOr.addSubTerm( ResourceTypeTerm( Nepomuk::Vocabulary::NFO::Audio() ) );
    mainTerm.addSubTerm( typeOr );
    mainTerm.addSubTerm( NegationTerm::negateTerm( ComparisonTerm( Nepomuk::Types::Property(), ResourceTerm( res ) ).inverted() ) );

    // an empty comparisonterm results in "?r ?v1 ?v2"
    ComparisonTerm ct;

    // change the var name: "?r ?v1 ?cnt"
    ct.setVariableName( "cnt" );
    ct.setAggregateFunction( ComparisonTerm::Count );

    // by default all have 0, Query::toSparqlQuery will add ORDER BY for all ComparisonTerm with sortweight != 0
    ct.setSortWeight( 1, Qt::DescendingOrder );

    mainTerm.addSubTerm(ct.inverted());

    QTest::newRow( "a complex one" )
        << Query( mainTerm );


    // try to combine it all
    Query allQuery( OrTerm(
                        NegationTerm::negateTerm( ResourceTypeTerm( Soprano::Vocabulary::NAO::Tag() ) ),
                        OptionalTerm::optionalizeTerm( LiteralTerm("hello 3") ),
                        AndTerm(
                            LiteralTerm("hello"),
                            LiteralTerm("hello 2"),
                            ResourceTerm( QUrl("nepomuk:/res/foobar") ),
                            ResourceTypeTerm( Soprano::Vocabulary::NAO::Tag() ),
                            ComparisonTerm( Nepomuk::Types::Property(),
                                            ResourceTerm( QUrl("nepomuk:/res/foobar") ) ),
                            ComparisonTerm( Nepomuk::Types::Property(),
                                            OrTerm( ResourceTerm( QUrl("nepomuk:/res/foobar1") ),
                                                    ResourceTerm( QUrl("nepomuk:/res/foobar2") ) ) ) ) ) );
    allQuery.setLimit( 10 );
    allQuery.setOffset( 2 );

    QTest::newRow( "all" )
        << allQuery;

    FileQuery fileAllQuery( allQuery );
    fileAllQuery.addIncludeFolder( KUrl("/home/trueg/1") );
    fileAllQuery.addIncludeFolder( KUrl("/home/trueg/2") );
    fileAllQuery.addExcludeFolder( KUrl("/home/trueg/3") );
    fileAllQuery.addExcludeFolder( KUrl("/home/trueg/4") );
    QTest::newRow( "fileall" )
        << Query(fileAllQuery);
}


void QuerySerializerTest::testSerialization()
{
    QFETCH( Nepomuk::Query::Query, query );

//     qDebug() << "Before:" << serializeQuery( query );
//     qDebug() << "After: " << serializeQuery( parseQuery( serializeQuery( query ) ) );

    QCOMPARE( Query::fromString( query.toString() ), query );
    QCOMPARE( Term::fromString( query.term().toString() ), query.term() );
    QCOMPARE( Query::fromString( query.toString() ).toSparqlQuery(), query.toSparqlQuery() );
}

QTEST_KDEMAIN_CORE( QuerySerializerTest )

#include "queryserializertest.moc"
