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

#include "querytest.h"

#include "query.h"
#include "literalterm.h"
#include "resourceterm.h"
#include "andterm.h"
#include "orterm.h"
#include "negationterm.h"
#include "comparisonterm.h"
#include "resourcetypeterm.h"
#include "nie.h"

#include <QtTest>

#include <Soprano/LiteralValue>
#include <Soprano/Node>
#include <Soprano/Vocabulary/NAO>
#include <Soprano/Vocabulary/RDFS>

#include <kdebug.h>

Q_DECLARE_METATYPE( Nepomuk::Query::Query )

using namespace Nepomuk::Query;


// this is a tricky one as we nee to match the variable names and order of the queries exactly.
void QueryTest::testToSparql_data()
{
    QTest::addColumn<Nepomuk::Query::Query>( "query" );
    QTest::addColumn<QString>( "queryString" );

    QTest::newRow( "simple literal query" )
        << Query( LiteralTerm( "Hello" ) )
        << QString::fromLatin1( "select distinct ?r where { ?r ?v1 ?v2 . ?v2 bif:contains \"'Hello*'\" . }" );

    QTest::newRow( "type query" )
        << Query( ResourceTypeTerm( Soprano::Vocabulary::NAO::Tag() ) )
        << QString::fromLatin1("select distinct ?r where { ?r a ?v1 . ?v1 %1 %2 . }")
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::RDFS::subClassOf()))
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::Tag()));

    QDateTime now = QDateTime::currentDateTime();
    QTest::newRow( "nie:lastModified" )
        << Query( ComparisonTerm( Nepomuk::Vocabulary::NIE::lastModified(), LiteralTerm( now ), ComparisonTerm::GreaterOrEqual ) )
        << QString::fromLatin1("select distinct ?r where { ?r %1 ?v1 . FILTER(?v1>=%2) . }")
        .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NIE::lastModified()),
             Soprano::Node::literalToN3(now));

    QTest::newRow( "hastag with literal term" )
        << Query( ComparisonTerm( Soprano::Vocabulary::NAO::hasTag(), LiteralTerm( QLatin1String("nepomuk")) ) )
        << QString::fromLatin1("select distinct ?r where { ?r %1 ?v1 . ?v1 ?v2 ?v3 . ?v2 %2 %3 . ?v3 bif:contains \"'nepomuk*'\" . }")
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::hasTag()))
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::RDFS::subPropertyOf()))
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::RDFS::label()));

    QTest::newRow( "hastag with resource" )
        << Query( ComparisonTerm( Soprano::Vocabulary::NAO::hasTag(), ResourceTerm( QUrl("nepomuk:/res/foobar") ) ))
        << QString::fromLatin1("select distinct ?r where { ?r %1 <nepomuk:/res/foobar> . }")
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::hasTag()));

    QTest::newRow( "negated hastag with resource" )
        << Query( NegationTerm::negateTerm(ComparisonTerm( Soprano::Vocabulary::NAO::hasTag(), ResourceTerm( QUrl("nepomuk:/res/foobar") ) )))
        << QString::fromLatin1("select distinct ?r where { OPTIONAL { ?v1 %1 <nepomuk:/res/foobar> . FILTER(?v1=?r) . } . FILTER(!BOUND(?v1)) . }")
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::hasTag()));
}


void QueryTest::testToSparql()
{
    QFETCH( Nepomuk::Query::Query, query );
    QFETCH( QString, queryString );

    QCOMPARE( query.toSparqlQuery().simplified(), queryString );
}

QTEST_MAIN( QueryTest )

#include "querytest.moc"
