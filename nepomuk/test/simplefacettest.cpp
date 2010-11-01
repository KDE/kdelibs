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

#include "simplefacettest.h"
#include "qtest_querytostring.h"

#include "simplefacet.h"
#include "comparisonterm.h"
#include "resourcetypeterm.h"

#include <Soprano/Vocabulary/NAO>

#include <QtTest>
#include "qtest_kde.h"


using namespace Nepomuk::Utils;
using namespace Nepomuk::Query;

void SimpleFacetTest::testAddTerm()
{
    SimpleFacet f;

    QCOMPARE(f.count(), 0);

    QString label1 = QLatin1String("Label 1");
    Term term1;
    QString label2 = QLatin1String("Label 2");
    Term term2 = ResourceTypeTerm(Soprano::Vocabulary::NAO::Tag());

    f.addTerm(label1, term1);
    QCOMPARE(f.count(), 1);

    f.addTerm(label2, term2);
    QCOMPARE(f.count(), 2);

    QCOMPARE(f.text(0), label1);
    QCOMPARE(f.text(1), label2);

    QCOMPARE(f.termAt(0), term1);
    QCOMPARE(f.termAt(1), term2);
}

void SimpleFacetTest::testMatchOne()
{
    SimpleFacet f;
    f.setSelectionMode(Facet::MatchOne);

    QString label1 = QLatin1String("Label 1");
    Term term1;
    QString label2 = QLatin1String("Label 2");
    Term term2 = ResourceTypeTerm(Soprano::Vocabulary::NAO::Tag());

    f.addTerm(label1, term1);
    f.addTerm(label2, term2);

    // by default the first entry needs to be selected
    QVERIFY(f.isSelected(0));
    QCOMPARE(f.queryTerm(), term1);

    // re-selecting it should not change anything
    f.setSelected(0, true);
    QVERIFY(f.isSelected(0));
    QVERIFY(!f.isSelected(1));
    QCOMPARE(f.queryTerm(), term1);

    // de-selecting the first entry should also revert to it being selected
    f.setSelected(0, false);
    QVERIFY(f.isSelected(0));
    QVERIFY(!f.isSelected(1));
    QCOMPARE(f.queryTerm(), term1);

    // selecting the second
    f.setSelected(1, true);
    QVERIFY(!f.isSelected(0));
    QVERIFY(f.isSelected(1));
    QCOMPARE(f.queryTerm(), term2);

    // clearing the selection should result in the first being selected
    f.clearSelection();
    QVERIFY(f.isSelected(0));
    QVERIFY(!f.isSelected(1));
    QCOMPARE(f.queryTerm(), term1);
}

void SimpleFacetTest::testMatchAll()
{
    SimpleFacet f;
    f.setSelectionMode(Facet::MatchAll);

    QString label1 = QLatin1String("Label 1");
    Term term1 = ResourceTypeTerm(Soprano::Vocabulary::NAO::Party());
    QString label2 = QLatin1String("Label 2");
    Term term2 = ResourceTypeTerm(Soprano::Vocabulary::NAO::Tag());

    f.addTerm(label1, term1);
    f.addTerm(label2, term2);

    // by default nothing should be selected
    QVERIFY(!f.isSelected(0));
    QVERIFY(!f.isSelected(1));
    QCOMPARE(f.queryTerm(), Term());

    // selecting the first
    f.setSelected(0, true);
    QVERIFY(f.isSelected(0));
    QVERIFY(!f.isSelected(1));
    QCOMPARE(f.queryTerm(), term1);

    // de-selecting the first entry should revert to no selection
    f.setSelected(0, false);
    QVERIFY(!f.isSelected(0));
    QVERIFY(!f.isSelected(1));
    QCOMPARE(f.queryTerm(), Term());

    // selecting the second
    f.setSelected(1, true);
    QVERIFY(!f.isSelected(0));
    QVERIFY(f.isSelected(1));
    QCOMPARE(f.queryTerm(), term2);

    // selecting the first and the second
    f.setSelected(0, true);
    QVERIFY(f.isSelected(0));
    QVERIFY(f.isSelected(1));
    QCOMPARE(f.queryTerm(), term1 && term2);

    // clearing the selection should result in nothing being selected
    f.clearSelection();
    QVERIFY(!f.isSelected(0));
    QVERIFY(!f.isSelected(1));
    QCOMPARE(f.queryTerm(), Term());
}

void SimpleFacetTest::testMatchAny()
{
    SimpleFacet f;
    f.setSelectionMode(Facet::MatchAny);

    QString label1 = QLatin1String("Label 1");
    Term term1 = ResourceTypeTerm(Soprano::Vocabulary::NAO::Party());
    QString label2 = QLatin1String("Label 2");
    Term term2 = ResourceTypeTerm(Soprano::Vocabulary::NAO::Tag());

    f.addTerm(label1, term1);
    f.addTerm(label2, term2);

    // by default nothing should be selected
    QVERIFY(!f.isSelected(0));
    QVERIFY(!f.isSelected(1));
    QCOMPARE(f.queryTerm(), Term());

    // selecting the first
    f.setSelected(0, true);
    QVERIFY(f.isSelected(0));
    QVERIFY(!f.isSelected(1));
    QCOMPARE(f.queryTerm(), term1);

    // de-selecting the first entry should revert to no selection
    f.setSelected(0, false);
    QVERIFY(!f.isSelected(0));
    QVERIFY(!f.isSelected(1));
    QCOMPARE(f.queryTerm(), Term());

    // selecting the second
    f.setSelected(1, true);
    QVERIFY(!f.isSelected(0));
    QVERIFY(f.isSelected(1));
    QCOMPARE(f.queryTerm(), term2);

    // selecting the first and the second
    f.setSelected(0, true);
    QVERIFY(f.isSelected(0));
    QVERIFY(f.isSelected(1));
    QCOMPARE(f.queryTerm(), term1 || term2);
}

void SimpleFacetTest::testSelectFromTerm()
{
    SimpleFacet f;
    f.setSelectionMode(Facet::MatchOne);

    QString label1 = QLatin1String("Label 1");
    Term term1 = ResourceTypeTerm(Soprano::Vocabulary::NAO::Party());
    QString label2 = QLatin1String("Label 2");
    Term term2 = ResourceTypeTerm(Soprano::Vocabulary::NAO::Tag());

    f.addTerm(label1, term1);
    f.addTerm(label2, term2);


    // in MatchOne mode we should be able to select term1 and term2 but no combination
    QVERIFY(f.selectFromTerm(term1));
    QVERIFY(f.isSelected(0));
    QVERIFY(!f.isSelected(1));
    QCOMPARE(f.queryTerm(), term1);

    QVERIFY(f.selectFromTerm(term2));
    QVERIFY(!f.isSelected(0));
    QVERIFY(f.isSelected(1));
    QCOMPARE(f.queryTerm(), term2);

    QVERIFY(!f.selectFromTerm(term1 && term2));
    QVERIFY(!f.selectFromTerm(term1 || term2));


    // in MatchAll mode we should be able to select term1, term2, and an AndTerm combination of both
    f.setSelectionMode(Facet::MatchAll);

    QVERIFY(f.selectFromTerm(term1));
    QVERIFY(f.isSelected(0));
    QVERIFY(!f.isSelected(1));
    QCOMPARE(f.queryTerm(), term1);

    QVERIFY(f.selectFromTerm(term2));
    QVERIFY(f.isSelected(0));
    QVERIFY(f.isSelected(1));
    QCOMPARE(f.queryTerm(), term1 && term2);

    f.clearSelection();

    QVERIFY(f.selectFromTerm(term1 && term2));
    QVERIFY(f.isSelected(0));
    QVERIFY(f.isSelected(1));
    QCOMPARE(f.queryTerm(), term1 && term2);

    QVERIFY(!f.selectFromTerm(term1 || term2));
    // the selection should not have changed
    QCOMPARE(f.queryTerm(), term1 && term2);


    // in MatchAny more we should be able to select term1, term2, and an OrTerm combination of both
    f.setSelectionMode(Facet::MatchAny);

    QVERIFY(f.selectFromTerm(term1));
    QVERIFY(f.isSelected(0));
    QVERIFY(!f.isSelected(1));
    QCOMPARE(f.queryTerm(), term1);

    QVERIFY(f.selectFromTerm(term2));
    QVERIFY(f.isSelected(0));
    QVERIFY(f.isSelected(1));
    QCOMPARE(f.queryTerm(), term1 || term2);

    f.clearSelection();

    QVERIFY(f.selectFromTerm(term1 || term2));
    QVERIFY(f.isSelected(0));
    QVERIFY(f.isSelected(1));
    QCOMPARE(f.queryTerm(), term1 || term2);

    QVERIFY(!f.selectFromTerm(term1 && term2));
    // the selection should not have changed
    QCOMPARE(f.queryTerm(), term1 || term2);

    // clearing the selection should result in nothing being selected
    f.clearSelection();
    QVERIFY(!f.isSelected(0));
    QVERIFY(!f.isSelected(1));
    QCOMPARE(f.queryTerm(), Term());
}

QTEST_KDEMAIN_CORE(SimpleFacetTest)
