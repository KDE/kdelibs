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

#include "querytest.h"

#include "query.h"
#include "filequery.h"
#include "query_p.h"
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
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>

#include <kdebug.h>
#include <qtest_kde.h>

Q_DECLARE_METATYPE( Nepomuk::Query::Query )

using namespace Nepomuk::Query;

namespace QTest {
template<>
char* toString(const Nepomuk::Query::Query& query) {
    return qstrdup( query.toString().toUtf8().data() );
}

template<>
char* toString(const Nepomuk::Query::Term& term) {
    return qstrdup( term.toString().toUtf8().data() );
}
}


// this is a tricky one as we nee to match the variable names and order of the queries exactly.
void QueryTest::testToSparql_data()
{
    QTest::addColumn<Nepomuk::Query::Query>( "query" );
    QTest::addColumn<QString>( "queryString" );

    QTest::newRow( "simple literal query" )
        << Query( LiteralTerm( "Hello" ) )
        << QString::fromLatin1( "select distinct ?r max(?v5) as ?_n_f_t_m_s_ where { { ?r ?v1 ?v2 . ?v2 bif:contains \"'Hello'\" OPTION (score ?v5) . } "
                                "UNION { ?r ?v1 ?v3 . ?v3 ?v4 ?v2 . ?v4 %1 %2 . ?v2 bif:contains \"'Hello'\" OPTION (score ?v5) . } . }" )
        .arg( Soprano::Node::resourceToN3(Soprano::Vocabulary::RDFS::subPropertyOf()),
              Soprano::Node::resourceToN3(Soprano::Vocabulary::RDFS::label()) );

    QString helloWorldQuery = QString::fromLatin1( "select distinct ?r max(?v5) as ?_n_f_t_m_s_ where { { ?r ?v1 ?v2 . ?v2 bif:contains \"'Hello World'\" OPTION (score ?v5) . } "
                                                   "UNION "
                                                   "{ ?r ?v1 ?v3 . ?v3 ?v4 ?v2 . ?v4 %1 %2 . ?v2 bif:contains \"'Hello World'\" OPTION (score ?v5) . } . }" )
                              .arg( Soprano::Node::resourceToN3(Soprano::Vocabulary::RDFS::subPropertyOf()),
                                    Soprano::Node::resourceToN3(Soprano::Vocabulary::RDFS::label()) );
    QTest::newRow( "simple literal query with space" )
        << Query( LiteralTerm( "Hello World" ) )
        << helloWorldQuery;
    QTest::newRow( "simple literal query with space and quotes" )
        << Query( LiteralTerm( "'Hello World'" ) )
        << helloWorldQuery;
    QTest::newRow( "simple literal query with space and quotes" )
        << Query( LiteralTerm( "\"Hello World\"" ) )
        << helloWorldQuery;

    QTest::newRow( "literal query with depth 2" )
        << Query( AndTerm( LiteralTerm("foo"), ComparisonTerm( Soprano::Vocabulary::NAO::hasTag(), ComparisonTerm( Soprano::Vocabulary::NAO::prefLabel(), LiteralTerm("bar") ) ) ) )
        << QString::fromLatin1("select distinct ?r max((?v8/2)+?v5) as ?_n_f_t_m_s_ where { "
                               "{ { ?r ?v1 ?v2 . ?v2 bif:contains \"'foo'\" OPTION (score ?v5) . } "
                               "UNION "
                               "{ ?r ?v1 ?v3 . ?v3 ?v4 ?v2 . "
                               "?v4 <http://www.w3.org/2000/01/rdf-schema#subPropertyOf> <http://www.w3.org/2000/01/rdf-schema#label> . ?v2 bif:contains \"'foo'\" OPTION (score ?v5) . } . "
                               "?r <http://www.semanticdesktop.org/ontologies/2007/08/15/nao#hasTag> ?v6 . ?v6 <http://www.semanticdesktop.org/ontologies/2007/08/15/nao#prefLabel> ?v7 . "
                               "?v7 bif:contains \"'bar'\" OPTION (score ?v8) . } . }");

    QTest::newRow( "type query" )
        << Query( ResourceTypeTerm( Soprano::Vocabulary::NAO::Tag() ) )
        << QString::fromLatin1("select distinct ?r where { ?r a ?v1 . ?v1 %1 %2 . }")
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::RDFS::subClassOf()))
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::Tag()));

    QTest::newRow( "negated resource type" )
        << Query( NegationTerm::negateTerm( ResourceTypeTerm( Soprano::Vocabulary::NAO::Tag() ) ) )
        << QString::fromLatin1("select distinct ?r where { { FILTER(!bif:exists((select (1) where { ?r a ?v1 . ?v1 %1 %2 . }))) . "
                               "?r %3 ?v2 . } . }")
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::RDFS::subClassOf()),
             Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::Tag()),
             Soprano::Node::resourceToN3(Soprano::Vocabulary::RDF::type()));

    QDateTime now = QDateTime::currentDateTime();
    QTest::newRow( "nie:lastModified" )
        << Query( ComparisonTerm( Nepomuk::Vocabulary::NIE::lastModified(), LiteralTerm( now ), ComparisonTerm::GreaterOrEqual ) )
        << QString::fromLatin1("select distinct ?r where { ?r %1 ?v1 . FILTER(?v1>=%2) . }")
        .arg(Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NIE::lastModified()),
             Soprano::Node::literalToN3(now));

    QTest::newRow( "hastag with literal term" )
        << Query( ComparisonTerm( Soprano::Vocabulary::NAO::hasTag(), LiteralTerm( QLatin1String("nepomuk")) ) )
        << QString::fromLatin1("select distinct ?r max(?v4) as ?_n_f_t_m_s_ where { ?r %1 ?v1 . ?v1 ?v2 ?v3 . ?v2 %2 %3 . ?v3 bif:contains \"'nepomuk'\" OPTION (score ?v4) . }")
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::hasTag()))
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::RDFS::subPropertyOf()))
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::RDFS::label()));

    QTest::newRow( "hastag with resource" )
        << Query( ComparisonTerm( Soprano::Vocabulary::NAO::hasTag(), ResourceTerm( QUrl("nepomuk:/res/foobar") ) ))
        << QString::fromLatin1("select distinct ?r where { ?r %1 <nepomuk:/res/foobar> . }")
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::hasTag()));

    QTest::newRow( "negated hastag with resource" )
        << Query( NegationTerm::negateTerm(ComparisonTerm( Soprano::Vocabulary::NAO::hasTag(), ResourceTerm( QUrl("nepomuk:/res/foobar") ) )))
        << QString::fromLatin1("select distinct ?r where { { FILTER(!bif:exists((select (1) where { ?r %1 <nepomuk:/res/foobar> . }))) . "
                               "?r %2 ?v1 . } . }")
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::hasTag()),
             Soprano::Node::resourceToN3(Soprano::Vocabulary::RDF::type()));

    QTest::newRow( "comparators <" )
        << Query( ComparisonTerm( Soprano::Vocabulary::NAO::numericRating(), LiteralTerm(4), ComparisonTerm::Smaller ) )
        << QString::fromLatin1("select distinct ?r where { ?r %1 ?v1 . FILTER(?v1<\"4\"^^%2) . }")
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::numericRating()),
             Soprano::Node::resourceToN3(Soprano::Vocabulary::XMLSchema::xsdInt()) );

    QTest::newRow( "comparators <=" )
        << Query( ComparisonTerm( Soprano::Vocabulary::NAO::numericRating(), LiteralTerm(4), ComparisonTerm::SmallerOrEqual ) )
        << QString::fromLatin1("select distinct ?r where { ?r %1 ?v1 . FILTER(?v1<=\"4\"^^%2) . }")
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::numericRating()),
             Soprano::Node::resourceToN3(Soprano::Vocabulary::XMLSchema::xsdInt()) );

    QTest::newRow( "comparators >" )
        << Query( ComparisonTerm( Soprano::Vocabulary::NAO::numericRating(), LiteralTerm(4), ComparisonTerm::Greater ) )
        << QString::fromLatin1("select distinct ?r where { ?r %1 ?v1 . FILTER(?v1>\"4\"^^%2) . }")
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::numericRating()),
             Soprano::Node::resourceToN3(Soprano::Vocabulary::XMLSchema::xsdInt()) );

    QTest::newRow( "comparators >=" )
        << Query( ComparisonTerm( Soprano::Vocabulary::NAO::numericRating(), LiteralTerm(4), ComparisonTerm::GreaterOrEqual ) )
        << QString::fromLatin1("select distinct ?r where { ?r %1 ?v1 . FILTER(?v1>=\"4\"^^%2) . }")
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::numericRating()),
             Soprano::Node::resourceToN3(Soprano::Vocabulary::XMLSchema::xsdInt()) );

    QTest::newRow( "inverted comparisonterm" )
        << Query( ComparisonTerm( Soprano::Vocabulary::NAO::hasTag(), ResourceTerm( QUrl("nepomuk:/res/foobar") ) ).inverted() )
        << QString::fromLatin1("select distinct ?r where { <nepomuk:/res/foobar> %1 ?r . }")
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::hasTag()));

    QTest::newRow( "optional term" )
        << Query(OptionalTerm::optionalizeTerm(ComparisonTerm( Soprano::Vocabulary::NAO::hasTag(), ResourceTerm( QUrl("nepomuk:/res/foobar") ) )))
        << QString::fromLatin1("select distinct ?r where { OPTIONAL { ?r %1 <nepomuk:/res/foobar> . } }")
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::hasTag()));

    QTest::newRow( "and term" )
        << Query( AndTerm( ComparisonTerm( Soprano::Vocabulary::NAO::numericRating(), LiteralTerm(4), ComparisonTerm::Greater ),
                           ComparisonTerm( Soprano::Vocabulary::NAO::hasTag(), ResourceTerm(QUrl("nepomuk:/test")) ) ) )
        << QString::fromLatin1("select distinct ?r where { { ?r %1 ?v1 . FILTER(?v1>\"4\"^^%2) . ?r %3 <nepomuk:/test> . } . }")
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::numericRating()),
             Soprano::Node::resourceToN3(Soprano::Vocabulary::XMLSchema::xsdInt()),
             Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::hasTag()));

    ComparisonTerm setVarNameTerm1( Soprano::Vocabulary::NAO::hasTag(), ResourceTypeTerm( Soprano::Vocabulary::NAO::Tag() ) );
    setVarNameTerm1.setVariableName("myvar");
    QTest::newRow( "set variable name 1" )
        << Query( setVarNameTerm1 )
        << QString::fromLatin1("select distinct ?r ?myvar where { ?r %1 ?myvar . ?myvar a ?v1 . ?v1 %2 %3 . }")
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::hasTag()),
             Soprano::Node::resourceToN3(Soprano::Vocabulary::RDFS::subClassOf()),
             Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::Tag()));

    ComparisonTerm setVarNameTerm2( Soprano::Vocabulary::NAO::hasTag(), LiteralTerm( "nepomuk" ) );
    setVarNameTerm2.setVariableName( "myvar" );
    QTest::newRow( "set variable name 2" )
        << Query( setVarNameTerm2 )
        << QString::fromLatin1("select distinct ?r ?myvar max(?v3) as ?_n_f_t_m_s_ where { ?r %1 ?myvar . ?myvar ?v1 ?v2 . ?v1 %2 %3 . ?v2 bif:contains \"'nepomuk'\" OPTION (score ?v3) . }")
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::hasTag()),
             Soprano::Node::resourceToN3(Soprano::Vocabulary::RDFS::subPropertyOf()),
             Soprano::Node::resourceToN3(Soprano::Vocabulary::RDFS::label()));

    ComparisonTerm setVarNameTerm3( Soprano::Vocabulary::NAO::numericRating(), LiteralTerm(4), ComparisonTerm::Smaller );
    setVarNameTerm3.setVariableName("myvar");
    QTest::newRow( "set variable name 3" )
        << Query( setVarNameTerm3 )
        << QString::fromLatin1("select distinct ?r ?myvar where { ?r %1 ?myvar . FILTER(?myvar<\"4\"^^%2) . }")
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::numericRating()),
             Soprano::Node::resourceToN3(Soprano::Vocabulary::XMLSchema::xsdInt()) );

    ComparisonTerm setVarNameTerm4( Soprano::Vocabulary::NAO::numericRating(), LiteralTerm(4), ComparisonTerm::Smaller );
    setVarNameTerm3.setVariableName("myvar" );
    setVarNameTerm3.setAggregateFunction(ComparisonTerm::Count);
    QTest::newRow( "set variable name 4 (with aggregate function count)" )
        << Query( setVarNameTerm3 )
        << QString::fromLatin1("select distinct ?r count(?v1) as ?myvar where { ?r %1 ?v1 . FILTER(?v1<\"4\"^^%2) . }")
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::numericRating()),
             Soprano::Node::resourceToN3(Soprano::Vocabulary::XMLSchema::xsdInt()) );

    ComparisonTerm orderByTerm1( Soprano::Vocabulary::NAO::numericRating(), LiteralTerm(4), ComparisonTerm::Smaller );
    orderByTerm1.setSortWeight( 1 );
    QTest::newRow( "order by 1" )
        << Query( orderByTerm1 )
        << QString::fromLatin1("select distinct ?r where { ?r %1 ?v1 . FILTER(?v1<\"4\"^^%2) . } ORDER BY ASC ( ?v1 )")
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::numericRating()),
             Soprano::Node::resourceToN3(Soprano::Vocabulary::XMLSchema::xsdInt()) );

    orderByTerm1.setSortWeight( 1, Qt::DescendingOrder );
    QTest::newRow( "order by 2" )
        << Query( orderByTerm1 )
        << QString::fromLatin1("select distinct ?r where { ?r %1 ?v1 . FILTER(?v1<\"4\"^^%2) . } ORDER BY DESC ( ?v1 )")
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::numericRating()),
             Soprano::Node::resourceToN3(Soprano::Vocabulary::XMLSchema::xsdInt()) );

    ComparisonTerm orderByTerm2( Soprano::Vocabulary::NAO::prefLabel(), LiteralTerm("hello") );
    orderByTerm2.setSortWeight( 2 );

    QTest::newRow( "order by 3" )
        << Query( AndTerm( orderByTerm1, orderByTerm2 ) )
        << QString::fromLatin1("select distinct ?r max(?v3) as ?_n_f_t_m_s_ where { { ?r %1 ?v1 . FILTER(?v1<\"4\"^^%2) . ?r %3 ?v2 . ?v2 bif:contains \"'hello'\" OPTION (score ?v3) . } . } ORDER BY ASC ( ?v2 ) DESC ( ?v1 )")
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::numericRating()),
             Soprano::Node::resourceToN3(Soprano::Vocabulary::XMLSchema::xsdInt()),
             Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::prefLabel()) );

    orderByTerm1.setVariableName("myvar");
    QTest::newRow( "order by 4" )
        << Query( orderByTerm1 )
        << QString::fromLatin1("select distinct ?r ?myvar where { ?r %1 ?myvar . FILTER(?myvar<\"4\"^^%2) . } ORDER BY DESC ( ?myvar )")
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::numericRating()),
             Soprano::Node::resourceToN3(Soprano::Vocabulary::XMLSchema::xsdInt()) );

    QTest::newRow( "ComparisonTerm with invalid property" )
        << Query( ComparisonTerm( Nepomuk::Types::Property(), ResourceTerm( QUrl("nepomuk:/res/foobar") ) ))
        << QString::fromLatin1("select distinct ?r where { ?r ?v1 <nepomuk:/res/foobar> . }");

    QTest::newRow( "ComparisonTerm with invalid subterm" )
        << Query( ComparisonTerm( Soprano::Vocabulary::NAO::hasTag(), Term() ) )
        << QString::fromLatin1("select distinct ?r where { ?r %1 ?v1 . }")
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::hasTag()));

    QTest::newRow( "ComparisonTerm with invalid property and subterm" )
        << Query( ComparisonTerm( Nepomuk::Types::Property(), Term() ) )
        << QString::fromLatin1("select distinct ?r where { ?r ?v1 ?v2 . }");

    ComparisonTerm orderByTerm5( Soprano::Vocabulary::NAO::numericRating(), Term() );
    orderByTerm5.setSortWeight( 1 );
    orderByTerm5.setAggregateFunction( ComparisonTerm::Max );
    QTest::newRow( "order by 5 (with aggregate function and invalid subterm)" )
        << Query( orderByTerm5 )
        << QString::fromLatin1("select distinct ?r where { ?r %1 ?v1 . } ORDER BY ASC ( max(?v1) )")
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::numericRating()) );

    orderByTerm5.setVariableName( "myvar" );
    QTest::newRow( "order by 5 (with aggregate function and invalid subterm and varname)" )
        << Query( orderByTerm5 )
        << QString::fromLatin1("select distinct ?r max(?v1) as ?myvar where { ?r %1 ?v1 . } ORDER BY ASC ( ?myvar )")
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::numericRating()) );

    orderByTerm5.setVariableName( QString() );
    orderByTerm5.setAggregateFunction( ComparisonTerm::DistinctCount );
    QTest::newRow( "order by 5 (with aggregate function and invalid subterm and varname)" )
        << Query( orderByTerm5 )
        << QString::fromLatin1("select distinct ?r where { ?r %1 ?v1 . } ORDER BY ASC ( count(distinct ?v1) )")
        .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::numericRating()) );


    // the empty file query should query all files
    FileQuery emptyFileQuery;
    QTest::newRow( "empty file query" )
        << Query(emptyFileQuery)
        << QString::fromLatin1("select distinct ?r where { { ?r %1 %2 . } UNION { ?r %1 %3 . } . }")
        .arg( Soprano::Node::resourceToN3(Soprano::Vocabulary::RDF::type()),
              Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NFO::FileDataObject()),
              Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NFO::Folder()) );


    FileQuery fileQuery( ComparisonTerm( Soprano::Vocabulary::NAO::hasTag(), ResourceTerm(QUrl("nepomuk:/res/foobar")) ) );
    QTest::newRow( "file query" )
        << Query(fileQuery)
        << QString::fromLatin1("select distinct ?r where { { ?r %1 <nepomuk:/res/foobar> . { ?r %2 %3 . } UNION { ?r %2 %4 . } . } . }")
        .arg( Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::hasTag()),
              Soprano::Node::resourceToN3(Soprano::Vocabulary::RDF::type()),
              Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NFO::FileDataObject()),
              Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NFO::Folder()) );

    fileQuery.setFileMode(FileQuery::QueryFiles);
    QTest::newRow( "file query (only files)" )
        << Query(fileQuery)
        << QString::fromLatin1("select distinct ?r where { { ?r %1 <nepomuk:/res/foobar> . ?r %2 %3 . FILTER(!bif:exists((select (1) where { ?r %2 %4 . }))) . } . }")
        .arg( Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::hasTag()),
              Soprano::Node::resourceToN3(Soprano::Vocabulary::RDF::type()),
              Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NFO::FileDataObject()),
              Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NFO::Folder()) );

    fileQuery.setFileMode(FileQuery::QueryFolders);
    QTest::newRow( "file query (only folders)" )
        << Query(fileQuery)
        << QString::fromLatin1("select distinct ?r where { { ?r %1 <nepomuk:/res/foobar> . ?r %2 %3 . FILTER(!bif:exists((select (1) where { ?r %2 %4 . }))) . } . }")
        .arg( Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::hasTag()),
              Soprano::Node::resourceToN3(Soprano::Vocabulary::RDF::type()),
              Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NFO::Folder()),
              Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NFO::FileDataObject()) );

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

    QString sparql = QString::fromLatin1("select distinct ?r count(?v5) as ?cnt where { { "
                                         "{ ?r a ?v1 . ?v1 %1 %2 . } UNION { ?r a ?v2 . ?v2 %1 %3 . } . "
                                         "FILTER(!bif:exists((select (1) where { <nepomuk:/res/foobar> ?v3 ?r . }))) . "
                                         "?r ?v4 ?v5 . } . } ORDER BY DESC ( ?cnt )")
                     .arg(Soprano::Node::resourceToN3(Soprano::Vocabulary::RDFS::subClassOf()),
                          Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NFO::RasterImage()),
                          Soprano::Node::resourceToN3(Nepomuk::Vocabulary::NFO::Audio()));

    QTest::newRow( "a complex one" )
        << Query( mainTerm )
        << sparql;

#if 0
// subquery to match grouding occurrences of nepomuk:/TEST
    ComparisonTerm goterm( Nepomuk::Vocabulary::PIMO::groundingOccurrence(),
                           ResourceTerm( Nepomuk::Resource( QUrl("nepomuk:/TEST")) ) );
    goterm.setInverted(true);

// combine that with only nco:PersonContacts
    AndTerm pcgoterm( ResourceTypeTerm( Nepomuk::Vocabulary::NCO::PersonContact() ),
                      goterm );

// now look for im accounts of those grounding occurrences (pcgoterm will become the subject of this comparison,
// thus the comparison will match the im accounts)
    ComparisonTerm impcgoterm( Nepomuk::Vocabulary::NCO::hasIMAccount(),
                               pcgoterm );
    impcgoterm.setInverted(true);

// now look for all buddies of the accounts
    ComparisonTerm buddyTerm( QUrl("http://kde.org/telepathy#isBuddyOf")/*Nepomuk::Vocabulary::Telepathy::isBuddyOf()*/,
                              impcgoterm );

// set the name of the variable (i.e. the buddies) to be able to match it later
    buddyTerm.setVariableName("t");

// same comparison, other property, but use the same variable name to match them
    ComparisonTerm ppterm( QUrl("http://kde.org/telepathy#publishesPresenceTo")/*Nepomuk::Vocabulary::Telepathy::publishesPresenceTo()*/,
                           ResourceTypeTerm( Nepomuk::Vocabulary::NCO::IMAccount() ) );
    ppterm.setVariableName("t");

// combine both to complete the matching of the im account ?account
    AndTerm accountTerm( ResourceTypeTerm( Nepomuk::Vocabulary::NCO::IMAccount() ),
                         buddyTerm,
                         ppterm );

// match the account and select it for the results
    ComparisonTerm imaccountTerm( Nepomuk::Vocabulary::NCO::hasIMAccount(), accountTerm );
    imaccountTerm.setVariableName("account");

// and finally the exclusion of those person contacts that already have a pimo person attached
    ComparisonTerm personTerm( Nepomuk::Vocabulary::PIMO::groundingOccurrence(),
                               ResourceTypeTerm( Nepomuk::Vocabulary::PIMO::Person() ) );
    personTerm.setInverted(true);

// and all combined
    Query theQuery( AndTerm( ResourceTypeTerm( Nepomuk::Vocabulary::NCO::PersonContact() ),
                             imaccountTerm,
                             NegationTerm::negateTerm(personTerm)) );


    QTest::newRow( "and term" )
        << theQuery
        << QString::fromLatin1("");
#endif
}


void QueryTest::testToSparql()
{
    QFETCH( Nepomuk::Query::Query, query );
    QFETCH( QString, queryString );

//    kDebug() << query;
    QCOMPARE( query.toSparqlQuery().simplified(), queryString );
}


void QueryTest::testOptimization()
{
    LiteralTerm literal("Hello World");
    AndTerm and1;
    and1.addSubTerm(literal);
    Term optimized = QueryPrivate::optimizeTerm(and1);
    QVERIFY(optimized.isLiteralTerm());

    AndTerm and2;
    and2.addSubTerm(and1);
    optimized = QueryPrivate::optimizeTerm(and2);
    QVERIFY(optimized.isLiteralTerm());

    Term invalidTerm;
    and2.addSubTerm(invalidTerm);
    optimized = QueryPrivate::optimizeTerm(and2);
    QVERIFY(optimized.isLiteralTerm());

    and1.setSubTerms(QList<Term>() << invalidTerm);
    and2.setSubTerms(QList<Term>() << and1 << literal);
    optimized = QueryPrivate::optimizeTerm(and2);
    QVERIFY(optimized.isLiteralTerm());

    // make sure duplicate negations are removed
    QCOMPARE( QueryPrivate::optimizeTerm(
                  NegationTerm::negateTerm(
                      NegationTerm::negateTerm(
                          ResourceTypeTerm(Soprano::Vocabulary::NAO::Tag())))),
              Term( ResourceTypeTerm(Soprano::Vocabulary::NAO::Tag()) ) );

    // make sure more than two negations are removed
    QCOMPARE( QueryPrivate::optimizeTerm(
                  NegationTerm::negateTerm(
                      NegationTerm::negateTerm(
                          NegationTerm::negateTerm(
                              NegationTerm::negateTerm(
                                  ResourceTypeTerm(Soprano::Vocabulary::NAO::Tag())))))),
              Term( ResourceTypeTerm(Soprano::Vocabulary::NAO::Tag()) ) );

    // test negation removal and
    QCOMPARE( QueryPrivate::optimizeTerm(
                  NegationTerm::negateTerm(
                      NegationTerm::negateTerm(
                          NegationTerm::negateTerm(
                              ResourceTypeTerm(Soprano::Vocabulary::NAO::Tag()))))),
              Term( AndTerm( NegationTerm::negateTerm(ResourceTypeTerm(Soprano::Vocabulary::NAO::Tag())),
                             ComparisonTerm(Soprano::Vocabulary::RDF::type(), Term())) ) );


    // make sure duplicate optionals are removed
    QCOMPARE( QueryPrivate::optimizeTerm(
                  OptionalTerm::optionalizeTerm(
                      OptionalTerm::optionalizeTerm(
                          OptionalTerm::optionalizeTerm(
                              ResourceTypeTerm(Soprano::Vocabulary::NAO::Tag()))))),
              Term( OptionalTerm::optionalizeTerm(
                        ResourceTypeTerm(Soprano::Vocabulary::NAO::Tag()))) );
}


void QueryTest::testLogicalOperators()
{
    // test negation
    ComparisonTerm ct1( Soprano::Vocabulary::NAO::hasTag(), LiteralTerm("bla") );
    QCOMPARE( NegationTerm::negateTerm(ct1), !ct1 );

    // test logical and
    ComparisonTerm ct2( Soprano::Vocabulary::NAO::hasTag(), LiteralTerm("foo") );
    LiteralTerm lt1( "bar" );
    QCOMPARE( Term(AndTerm( ct1, ct2, lt1 )), ct1 && ct2 && lt1 );

    // test logical or
    QCOMPARE( Term(OrTerm( ct1, ct2, lt1 )), ct1 || ct2 || lt1 );
}


QTEST_KDEMAIN_CORE( QueryTest )

#include "querytest.moc"
