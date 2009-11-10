/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2009 Sebastian Trueg <trueg@kde.org>

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

#include <Soprano/LiteralValue>
#include <Soprano/Node>
#include <Soprano/Vocabulary/NAO>

#include <nepomuk/property.h>


Q_DECLARE_METATYPE( Nepomuk::Query::Query )

using namespace Nepomuk::Query;

void QueryParserTest::testQueryParser_data()
{
    QTest::addColumn<QString>( "queryString" );
    QTest::addColumn<Nepomuk::Query::Query>( "query" );

    // simple literal queries
    QTest::newRow( "simple literal query" ) << QString( "Hello" ) << Query( LiteralTerm( "Hello" ) );
    QTest::newRow( "literal with spaces query" ) << QString( "'Hello World'" ) << Query( LiteralTerm( "Hello World" ) );

    // comparison queries
    QTest::newRow( "simple field query" )    << QString( "hastag:nepomuk" )
                                             << Query( ComparisonTerm( "hastag", LiteralTerm( "nepomuk" ) ) );
    QTest::newRow( "simple property query" ) << QString( "%1:nepomuk" ).arg( Soprano::Node::resourceToN3( Soprano::Vocabulary::NAO::hasTag() ) )
                                             << Query( ComparisonTerm( Soprano::Vocabulary::NAO::hasTag(), LiteralTerm( "nepomuk" ) ) );
#ifdef QUERY_PARSER_SUPPORTS_RESOURCE_VALUES
    QTest::newRow( "resource field query" )  << QString( "hastag:<nepomuk:/Nepomuk>" )
                                             << Query( ComparisonTerm( "hastag", ResourceTerm( QUrl( "nepomuk:/Nepomuk" ) ) ) );
#endif
    QTest::newRow( "nested resource query" ) << QString( "hastag:(label:nepomuk)" )
                                             << Query( ComparisonTerm( "hastag", ComparisonTerm( "label", LiteralTerm( "nepomuk" ) ) ) );


    // negation
    QTest::newRow( "simple negation" ) << QString( "-Hello" ) << Query( NegationTerm::negateTerm( LiteralTerm( "Hello" ) ) );
    QTest::newRow( "field negation" ) << QString( "-hastag:nepomuk" ) << Query( NegationTerm::negateTerm( ComparisonTerm( "hastag", LiteralTerm( "nepomuk" ) ) ) );

    // and query
    QTest::newRow( "and: two literals" )          << QString( "Hello World" ) << Query( AndTerm( QList<Term>() << LiteralTerm( "Hello" ) << LiteralTerm( "World" ) ) );
    QTest::newRow( "and: two literals with AND" ) << QString( "Hello AND World" ) << Query( AndTerm( QList<Term>() << LiteralTerm( "Hello" ) << LiteralTerm( "World" ) ) );

    // or queries
    QTest::newRow( "or: two literals" )          << QString( "Hello OR World" ) << Query( OrTerm( QList<Term>() << LiteralTerm( "Hello" ) << LiteralTerm( "World" ) ) );
}


void QueryParserTest::testQueryParser()
{
    QFETCH( QString, queryString );
    QFETCH( Nepomuk::Query::Query, query );

    qDebug() << "Wanted query:" << query;

    Query q = QueryParser::parseQuery( queryString );

    qDebug() << "Parsed query:" << q;

    QCOMPARE( q, query );
}


QTEST_MAIN( QueryParserTest )

#include "queryparsertest.moc"
