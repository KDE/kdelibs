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

#include "proxyfacettest.h"
#include "qtest_querytostring.h"

#include "simplefacet.h"
#include "proxyfacet.h"
#include "comparisonterm.h"
#include "resourcetypeterm.h"
#include "query.h"

#include <Soprano/Vocabulary/NAO>

#include <QtTest>
#include "qtest_kde.h"

using namespace Nepomuk::Utils;
using namespace Nepomuk::Query;

Q_DECLARE_METATYPE(Nepomuk::Utils::Facet*)

void ProxyFacetTest::initTestCase()
{
    qRegisterMetaType<Nepomuk::Utils::Facet*>();
}

void ProxyFacetTest::testForward()
{
    SimpleFacet f;
    f.setSelectionMode(Facet::MatchOne);

    QString label1 = QLatin1String("Label 1");
    Term term1;
    QString label2 = QLatin1String("Label 2");
    Term term2 = ResourceTypeTerm(Soprano::Vocabulary::NAO::Tag());

    f.addTerm(label1, term1);
    f.addTerm(label2, term2);

    ProxyFacet pf;
    pf.setSourceFacet(&f);

    QCOMPARE(f.count(), pf.count());

    QCOMPARE(f.text(0), pf.text(0));
    QCOMPARE(f.text(1), pf.text(1));

    QCOMPARE(f.queryTerm(), pf.queryTerm());

    f.setSelected(1);
    QCOMPARE(f.isSelected(0), pf.isSelected(0));
    QCOMPARE(f.isSelected(1), pf.isSelected(1));
    QCOMPARE(f.queryTerm(), pf.queryTerm());
}

void ProxyFacetTest::testSignals()
{
    SimpleFacet f;
    f.setSelectionMode(Facet::MatchOne);

    QString label1 = QLatin1String("Label 1");
    Term term1;
    QString label2 = QLatin1String("Label 2");
    Term term2 = ResourceTypeTerm(Soprano::Vocabulary::NAO::Tag());

    f.addTerm(label1, term1);
    f.addTerm(label2, term2);

    ProxyFacet pf;
    pf.setSourceFacet(&f);

    QSignalSpy selectionSpy(&pf, SIGNAL(selectionChanged(Nepomuk::Utils::Facet*)));
    QSignalSpy termChangedSpy(&pf, SIGNAL(termChanged(Nepomuk::Utils::Facet*,Nepomuk::Query::Term)));

    pf.setSelected(1);
    QCOMPARE(selectionSpy.count(), 1);
    selectionSpy.takeFirst();
    QCOMPARE(termChangedSpy.count(), 1);
    termChangedSpy.takeFirst();

    pf.clearSelection();
    QCOMPARE(selectionSpy.count(), 1);
    selectionSpy.takeFirst();
    QCOMPARE(termChangedSpy.count(), 1);
    termChangedSpy.takeFirst();

    QSignalSpy laySpy(&pf, SIGNAL(layoutChanged(Nepomuk::Utils::Facet*)));
    f.addTerm(QLatin1String("Hello World"), Term());
    QCOMPARE(laySpy.count(), 1);
    laySpy.takeFirst();
}

void ProxyFacetTest::testFacetCondition()
{
    SimpleFacet f;
    f.setSelectionMode(Facet::MatchOne);

    QString label1 = QLatin1String("Label 1");
    Term term1;
    QString label2 = QLatin1String("Label 2");
    Term term2 = ResourceTypeTerm(Soprano::Vocabulary::NAO::Tag());

    f.addTerm(label1, term1);
    f.addTerm(label2, term2);

    ProxyFacet pf;
    pf.setSourceFacet(&f);

    QSignalSpy laySpy(&pf, SIGNAL(layoutChanged(Nepomuk::Utils::Facet*)));

    pf.setFacetCondition(ResourceTypeTerm(Soprano::Vocabulary::NAO::Tag()));

    QCOMPARE(laySpy.count(), 1);
    laySpy.takeFirst();

    QCOMPARE(pf.count(), 0);
    QCOMPARE(pf.queryTerm(), Term());

    pf.setClientQuery(Query(ResourceTypeTerm(Soprano::Vocabulary::NAO::Tag())));

    QCOMPARE(laySpy.count(), 1);
    laySpy.takeFirst();

    QCOMPARE(f.count(), pf.count());

    QCOMPARE(f.text(0), pf.text(0));
    QCOMPARE(f.text(1), pf.text(1));

    QCOMPARE(f.queryTerm(), pf.queryTerm());
}

QTEST_KDEMAIN_CORE(ProxyFacetTest)
