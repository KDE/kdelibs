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

#include "typefacettest.h"
#include "qtest_querytostring.h"

#include "typefacet.h"
#include "resourcetypeterm.h"
#include "nfo.h"
#include "class.h"
#include "negationterm.h"
#include "class.h"

#include <Soprano/Vocabulary/NAO>

#include <QtTest>
#include <QScopedPointer>
#include "qtest_kde.h"


using namespace Nepomuk::Utils;
using namespace Nepomuk::Query;
using namespace Nepomuk::Vocabulary;
using namespace Nepomuk::Types;

void TypeFacetTest::testBasicLayout()
{
    QScopedPointer<Facet> f( Facet::createTypeFacet() );
    QCOMPARE(f->count(), 2);
    QCOMPARE(f->queryTerm(), Term());
}

void TypeFacetTest::testFileRootSelection()
{
    QScopedPointer<Facet> f( Facet::createTypeFacet() );
    f->setSelected(0);
    QCOMPARE(f->count(), 5); // The checked file and the file choices
    QCOMPARE(f->queryTerm(), Term(ResourceTypeTerm(NFO::FileDataObject())));
    f->setSelected(0, false);
    QCOMPARE(f->count(), 2);
    QCOMPARE(f->queryTerm(), Term());
}

void TypeFacetTest::testOtherRootSelection()
{
    QScopedPointer<Facet> f( Facet::createTypeFacet() );
    f->setSelected(1);
    QCOMPARE(f->count(), 5);
    QCOMPARE(f->queryTerm(), Term(NegationTerm::negateTerm(ResourceTypeTerm(NFO::FileDataObject()))));
    f->setSelected(0, false);
    QCOMPARE(f->count(), 2);
    QCOMPARE(f->queryTerm(), Term());
}

void TypeFacetTest::testCustomTypes()
{
    QScopedPointer<Facet> f( Facet::createTypeFacet() );
    f->selectFromTerm(ResourceTypeTerm(NFO::Application()));
    QCOMPARE(f->count(), 6);
    QVERIFY(f->isSelected(0));
    QCOMPARE(f->text(5), Class(NFO::Application()).label());
    QVERIFY(f->isSelected(5));
    QCOMPARE(f->queryTerm(), Term(ResourceTypeTerm(NFO::Application())));
}

void TypeFacetTest::testSelectFromTerm()
{
    QScopedPointer<Facet> f( Facet::createTypeFacet() );

    // test nfo:FileDataObject which should only select the file thing
    f->selectFromTerm(ResourceTypeTerm(NFO::FileDataObject()));
    QCOMPARE(f->count(), 5);
    QVERIFY(f->isSelected(0));
    QVERIFY(!f->isSelected(1));
    QVERIFY(!f->isSelected(2));
    QVERIFY(!f->isSelected(3));
    QVERIFY(!f->isSelected(4));
    QCOMPARE(f->queryTerm(), Term(ResourceTypeTerm(NFO::FileDataObject())));

    // in file mode non-file types should not be selectable
    QVERIFY(!f->selectFromTerm(ResourceTypeTerm(NFO::Application())));
}

QTEST_KDEMAIN_CORE(TypeFacetTest)

#include "typefacettest.moc"
