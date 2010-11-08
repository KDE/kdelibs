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

#include "facetmodeltest.h"
#include "qtest_kde.h"
#include "qtest_querytostring.h"

#include "facetmodel.h"
#include "simplefacet.h"
#include "comparisonterm.h"
#include "resourcetypeterm.h"
#include "literalterm.h"
#include "proxyfacet.h"
#include "nfo.h"

#include <Soprano/Vocabulary/NAO>


using namespace Nepomuk::Utils;
using namespace Nepomuk::Query;


void FacetModelTest::testExtractFacetsFromTermWithVaryingSelectionMode()
{
    const Term term1 = ResourceTypeTerm(Soprano::Vocabulary::NAO::Tag());
    const Term term2 = ResourceTypeTerm(Soprano::Vocabulary::NAO::Party());

    SimpleFacet* f = new SimpleFacet;
    f->addTerm( QLatin1String("t1"), term1 );
    f->addTerm( QLatin1String("t2"), term2 );

    FacetModel model;
    model.addFacet(f);

    // Test AND combination of terms for each selection mode
    // =====================================================

    // in MatchOne mode only one term should be extracted
    f->setSelectionMode(Facet::MatchOne);
    QCOMPARE( model.extractFacetsFromQuery(Query(term1 && term2)), Query(term2) );
    QCOMPARE( model.queryTerm(), term1 );

    // in MatchAll mode both terms should be extracted
    f->setSelectionMode(Facet::MatchAll);
    QCOMPARE( model.extractFacetsFromQuery(Query(term1 && term2)), Query() );
    QCOMPARE( model.queryTerm(), term1 && term2 );

    // in MatchAny mode no term should be extracted
    f->setSelectionMode(Facet::MatchAny);
    QCOMPARE( model.extractFacetsFromQuery(Query(term1 && term2)), Query(term2) );
    QCOMPARE( model.queryTerm(), term1 );


    // Test OR combination of terms for each selection mode
    // =====================================================

    // in MatchOne mode no term should be selected
    f->setSelectionMode(Facet::MatchOne);
    QCOMPARE( model.extractFacetsFromQuery(Query(term1 || term2)), Query(term1 || term2) );
    QCOMPARE( model.queryTerm(), term1 );

    // in MatchAll mode no term should be extracted
    f->setSelectionMode(Facet::MatchAll);
    QCOMPARE( model.extractFacetsFromQuery(Query(term1 || term2)), Query(term1 || term2) );
    QCOMPARE( model.queryTerm(), Term() );

    // in MatchAny mode both terms should be extracted
    f->setSelectionMode(Facet::MatchAny);
    QCOMPARE( model.extractFacetsFromQuery(Query(term1 || term2)), Query() );
    QCOMPARE( model.queryTerm(), term1 || term2 );
}


void FacetModelTest::testExtractFacetsFromTermWithMultipleFacets()
{
    FacetModel model;

    SimpleFacet* f1 = new SimpleFacet;
    f1->setSelectionMode(Facet::MatchOne);
    f1->addTerm( QLatin1String("t1"), Term() );
    f1->addTerm( QLatin1String("t2"), ResourceTypeTerm(Soprano::Vocabulary::NAO::Tag()) );
    f1->addTerm( QLatin1String("t3"), ResourceTypeTerm(Soprano::Vocabulary::NAO::Party()) );
    f1->addTerm( QLatin1String("t4"), ResourceTypeTerm(Soprano::Vocabulary::NAO::Symbol()) );
    model.addFacet(f1);

    SimpleFacet* f2 = new SimpleFacet;
    f2->setSelectionMode(Facet::MatchAll);
    f2->addTerm( QLatin1String("t2"), Soprano::Vocabulary::NAO::numericRating() == LiteralTerm(2) );
    f2->addTerm( QLatin1String("t3"), Soprano::Vocabulary::NAO::numericRating() == LiteralTerm(3) );
    f2->addTerm( QLatin1String("t4"), Soprano::Vocabulary::NAO::numericRating() == LiteralTerm(4) );
    model.addFacet(f2);

    SimpleFacet* f3 = new SimpleFacet;
    f3->setSelectionMode(Facet::MatchAny);
    f3->addTerm( QLatin1String("t2"), Soprano::Vocabulary::NAO::prefLabel() == LiteralTerm(QLatin1String("l1")) );
    f3->addTerm( QLatin1String("t3"), Soprano::Vocabulary::NAO::prefLabel() == LiteralTerm(QLatin1String("l2")) );
    f3->addTerm( QLatin1String("t4"), Soprano::Vocabulary::NAO::prefLabel() == LiteralTerm(QLatin1String("l3")) );
    model.addFacet(f3);

    // now we test all kinds of combinations of the above terms to see if they are extracted correctly

    model.clearSelection();
    Term testTerm = f1->termAt(1) && f2->termAt(0) && f3->termAt(0);
    Term restTerm;
    Term queryTerm = testTerm;
    QCOMPARE( model.extractFacetsFromQuery(Query(testTerm)), Query(restTerm) );
    QCOMPARE( model.queryTerm(), queryTerm );
    QCOMPARE( model.queryTerm() && restTerm, testTerm );

    model.clearSelection();
    testTerm = f2->termAt(0) && f2->termAt(1) && f3->termAt(0);
    restTerm = Term();
    queryTerm = testTerm;
    QCOMPARE( model.extractFacetsFromQuery(Query(testTerm)), Query(restTerm) );
    QCOMPARE( model.queryTerm(), queryTerm );
    QCOMPARE( model.queryTerm() && restTerm, testTerm );

    model.clearSelection();
    testTerm = f1->termAt(1) && f2->termAt(0) && f3->termAt(0) && f3->termAt(1);
    restTerm = f3->termAt(1);
    queryTerm = f1->termAt(1) && f2->termAt(0) && f3->termAt(0);
    QCOMPARE( model.extractFacetsFromQuery(Query(testTerm)), Query(restTerm) );
    QCOMPARE( model.queryTerm(), queryTerm );
    QCOMPARE( model.queryTerm() && restTerm, testTerm );

    model.clearSelection();
    testTerm = f1->termAt(1) && f2->termAt(0) && ( f3->termAt(0) || f3->termAt(1) );
    restTerm = Term();
    queryTerm = testTerm;
    QCOMPARE( model.extractFacetsFromQuery(Query(testTerm)), Query(restTerm) );
    QCOMPARE( model.queryTerm(), queryTerm );
    QCOMPARE( model.queryTerm() && restTerm, testTerm );

    model.clearSelection();
    testTerm = f3->termAt(0) || f3->termAt(1) || f3->termAt(2);
    restTerm = Term();
    queryTerm = testTerm;
    QCOMPARE( model.extractFacetsFromQuery(Query(testTerm)), Query(restTerm) );
    QCOMPARE( model.queryTerm(), queryTerm );
    QCOMPARE( model.queryTerm() && restTerm, testTerm );

    model.clearSelection();
    testTerm = f3->termAt(0) || f3->termAt(1) || f3->termAt(2) || f1->termAt(1);
    restTerm = testTerm;
    queryTerm = Term();
    QCOMPARE( model.extractFacetsFromQuery(Query(testTerm)), Query(restTerm) );
    QCOMPARE( model.queryTerm(), queryTerm );
    QCOMPARE( model.queryTerm() && restTerm, testTerm );

    model.clearSelection();
    testTerm = f3->termAt(0) && f3->termAt(1);
    restTerm = f3->termAt(1);
    queryTerm = f3->termAt(0);
    QCOMPARE( model.extractFacetsFromQuery(Query(testTerm)), Query(restTerm) );
    QCOMPARE( model.queryTerm(), queryTerm );
    QCOMPARE( model.queryTerm() && restTerm, testTerm );
}


void FacetModelTest::testExtractFacetsFromTermWithProxyFacet()
{
    FacetModel model;

    SimpleFacet* f1 = new SimpleFacet;
    f1->setSelectionMode(Facet::MatchAny);
    f1->addTerm( QLatin1String("t1"), ResourceTypeTerm(Soprano::Vocabulary::NAO::Tag()) );
    f1->addTerm( QLatin1String("t2"), ResourceTypeTerm(Nepomuk::Vocabulary::NFO::FileDataObject()) );
    model.addFacet(f1);

    SimpleFacet* f2 = new SimpleFacet;
    f2->setSelectionMode(Facet::MatchAny);
    f2->addTerm( QLatin1String("t1"), ResourceTypeTerm(Nepomuk::Vocabulary::NFO::Image()) );
    f2->addTerm( QLatin1String("t2"), ResourceTypeTerm(Nepomuk::Vocabulary::NFO::Document()) );

    ProxyFacet* f2p = new ProxyFacet;
    f2p->setSourceFacet(f2);
    f2p->setFacetCondition(f1->termAt(1));
    model.addFacet(f2p);

    model.clearSelection();
    Term testTerm = f1->termAt(0) && f2->termAt(0);
    Term restTerm = f2->termAt(0);
    Term queryTerm = f1->termAt(0);
    QCOMPARE( model.extractFacetsFromQuery(Query(testTerm)), Query(restTerm) );
    QCOMPARE( model.queryTerm(), queryTerm );
    QCOMPARE( model.queryTerm() && restTerm, testTerm );

    model.clearSelection();
    testTerm = f1->termAt(1) && f2->termAt(0);
    restTerm = Term();
    queryTerm = testTerm;
    QCOMPARE( model.extractFacetsFromQuery(Query(testTerm)), Query(restTerm) );
    QCOMPARE( model.queryTerm(), queryTerm );
    QCOMPARE( model.queryTerm() && restTerm, testTerm );
}

QTEST_KDEMAIN_CORE(FacetModelTest)
